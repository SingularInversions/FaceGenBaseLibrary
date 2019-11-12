//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
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
combinesurfs(const CLArgs & args)
{
    Syntax    syntax(args,
        "(<mesh>.<extIn>)+ <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription() + "\n"
        "    All input meshes must have identical vertex lists.\n"
        );
    Mesh    mesh = meshLoadAnyFormat(syntax.next());
    while (syntax.more()) {
        string  name = syntax.next();
        if (syntax.more()) {
            Mesh    next = meshLoadAnyFormat(name);
            cat_(mesh.surfaces,next.surfaces);
        }
        else
            meshSaveAnyFormat(mesh,name);
    }
}

void
convert(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh    mesh = meshLoadAnyFormat(syntax.next());
    meshSaveAnyFormat(mesh,syntax.next());
}

void
copyUvList(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<ext0> <out>.<ext1>\n"
        "    <ext0> = " + meshLoadFormatsCLDescription() + "\n"
        "    <ext1> = " + meshSaveFormatsCLDescription()
        );
    Mesh        in = meshLoadAnyFormat(syntax.next());
    Mesh        out = meshLoadAnyFormat(syntax.next());
    if (in.uvs.size() != out.uvs.size())
        syntax.error("Incompatible UV list sizes");
    out.uvs = in.uvs;
    meshSaveAnyFormat(out,syntax.curr());
    return;
}

void
copyUvs(const CLArgs & args)
{
    Syntax    syntax(args,
        "<from>.<ext0> <to>.<ext1>\n"
        "    <ext0> = " + meshLoadFormatsCLDescription() + "\n"
        "    <ext1> = " + meshSaveFormatsCLDescription()
        );
    Mesh        in = meshLoadAnyFormat(syntax.next());
    Mesh        out = meshLoadAnyFormat(syntax.next());
    out.uvs = in.uvs;
    if (in.surfaces.size() != out.surfaces.size())
        fgThrow("Incompatible number of surfaces");
    for (size_t ss=0; ss<in.surfaces.size(); ++ss) {
        const Surf &     sin = in.surfaces[ss];
        Surf &           sout = out.surfaces[ss];
        if ((sin.tris.size() != sout.tris.size()) ||
            (sin.quads.size() != sout.quads.size()))
            fgThrow("Incompatible facet counts");
        sout.tris.uvInds = sin.tris.uvInds;
        sout.quads.uvInds = sin.quads.uvInds;
    }
    meshSaveAnyFormat(out,syntax.curr());
    return;
}

void
copyverts(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<ext0> <out>.<ext1>\n"
        "    <ext0> = " + meshLoadFormatsCLDescription() + "\n"
        "    <ext1> = " + meshSaveFormatsCLDescription() + "\n"
        "    <out> is modified to have the vertex list from <in>"
        );
    Mesh        in = meshLoadAnyFormat(syntax.next());
    Mesh        out = meshLoadAnyFormat(syntax.next());
    if (in.verts.size() != out.verts.size())
        syntax.error("Incompatible vertex list sizes");
    out.verts = in.verts;
    meshSaveAnyFormat(out,syntax.curr());
    return;
}

void
emboss(const CLArgs & args)
{
    Syntax    syntax(args,
        "<uvImage>.<img> <meshin>.<ext0> <val> <out>.<ext1>\n"
        "    <uvImage> = a UV-layout image whose grescale values will be used to emboss (0 - none, 255 - full)\n"
        "    <img>     = " + imgFileExtensionsDescription() + "\n"
        "    <ext0>    = " + meshLoadFormatsCLDescription() + "\n"
        "    <val>     = Embossing factor as ratio of the max bounding box dimension.\n"
        "    <ext1>    = " + meshSaveFormatsCLDescription()
        );
    ImgUC         img;
    imgLoadAnyFormat(syntax.next(),img);      // Treat as greyscale
    Mesh        mesh = meshLoadAnyFormat(syntax.next());
    if (mesh.uvs.empty())
        fgThrow("Mesh has no UVs",syntax.curr());
    if (mesh.surfaces.size() != 1)
        fgThrow("Only 1 surface currently supported",syntax.curr());
    float           val = syntax.nextAs<float>();
    if (!(val > 0.0f))
        fgThrow("Emboss value must be > 0",toString(val));
    mesh.verts = fgEmboss(mesh,img,val);
    meshSaveAnyFormat(mesh,syntax.next());
}

void
invWind(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription() + "\n"
        "    Inverts the winding of all facets in <in> and saves to <out>"
        );
    Mesh    mesh = meshLoadAnyFormat(syntax.next());
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        Surf &   surf = mesh.surfaces[ss];
        for (size_t ii=0; ii<surf.tris.size(); ++ii)
            std::swap(surf.tris.vertInds[ii][1],surf.tris.vertInds[ii][2]);
        for (size_t ii=0; ii<surf.tris.uvInds.size(); ++ii)
            std::swap(surf.tris.uvInds[ii][1],surf.tris.uvInds[ii][2]);
        for (size_t ii=0; ii<surf.quads.size(); ++ii)
            std::swap(surf.quads.vertInds[ii][1],surf.quads.vertInds[ii][3]);
        for (size_t ii=0; ii<surf.quads.uvInds.size(); ++ii)
            std::swap(surf.quads.uvInds[ii][1],surf.quads.uvInds[ii][3]);
    }
    meshSaveAnyFormat(mesh,syntax.next());
}

void
markVerts(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.tri <verts>.<ext> <out>.tri\n"
        "    <ext> = " + meshLoadFormatsCLDescription() + "\n"
        "    <out>.tri will be saved after marking a vertex in <in>.tri that is closest to each vertex in <verts>.<ext>."
        );
    Mesh    mesh = loadTri(syntax.next());
    Vec3Fs     verts = meshLoadAnyFormat(syntax.next()).verts;
    float       dim = fgMaxElem(cDims(mesh.verts));
    uint        poorMatches = 0,
                totalMatches = 0;
    for (size_t vv=0; vv<verts.size(); ++vv) {
        Vec3F        v = verts[vv];
        float           bestMag = numeric_limits<float>::max();
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
        if (!fgContains(mesh.markedVerts,bestIdx)) {
            mesh.markedVerts.push_back(MarkedVert(bestIdx));
            ++totalMatches;
        }
    }
    if (poorMatches > 0)
        fgout << fgnl << "WARNING: " << poorMatches << " poor matches.";
    if (totalMatches < verts.size())
        fgout << fgnl << "WARNING: duplicate matches.";
    fgout << fgnl << totalMatches << " vertices marked.";
    saveTri(syntax.next(),mesh);
}

void
mmerge(const CLArgs & args)
{
    Syntax    syntax(args,
        "(<mesh>.<extIn>)+ <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh    mesh = meshLoadAnyFormat(syntax.next());
    while (syntax.more()) {
        string  name = syntax.next();
        if (syntax.more())
            mesh = fgMergeMeshes(mesh,meshLoadAnyFormat(name));
        else
            meshSaveAnyFormat(mesh,name);
    }
}

void
mergesurfs(const CLArgs & args)
{
    Syntax      syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh        mesh = meshLoadAnyFormat(syntax.next());
    if (mesh.surfaces.size() < 2)
        fgout << "WARNING: No extra surfaces to merge.";
    else
        mesh.surfaces = {mergeSurfaces(mesh.surfaces)};
    meshSaveAnyFormat(mesh,syntax.next());
}

void
rdf(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.tri [<out>.tri]\n"
        "    <in>.tri       - Will be overwritten if <out>.tri is not specified\n"
        "NOTES:\n"
        "    Duplicates are determined by vertex index. To remove duplicates by vertex\n"
        "    value, first remove duplicate vertices."
        );
    Ustring        fni(syntax.next()),
                    fno = fni;
    if (syntax.more())
        fno = syntax.next();
    saveTri(fno,fgRemoveDuplicateFacets(loadTri(fni)));
}

void
rt(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<extIn> <out>.<extOut> (<surfIndex> <triEquivIndex>)+\n"
        "    <extIn>    - " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut>   - " + meshSaveFormatsCLDescription() + "\n"
        "    <triEquivIndex> - Tri-equivalent index of triangle or half-quad to remove."
        );
    Mesh        mesh = meshLoadAnyFormat(syntax.next());
    Ustring        outName = syntax.next();
    while (syntax.more()) {
        size_t          si = syntax.nextAs<size_t>(),
                        ti = syntax.nextAs<size_t>();
        if (si >= mesh.surfaces.size())
            fgThrow("Surface index out of bounds",toString(si),toString(mesh.surfaces.size()));
        Surf &   surf = mesh.surfaces[si];
        if (ti >= surf.numTriEquivs())
            fgThrow("Tri equiv index out of bounds",toString(ti),toString(surf.numTriEquivs()));
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
    meshSaveAnyFormat(mesh,outName);
}

void
ruv(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh    mesh = meshLoadAnyFormat(syntax.next());
    mesh = meshRemoveUnusedVerts(mesh);
    meshSaveAnyFormat(mesh,syntax.next());
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
    Mesh        mesh = meshLoadAnyFormat(syn.next());
    ImgC4UC     albedo = imgLoadAnyFormat(syn.next());
    Ustring        outName = syn.next();
    Mesh        opaque;
    while (syn.more())
        opaque = fgMergeMeshes(opaque,meshLoadAnyFormat(syn.next()));
    mesh = fgSortTransparentFaces(mesh,albedo,opaque);
    meshSaveAnyFormat(mesh,outName);
}

void
mergenamedsurfs(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh    mesh = meshLoadAnyFormat(syntax.next());
    mesh = fgMergeSameNameSurfaces(mesh);
    meshSaveAnyFormat(mesh,syntax.next());
}

void
splitObjByMtl(const CLArgs & args)
{
    Syntax    syntax(args,
        "<mesh>.[w]obj <base>\n"
        "    Creates a <base>_<name>.tri file for each 'usemtl' name referenced");
    Mesh    mesh = loadWobj(syntax.next(),"usemtl");
    string      base = syntax.next();
    Mesh    m = mesh;
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        m.surfaces = fgSvec(mesh.surfaces[ii]);
        saveTri(base+"_"+mesh.surfaces[ii].name+".tri",m);
    }
}

void
splitsurfsbyuvs(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh    mesh = meshLoadAnyFormat(syntax.next());
    mesh = fgSplitSurfsByUvs(mesh);
    meshSaveAnyFormat(mesh,syntax.next());
}

void
splitsurface(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<extIn>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "COMMENTS:\n"
        "    * Splits surfaces by connected vertex indices.\n"
        "    * Stores results to separate meshes with suffix '_<num>.tri'"
        );
    Mesh                mesh = meshLoadAnyFormat(syntax.next());
    Ustring                base = fgPathToBase(syntax.curr());
    uint                    idx = 0;
    Mesh                out = mesh;
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        vector<Surf>     surfs = fgSplitSurface(mesh.surfaces[ss]);
        for (size_t ii=0; ii<surfs.size(); ++ii) {
            out.surfaces = fgSvec(surfs[ii]);
            saveTri(base+"_"+toString(idx++)+".tri",out);
        }
    }
}

void
surfAdd(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<ext> <name> <out>.fgmesh\n"
        "    <ext>  - " + meshLoadFormatsCLDescription() + "\n"
        "    <name> - Surface name"
        );
    Mesh        mesh = meshLoadAnyFormat(syntax.next());
    Surf     surf;
    surf.name = syntax.next();
    mesh.surfaces.push_back(surf);
    saveFgmesh(syntax.next(),mesh);
}

void
surfCopy(const CLArgs & args)
{
    Syntax    syntax(args,
        "<from>.fgmesh <to>.<ext> <out>.fgmesh\n"
        "    <ext>  - " + meshLoadFormatsCLDescription() + "\n"
        " * tris only, uvs not preserved."
        );
    Mesh        from = loadFgmesh(syntax.next()),
                    to = meshLoadAnyFormat(syntax.next());
    saveFgmesh(syntax.next(),fgCopySurfaceStructure(from,to));
}

void
surfDel(const CLArgs & args)
{
    Syntax        syntax(args,
        "<in>.<ext> <idx> <out>.<ext>\n"
        "    <ext> - " + meshLoadFormatsCLDescription() + "\n"
        "    <idx> - Which surface index to delete"
        );
    Mesh        mesh = meshLoadAnyFormat(syntax.next());
    size_t          idx = syntax.nextAs<uint>();
    if (idx >= mesh.surfaces.size())
        syntax.error("Selected surface index out of range",toString(idx));
    mesh.surfaces.erase(mesh.surfaces.begin()+idx);
    meshSaveAnyFormat(mesh,syntax.next());
}

void
surfList(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<ext>\n"
        "    <ext> - " + meshLoadFormatsCLDescription()
        );
    Mesh    mesh = meshLoadAnyFormat(syntax.next());
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        const Surf & surf = mesh.surfaces[ss];
        fgout << fgnl << ss << ": " << surf.name;
    }
}

void
surfRen(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.fgmesh <idx> <name>\n"
        "   <idx>  - Which surface\n"
        "   <name> - Surface name"
        );
    Ustring        meshFname = syntax.next();
    Mesh        mesh = loadFgmesh(meshFname);
    size_t          idx = syntax.nextAs<size_t>();
    if (idx >= mesh.surfaces.size())
        fgThrow("Index value larger than available surfaces");
    mesh.surfaces[idx].name = syntax.next();
    saveFgmesh(meshFname,mesh);
}

void
spCopy(const CLArgs & args)
{
    Syntax    syntax(args,"<from>.fgmesh <to>.fgmesh <out>.fgmesh");
    Mesh    from = loadFgmesh(syntax.next()),
                to = loadFgmesh(syntax.next());
    if (from.surfaces.size() != to.surfaces.size())
        fgThrow("'from' and 'to' meshes have different surface counts");
    for (size_t ss=0; ss<to.surfaces.size(); ++ss)
        cat_(to.surfaces[ss].surfPoints,from.surfaces[ss].surfPoints);
    saveFgmesh(syntax.next(),to);
}

void
spDel(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.tri <ptIdx>\n"
        "   <spIdx>   - Which point on that surface to delete"
        );
    Ustring        meshFname = syntax.next();
    Mesh        mesh = loadTri(meshFname);
    size_t          ii = syntax.nextAs<size_t>();
    Surf &   surf = mesh.surfaces[0];
    if (ii >= surf.surfPoints.size())
        fgThrow("Point index value larger than availables points");
    surf.surfPoints.erase(surf.surfPoints.begin() + ii);
    saveTri(meshFname,mesh);
}

void
spList(const CLArgs & args)
{
    Syntax    syntax(args,"<in>.fgmesh");
    Mesh    mesh = meshLoadAnyFormat(syntax.next());
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        const Surf & surf = mesh.surfaces[ss];
        fgout << fgnl << "Surface " << ss << ": " << surf.name << fgpush;
        for (size_t ii=0; ii<surf.surfPoints.size(); ++ii)
            fgout << fgnl << ii << " : " << surf.surfPoints[ii].label;
        fgout << fgpop;
    }
}

void
spRen(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.fgmesh <surfIdx> <ptIdx> <name>\n"
        "   <surfIdx> - Which surface\n"
        "   <spIdx>   - Which point on that surface\n"
        "   <name>    - Name"
        );
    Ustring        meshFname = syntax.next();
    Mesh        mesh = loadFgmesh(meshFname);
    size_t          ss = syntax.nextAs<size_t>(),
                    ii = syntax.nextAs<size_t>();
    if (ss >= mesh.surfaces.size())
        fgThrow("Surface index value larger than available surfaces");
    Surf &   surf = mesh.surfaces[ss];
    if (ii >= surf.surfPoints.size())
        fgThrow("Point index value larger than availables points");
    surf.surfPoints[ii].label = syntax.next();
    saveFgmesh(meshFname,mesh);
}

void
spsToVerts(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.tri <out>.tri\n"
        "    <out>.tri will be appended with the new marked vertices."
        );
    Mesh    in = loadTri(syntax.next());
    Mesh    out = loadTri(syntax.next());
    surfPointsToMarkedVerts_(in,out);
    saveTri(syntax.curr(),out);
}

void
surf(const CLArgs & args)
{
    vector<Cmd>   ops;
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
toTris(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh    mesh = meshLoadAnyFormat(syntax.next());
    mesh.convertToTris();
    meshSaveAnyFormat(mesh,syntax.next());
}

void
unifyuvs(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh    mesh = meshLoadAnyFormat(syntax.next());
    mesh = fgUnifyIdenticalUvs(mesh);
    meshSaveAnyFormat(mesh,syntax.next());
}

void
unifyverts(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + meshLoadFormatsCLDescription() + "\n"
        "    <extOut> = " + meshSaveFormatsCLDescription()
        );
    Mesh    mesh = meshLoadAnyFormat(syntax.next());
    mesh = fgUnifyIdenticalVerts(mesh);
    meshSaveAnyFormat(mesh,syntax.next());
}

void
uvclamp(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<ext0> [<out>.<ext1>]\n"
        "    <ext0> = " + meshLoadFormatsCLDescription() + "\n"
        "    <ext1> = " + meshSaveFormatsCLDescription()
        );
    Mesh        in = meshLoadAnyFormat(syntax.next());
    Mat22F        cb(0,1,0,1);
    for (size_t ii=0; ii<in.uvs.size(); ++ii)
        in.uvs[ii] = clampBounds(in.uvs[ii],cb);
    if (syntax.more())
        meshSaveAnyFormat(in,syntax.next());
    else
        meshSaveAnyFormat(in,syntax.curr());
    return;
}

void
uvSolidImage(const CLArgs & args)
{
    Syntax    syn(args,
        "<mesh>.<extM> <size> <image>.<extI>\n"
        "    <mesh>     - The mesh must contains UVs for anything to be done.\n"
        "    <extM>     - " + meshLoadFormatsCLDescription() + "\n"
        "    <size>     - Output image size (will be square).\n"
        "    <image>    - Output image.\n"
        "    <extI>     - " + imgFileExtensionsDescription()
        );
    Mesh        mesh = meshLoadAnyFormat(syn.next());
    uint            sz = syn.nextAs<uint>();
    if (sz > (1 << 12))
        syn.error("<size> is too large",syn.curr());
    ImgUC         img = fgGetUvCover(mesh,Vec2UI(sz*4));
    img = fgShrink2(img);
    img = fgShrink2(img);
    imgSaveAnyFormat(syn.next(),img);
}

void
uvWireframeImage(const CLArgs & args)
{
    Syntax    syn(args,
        "<mesh>.<extM> <image>.<extI>\n"
        "    <mesh>     - The mesh must contains UVs for anything to be done.\n"
        "    <extM>     - " + meshLoadFormatsCLDescription() + "\n"
        "    <image>    - If this file already exists it will be used as the background.\n"
        "    <extI>     - " + imgFileExtensionsDescription()
        );
    Mesh        mesh = meshLoadAnyFormat(syn.next());
    ImgC4UC     img;
    if (pathExists(syn.peekNext()))
        imgLoadAnyFormat(syn.peekNext(),img);
    imgSaveAnyFormat(syn.next(),fgUvWireframeImage(mesh,img));
}

void
uvmask(const CLArgs & args)
{
    Syntax    syntax(args,
        "<meshIn>.<ext0> <imageIn>.<ext1> <meshOut>.<ext2>\n"
        "    <ext0> = " + meshLoadFormatsCLDescription() + "\n"
        "    <ext1> = " + imgFileExtensionsDescription() + "\n"
        "    <ext2> = " + meshSaveFormatsCLDescription()
        );
    Mesh        mesh = meshLoadAnyFormat(syntax.next());
    ImgC4UC     img;
    imgLoadAnyFormat(syntax.next(),img);
    Img<FgBool> mask = Img<FgBool>(img.dims());
    for (Iter2UI it(img.dims()); it.valid(); it.next()) {
        Vec4UC   px = img[it()].m_c;
        mask[it()] = (px[0] > 0) || (px[1] > 0) || (px[2] > 0); }
    mask = fgAnd(mask,fgFlipHoriz(mask));
    mesh = fg3dMaskFromUvs(mesh,mask);
    meshSaveAnyFormat(mesh,syntax.next());
}

void
uvunwrap(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<ext0> [<out>.<ext1>]\n"
        "    <ext0> = " + meshLoadFormatsCLDescription() + "\n"
        "    <ext1> = " + meshSaveFormatsCLDescription()
        );
    Mesh        in = meshLoadAnyFormat(syntax.next());
    for (size_t ii=0; ii<in.uvs.size(); ++ii) {
        Vec2F    uv = in.uvs[ii];
        in.uvs[ii][0] = cMod(uv[0],1.0f);
        in.uvs[ii][1] = cMod(uv[1],1.0f);
    }
    if (syntax.more())
        meshSaveAnyFormat(in,syntax.next());
    else
        meshSaveAnyFormat(in,syntax.curr());
    return;
}

void
xformApply(const CLArgs & args)
{
    Syntax    syntax(args,
        "<similarity>.xml <in>.<ext0> <out>.<ext1>\n"
        "    <ext0> = " + meshLoadFormatsCLDescription() + "\n"
        "    <ext1> = " + meshSaveFormatsCLDescription()
        );
    SimilarityD    xform;
    fgLoadXml(syntax.next(),xform);
    Mesh        in = meshLoadAnyFormat(syntax.next());
    Mesh        out(in);
    out.transform(Affine3F(xform.asAffine()));
    meshSaveAnyFormat(out,syntax.next());
    return;
}

void
xformCreateMeshes(const CLArgs & args)
{
    Syntax    syntax(args,
        "<similarity>.xml <base>.<ex> <transformed>.<ex>\n"
        "    <ex> = " + meshLoadFormatsCLDescription()
        );
    string      simFname = syntax.next();
    Mesh    base = meshLoadAnyFormat(syntax.next());
    Mesh    targ = meshLoadAnyFormat(syntax.next());
    if (base.verts.size() != targ.verts.size())
        fgThrow("Base and target mesh vertex counts are different");
    vector<Vec3D>    bv = scast<double>(base.verts),
                        tv = scast<double>(targ.verts);
    SimilarityD        sim = similarityApprox(bv,tv);
    double              ssd = cSsd(mapXft(bv,sim.asAffine()),tv),
                        sz = fgMaxElem(cDims(tv));
    fgout << fgnl << "Transformed base to target relative RMS delta: " << sqrt(ssd / tv.size()) / sz;
    fgSaveXml(simFname,sim);
}

void
xformCreateIdentity(const CLArgs & args)
{
    Syntax    syntax(args,
        "<similarity>.xml\n"
        "    Edit the values in this file or apply subsequent transforms with other commands"
        );
    string      simFname = syntax.next();
    fgSaveXml(simFname,SimilarityD::identity());
}

void
xformCreateTrans(const CLArgs & args)
{
    Syntax    syntax(args,
        "<similarity>.xml <X> <Y> <Z>"
        );
    string          simFname = syntax.next();
    SimilarityD    sim;
    if (pathExists(simFname))
        fgLoadXml(simFname,sim);
    Vec3D    trans;
    trans[0] = syntax.nextAs<double>();
    trans[1] = syntax.nextAs<double>();
    trans[2] = syntax.nextAs<double>();
    fgSaveXml(simFname,SimilarityD(trans)*sim);
}

void
xformCreateRotate(const CLArgs & args)
{
    Syntax    syntax(args,
        "<similarity>.xml <axis> <degrees>\n"
        "    <axis> = (x | y | z)   - Right-hand-rule axis of rotation"
        );
    string          simFname = syntax.next();
    SimilarityD    sim;
    if (pathExists(simFname))
        fgLoadXml(simFname,sim);
    string          axisStr = syntax.next();
    if (axisStr.empty())
        syntax.error("<axis> cannot be the empty string");
    char            axisName = std::tolower(axisStr[0]);
    double          degs = syntax.nextAs<double>(),
                    rads = fgDegToRad(degs);
    int             axisNum = int(axisName) - int('x');
    if ((axisNum < 0) || (axisNum > 2))
        syntax.error("Invalid value for <axis>",axisStr);
    QuaternionD   rot(rads,uint(axisNum));
    fgSaveXml(simFname,SimilarityD(rot)*sim);
}

void
xformCreateScale(const CLArgs & args)
{
    Syntax    syntax(args,
        "<similarity>.xml <scale>"
        );
    string          simFname = syntax.next();
    SimilarityD    sim;
    if (pathExists(simFname))
        fgLoadXml(simFname,sim);
    double          scale = syntax.nextAs<double>();
    fgSaveXml(simFname,SimilarityD(scale)*sim);
}

void
xformCreate(const CLArgs & args)
{
    doMenu(args,
        fgSvec(
            Cmd(xformCreateMeshes,"meshes","Create similarity transform from base and transformed meshes with matching vertex lists"),
            Cmd(xformCreateIdentity,"identity","Create the identity similarity transform"),
            Cmd(xformCreateTrans,"translate","Apply a translation to a similarity transform"),
            Cmd(xformCreateRotate,"rotate","Apply a rotation to a similarity transform"),
            Cmd(xformCreateScale,"scale","Apply a scaling to a similarity transform")
            ));
}

void
xformMirror(const CLArgs & args)
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
    Mesh        mesh = meshLoadAnyFormat(syn.next());
    Mat33F        xf = Mat33F::identity();
    xf.rc(axis,axis) = -1.0f;
    mesh.transform(xf);
    Ustring        fname = (syn.more() ? syn.next() : syn.curr());
    meshSaveAnyFormat(mesh,fname);
}

void
xform(const CLArgs & args)
{
    Cmds      cmds;
    cmds.push_back(Cmd(xformApply,"apply","Apply a simiarlity transform (from .XML file) to a mesh"));
    cmds.push_back(Cmd(xformCreate,"create","Create a similarity transform (to .XML file)"));
    cmds.push_back(Cmd(xformMirror,"mirror","Mirror a mesh"));
    doMenu(args,cmds);
}

void
meshops(const CLArgs & args)
{
    vector<Cmd>   ops;
    ops.push_back(Cmd(combinesurfs,"combinesurfs","Combine surfaces from meshes with identical vertex lists"));
    ops.push_back(Cmd(convert,"convert","Convert the mesh between different formats"));
    ops.push_back(Cmd(copyUvList,"copyUvList","Copy UV list from one mesh to another with same UV count"));
    ops.push_back(Cmd(copyUvs,"copyUvs","Copy UVs from one mesh to another with identical facet structure"));
    ops.push_back(Cmd(copyverts,"copyverts","Copy verts from one mesh to another with same vertex count"));
    ops.push_back(Cmd(emboss,"emboss","Emboss a mesh based on greyscale values of a UV image"));
    ops.push_back(Cmd(invWind,"invWind","Invert facet winding of a mesh"));
    ops.push_back(Cmd(markVerts,"markVerts","Mark vertices in a .TRI file from a given list"));
    ops.push_back(Cmd(mmerge,"merge","Merge multiple meshes into one. No optimization is done"));
    ops.push_back(Cmd(rdf,"rdf","Remove Duplicate Facets within each surface"));
    ops.push_back(Cmd(rt,"rt","Remove specific tris from a mesh"));
    ops.push_back(Cmd(ruv,"ruv","Remove vertices and uvs not referenced by a surface or marked vertex"));
    ops.push_back(Cmd(sortFacets,"sortFacets","Sort facets for optimal transparency viewing"));
    ops.push_back(Cmd(splitObjByMtl,"splitObjByMtl","Split up an OBJ mesh by 'usemtl' name"));
    ops.push_back(Cmd(splitsurface,"splitSurface","Split up surface by connected vertex indices"));
    ops.push_back(Cmd(splitsurfsbyuvs,"splitSurfsByUvs","Split up surfaces with discontiguous UV mappings"));
    ops.push_back(Cmd(surf,"surf","Operations on mesh surface structure"));
    ops.push_back(Cmd(toTris,"toTris","Convert all facets to tris"));
    ops.push_back(Cmd(unifyuvs,"unifyUVs","Unify identical UV coordinates"));
    ops.push_back(Cmd(unifyverts,"unifyVerts","Unify identical vertices"));
    ops.push_back(Cmd(uvclamp,"uvclamp","Clamp UV coords to the range [0,1]"));
    ops.push_back(Cmd(uvWireframeImage,"uvImgW","Wireframe image of mesh UV map"));
    ops.push_back(Cmd(uvSolidImage,"uvImgS","Solid white inside UV facets, black outside, 4xFSAA"));
    ops.push_back(Cmd(uvmask,"uvmask","Mask out geometry for any black areas of a texture image (auto symmetrized)"));
    ops.push_back(Cmd(uvunwrap,"uvunwrap","Unwrap wrap-around UV coords to the range [0,1]"));
    ops.push_back(Cmd(xform,"xform","Create or apply similarity transforms from/to meshes"));
    doMenu(args,ops);
}

}

Cmd
fgCmdMeshopsInfo()
{return Cmd(meshops,"mesh","3D Mesh tools"); }

}

// */
