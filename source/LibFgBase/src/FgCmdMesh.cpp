//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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

void
combinesurfs(CLArgs const & args)
{
    Syntax    syn(args,
        "(<mesh>.<extIn>)+ <out>.<extOut>\n"
        "    <extIn> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <extOut> = " + getMeshSaveExtsCLDescription() + "\n"
        "    All input meshes must have identical vertex lists.\n"
        );
    Mesh    mesh = loadMesh(syn.next());
    while (syn.more()) {
        string  name = syn.next();
        if (syn.more()) {
            Mesh    next = loadMesh(name);
            cat_(mesh.surfaces,next.surfaces);
        }
        else
            saveMesh(mesh,name);
    }
}

void
convert(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <extOut> = " + getMeshSaveExtsCLDescription()
        );
    Mesh    mesh = loadMesh(syn.next());
    saveMesh(mesh,syn.next());
}

void
copyUvList(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<ext0> <out>.<ext1>\n"
        "    <ext0> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <ext1> = " + getMeshSaveExtsCLDescription()
        );
    Mesh        in = loadMesh(syn.next());
    Mesh        out = loadMesh(syn.next());
    if (in.uvs.size() != out.uvs.size())
        syn.error("Incompatible UV list sizes");
    out.uvs = in.uvs;
    saveMesh(out,syn.curr());
    return;
}

void
copyUvs(CLArgs const & args)
{
    Syntax    syn(args,
        "<from>.<ext0> <to>.<ext1>\n"
        "    <ext0> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <ext1> = " + getMeshSaveExtsCLDescription()
        );
    Mesh        in = loadMesh(syn.next());
    Mesh        out = loadMesh(syn.next());
    out.uvs = in.uvs;
    if (in.surfaces.size() != out.surfaces.size())
        fgThrow("Incompatible number of surfaces");
    for (size_t ss=0; ss<in.surfaces.size(); ++ss) {
        Surf const &     sin = in.surfaces[ss];
        Surf &           sout = out.surfaces[ss];
        if ((sin.tris.size() != sout.tris.size()) ||
            (sin.quads.size() != sout.quads.size()))
            fgThrow("Incompatible facet counts");
        sout.tris.uvInds = sin.tris.uvInds;
        sout.quads.uvInds = sin.quads.uvInds;
    }
    saveMesh(out,syn.curr());
    return;
}

void
copyverts(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<ext0> <out>.<ext1>\n"
        "    <ext0> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <ext1> = " + getMeshSaveExtsCLDescription() + "\n"
        "    <out> is modified to have the vertex list from <in>"
        );
    Mesh        in = loadMesh(syn.next());
    Mesh        out = loadMesh(syn.next());
    if (in.verts.size() != out.verts.size())
        syn.error("Incompatible vertex list sizes");
    out.verts = in.verts;
    saveMesh(out,syn.curr());
    return;
}

void
emboss(CLArgs const & args)
{
    Syntax          syn(args,
        "<uvImage>.<img> <meshin>.<ext0> <val> <out>.<ext1>\n"
        "    <uvImage> = a UV-layout image whose grescale values will be used to emboss (0 - none, 255 - full)\n"
        "    <img>     = " + getImageFileExtCLDescriptions() + "\n"
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

void
cmdInject(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<src>.[w]obj <verts>.<mesh> <dst>.obj
    <mesh>          - )" + getMeshLoadExtsCLDescription() + R"(
OUTPUT:
    <dst>.obj       Identical to <src>.[w]obj but with all vertex positions updated to the values in <verts>.<mesh>
NOTES:
    * The vertex list in <verts>.<mesh> must be in exact correspondence with the list in <src>.[w]obj)"
    };
    Strings             lines = splitLines(loadRaw(syn.next()));
    Vec3Fs              verts = loadMesh(syn.next()).verts;
    size_t              cnt {0};
    for (String & line : lines) {
        if (beginsWith(line,"v ")) {
            if (cnt >= verts.size())
                syn.error("<verts> contains fewer vertices than <src>");
            Vec3F           vert = verts[cnt++];
            line = "v " + toStr(vert[0]) + " " + toStr(vert[1]) + " " + toStr(vert[2]);
        }
    }
    if (cnt < verts.size())
        fgout << fgnl << "WARNING: <verts> contains more vertices than <src>";
    saveRaw(cat(lines,"\n"),syn.next());
}

void
revWind(CLArgs const & args)
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

void
markVerts(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.tri <verts>.<ext> <out>.tri\n"
        "    <ext> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <out>.tri will be saved after marking a vertex in <in>.tri that is closest to each vertex in <verts>.<ext>."
        );
    Mesh    mesh = loadTri(syn.next());
    Vec3Fs     verts = loadMesh(syn.next()).verts;
    float       dim = cMaxElem(cDims(mesh.verts));
    uint        poorMatches = 0,
                totalMatches = 0;
    for (size_t vv=0; vv<verts.size(); ++vv) {
        Vec3F        v = verts[vv];
        float           bestMag = floatMax();
        uint            bestIdx = 0;
        for (size_t ii=0; ii<mesh.verts.size(); ++ii) {
            float       mag = cMag(mesh.verts[ii]-v);
            if (mag < bestMag) {
                bestMag = mag;
                bestIdx = uint(ii);
            }
        }
        if (sqrt(bestMag) / dim > 0.00001f)
            ++poorMatches;
        if (!contains(mesh.markedVerts,bestIdx)) {
            mesh.markedVerts.push_back(MarkedVert(bestIdx));
            ++totalMatches;
        }
    }
    if (poorMatches > 0)
        fgout << fgnl << "WARNING: " << poorMatches << " poor matches.";
    if (totalMatches < verts.size())
        fgout << fgnl << "WARNING: duplicate matches.";
    fgout << fgnl << totalMatches << " vertices marked.";
    saveTri(syn.next(),mesh);
}

void
mmerge(CLArgs const & args)
{
    Syntax    syn(args,
        "(<mesh>.<extIn>)+ <out>.<extOut>\n"
        "    <extIn> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <extOut> = " + getMeshSaveExtsCLDescription()
        );
    Mesh    mesh = loadMesh(syn.next());
    while (syn.more()) {
        string  name = syn.next();
        if (syn.more())
            mesh = mergeMeshes(mesh,loadMesh(name));
        else
            saveMesh(mesh,name);
    }
}

void
mergesurfs(CLArgs const & args)
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
        mesh.surfaces = {mergeSurfaces(mesh.surfaces)};
    saveMesh(mesh,syn.next());
}

void
rdf(CLArgs const & args)
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

void
rtris(CLArgs const & args)
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
            Vec4UI       qvs = surf.quads.posInds[ti/2];
            Vec4UI       uvs(0);
            if (!surf.quads.uvInds.empty())
                uvs = surf.quads.uvInds[ti/2];
            surf.removeQuad(ti/2);
            // The other tri making up the quad needs to be appended to tris:
            if (ti & 0x1)
                surf.tris.posInds.push_back(Vec3UI(qvs[0],qvs[1],qvs[2]));
            else
                surf.tris.posInds.push_back(Vec3UI(qvs[2],qvs[3],qvs[0]));
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

void
ruv(CLArgs const & args)
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

void
sortFacets(CLArgs const & args)
{
    Syntax    syn(args,
        "<meshIn>.<ext> <albedo>.<img> <meshOut>.<ext> [<opaque>.<ext>]*\n"
        "    <mesh>     - Mesh to have facets sorted\n"
        "    <ext>      - " + getMeshLoadExtsCLDescription() + "\n"
        "    <albedo>   - Map containing the alpha channel\n"
        "    <img>      - " + getImageFileExtCLDescriptions() + "\n"
        "    <opaque>   - Any opaque objects blocking view which affects sorting\n"
        "Will find the best compromise sorted rendering order for front/back (+/-Z), side (+/-X)\n"
        "and top (Y) views, on a per-surface basis, and save the sorted result. All quads are\n"
        "converted to tris and all surfaces are merged into one.");
    Mesh        mesh = loadMesh(syn.next());
    ImgRgba8     albedo = loadImage(syn.next());
    String8        outName = syn.next();
    Mesh        opaque;
    while (syn.more())
        opaque = mergeMeshes(opaque,loadMesh(syn.next()));
    mesh = sortTransparentFaces(mesh,albedo,opaque);
    saveMesh(mesh,outName);
}

void
mergenamedsurfs(CLArgs const & args)
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

static void
retopo(CLArgs const & args)
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
        float               bestMag = floatMax();
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
            mapRI.push_back(uintMax());
    }
    Uints               mapIR(meshIn.verts.size(),uintMax());
    for (size_t rr=0; rr<mapRI.size(); ++rr) {
        uint            ii = mapRI[rr];
        if (ii != uintMax()) {
            FGASSERT(ii < mapIR.size());
            mapIR[ii] = uint(rr);
        }
    }
    Mesh                meshOut = meshRe;
    for (Morph const & morphIn : meshIn.deltaMorphs) {
        Morph               morphOut {morphIn.name};
        for (size_t vv=0; vv<meshRe.verts.size(); ++vv) {
            uint                idx = mapRI[vv];
            if (idx == uintMax())
                morphOut.verts.push_back(Vec3F{0});
            else
                morphOut.verts.push_back(morphIn.verts[idx]);
        }
        meshOut.deltaMorphs.push_back(morphOut);
    }
    for (IndexedMorph const & morphIn : meshIn.targetMorphs) {
        IndexedMorph        morphOut {morphIn.name,{},{}};
        for (size_t vv=0; vv<morphIn.baseInds.size(); ++vv) {
            uint            idxI = morphIn.baseInds[vv],
                            idxR = mapIR[idxI];
            if (idxR != uintMax()) {
                morphOut.baseInds.push_back(idxR);
                morphOut.verts.push_back(morphIn.verts[vv]);
            }
        }
        meshOut.targetMorphs.push_back(morphOut);
    }
    saveTri(baseOut+".tri",meshOut);
}

void
cmdBoundEdges(CLArgs const & args)
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

void
cmdSplitSurf(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<mesh>.<ext> <root>
    <ext>       - )" + getMeshLoadExtsCLDescription() + R"(
OUTPUT:
    <root>-<name>.fgmesh    For each surface <name> in <mesh>.<ext>
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
            name = "Unnamed-" + toStr(cnt++);
        saveFgmesh(root+name+".fgmesh",{out});
    }
}

void
cmdUvsSplitContig(CLArgs const & args)
{
    Syntax              syn {args,
        "<in>.<extIn> <out>.fgmesh\n"
        "    <extIn> = " + getMeshLoadExtsCLDescription() + "\n"
        "Each UV-contiguous patch is given a separate surface."
    };
    Mesh                mesh = loadMesh(syn.next());
    saveMesh(splitSurfsByUvContiguous(mesh),syn.next());
}

void
splitCont(CLArgs const & args)
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

void
surfAdd(CLArgs const & args)
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

void
surfCopy(CLArgs const & args)
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

void
surfDel(CLArgs const & args)
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

void
surfIso(CLArgs const & args)
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
    mesh.surfaces = permute(mesh.surfaces,inds);
    saveMesh(mesh,outName);
}

void
surfList(CLArgs const & args)
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

void
surfRen(CLArgs const & args)
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

void
spCopy(CLArgs const & args)
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

void
spDel(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.tri <ptIdx>\n"
        "   <spIdx>   - Which point on that surface to delete"
        );
    String8        meshFname = syn.next();
    Mesh        mesh = loadTri(meshFname);
    size_t          ii = syn.nextAs<size_t>();
    Surf &   surf = mesh.surfaces[0];
    if (ii >= surf.surfPoints.size())
        fgThrow("Point index value larger than availables points");
    surf.surfPoints.erase(surf.surfPoints.begin() + ii);
    saveTri(meshFname,mesh);
}

void
spList(CLArgs const & args)
{
    Syntax    syn(args,"<in>.fgmesh");
    Mesh    mesh = loadMesh(syn.next());
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        Surf const & surf = mesh.surfaces[ss];
        fgout << fgnl << "Surface " << ss << ": " << surf.name << fgpush;
        for (size_t ii=0; ii<surf.surfPoints.size(); ++ii)
            fgout << fgnl << ii << " : " << surf.surfPoints[ii].label;
        fgout << fgpop;
    }
}

void
spRen(CLArgs const & args)
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

void
spsToVerts(CLArgs const & args)
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

void
toTris(CLArgs const & args)
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

void
cmdFuseUvs(CLArgs const & args)
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

void
cmdFuseVerts(CLArgs const & args)
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

void
uvclamp(CLArgs const & args)
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

void
uvSolidImage(CLArgs const & args)
{
    Syntax    syn(args,
        "<mesh>.<extM> <size> <image>.<extI>\n"
        "    <mesh>     - The mesh must contains UVs for anything to be done.\n"
        "    <extM>     - " + getMeshLoadExtsCLDescription() + "\n"
        "    <size>     - Output image size (will be square).\n"
        "    <image>    - Output image.\n"
        "    <extI>     - " + getImageFileExtCLDescriptions()
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

void
cmdUvWireframe(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<mesh>.<extm> [<background>.<exti>]+
    <mesh>     - The mesh must contains UVs
    <extm>     - )" + getMeshLoadExtsCLDescription() + R"(
    <image>    - If provided, will be used as the background
    <exti>     - )" + getImageFileExtCLDescriptions() + R"(
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

void
uvmask(CLArgs const & args)
{
    Syntax    syn(args,
        "<meshIn>.<ext0> <imageIn>.<ext1> <meshOut>.<ext2>\n"
        "    <ext0> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <ext1> = " + getImageFileExtCLDescriptions() + "\n"
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

void
uvunwrap(CLArgs const & args)
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

void
xformApply(CLArgs const & args)
{
    Syntax              syn(args,
        "<similarity>.xml <in>.<ext0> <out>.<ext1>\n"
        "    <ext0> = " + getMeshLoadExtsCLDescription() + "\n"
        "    <ext1> = " + getMeshSaveExtsCLDescription()
        );
    SimilarityD         xform;
    loadBsaXml(syn.next(),xform);
    Mesh            in = loadMesh(syn.next()),
                    out = transform(in,xform);
    saveMesh(out,syn.next());
}

void
xformCreateIdentity(CLArgs const & args)
{
    Syntax    syn(args,
        "<output>.xml \n"
        "    Edit the values in this file or apply subsequent transforms with other commands"
        );
    string      simFname = syn.next();
    saveBsaXml(simFname,SimilarityD::identity());
}

void
xformCreateMeshes(CLArgs const & args)
{
    Syntax    syn(args,
        "<similarity>.xml <base>.<ex> <transformed>.<ex>\n"
        "    <ex> = " + getMeshLoadExtsCLDescription()
        );
    string      simFname = syn.next();
    Mesh    base = loadMesh(syn.next());
    Mesh    targ = loadMesh(syn.next());
    if (base.verts.size() != targ.verts.size())
        fgThrow("Base and target mesh vertex counts are different");
    Vec3Ds              bv = deepCast<double>(base.verts),
                        tv = deepCast<double>(targ.verts);
    SimilarityD         sim = solveSimilarity(bv,tv);
    double              ssd = cSsd(mapMul(sim.asAffine(),bv),tv),
                        sz = cMaxElem(cDims(tv));
    fgout << fgnl << "Transformed base to target relative RMS delta: " << sqrt(ssd / tv.size()) / sz;
    saveBsaXml(simFname,sim);
}

void
xformCreateRotate(CLArgs const & args)
{
    Syntax          syn(args,
        "<output>.xml <axis> <degrees> <point> [<input>.xml]\n"
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
        xf = xf * loadBsaXml<SimilarityD>(syn.next());
    saveBsaXml(outName,xf);
}

void
xformCreateScale(CLArgs const & args)
{
    Syntax    syn(args,
        "<similarity>.xml <scale>"
        );
    string          simFname = syn.next();
    SimilarityD    sim;
    if (pathExists(simFname))
        loadBsaXml(simFname,sim);
    double          scale = syn.nextAs<double>();
    saveBsaXml(simFname,SimilarityD(scale)*sim);
}

void
xformCreateTrans(CLArgs const & args)
{
    Syntax    syn(args,
        "<similarity>.xml <X> <Y> <Z>"
        );
    string          simFname = syn.next();
    SimilarityD    sim;
    if (pathExists(simFname))
        loadBsaXml(simFname,sim);
    Vec3D    trans;
    trans[0] = syn.nextAs<double>();
    trans[1] = syn.nextAs<double>();
    trans[2] = syn.nextAs<double>();
    saveBsaXml(simFname,SimilarityD(trans)*sim);
}

void
xformCreate(CLArgs const & args)
{
    Cmds            cmds {
        {xformCreateIdentity,"identity","Create identity similarity transform XML file for editing"},
        {xformCreateMeshes,"meshes","Create similarity transform from base and transformed meshes with matching vertex lists"},
        {xformCreateRotate,"rotate","Combine a rotation with a similarity transform XML file"},
        {xformCreateScale,"scale","Combine a scaling with a similarity transform XML file"},
        {xformCreateTrans,"translate","Combine a translation with a similarity transform XML file"}
    };
    doMenu(args,cmds);
}

void
xformMirror(CLArgs const & args)
{
    Syntax        syn(args,
        "<axis> <meshIn>.<ext1> [<meshOut>.<ext2>]\n"
        "    <axis>     - (x | y | z)\n"
        "    <ext1>     - " + getMeshLoadExtsCLDescription() + "\n"
        "    <ext2>     - " + getMeshSaveExtsCLDescription()
    );
    uint            axis = 0;
    string          axisStr = syn.nextLower().as_ascii();
    if (axisStr == "x")
        axis = 0;
    else if (axisStr == "y")
        axis = 1;
    else if (axisStr == "z")
        axis = 3;
    else
        syn.error("Invalid value of <axis>",axisStr);
    Mesh            mesh = cMirror(loadMesh(syn.next()),axis);
    String8         fname = (syn.more() ? syn.next() : syn.curr());
    saveMesh(mesh,fname);
}

void
cmdUvs(CLArgs const & args)
{
    Cmds            cmds {
        {copyUvList,"copyUvList","Copy UV list from one mesh to another with same UV count"},
        {copyUvs,"copyUvs","Copy UVs from one mesh to another with identical facet structure"},
        {cmdUvsSplitContig,"splitcon","Split surfaces by contiguous UV mappings"},
        {cmdFuseUvs,"fuse","fuse identical UV coordinates"},
        {uvclamp,"uvclamp","Clamp UV coords to the range [0,1]"},
        {cmdUvWireframe,"uvWire","Create a wireframe image of meshes UV map(s)"},
        {uvSolidImage,"uvImgS","Solid white inside UV facets, black outside, 4xFSAA"},
        {uvmask,"uvmask","Mask out geometry for any black areas of a texture image {auto symmetrized)"},
        {uvunwrap,"uvunwrap","Unwrap wrap-around UV coords to the range [0,1]"},
    };
    doMenu(args,cmds);
}

void
cmdSurfVertInds(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<in>.fgmesh <out>.txt (<surfIdx>)+
    <in>.fgmesh     - mesh with surfaces
    <surfIdx>       - index of a surface in the mesh
OUTPUT:
    <out>.txt       - space-sparated list of all vertex indices used by the specified surfaces
NOTES:
    * The output vertex indices list can be used by 'fg3t splice' to select vertices to transform as eyes
      instead of skin)"
    };
    Mesh                mesh = loadMesh(syn.next());
    String              out = syn.next();
    Sizes               surfInds;
    do {
        surfInds.push_back(syn.nextAs<size_t>());
    } while (syn.more());
    set<uint>           vertInds;
    for (size_t ss : surfInds) {
        if (ss >= mesh.surfaces.size())
            syn.error("Surface index is larger than number of surfaces in mesh",toStr(ss));
        Surf const &        surf = mesh.surfaces[ss];
        for (Vec3UI inds : surf.tris.posInds)
            for (uint ind : inds.m)
                vertInds.insert(ind);
        for (Vec4UI inds : surf.quads.posInds)
            for (uint ind : inds.m)
                vertInds.insert(ind);
    }
    String              content;
    for (uint idx : vertInds)
        content += toStr(idx) + " ";
    saveRaw(content,out);
}

void
cmdSurf(CLArgs const & args)
{
    Cmds            cmds {
        {surfAdd,"add","Add an empty surface to a mesh"},
        {surfCopy,"copy","Copy surface structure between aligned meshes"},
        {surfDel,"del","Delete specified surfaces"},
        {surfIso,"isolate","Delete all surfaces other than specified ones"},
        {surfList,"list","List surfaces in mesh"},
        {mergenamedsurfs,"mergeNamed","Merge surfaces with identical names"},
        {mergesurfs,"merge","Merge all surfaces in a mesh into one"},
        {splitCont,"splitCon","Split surfaces by contiguous vertex indices"},
        {surfRen,"ren","Rename a surface in a mesh"},
        {spCopy,"spCopy","Copy surf points between meshes with identical surface topology"},
        {spDel,"spDel","Delete a surface point"},
        {spList,"spList","List surface points in each surface"},
        {spRen,"spRen","Rename a surface point"},
        {spsToVerts,"spVert","Convert surface points to marked vertices"},
        {cmdSurfVertInds,"vinds","Save a TXT list of all vertex indices referenced by given surfaces"},
    };
    doMenu(args,cmds);
}

void
xform(CLArgs const & args)
{
    Cmds            cmds {
        {xformApply,"apply","Apply a simiarlity transform (from XML file) to a mesh"},
        {xformCreate,"create","Create a similarity transform XML file"},
        {xformMirror,"mirror","Mirror a mesh"},
    };
    doMenu(args,cmds);
}

void
cmdMesh(CLArgs const & args)
{
    Cmds            cmds {
        {cmdBoundEdges,"edges","extract each boundary edge of a manifold mesh as a copy with edge verts marked"},
        {combinesurfs,"combinesurfs","Combine surfaces from meshes with identical vertex lists"},
        {convert,"convert","Convert the mesh between different formats"},
        {copyverts,"copyverts","Copy verts from one mesh to another with same vertex count"},
        {emboss,"emboss","Emboss a mesh based on greyscale values of a UV image"},
        {cmdExportInfo()},
        {cmdInject,"inject","Inject updated vertex positions into a Wavefront OBJ file"},
        {markVerts,"markVerts","Mark vertices in a .TRI file from a given list"},
        {mmerge,"merge","Merge multiple meshes into one. No optimization is done"},
        {rdf,"rdf","Remove Duplicate Facets within each surface"},
        {retopo,"retopo","Rebase a mesh topology with an exactly aligned mesh"},
        {rtris,"rtris","Remove specific tris from a mesh"},
        {ruv,"ruv","Remove vertices and uvs not referenced by a surface or marked vertex"},
        {revWind,"rwind","Reverse facet winding of a mesh"},
        {sortFacets,"sortFacets","Sort facets for optimal transparency viewing"},
        {cmdSplitSurf,"splitSurf","Split mesh by surface"},
        {cmdSurf,"surf","Operations on mesh surface structure"},
        {toTris,"toTris","Convert all facets to tris"},
        {cmdFuseVerts,"fuse","fuse identical vertices"},
        {cmdUvs,"uvs","UV-specific commands"},
        {xform,"xform","Create or apply similarity transforms from/to meshes"},
    };
    doMenu(args,cmds);
}

}

Cmd
getCmdMesh()
{return Cmd(cmdMesh,"mesh","3D Mesh IO and manipulation tools"); }

}

// */
