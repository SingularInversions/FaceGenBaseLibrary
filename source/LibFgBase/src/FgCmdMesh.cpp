//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgSyntax.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dMesh.hpp"
#include "FgGeometry.hpp"
#include "FgMetaFormat.hpp"
#include "FgSimilarity.hpp"
#include "FgTopology.hpp"
#include "Fg3dDisplay.hpp"
#include "FgBestN.hpp"
#include "FgCmd.hpp"
#include "FgParse.hpp"

using namespace std;

namespace Fg {

namespace {

void                cmdSurfCombine(CLArgs const & args)
{
    Syntax              syn {args,R"((<mesh>.<extIn>)+ <out>.<extOut>
    <extIn> = )" + getMeshLoadExtsCLDescription() + R"(
    <extOut> = )" + getMeshSaveExtsCLDescription() + R"(
NOTES:
    All input meshes must have identical vertex lists)"
    };
    Mesh                mesh = loadMesh(syn.next());
    while (syn.more()) {
        string              name = syn.next();
        if (syn.more()) {
            Mesh                next = loadMesh(name);
            cat_(mesh.surfaces,next.surfaces);
        }
        else
            saveMesh(mesh,name);
    }
}

void                cmdConvert(CLArgs const & args)
{
    Syntax          syn {args,
        R"([-m] <in>.<exti> <out>.<exto>
    -m          - if <in> contains multiple meshes, merge them preserving vertex order
    <exti>      - )" + getMeshLoadExtsCLDescription() + R"(
    <exto>      - )" + getMeshSaveExtsCLDescription() + R"(
OUTPUT:
    <out>.<exto>        If <exto> format can contain all meshes in <in> (or merge option chosen), otherwise:
    <out>-<name>.<exto> A file for each mesh contained in <in>, where <name> is taken from <in>,
                        and if none is given, generated as sequential integers)"
    };
    bool            merge = false;
    while (syn.peekNext()[0] == '-') {
        if (syn.next() == "-m")
            merge = true;
        else
            syn.error("Unrecognized option",syn.curr());
    }
    Meshes          meshes = loadMeshes(syn.next());
    if ((meshes.size() > 1) && merge)
        meshes = {mergeMeshes(meshes)};
    Path            out {syn.next()};
    if (meshes.size() > 1) {
        if (meshFormatSupportsMulti(getMeshFormat(out.ext.m_str)))
            saveMergeMesh(meshes,out.str());
        else {
            String8s        namesUsed {""};     // contains empty name to force appending a number
            for (Mesh const & mesh : meshes) {
                String8         name = toUtf8(removeNonFilenameChars(mesh.name.as_utf32())),
                                uniqueName = name;
                size_t          cnt {1};
                while (contains(namesUsed,uniqueName))
                    uniqueName = name + toStr(cnt++);
                namesUsed.push_back(uniqueName);
                String8         fname = out.dirBase()+"-"+uniqueName+"."+out.ext;
                saveMesh(mesh,fname);
                fgout << fgnl << fname << " saved.";
            }
        }
    }
    else
        saveMesh(meshes[0],out.str());
}

void                copyUvList(CLArgs const & args)
{
    Syntax          syn {args,R"(<in>.<ext0> <out>.<ext1>
    <ext0> = )" + getMeshLoadExtsCLDescription() + R"(
    <ext1> = )" + getMeshSaveExtsCLDescription()
    };
    Mesh            in = loadMesh(syn.next());
    Mesh            out = loadMesh(syn.next());
    if (in.uvs.size() != out.uvs.size())
        syn.error("Incompatible UV list sizes");
    out.uvs = in.uvs;
    saveMesh(out,syn.curr());
}

void                copyUvsInds(CLArgs const & args)
{
    Syntax          syn {args,R"(<from>.<ext0> <to>.<ext1>
    <ext0> = )" + getMeshLoadExtsCLDescription() + R"(
    <ext1> = )" + getMeshSaveExtsCLDescription() + R"(
    OUTPUT
        <to>.<ext1> - polygon UV indices in this mesh are updated to match <from>.<ext1> and the uv list is copied.
    NOTES
        The surface structures of <to> must match that of <from>)"
    };
    Mesh            in = loadMesh(syn.next());
    Mesh            out = loadMesh(syn.next());
    out.uvs = in.uvs;
    if (in.surfaces.size() != out.surfaces.size())
        syn.error("Incompatible number of surfaces");
    for (size_t ss=0; ss<in.surfaces.size(); ++ss) {
        Surf const &     sin = in.surfaces[ss];
        Surf &           sout = out.surfaces[ss];
        if ((sin.tris.size() != sout.tris.size()) ||
            (sin.quads.size() != sout.quads.size()))
            syn.error("Incompatible poly count for surface "+toStr(ss));
        sout.tris.uvInds = sin.tris.uvInds;
        sout.quads.uvInds = sin.quads.uvInds;
    }
    saveMesh(out,syn.curr());
}

void                copyUvsImv(CLArgs const & args)
{
    Syntax              syn {args,R"(<from>.<ext0> <to>.<ext1>
    <ext0> = )" + getMeshLoadExtsCLDescription() + R"(
    <ext1> = )" + getMeshSaveExtsCLDescription() + R"(
    OUTPUT
        <to>.<ext1> - tri UV indices in this mesh are updated to from the tri with matching vertex indices in <from>.<ext1> and the uv list is copied
    NOTES
        All tris in <to> must correspond to a tri in <from> with an identical list of vertex indices)"
    };
    Mesh                in = loadMesh(syn.next());
    Mesh                out = loadMesh(syn.next());
    out.uvs = in.uvs;
    map<Vec3UI,Vec3UI>  vertIndsToUvInds;
    for (Surf const & inSurf : in.surfaces) {
        NPolys<3> const &   tris = inSurf.tris;
        if (tris.vertInds.size() == tris.uvInds.size())
            for (size_t tt=0; tt<tris.vertInds.size(); ++tt)
                vertIndsToUvInds[tris.vertInds[tt]] = tris.uvInds[tt];
    }
    for (Surf & outSurf : out.surfaces) {
        NPolys<3> &         tris = outSurf.tris;
        tris.uvInds.clear();
        for (size_t tt=0; tt<tris.vertInds.size(); ++tt) {
            auto                it = vertIndsToUvInds.find(tris.vertInds[tt]);
            if (it == vertIndsToUvInds.end())
                syn.error("<to> contains a tri which has no identical list of vertex indices in <from>");
            tris.uvInds.push_back(it->second);
        }
    }
    saveMesh(out,syn.curr());
}

void                cmdEdit(CLArgs const & args)
{
    Syntax              syn {args,
        R"((<mesh>.<ext> | <filenames>.txt)+
    <mesh>.<ext>    - the mesh(es) to be sequentially viewed and edited
    <filenames>.txt - must contain one <mesh>.<ext> as above per non-empty line
    <ext>           - )" + getMeshSaveExtsCLDescription() + R"(
OUTPUT:
    <mesh>.<ext>    - changes are saved back to this file if the 'Edit' -> 'Save' button is clicked
NOTES:
    Editing controls are found under the 'Edit' tab.
)"
    };
    String8s            filenames;
    while (syn.more()) {
        if (toLower(pathToExt(syn.peekNext())) == "txt")
            cat_(filenames,splitLinesUtf8(loadRawString(syn.next()),'#'));
        else
            filenames.push_back(syn.next());
    }
    if (filenames.empty())
        syn.error("no files to edit");
    for (String8 const & filename : filenames)
        viewMesh({loadMesh(filename)},false,filename);
}

void                cmdEmboss(CLArgs const & args)
{
    Syntax          syn(args,
        "<uvImage>.<img> <meshin>.<ext0> <val> <out>.<ext1>\n"
        "    <uvImage> = a UV-layout image whose grescale values will be used to emboss (0 - none, 255 - full)\n"
        "    <img>     = " + clOptionsStr(getImgExts()) + "\n"
        "    <ext0>    = " + getMeshLoadExtsCLDescription() + "\n"
        "    <val>     = Embossing factor as ratio of the max bounding box dimension.\n"
        "    <ext1>    = " + getMeshSaveExtsCLDescription()
        );
    ImgUC           img = toUC(loadImage(syn.next()));
    Mesh            mesh = loadMesh(syn.next());
    if (mesh.uvs.empty())
        fgThrow("Mesh has no UVs",syn.curr());
    if (mesh.surfaces.size() != 1)
        fgThrow("Only 1 surface currently supported",syn.curr());
    float           val = syn.nextAs<float>();
    if (!(val > 0.0f))
        fgThrow("Emboss value must be > 0",toStr(val));
    mesh.verts = embossMesh(mesh,img,val);
    saveMesh(mesh,syn.next());
}

void                cmdExport(CLArgs const & args)
{
    Syntax          syn {args,
        R"(<out>.<exto> (<mesh>.<extm> [<image>.<exti>])+
    <exto>          - )" + getMeshLoadExtsCLDescription() + R"(
    <extm>          - )" + getMeshSaveExtsCLDescription() + R"(
    <exti>          - )" + clOptionsStr(getImgExts()) + R"(
OUTPUT:
    <out>.<exto>    - the combined meshes
    <out>#.png      - related image files are only stored if <exto> supports referencing image files
NOTES:
    * If the output format does not support multiple meshes they will be merged into one
    * If the output format does not support references to color maps they will be ignored)"
    };
    String          outFile = syn.next();
    Meshes          meshes;
    do {
        Mesh            mesh = loadMesh(syn.next());
        size_t          cnt = 0;
        while (syn.more() && toLower(pathToExt(syn.peekNext())) != "tri") {
            String              imgFile(syn.next()),
                                ext = pathToExt(imgFile);
            Strings             exts = getImgExts();
            auto                it = find(exts.begin(),exts.end(),ext);
            if (it == exts.end())
                syn.error("Unknown image file type",imgFile);
            if (cnt < mesh.surfaces.size())
                loadImage_(imgFile,mesh.surfaces[cnt++].albedoMapRef());
            else
                syn.error("More albedo map images specified than surfaces in",mesh.name);
        }
        meshes.push_back(mesh);
    } while (syn.more());
    saveMergeMesh(meshes,outFile);
}

void                cmdInject(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<in>.<exto> <verts>.<exti> [-d <vertsBase>.<exti>] <out>.<exto>
    <exto>          - ([w]obj , fbx)
    <exti>          - )" + getMeshLoadExtsCLDescription() + R"(
    -d              - inject vertex deltas rather than absolute positions, where the deltas are given
                      by <verts> - <vertsBase>
OUTPUT:
    <out>.<exto>    - <in>.<exto> but with all vertex positions updated to the values in <verts> [-<vertsBase>]
NOTES:
    * The vertex list in <verts>.<exti> must be in exact correspondence with <in>.<exto>
    * If <in>.<exto> is a multi-mesh format (eg. FBX) the vertices are updated in order of appearance in the file
    * For 'fbx' only the binary format is currently supported
    * Unlike 'mesh copyv' this command exactly preserves all other contents of <in>.<exto>
)"
    };
    String              inName = syn.next(),
                        inExt = toLower(pathToExt(inName));
    Vec3Fs              vertsIn = mergeMeshes(loadMeshes(inName)).verts;
    Vec3Fs              verts = loadMesh(syn.next()).verts;
    if (verts.size() != vertsIn.size())
        syn.error("Vertex count mismatch between <in> and <verts>");
    if (syn.peekNext()[0] == '-') {
        if (syn.next() == "-d") {
            Vec3Fs          vertsBase = loadMesh(syn.next()).verts;
            if (vertsBase.size() != verts.size())
                syn.error("Vertex count mismatch between <verts> and <vertsBase>");
            verts = vertsIn + verts - vertsBase;
        }
        else
            syn.error("Unrecognized option",syn.curr());
    }
    String              outName = syn.next();
    if ((inExt == "wobj") || (inExt == "obj"))
        injectVertsWObj(inName,verts,outName);
    else if (inExt == "fbx")
        injectVertsFbxBin(inName,{verts},outName);
    else
        syn.error("Unsupported format",inExt);
}

void                cmdRevWind(CLArgs const & args)
{
    Syntax              syn(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <extOut> = " + getMeshSaveExtsCLDescription() + "\n"
        "    Inverts the winding of all facets in <in> and saves to <out>"
        );
    Mesh                mesh = loadMesh(syn.next());
    mesh.surfaces = mapCall<Surf>(mesh.surfaces,[&](Surf const & s){return reverseWinding(s); });
    saveMesh(mesh,syn.next());
}

void                cmdMarkLabel(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<in>.fgmesh <idx> <name> <out>.fgmesh
OUTPUT:
    <out>.fgmesh    - identical to <in>.fgmesh except that marked vertex <idx> is now labelled <name>
NOTES:
    .tri format can also be used but other mesh formats are not supported)"
    };
    Mesh                mesh = loadMesh(syn.next());
    uint                idx = syn.nextAs<uint>();
    if (idx >= mesh.markedVerts.size())
        syn.error("<idx> is larger than available marked vertices",toStr(mesh.markedVerts.size()));
    mesh.markedVerts[idx].label = syn.next();
    saveMesh(mesh,syn.next());
}

void                cmdMarkList(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<in>.fgmesh
NOTES:
    .tri format can also be used but other mesh formats are not supported)"
    };
    Mesh                mesh = loadMesh(syn.next());
    PushIndent          pind {toStr(mesh.markedVerts.size()) + " Marked Vertices"};
    size_t              cnt {0};
    for (auto const & mv : mesh.markedVerts)
        fgout << fgnl << toStrDigits(cnt,2) << ": idx " << mv.idx << " " << mv.label;
}

void                cmdMark(CLArgs const & args)
{
    Cmds                cmds {
        {cmdMarkLabel,"label","label a marked vertex"},
        {cmdMarkList,"list","list all marked vertices"},
    };
    doMenu(args,cmds,false,false,"Use 'view mesh' to interactively create marked vertices");
}

void                cmdMergeMeshes(CLArgs const & args)
{
    Syntax              syn {args,
        R"((<in>.<exti>)+ -o <out>.<exto>
    <exti>      - )" + getMeshLoadExtsCLDescription() + R"(
    <exto>      - )" + getMeshSaveExtsCLDescription() + R"(
OUTPUT:
    <out>.<exto>        All <in> mesh surfaces with a single vertex list
NOTES:
    * If <exto> supports multiple surfaces, each surface of each input mesh will be a separate surface.)"
    };
    Meshes              meshes = loadMeshes(syn.next());
    while (syn.next() != "-o")
        cat_(meshes,loadMeshes(syn.curr()));
    String              outName = syn.next();
    saveMesh(mergeMeshes(mapAddr(meshes)),outName);
}

void                cmdMeshMirror(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<in>.<exti> <out>.<exto>
    <exti>      - )" + getMeshLoadExtsCLDescription() + R"(
    <exto>      - )" + getMeshSaveExtsCLDescription() + R"(
OUTPUT:
    <out>.<exto>
NOTES:
    * all vertex coordinates will be mirrored around X=0
    * windings will be reversed (to preserve surface orientation)
    * any point labels ending in 'L' or 'R' will be reversed)"
    };
    Mesh                mesh = loadMesh(syn.next());
    saveMesh(cMirrorX(mesh),syn.next());
}

void                cmdMeshMirrorFuse(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<in>.<exti> <out>.<exto>
    <exti>      - )" + getMeshLoadExtsCLDescription() + R"(
    <exto>      - )" + getMeshSaveExtsCLDescription() + R"(
OUTPUT:
    <out>.<exto>
NOTES:
    * all vertex coordinates will be mirrored around X=0
    * windings will be reversed (to preserve surface orientation)
    * any point labels ending in 'L' or 'R' will be reversed
    * the mesh will be fused with the mirrored one)"
    };
    Mesh                mesh = loadMesh(syn.next());
    saveMesh(mirrorXFuse(mesh),syn.next());
}

void                mergesurfs(CLArgs const & args)
{
    Syntax      syn(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <extOut> = " + getMeshSaveExtsCLDescription()
        );
    Mesh        mesh = loadMesh(syn.next());
    if (mesh.surfaces.size() < 2)
        fgout << "WARNING: No extra surfaces to merge.";
    else
        mesh.surfaces = {merge(mesh.surfaces)};
    saveMesh(mesh,syn.next());
}

void                cmdRdf(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.tri [<out>.tri]\n"
        "    <in>.tri       - Will be overwritten if <out>.tri is not specified\n"
        "NOTES:\n"
        "    Duplicates are determined by vertex index. To remove duplicates by vertex\n"
        "    value, first remove duplicate vertices."
        );
    String8        fni(syn.next()),
                    fno = fni;
    if (syn.more())
        fno = syn.next();
    saveTri(fno,removeDuplicateFacets(loadTri(fni)));
}

void                cmdRtris(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<extIn> <out>.<extOut> (<surfIndex> <triEquivIndex>)+\n"
        "    <extIn>    - " + getMeshLoadExtsCLDescription() + "\n"
        "    <extOut>   - " + getMeshSaveExtsCLDescription() + "\n"
        "    <triEquivIndex> - Tri-equivalent index of triangle or half-quad to remove."
        );
    Mesh        mesh = loadMesh(syn.next());
    String8        outName = syn.next();
    while (syn.more()) {
        size_t          si = syn.nextAs<size_t>(),
                        ti = syn.nextAs<size_t>();
        if (si >= mesh.surfaces.size())
            fgThrow("Surface index out of bounds",toStr(si),toStr(mesh.surfaces.size()));
        Surf &   surf = mesh.surfaces[si];
        if (ti >= surf.numTriEquivs())
            fgThrow("Tri equiv index out of bounds",toStr(ti),toStr(surf.numTriEquivs()));
        if (ti < surf.tris.size())
            surf.removeTri(ti);
        else {
            ti -= surf.tris.size();
            Vec4UI       qvs = surf.quads.vertInds[ti/2];
            Vec4UI       uvs(0);
            if (!surf.quads.uvInds.empty())
                uvs = surf.quads.uvInds[ti/2];
            surf.removeQuad(ti/2);
            // The other tri making up the quad needs to be appended to tris:
            if (ti & 0x1)
                surf.tris.vertInds.push_back(Vec3UI(qvs[0],qvs[1],qvs[2]));
            else
                surf.tris.vertInds.push_back(Vec3UI(qvs[2],qvs[3],qvs[0]));
            if (!surf.tris.uvInds.empty()) {
                if (ti & 0x1)
                    surf.tris.uvInds.push_back(Vec3UI(uvs[0],uvs[1],uvs[2]));
                else
                    surf.tris.uvInds.push_back(Vec3UI(uvs[2],uvs[3],uvs[0]));
            }
        }
    }
    saveMesh(mesh,outName);
}

void                cmdRuv(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <extOut> = " + getMeshSaveExtsCLDescription()
        );
    Mesh    mesh = loadMesh(syn.next());
    mesh = removeUnusedVerts(mesh);
    saveMesh(mesh,syn.next());
}

void                mergenamedsurfs(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <extOut> = " + getMeshSaveExtsCLDescription()
        );
    Mesh    mesh = loadMesh(syn.next());
    mesh = mergeSameNameSurfaces(mesh);
    saveMesh(mesh,syn.next());
}

void                cmdRetopo(CLArgs const & args)
{
    Syntax              syn { args,
        "<baseName> <retopoBase> <outBase>\n"
        "    <baseName>.tri must exist and optionally <baseName>.egm\n"
        "    <retopoBase>.tri and optionally <retopoBase>.emg will be created\n"
        "DESCRIPTION:\n"
        "    * the surfaces must be in exact alignment\n"
        "    * unchanged verts will preserve their morph and EGM values\n"
        "    * new verts will have zero values for morph and EGM\n"
        "    * surface points and marked verts are discarded"
    };
    string              baseIn = syn.next(),
                        baseRe = syn.next(),
                        baseOut = syn.next();
    syn.noMoreArgsExpected();
    Mesh                meshIn = loadTri(baseIn+".tri"),
                        meshRe = loadTri(baseRe+".tri");
    float               maxDim = cMaxElem(cDims(meshIn.verts)),
                        threshMag = maxDim * 0.000001f;         // One part in 1M match threshold
    Uints               mapRI;
    for (Vec3F vr : meshRe.verts) {
        float               bestMag = lims<float>::max();
        uint                bestIdx;
        for (size_t vv=0; vv<meshIn.verts.size(); ++vv) {
            Vec3F const &       vi = meshIn.verts[vv];
            float               mag = cMag(vi-vr);
            if (mag < bestMag) {
                bestMag = mag;
                bestIdx = uint(vv);
            }
        }
        if (bestMag < threshMag)
            mapRI.push_back(bestIdx);
        else
            mapRI.push_back(lims<uint>::max());
    }
    Uints               mapIR(meshIn.verts.size(),lims<uint>::max());
    for (size_t rr=0; rr<mapRI.size(); ++rr) {
        uint            ii = mapRI[rr];
        if (ii != lims<uint>::max()) {
            FGASSERT(ii < mapIR.size());
            mapIR[ii] = uint(rr);
        }
    }
    Mesh                meshOut = meshRe;
    for (DirectMorph const & morphIn : meshIn.deltaMorphs) {
        DirectMorph               morphOut {morphIn.name};
        for (size_t vv=0; vv<meshRe.verts.size(); ++vv) {
            uint                idx = mapRI[vv];
            if (idx == lims<uint>::max())
                morphOut.verts.push_back(Vec3F{0});
            else
                morphOut.verts.push_back(morphIn.verts[idx]);
        }
        meshOut.deltaMorphs.push_back(morphOut);
    }
    for (IndexedMorph const & morphIn : meshIn.targetMorphs) {
        IndexedMorph        morphOut {morphIn.name,{}};
        for (size_t vv=0; vv<morphIn.ivs.size(); ++vv) {
            uint            idxI = morphIn.ivs[vv].idx,
                            idxR = mapIR[idxI];
            if (idxR != lims<uint>::max())
                morphOut.ivs.emplace_back(idxR,morphIn.ivs[vv].vec);
        }
        meshOut.targetMorphs.push_back(morphOut);
    }
    saveTri(baseOut+".tri",meshOut);
}

void                cmdBoundEdges(CLArgs const & args)
{
    Syntax          syn(args,
        R"(<in>.<ext>
    <ext>       - )" + getMeshLoadExtsCLDescription() + R"(
    Saves a mesh for each contiguous boundary edge seam with the vertices of that seam marked.
OUTPUT:
    <in>-##.fgmesh      A numbered file for each connected boundary edge)"
    );
    string              fname = syn.next();
    Mesh                mesh = loadMesh(fname);
    SurfTopo            topo(mesh.verts.size(),mesh.asTriSurf().tris);
    auto                boundaries = topo.boundaries();
    String8             fbase = pathToBase(fname) + "-";
    size_t              cnt = 0;
    for (auto const & boundary : boundaries) {
        Mesh            mm = mesh;
        for (auto const & be : boundary)
            mm.markedVerts.push_back(MarkedVert{be.vertIdx});
        saveMesh(mm,fbase + toStrDigits(cnt++,2) + ".fgmesh");
    }
}

void                cmdSplitSurf(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<mesh>.<ext> <root>
    <ext>       - )" + getMeshLoadExtsCLDescription() + R"(
OUTPUT:
    <root>-<name>.fgmesh    For each surface <name> in <mesh>.<ext>. If the surfaces do not have names,
                            they will be assigned sequential integers.
NOTES:
    * Each output file retains the full list of vertices and uvs)"
    };
    Mesh                mesh = loadMesh(syn.next());
    string              root = syn.next();
    size_t              cnt {0};
    for (Surf const & surf : mesh.surfaces) {
        Mesh                out = mesh;
        out.surfaces = {surf};
        String8             name = surf.name;
        if (name.empty())
            name = toStrDigits(cnt++,2);
        saveFgmesh(root+"-"+name+".fgmesh",out);
    }
}

void                cmdUvsSplitContig(CLArgs const & args)
{
    Syntax              syn {args,
        "<in>.<extIn> <out>.fgmesh\n"
        "    <extIn> = " + getMeshLoadExtsCLDescription() + "\n"
        "Each UV-contiguous patch is given a separate surface."
    };
    Mesh                mesh = loadMesh(syn.next());
    saveMesh(splitSurfsByUvContiguous(mesh),syn.next());
}

void                cmdSplitContigVerts(CLArgs const & args)
{
    Syntax              syn {args,"<in>.<extIn> <out>.fgmesh\n"
        "    <extIn> = " + getMeshLoadExtsCLDescription() + "\n"
        "COMMENTS:\n"
        "    Splits all surfaces by connected vertex indices"
    };
    Mesh                mesh = loadMesh(syn.next());
    Surfs               surfs;
    for (Surf const & surf : mesh.surfaces)
        cat_(surfs,splitByContiguous(surf));
    mesh.surfaces = surfs;
    saveFgmesh(syn.next(),mesh);
}

void                surfAdd(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<ext> <name> <out>.fgmesh\n"
        "    <ext>  - " + getMeshLoadExtsCLDescription() + "\n"
        "    <name> - Surface name"
        );
    Mesh        mesh = loadMesh(syn.next());
    Surf     surf;
    surf.name = syn.next();
    mesh.surfaces.push_back(surf);
    saveFgmesh(syn.next(),mesh);
}

void                surfCopy(CLArgs const & args)
{
    Syntax    syn(args,
        "<from>.fgmesh <to>.<ext> <out>.fgmesh\n"
        "    <ext>  - " + getMeshLoadExtsCLDescription() + "\n"
        " * tris only, uvs not preserved."
        );
    Mesh        from = loadFgmesh(syn.next()),
                    to = loadMesh(syn.next());
    saveFgmesh(syn.next(),copySurfaceStructure(from,to));
}

void                surfDel(CLArgs const & args)
{
    Syntax              syn {args,"<in>.fgmesh <out>.fgmesh <idx>+\n"
        "    <idx> - surface indices to delete"
    };
    Mesh                mesh = loadMesh(syn.next());
    String              outName = syn.next();
    Uints               inds;
    while (syn.more()) {
        uint                idx = syn.nextAs<uint>();
        if (idx >= mesh.surfaces.size())
            syn.error("Selected surface index out of range",toStr(idx));
        inds.push_back(idx);
    }
    sort(inds.rbegin(),inds.rend());        // Largest to smallest
    for (uint idx : inds)
        mesh.surfaces.erase(mesh.surfaces.begin()+idx);
    saveMesh(mesh,outName);
}

void                surfIso(CLArgs const & args)
{
    Syntax              syn {args,"<in>.fgmesh <out>.fgmesh (<idx>)+\n"
        "    <idx> - surface index to delete"
    };
    Mesh                mesh = loadMesh(syn.next());
    String              outName = syn.next();
    Uints               inds;
    while (syn.more()) {
        uint                idx = syn.nextAs<uint>();
        if (idx >= mesh.surfaces.size())
            syn.error("Selected surface index out of range",toStr(idx));
        inds.push_back(idx);
    }
    mesh.surfaces = select(mesh.surfaces,inds);
    saveMesh(mesh,outName);
}

void                surfList(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<ext>\n"
        "    <ext> - " + getMeshLoadExtsCLDescription()
        );
    Mesh    mesh = loadMesh(syn.next());
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        Surf const & surf = mesh.surfaces[ss];
        fgout << fgnl << ss << ": " << surf.name;
    }
}

void                surfRen(CLArgs const & args)
{
    Syntax                  syn(args,
        "<in>.fgmesh <idx> <name>\n"
        "   <idx>  - Which surface\n"
        "   <name> - Surface name"
        );
    String8                 meshFname = syn.next();
    Mesh                    mesh = loadFgmesh(meshFname);
    size_t                  idx = syn.nextAs<size_t>();
    if (idx >= mesh.surfaces.size())
        syn.error("<idx> value larger than available surfaces");
    mesh.surfaces[idx].name = syn.next();
    saveFgmesh(meshFname,mesh);
}

void                cmdSurfPointCopy(CLArgs const & args)
{
    Syntax          syn(args,"<from>.<mi> <to>.<mi> <out>.<mo>\n"
        "    <mi>   - " + getMeshLoadExtsCLDescription() + "\n"
        "    <mo>   - " + getMeshSaveExtsCLDescription()
    );
    Mesh            from = loadMesh(syn.next()),
                    to = loadMesh(syn.next());
    if (from.surfaces.size() != to.surfaces.size())
        fgThrow("'from' and 'to' meshes have different surface counts");
    for (size_t ss=0; ss<to.surfaces.size(); ++ss)
        cat_(to.surfaces[ss].surfPoints,from.surfaces[ss].surfPoints);
    saveMesh(to,syn.next());
}

void                cmdSurfPointDel(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<in>.<ext> (<surfIdx> <pointIdx> | all)
    <ext>           - (tri | fgmesh)
    <surfIdx>       - surface index
    <pointIdx>      - point index within that surface
OUTPUT:
    <in>.<ext> is modified to delete the specified surface point(s)
NOTES:
    Use 'fgbl mesh surf point list' to see the surface and point indices of all surface points
)"
    };
    String              meshFname = syn.next();
    Mesh                mesh = loadMesh(meshFname);
    if (syn.peekNext() == "all") {
        syn.next();
        for (Surf & surf : mesh.surfaces)
            surf.surfPoints.clear();
    }
    else {
        size_t              ss = syn.nextAs<size_t>();
        if (ss >= mesh.surfaces.size())
            syn.error("surface index out of bounds",syn.curr());
        Surf &              surf = mesh.surfaces[ss];
        size_t              ii = syn.nextAs<size_t>();
        if (ii >= surf.surfPoints.size())
            syn.error("point index out of bounds",syn.curr());
        surf.surfPoints.erase(surf.surfPoints.begin() + ii);
    }
    saveMesh(mesh,meshFname);
}

void                cmdSurfPointList(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<in>.<ext>
    <ext>           - (tri | fgmesh)
OUTPUT:
    prints out a list of all surface points for each surface in the mesh by index number)"
    };
    Mesh                mesh = loadMesh(syn.next());
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        Surf const &        surf = mesh.surfaces[ss];
        fgout << fgnl << "Surface " << ss << ": ";
        if (surf.surfPoints.empty())
            fgout << "no surface points";
        else {
            PushIndent          pind;
            for (size_t ii=0; ii<surf.surfPoints.size(); ++ii)
                fgout << fgnl << ii << " : " << surf.surfPoints[ii].label;
        }
    }
}

void                cmdSurfPointMark(CLArgs const & args)
{
    Syntax              syn {args,
        R"([-c] (<mesh>.<ext> | <meshList>.txt) (<landmark>+ | <fidList>.txt)
    -c              - if there is an image file of the same base name use it as a color map
    <ext>           - (tri | fgmesh)
    <meshList>.txt  - contains one or more lines, each of the format <mesh>.<ext>
    <landmark>      - name of the surface point to place
    <fidList>.txt   - contains one or more lines, each of the format <landmark>
OUTPUT:
    <mesh>.<ext> is updated with the interactively placed surface points
NOTES:
    * Requires a GUI; Windows only.
    * Repeated point placement will update the position of the current point.
    * Close the GUI window to select each point placement. It will re-open for the next point.
    * The list files can contain comment lines beginning with the '#' character.
    * The 'fgbl view mesh' command can be used to mark surface points on a single mesh (from 'Edit' tab))"
    };
    bool                useColor = false;
    if (syn.peekNext()[0] == '-') {
        if (syn.next() == "-c")
            useColor = true;
        else
            syn.error("unknown option",syn.curr());
    }
    Strings             meshPaths;
    if (endsWith(syn.peekNext(),".txt"))
        meshPaths = splitLines(loadRawString(syn.next()),'#');
    else
        meshPaths.emplace_back(syn.next());
    Strings             lmNames;
    if (endsWith(syn.peekNext(),".txt"))
        lmNames = splitLines(loadRawString(syn.next()),'#');
    else {
        do
            lmNames.push_back(syn.next());
        while (syn.more());
    }
    for (String const & meshPath : meshPaths) {
        Mesh                mesh = loadMesh(meshPath);
        PushIndent          pind {meshPath};
        Strings             existing = sliceMember(mesh.surfPointsAsNameVecs(),&NameVec3F::name),
                            toPlace = setwiseSubtract(lmNames,existing);
        if (!toPlace.empty()) {
            if (useColor && !mesh.surfaces.empty()) {
                String8             dirBase = pathToDirBase(meshPath);
                Strings             imgExts = findExts(dirBase,getImgExts());
                if (!imgExts.empty())
                    mesh.surfaces[0].setAlbedoMap(loadImage(dirBase+"."+imgExts[0]));
            }
            if (guiPlaceSurfPoints_(lmNames,mesh))
                saveMesh(mesh,meshPath);
        }
    }
}

void                cmdSurfPointRen(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.fgmesh <surfIdx> <ptIdx> <name>\n"
        "   <surfIdx> - Which surface\n"
        "   <spIdx>   - Which point on that surface\n"
        "   <name>    - Name"
        );
    String8        meshFname = syn.next();
    Mesh        mesh = loadFgmesh(meshFname);
    size_t          ss = syn.nextAs<size_t>(),
                    ii = syn.nextAs<size_t>();
    if (ss >= mesh.surfaces.size())
        fgThrow("Surface index value larger than available surfaces");
    Surf &   surf = mesh.surfaces[ss];
    if (ii >= surf.surfPoints.size())
        fgThrow("Point index value larger than availables points");
    surf.surfPoints[ii].label = syn.next();
    saveFgmesh(meshFname,mesh);
}

void                cmdSurfPointToVert(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.tri <out>.tri\n"
        "    <out>.tri will be appended with the new marked vertices."
        );
    Mesh    in = loadTri(syn.next());
    Mesh    out = loadTri(syn.next());
    surfPointsToMarkedVerts_(in,out);
    saveTri(syn.curr(),out);
}

void                cmdToTris(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <extOut> = " + getMeshSaveExtsCLDescription()
        );
    Mesh    mesh = loadMesh(syn.next());
    mesh.convertToTris();
    saveMesh(mesh,syn.next());
}

void                cmdFuseUvs(CLArgs const & args)
{
    Syntax          syn {args,
        R"(<in>.<extIn> <out>.<extOut>
    <extIn>     - )" + getMeshLoadExtsCLDescription() + R"(
    <extOut>    - )" + getMeshSaveExtsCLDescription()
    };
    Mesh            in = loadMesh(syn.next()),
                    out = fuseIdenticalUvs(in);
    fgout << fgnl << in.uvs.size()-out.uvs.size() << " identical UVs fused";
    saveMesh(out,syn.next());
}

void                cmdVertsFuse(CLArgs const & args)
{
    Syntax          syn {args,
        R"(<in>.<extIn> <out>.<extOut>
    <extIn>     - )" + getMeshLoadExtsCLDescription() + R"(
    <extOut>    - )" + getMeshSaveExtsCLDescription() + R"(
NOTES:
    * morphs and marked vertices are not preserved)"
    };
    Mesh            in = loadMesh(syn.next()),
                    out = fuseIdenticalVerts(in);
    fgout << fgnl << in.verts.size()-out.verts.size() << " identical verts fused";
    saveMesh(out,syn.next());
}

void                cmdUvclamp(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<ext0> [<out>.<ext1>]\n"
        "    <ext0> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <ext1> = " + getMeshSaveExtsCLDescription()
        );
    Mesh        in = loadMesh(syn.next());
    Mat22F        cb(0,1,0,1);
    for (size_t ii=0; ii<in.uvs.size(); ++ii)
        in.uvs[ii] = mapClamp(in.uvs[ii],cb);
    if (syn.more())
        saveMesh(in,syn.next());
    else
        saveMesh(in,syn.curr());
    return;
}

void                cmdUvSolidImage(CLArgs const & args)
{
    Syntax    syn(args,
        "<mesh>.<extM> <size> <image>.<extI>\n"
        "    <mesh>     - The mesh must contains UVs for anything to be done.\n"
        "    <extM>     - " + getMeshLoadExtsCLDescription() + "\n"
        "    <size>     - Output image size (will be square).\n"
        "    <image>    - Output image.\n"
        "    <extI>     - " + clOptionsStr(getImgExts())
        );
    Mesh        mesh = loadMesh(syn.next());
    uint            sz = syn.nextAs<uint>();
    if (sz > (1 << 12))
        syn.error("<size> is too large",syn.curr());
    ImgUC         img = getUvCover(mesh,Vec2UI(sz*4));
    img = shrink2Fixed(img);
    img = shrink2Fixed(img);
    saveImage(syn.next(),toRgba8(img));
}

void                cmdUvWireframe(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<mesh>.<extm> [<background>.<exti>]+
    <mesh>     - The mesh must contains UVs
    <extm>     - )" + getMeshLoadExtsCLDescription() + R"(
    <image>    - If provided, will be used as the background
    <exti>     - )" + clOptionsStr(getImgExts()) + R"(
"OUTPUTS:
    * <mesh>-uvs-#.jpg          The UV wireframe image for each surface)"
    };
    Mesh                mesh = loadMesh(syn.next());
    ImgRgba8s           imgs = cUvWireframeImages(mesh,Rgba8{0,255,0,255});
    String8             base = pathToBase(syn.curr());
    size_t              cnt {0};
    for (ImgRgba8 const & img : imgs) {
        String8             fname = base + "-uvs-" + toStr(cnt++) + ".jpg";
        if (syn.more())
            saveJfif(composite(img,loadImage(syn.next())),fname);
        else
            saveJfif(img,fname);
    }
}

void                cmdUvmask(CLArgs const & args)
{
    Syntax    syn(args,
        "<meshIn>.<ext0> <imageIn>.<ext1> <meshOut>.<ext2>\n"
        "    <ext0> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <ext1> = " + clOptionsStr(getImgExts()) + "\n"
        "    <ext2> = " + getMeshSaveExtsCLDescription()
        );
    Mesh        mesh = loadMesh(syn.next());
    ImgRgba8     img = loadImage(syn.next());
    Img<FatBool> mask = Img<FatBool>(img.dims());
    for (Iter2UI it(img.dims()); it.valid(); it.next()) {
        Arr4UC      px = img[it()].m_c;
        mask[it()] = (px[0] > 0) || (px[1] > 0) || (px[2] > 0); }
    mask = mapAnd(mask,flipHoriz(mask));
    mesh = fg3dMaskFromUvs(mesh,mask);
    saveMesh(mesh,syn.next());
}

void                cmdUvunwrap(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<ext0> [<out>.<ext1>]\n"
        "    <ext0> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <ext1> = " + getMeshSaveExtsCLDescription()
        );
    Mesh        in = loadMesh(syn.next());
    for (size_t ii=0; ii<in.uvs.size(); ++ii) {
        Vec2F    uv = in.uvs[ii];
        in.uvs[ii][0] = cMod(uv[0],1.0f);
        in.uvs[ii][1] = cMod(uv[1],1.0f);
    }
    if (syn.more())
        saveMesh(in,syn.next());
    else
        saveMesh(in,syn.curr());
    return;
}

void                cmdXformApply(CLArgs const & args)
{
    Syntax              syn(args,
        "<similarity>.txt <in>.<ext0> <out>.<ext1>\n"
        "    <ext0> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <ext1> = " + getMeshSaveExtsCLDescription()
        );
    SimilarityD         cmdXform = dsrlzText<SimilarityD>(loadRawString(syn.next()));
    Mesh                in = loadMesh(syn.next()),
                        out = transform(in,cmdXform);
    saveMesh(out,syn.next());
}

void                cmdXformCreateIdentity(CLArgs const & args)
{
    Syntax              syn(args,
        "<output>.txt \n"
        "    Edit the values in this file or apply subsequent transforms with other commands"
        );
    saveRaw(srlzText(SimilarityD::identity()),syn.next());
}

void                cmdXformCreateMeshes(CLArgs const & args)
{
    Syntax    syn(args,
        "<similarity>.txt <base>.<ex> <transformed>.<ex>\n"
        "    <ex> = " + getMeshLoadExtsCLDescription()
        );
    string              simFname = syn.next();
    Mesh                base = loadMesh(syn.next());
    Mesh                targ = loadMesh(syn.next());
    if (base.verts.size() != targ.verts.size())
        fgThrow("Base and target mesh vertex counts are different");
    Vec3Ds              bv = mapCast<Vec3D>(base.verts),
                        tv = mapCast<Vec3D>(targ.verts);
    SimilarityD         sim = solveSimilarity(bv,tv);
    double              ssd = cSsd(mapMul(sim.asAffine(),bv),tv),
                        sz = cMaxElem(cDims(tv));
    fgout << fgnl << "Transformed base to target relative RMS delta: " << sqrt(ssd / tv.size()) / sz;
    saveRaw(srlzText(sim),simFname);
}

void                cmdXformCreateRotate(CLArgs const & args)
{
    Syntax          syn(args,
        "<output>.txt <axis> <degrees> <point> [<input>.txt]\n"
        "    <output>   - Save here\n"
        "    <axis>     - (x | y | z)  Right-hand-rule axis of rotation\n"
        "    <point>    - <x> <y> <z>  Point around which rotation is applied\n"
        "    <input>    - Compose rotation after this transform, if specified"
        );
    string          outName = syn.next();
    string          axisStr = syn.next();
    if (axisStr.size() != 1)
        syn.error("<axis> must be one character");
    char            axisName = tolower(axisStr[0]);
    int             axisNum = int(axisName) - int('x');
    if ((axisNum < 0) || (axisNum > 2))
        syn.error("Invalid value for <axis>",axisStr);
    double          degs = syn.nextAs<double>(),
                    rads = degToRad(degs);
    QuaternionD     rot(rads,uint(axisNum));
    Vec3D           point {0};
    point[0] = syn.nextAs<double>();
    point[1] = syn.nextAs<double>();
    point[2] = syn.nextAs<double>();
    SimilarityD     xf = SimilarityD{point} * SimilarityD{rot} * SimilarityD{-point};
    if (syn.more())
        xf = xf * dsrlzText<SimilarityD>(loadRawString(syn.next()));
    saveRaw(srlzText(xf),outName);
}

void                cmdXformCreateScale(CLArgs const & args)
{
    Syntax    syn(args,"<similarity>.txt <scale>"
        );
    string          simFname = syn.next();
    SimilarityD    sim;
    if (pathExists(simFname))
        sim = dsrlzText<SimilarityD>(loadRawString(simFname));
    double          scale = syn.nextAs<double>();
    saveRaw(srlzText(SimilarityD(scale)*sim),simFname);
}

void                cmdXformCreateTrans(CLArgs const & args)
{
    Syntax              syn {args,R"(<similarity>.txt <X> <Y> <Z>
    OUTPUT:
        <similarity>.xml will be saved with the post-applied transalation <X>,<Y>,<Z>
    NOTES:
        * If <similarity>.xml does not exist, it will be initialized to the identity transform)"
    };
    String              simFname = syn.next();
    SimilarityD         sim;
    if (pathExists(simFname))
        sim = dsrlzText<SimilarityD>(loadRawString(simFname));
    Vec3D               trans;
    trans[0] = syn.nextAs<double>();
    trans[1] = syn.nextAs<double>();
    trans[2] = syn.nextAs<double>();
    saveRaw(srlzText(SimilarityD(trans)*sim),simFname);
}

void                cmdXformCreate(CLArgs const & args)
{
    Cmds            cmds {
        {cmdXformCreateIdentity,"identity","Create identity similarity transform XML file for editing"},
        {cmdXformCreateMeshes,"meshes","Create similarity transform from base and transformed meshes with matching vertex lists"},
        {cmdXformCreateRotate,"rotate","Combine a rotation with a similarity transform XML file"},
        {cmdXformCreateScale,"scale","Combine a scaling with a similarity transform XML file"},
        {cmdXformCreateTrans,"translate","Combine a translation with a similarity transform XML file"}
    };
    doMenu(args,cmds);
}

void                cmdSurfVertInds(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<in>.<ext> <out>.txt <surfIdx>*
    <in>.<ext>      - mesh with multiple surfaces or surfaces that do not reference all vertices
    <ext>           - )" + getMeshLoadExtsCLDescription() + R"(
    <surfIdx>       - index of a surface in the mesh (use 'fgbl view mesh' to see surface indices).
                      If none are specified, all surfaces are used.
OUTPUT:
    <out>.txt       - space-sparated list of all vertex indices used by the specified surfaces
NOTES:
    * The output vertex indices list can be used with 'fg3t ssmEyeI' to select vertices to transform
      rigidly (as eyes) instead of deformably (as skin) )"
    };
    Mesh                mesh = loadMesh(syn.next());
    String              outName = syn.next();
    Sizes               surfInds;
    while (syn.more()) {
        surfInds.push_back(syn.nextAs<size_t>());
    }
    if (surfInds.empty())               // default to all surfs if none specified
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss)
            surfInds.push_back(ss);
    set<uint>           vertInds;
    for (size_t ss : surfInds) {
        if (ss >= mesh.surfaces.size())
            syn.error("Surface index is larger than number of surfaces in mesh",toStr(ss));
        Surf const &        surf = mesh.surfaces[ss];
        for (Vec3UI inds : surf.tris.vertInds)
            for (uint ind : inds.m)
                vertInds.insert(ind);
        for (Vec4UI inds : surf.quads.vertInds)
            for (uint ind : inds.m)
                vertInds.insert(ind);
    }
    String              content;
    for (uint idx : vertInds)
        content += toStr(idx) + " ";
    saveRaw(content,outName);
}

void                cmdVertsCopy(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<mesh>.<exti> <verts>.<exti> <out>.<exto> [<inds>.txt]
    <mesh>          - the mesh whose vertices will be replaced
    <verts>         - the mesh from which to take the updated vertex positions
    <inds>.txt      - whitespace-separated list of the indices to be copied (all verts copied if not specified)
    <exti>          - )" + getMeshLoadExtsCLDescription() + R"(
    <exto>          - )" + getMeshSaveExtsCLDescription() + R"(
OUTPUT:
    <out>.<exto>    - <mesh> but with selected vertices from <verts>
NOTES:
    <verts> must have the same vertex count as <mesh>)"
    };
    Mesh                mesh = loadMesh(syn.next());
    size_t              V = mesh.verts.size();
    Vec3Fs              verts = loadMesh(syn.next()).verts;
    if (verts.size() != V)
        fgThrow("<mesh> and <verts> vertex lists are different sizes",toStr(V)+"!="+toStr(verts.size()));
    String              outFname = syn.next();
    Uints               inds;
    if (syn.more()) {
        Strings             tinds = splitWhitespace(loadRawString(syn.next()));
        inds = mapCallT<uint>(tinds,[](String const & s){return fromStr<uint>(s).val(); });
    }
    if (cMax(inds) >= V)
        fgThrow("index values in <inds>.txt exceed vertex count");
    if (inds.empty())
        mesh.verts = verts;
    else 
        for (uint idx : inds)
            mesh.verts[idx] = verts[idx];
    saveMesh(mesh,outFname);
    fgout << fgnl << (inds.empty() ? V : inds.size()) << " vertices updated to " << outFname;
}

void                cmdVertsSeld(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<mesh0>.<ext> <mesh1>.<ext> <out>.txt [<epsBits>]
    <ext>           - )" + getMeshLoadExtsCLDescription() + R"(
OUTPUT:
    <out>.txt       - list of indices of all vertices that differ between <mesh0> and <mesh1>
    <epsBits>       - [2,22] the bit depth, relative to the <mesh> vertex bounds max dimension, at which a
                      difference in vertex position is considered significant.
                      If not specified, all non-identical vertices will be selected.)"
    };
    Mesh                mesh0 = loadMesh(syn.next()),
                        mesh1 = loadMesh(syn.next());
    size_t              V = mesh0.verts.size();
    if (mesh1.verts.size() != V)
        fgThrow("mesh vertex sizes are different",toStr(V)+"!="+toStr(mesh1.verts.size()));
    String              outFile = syn.next();
    float               maxDel {0};
    if (syn.more()) {
        size_t              bits = syn.nextAs<size_t>();
        if (bits < 2)
            fgThrow("<epsBits> too small; changes will be ignored");
        if (bits > 22)
            fgThrow("<epsBits> too large; float mantissa is only 23 bits");
        float               maxDim = cMaxElem(cDims(mesh0.verts));
        maxDel = epsBits(bits) * maxDim;
    }
    Uints               inds;
    for (size_t ii=0; ii<V; ++ii) {
        float               del = cMaxElem(mapAbs(mesh1.verts[ii]-mesh0.verts[ii]));
        if (del > maxDel)
            inds.push_back(uint(ii));
    }
    String              out;
    for (uint idx : inds)
        out += toStr(idx) + "\n";
    saveRaw(out,outFile);
    fgout << fgnl << inds.size() << " of " << V << " vertices selected.";
}

void                cmdSurfPoint(CLArgs const & args)
{
    Cmds            cmds {
        {cmdSurfPointCopy,"copy","copy surf points between meshes with identical surface topology"},
        {cmdSurfPointDel,"del","delete a surface point"},
        {cmdSurfPointList,"list","list surface points in each surface"},
        {cmdSurfPointMark,"mark","mark lists of surface points on lists of meshes"},
        {cmdSurfPointRen,"ren","rename a surface point"},
        {cmdSurfPointToVert,"vert","convert surface points to marked vertices"},
    };
    doMenu(args,cmds);
}

void                cmdSurf(CLArgs const & args)
{
    Cmds            cmds {
        {surfAdd,"add","Add an empty surface to a mesh"},
        {cmdSurfCombine,"combine","Combine surfaces from meshes with identical vertex lists"},
        {surfCopy,"copy","Copy surface structure between aligned meshes"},
        {surfDel,"del","Delete specified surfaces"},
        {surfIso,"isolate","Delete all surfaces other than specified ones"},
        {surfList,"list","List surfaces in mesh"},
        {mergenamedsurfs,"mergeNamed","Merge surfaces with identical names"},
        {mergesurfs,"merge","Merge all surfaces in a mesh into one"},
        {cmdSurfPoint,"point","operations on surface points"},
        {cmdSplitContigVerts,"splitV","Split surfaces by contiguous vertex indices"},
        {cmdSplitSurf,"splitS","Split mesh by surface"},
        {surfRen,"ren","Rename a surface in a mesh"},
        {cmdSurfVertInds,"vinds","Save a TXT list of all vertex indices referenced by given surfaces"},
    };
    doMenu(args,cmds);
}

void                cmdUvs(CLArgs const & args)
{
    Cmds            cmds {
        {copyUvList,"copylist","Copy the UV list from one mesh to another with same UV count"},
        {copyUvsInds,"copyinds","Copy UV poly indices from one mesh to another with identical poly structure"},
        {copyUvsImv,"copyimv","Copy UV poly indices from one mesh to another with by matching polys based on vertex list indices"},
        {cmdUvsSplitContig,"splitcon","Split surfaces by contiguous UV mappings"},
        {cmdFuseUvs,"fuse","fuse identical UV coordinates"},
        {cmdUvclamp,"clamp","Clamp UV coords to the range [0,1]"},
        {cmdUvWireframe,"wireImg","Create a wireframe image of meshes UV map(s)"},
        {cmdUvSolidImage,"coverImg","Create a coverage image; solid white inside UV facets, black outside, 4xFSAA"},
        {cmdUvmask,"mask","Mask out geometry for any black areas of a texture image (auto symmetrized)"},
        {cmdUvunwrap,"unwrap","Unwrap wrap-around UV coords to the range [0,1]"},
    };
    doMenu(args,cmds);
}

void                cmdVerts(CLArgs const & args)
{
    Cmds                cmds {
        {cmdVertsCopy,"copy","Copy vertices from one mesh to another with same vertex count"},
        {cmdVertsFuse,"fuse","fuse identical vertices"},
        {cmdVertsSeld,"seld","select vertices which differ between two meshes with identical vertex lists"},
    };
    doMenu(args,cmds);
}

void                cmdXform(CLArgs const & args)
{
    Cmds                cmds {
        {cmdXformApply,"apply","Apply a simiarlity transform (from XML file) to a mesh"},
        {cmdXformCreate,"create","Create a similarity transform XML file"},
        {cmdMeshMirror,"mirror","Mirror the mesh around the X=0 plane"},
    };
    doMenu(args,cmds);
}

}

void                cmdMesh(CLArgs const & args)
{
    Cmds            cmds {
        {cmdBoundEdges,"edges","extract each boundary edge of a manifold mesh as a copy with edge verts marked"},
        {cmdConvert,"convert","Convert a mesh to a different format"},
        {cmdEdit,"edit","GUI view and edit one or more meshes in sequence"},
        {cmdEmboss,"emboss","Emboss a mesh based on greyscale values of a UV image"},
        {cmdExport,"export","Convert multiple meshes and related color maps into another format"},
        {cmdInject,"inject","Inject updated vertex positions without otherwise modifying a mesh file"},
        {cmdMark,"mark","List and label marked vertices"},
        {cmdMergeMeshes,"merge","Merge multiple meshes into one. No optimization is done"},
        {cmdMeshMirrorFuse,"mirFuse","Mirror the mesh around the X=0 plane and fuse with mirrored"},
        {cmdRdf,"rdf","Remove Duplicate Facets within each surface"},
        {cmdRetopo,"retopo","Rebase a mesh topology with an exactly aligned mesh"},
        {cmdRtris,"rtris","Remove specific tris from a mesh"},
        {cmdRuv,"ruv","Remove vertices and uvs not referenced by a surface or marked vertex"},
        {cmdRevWind,"rwind","Reverse facet winding of a mesh"},
        {cmdSurf,"surf","Operations on mesh surface structure"},
        {cmdToTris,"toTris","Convert all facets to tris"},
        {cmdUvs,"uvs","UV-specific commands"},
        {cmdVerts,"verts","vertex-specific commands"},
        {cmdXform,"xform","Create / apply similarity transforms from / to meshes"},
    };
    doMenu(args,cmds);
}

}

// */
