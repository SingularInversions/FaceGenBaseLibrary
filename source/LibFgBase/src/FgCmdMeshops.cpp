//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgSyntax.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dMeshOps.hpp"
#include "FgGeometry.hpp"
#include "FgMetaFormat.hpp"
#include "Fg3dNormals.hpp"
#include "FgSimilarity.hpp"
#include "Fg3dTopology.hpp"
#include "Fg3dDisplay.hpp"
#include "FgBestN.hpp"

using namespace std;

namespace Fg {

namespace {

void
combinesurfs(CLArgs const & args)
{
    Syntax    syn(args,
        "(<mesh>.<extIn>)+ <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription() + "\n"
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
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh    mesh = loadMesh(syn.next());
    saveMesh(mesh,syn.next());
}

void
copyUvList(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<ext0> <out>.<ext1>\n"
        "    <ext0> = " + meshLoadFormatsCLDescription() + "\n"
        "    <ext1> = " + meshSaveFormatsCLDescription()
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
        "    <ext0> = " + meshLoadFormatsCLDescription() + "\n"
        "    <ext1> = " + meshSaveFormatsCLDescription()
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
        "    <ext0> = " + meshLoadFormatsCLDescription() + "\n"
        "    <ext1> = " + meshSaveFormatsCLDescription() + "\n"
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
    Syntax    syn(args,
        "<uvImage>.<img> <meshin>.<ext0> <val> <out>.<ext1>\n"
        "    <uvImage> = a UV-layout image whose grescale values will be used to emboss (0 - none, 255 - full)\n"
        "    <img>     = " + imgFileExtensionsDescription() + "\n"
        "    <ext0>    = " + meshLoadFormatsCLDescription() + "\n"
        "    <val>     = Embossing factor as ratio of the max bounding box dimension.\n"
        "    <ext1>    = " + meshSaveFormatsCLDescription()
        );
    ImgUC         img;
    loadImage_(syn.next(),img);      // Treat as greyscale
    Mesh        mesh = loadMesh(syn.next());
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
invWind(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription() + "\n"
        "    Inverts the winding of all facets in <in> and saves to <out>"
        );
    Mesh    mesh = loadMesh(syn.next());
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        Surf &   surf = mesh.surfaces[ss];
        for (size_t ii=0; ii<surf.tris.size(); ++ii)
            std::swap(surf.tris.posInds[ii][1],surf.tris.posInds[ii][2]);
        for (size_t ii=0; ii<surf.tris.uvInds.size(); ++ii)
            std::swap(surf.tris.uvInds[ii][1],surf.tris.uvInds[ii][2]);
        for (size_t ii=0; ii<surf.quads.size(); ++ii)
            std::swap(surf.quads.posInds[ii][1],surf.quads.posInds[ii][3]);
        for (size_t ii=0; ii<surf.quads.uvInds.size(); ++ii)
            std::swap(surf.quads.uvInds[ii][1],surf.quads.uvInds[ii][3]);
    }
    saveMesh(mesh,syn.next());
}

void
markVerts(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.tri <verts>.<ext> <out>.tri\n"
        "    <ext> = " + meshLoadFormatsCLDescription() + "\n"
        "    <out>.tri will be saved after marking a vertex in <in>.tri that is closest to each vertex in <verts>.<ext>."
        );
    Mesh    mesh = loadTri(syn.next());
    Vec3Fs     verts = loadMesh(syn.next()).verts;
    float       dim = cMaxElem(cDims(mesh.verts));
    uint        poorMatches = 0,
                totalMatches = 0;
    for (size_t vv=0; vv<verts.size(); ++vv) {
        Vec3F        v = verts[vv];
        float           bestMag = maxFloat();
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
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
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
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
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
    Ustring        fni(syn.next()),
                    fno = fni;
    if (syn.more())
        fno = syn.next();
    saveTri(fno,removeDuplicateFacets(loadTri(fni)));
}

void
rt(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<extIn> <out>.<extOut> (<surfIndex> <triEquivIndex>)+\n"
        "    <extIn>    - " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut>   - " + meshSaveFormatsCLDescription() + "\n"
        "    <triEquivIndex> - Tri-equivalent index of triangle or half-quad to remove."
        );
    Mesh        mesh = loadMesh(syn.next());
    Ustring        outName = syn.next();
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
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh    mesh = loadMesh(syn.next());
    mesh = meshRemoveUnusedVerts(mesh);
    saveMesh(mesh,syn.next());
}

void
sortFacets(CLArgs const & args)
{
    Syntax    syn(args,
        "<meshIn>.<ext> <albedo>.<img> <meshOut>.<ext> [<opaque>.<ext>]*\n"
        "    <mesh>     - Mesh to have facets sorted\n"
        "    <ext>      - " + meshLoadFormatsCLDescription() + "\n"
        "    <albedo>   - Map containing the alpha channel\n"
        "    <img>      - " + imgFileExtensionsDescription() + "\n"
        "    <opaque>   - Any opaque objects blocking view which affects sorting\n"
        "Will find the best compromise sorted rendering order for front/back (+/-Z), side (+/-X)\n"
        "and top (Y) views, on a per-surface basis, and save the sorted result. All quads are\n"
        "converted to tris and all surfaces are merged into one.");
    Mesh        mesh = loadMesh(syn.next());
    ImgC4UC     albedo = loadImage(syn.next());
    Ustring        outName = syn.next();
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
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
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
        float               bestMag = floatMax;
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
            mapRI.push_back(uintMax);
    }
    Uints               mapIR(meshIn.verts.size(),uintMax);
    for (size_t rr=0; rr<mapRI.size(); ++rr) {
        uint            ii = mapRI[rr];
        if (ii != uintMax) {
            FGASSERT(ii < mapIR.size());
            mapIR[ii] = uint(rr);
        }
    }
    Mesh                meshOut = meshRe;
    for (Morph const & morphIn : meshIn.deltaMorphs) {
        Morph               morphOut {morphIn.name};
        for (size_t vv=0; vv<meshRe.verts.size(); ++vv) {
            uint                idx = mapRI[vv];
            if (idx == uintMax)
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
            if (idxR != uintMax) {
                morphOut.baseInds.push_back(idxR);
                morphOut.verts.push_back(morphIn.verts[vv]);
            }
        }
        meshOut.targetMorphs.push_back(morphOut);
    }
    saveTri(baseOut+".tri",meshOut);
}

static void
seams(CLArgs const & args)
{
    Syntax          syn(args,"<in>.<ext>\n"
        "    <ext>    - " + meshLoadFormatsCLDescription() + "\n"
        "    Saves a mesh to <in>_<num>.tri for each contiguous seam in <in> with the vertices of that seam marked."
    );
    string              fname = syn.next();
    Mesh                mesh = loadMesh(fname);
    MeshTopology        topo(mesh.verts.size(),mesh.asTriSurf().tris);
    Svec<set<uint> >    seams = topo.seams();
    Ustring             fbase = pathToBase(fname) + "_";
    size_t              cnt = 0;
    for (set<uint> const & seam : seams) {
        Mesh            mm = mesh;
        for (uint vv : seam)
            mm.markedVerts.push_back(MarkedVert{vv});
        saveTri(fbase + toStrDigits(cnt++,2) + ".tri",mm);
    }
}

void
splitObjByMtl(CLArgs const & args)
{
    Syntax    syn(args,
        "<mesh>.[w]obj <base>\n"
        "    Creates a <base>_<name>.tri file for each 'usemtl' name referenced");
    Mesh    mesh = loadWObj(syn.next(),"usemtl");
    string      base = syn.next();
    Mesh    m = mesh;
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        m.surfaces = svec(mesh.surfaces[ii]);
        saveTri(base+"_"+mesh.surfaces[ii].name+".tri",m);
    }
}

void
splitsurfsbyuvs(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh    mesh = loadMesh(syn.next());
    mesh = splitSurfsByUvs(mesh);
    saveMesh(mesh,syn.next());
}

void
splitCont(CLArgs const & args)
{
    Syntax          syn(args,
        "<in>.<extIn>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "COMMENTS:\n"
        "    * Splits surfaces by connected vertex indices.\n"
        "    * Stores results to separate meshes with suffix '_<num>.tri'"
        );
    Mesh            mesh = loadMesh(syn.next());
    Ustring         base = pathToBase(syn.curr());
    uint            idx = 0;
    Mesh            out = mesh;
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        Surfs           surfs = splitByContiguous(mesh.surfaces[ss]);
        for (size_t ii=0; ii<surfs.size(); ++ii) {
            out.surfaces = svec(surfs[ii]);
            saveTri(base+"_"+toStr(idx++)+".tri",out);
        }
    }
}

void
surfAdd(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<ext> <name> <out>.fgmesh\n"
        "    <ext>  - " + meshLoadFormatsCLDescription() + "\n"
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
        "    <ext>  - " + meshLoadFormatsCLDescription() + "\n"
        " * tris only, uvs not preserved."
        );
    Mesh        from = loadFgmesh(syn.next()),
                    to = loadMesh(syn.next());
    saveFgmesh(syn.next(),copySurfaceStructure(from,to));
}

void
surfDel(CLArgs const & args)
{
    Syntax        syn(args,
        "<in>.<ext> <idx> <out>.<ext>\n"
        "    <ext> - " + meshLoadFormatsCLDescription() + "\n"
        "    <idx> - Which surface index to delete"
        );
    Mesh        mesh = loadMesh(syn.next());
    size_t          idx = syn.nextAs<uint>();
    if (idx >= mesh.surfaces.size())
        syn.error("Selected surface index out of range",toStr(idx));
    mesh.surfaces.erase(mesh.surfaces.begin()+idx);
    saveMesh(mesh,syn.next());
}

void
surfList(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<ext>\n"
        "    <ext> - " + meshLoadFormatsCLDescription()
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
    Syntax    syn(args,
        "<in>.fgmesh <idx> <name>\n"
        "   <idx>  - Which surface\n"
        "   <name> - Surface name"
        );
    Ustring        meshFname = syn.next();
    Mesh        mesh = loadFgmesh(meshFname);
    size_t          idx = syn.nextAs<size_t>();
    if (idx >= mesh.surfaces.size())
        fgThrow("Index value larger than available surfaces");
    mesh.surfaces[idx].name = syn.next();
    saveFgmesh(meshFname,mesh);
}

void
spCopy(CLArgs const & args)
{
    Syntax          syn(args,"<from>.<mi> <to>.<mi> <out>.<mo>\n"
        "    <mi>   - " + meshLoadFormatsCLDescription() + "\n"
        "    <mo>   - " + meshSaveFormatsCLDescription()
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
    Ustring        meshFname = syn.next();
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
    Ustring        meshFname = syn.next();
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
surf(CLArgs const & args)
{
    Cmds   ops;
    ops.push_back(Cmd(surfAdd,"add","Add an empty surface to a mesh"));
    ops.push_back(Cmd(surfCopy,"copy","Copy surface structure between aligned meshes"));
    ops.push_back(Cmd(surfDel,"del","Delete a surface in a mesh (leaves verts unchanged)"));
    ops.push_back(Cmd(surfList,"list","List surfaces in mesh"));
    ops.push_back(Cmd(mergenamedsurfs,"mergeNamed","Merge surfaces with identical names"));
    ops.push_back(Cmd(mergesurfs,"merge","Merge all surfaces in a mesh into one"));
    ops.push_back(Cmd(surfRen,"ren","Rename a surface in a mesh"));
    ops.push_back(Cmd(spCopy,"spCopy","Copy surf points between meshes with identical surface topology"));
    ops.push_back(Cmd(spDel,"spDel","Delete a surface point"));
    ops.push_back(Cmd(spList,"spList","List surface points in each surface"));
    ops.push_back(Cmd(spRen,"spRen","Rename a surface point"));
    ops.push_back(Cmd(spsToVerts,"spVert","Convert surface points to marked vertices"));
    doMenu(args,ops);
}

void
toTris(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh    mesh = loadMesh(syn.next());
    mesh.convertToTris();
    saveMesh(mesh,syn.next());
}

void
unifyuvs(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh    mesh = loadMesh(syn.next());
    mesh = unifyIdenticalUvs(mesh);
    saveMesh(mesh,syn.next());
}

void
unifyverts(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh    mesh = loadMesh(syn.next());
    mesh = unifyIdenticalVerts(mesh);
    saveMesh(mesh,syn.next());
}

void
uvclamp(CLArgs const & args)
{
    Syntax    syn(args,
        "<in>.<ext0> [<out>.<ext1>]\n"
        "    <ext0> = " + meshLoadFormatsCLDescription() + "\n"
        "    <ext1> = " + meshSaveFormatsCLDescription()
        );
    Mesh        in = loadMesh(syn.next());
    Mat22F        cb(0,1,0,1);
    for (size_t ii=0; ii<in.uvs.size(); ++ii)
        in.uvs[ii] = clampBounds(in.uvs[ii],cb);
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
        "    <extM>     - " + meshLoadFormatsCLDescription() + "\n"
        "    <size>     - Output image size (will be square).\n"
        "    <image>    - Output image.\n"
        "    <extI>     - " + imgFileExtensionsDescription()
        );
    Mesh        mesh = loadMesh(syn.next());
    uint            sz = syn.nextAs<uint>();
    if (sz > (1 << 12))
        syn.error("<size> is too large",syn.curr());
    ImgUC         img = getUvCover(mesh,Vec2UI(sz*4));
    img = fgShrink2(img);
    img = fgShrink2(img);
    saveImage(syn.next(),img);
}

void
uvWireframeImage(CLArgs const & args)
{
    Syntax    syn(args,
        "<mesh>.<extM> <image>.<extI>\n"
        "    <mesh>     - The mesh must contains UVs for anything to be done.\n"
        "    <extM>     - " + meshLoadFormatsCLDescription() + "\n"
        "    <image>    - If this file already exists it will be used as the background.\n"
        "    <extI>     - " + imgFileExtensionsDescription()
        );
    Mesh        mesh = loadMesh(syn.next());
    ImgC4UC     img;
    if (pathExists(syn.peekNext()))
        loadImage_(syn.peekNext(),img);
    saveImage(syn.next(),cUvWireframeImage(mesh,RgbaUC{0,255,0,255},img));
}

void
uvmask(CLArgs const & args)
{
    Syntax    syn(args,
        "<meshIn>.<ext0> <imageIn>.<ext1> <meshOut>.<ext2>\n"
        "    <ext0> = " + meshLoadFormatsCLDescription() + "\n"
        "    <ext1> = " + imgFileExtensionsDescription() + "\n"
        "    <ext2> = " + meshSaveFormatsCLDescription()
        );
    Mesh        mesh = loadMesh(syn.next());
    ImgC4UC     img = loadImage(syn.next());
    Img<FgBool> mask = Img<FgBool>(img.dims());
    for (Iter2UI it(img.dims()); it.valid(); it.next()) {
        Vec4UC   px = img[it()].m_c;
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
        "    <ext0> = " + meshLoadFormatsCLDescription() + "\n"
        "    <ext1> = " + meshSaveFormatsCLDescription()
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
    Syntax    syn(args,
        "<similarity>.xml <in>.<ext0> <out>.<ext1>\n"
        "    <ext0> = " + meshLoadFormatsCLDescription() + "\n"
        "    <ext1> = " + meshSaveFormatsCLDescription()
        );
    SimilarityD     xform;
    loadBsaXml(syn.next(),xform);
    Mesh            in = loadMesh(syn.next());
    Mesh            out(in);
    out.transform(Affine3F(xform.asAffine()));
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
        "    <ex> = " + meshLoadFormatsCLDescription()
        );
    string      simFname = syn.next();
    Mesh    base = loadMesh(syn.next());
    Mesh    targ = loadMesh(syn.next());
    if (base.verts.size() != targ.verts.size())
        fgThrow("Base and target mesh vertex counts are different");
    Vec3Ds    bv = scast<double>(base.verts),
                        tv = scast<double>(targ.verts);
    SimilarityD         sim = similarityApprox(bv,tv);
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
    Cmds        cmds {
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
        "    <ext1>     - " + meshLoadFormatsCLDescription() + "\n"
        "    <ext2>     - " + meshSaveFormatsCLDescription()
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
    Mesh        mesh = loadMesh(syn.next());
    Mat33F        xf = Mat33F::identity();
    xf.rc(axis,axis) = -1.0f;
    mesh.transform(xf);
    Ustring        fname = (syn.more() ? syn.next() : syn.curr());
    saveMesh(mesh,fname);
}

void
xform(CLArgs const & args)
{
    Cmds      cmds;
    cmds.push_back(Cmd(xformApply,"apply","Apply a simiarlity transform (from XML file) to a mesh"));
    cmds.push_back(Cmd(xformCreate,"create","Create a similarity transform XML file"));
    cmds.push_back(Cmd(xformMirror,"mirror","Mirror a mesh"));
    doMenu(args,cmds);
}

void
meshops(CLArgs const & args)
{
    Cmds        ops {
        {combinesurfs,"combinesurfs","Combine surfaces from meshes with identical vertex lists"},
        {convert,"convert","Convert the mesh between different formats"},
        {copyUvList,"copyUvList","Copy UV list from one mesh to another with same UV count"},
        {copyUvs,"copyUvs","Copy UVs from one mesh to another with identical facet structure"},
        {copyverts,"copyverts","Copy verts from one mesh to another with same vertex count"},
        {emboss,"emboss","Emboss a mesh based on greyscale values of a UV image"},
        {invWind,"invWind","Invert facet winding of a mesh"},
        {markVerts,"markVerts","Mark vertices in a .TRI file from a given list"},
        {mmerge,"merge","Merge multiple meshes into one. No optimization is done"},
        {rdf,"rdf","Remove Duplicate Facets within each surface"},
        {retopo,"retopo","Rebase a mesh topology with an exactly aligned mesh"},
        {rt,"rt","Remove specific tris from a mesh"},
        {ruv,"ruv","Remove vertices and uvs not referenced by a surface or marked vertex"},
        {seams,"seams","Extract each seam of a mesh as a file with seam verts marked"},
        {sortFacets,"sortFacets","Sort facets for optimal transparency viewing"},
        {splitObjByMtl,"splitObjByMtl","Split up an OBJ mesh by 'usemtl' name"},
        {splitCont,"splitCont","Split up surface by contiguous vertex indices"},
        {splitsurfsbyuvs,"splitSurfsByUvs","Split up surfaces with discontiguous UV mappings"},
        {surf,"surf","Operations on mesh surface structure"},
        {toTris,"toTris","Convert all facets to tris"},
        {unifyuvs,"unifyUVs","Unify identical UV coordinates"},
        {unifyverts,"unifyVerts","Unify identical vertices"},
        {uvclamp,"uvclamp","Clamp UV coords to the range [0,1]"},
        {uvWireframeImage,"uvImgW","Wireframe image of mesh UV map"},
        {uvSolidImage,"uvImgS","Solid white inside UV facets, black outside, 4xFSAA"},
        {uvmask,"uvmask","Mask out geometry for any black areas of a texture image {auto symmetrized)"},
        {uvunwrap,"uvunwrap","Unwrap wrap-around UV coords to the range [0,1]"},
        {xform,"xform","Create or apply similarity transforms from/to meshes"},
    };
    doMenu(args,ops);
}

}

Cmd
getMeshopsCmd()
{return Cmd(meshops,"mesh","3D Mesh tools"); }

}

// */
