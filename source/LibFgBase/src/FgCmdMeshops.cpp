//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Sept 22, 2011
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

static
void
uvclamp(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<ext0> [<out>.<ext1>]\n"
        "    <ext0> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <ext1> = " + fgMeshSaveFormatsString()
        );
    Fg3dMesh        in = fgLoadMeshAnyFormat(syntax.next());
    FgMat22F        cb(0,1,0,1);
    for (size_t ii=0; ii<in.uvs.size(); ++ii)
        in.uvs[ii] = fgClipToBounds(in.uvs[ii],cb);
    if (syntax.more())
        fgSaveMeshAnyFormat(in,syntax.next());
    else
        fgSaveMeshAnyFormat(in,syntax.curr());
    return;
}

static
void
uvunwrap(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<ext0> [<out>.<ext1>]\n"
        "    <ext0> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <ext1> = " + fgMeshSaveFormatsString()
        );
    Fg3dMesh        in = fgLoadMeshAnyFormat(syntax.next());
    for (size_t ii=0; ii<in.uvs.size(); ++ii) {
        FgVect2F    uv = in.uvs[ii];
        in.uvs[ii][0] = fgMod(uv[0],1.0f);
        in.uvs[ii][1] = fgMod(uv[1],1.0f);
    }
    if (syntax.more())
        fgSaveMeshAnyFormat(in,syntax.next());
    else
        fgSaveMeshAnyFormat(in,syntax.curr());
    return;
}

static
void
combinesurfs(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "(<mesh>.<extIn>)+ <out>.<extOut>\n"
        "    <extIn> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <extOut> = " + fgMeshSaveFormatsString() + "\n"
        "    All input meshes must have identical vertex lists.\n"
        );
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(syntax.next());
    while (syntax.more()) {
        string  name = syntax.next();
        if (syntax.more()) {
            Fg3dMesh    next = fgLoadMeshAnyFormat(name);
            fgCat_(mesh.surfaces,next.surfaces);
        }
        else
            fgSaveMeshAnyFormat(mesh,name);
    }
}

static
void
copyUvList(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<ext0> <out>.<ext1>\n"
        "    <ext0> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <ext1> = " + fgMeshSaveFormatsString()
        );
    Fg3dMesh        in = fgLoadMeshAnyFormat(syntax.next());
    Fg3dMesh        out = fgLoadMeshAnyFormat(syntax.next());
    if (in.uvs.size() != out.uvs.size())
        syntax.error("Incompatible UV list sizes");
    out.uvs = in.uvs;
    fgSaveMeshAnyFormat(out,syntax.curr());
    return;
}

static
void
copyUvs(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<from>.<ext0> <to>.<ext1>\n"
        "    <ext0> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <ext1> = " + fgMeshSaveFormatsString()
        );
    Fg3dMesh        in = fgLoadMeshAnyFormat(syntax.next());
    Fg3dMesh        out = fgLoadMeshAnyFormat(syntax.next());
    out.uvs = in.uvs;
    if (in.surfaces.size() != out.surfaces.size())
        fgThrow("Incompatible number of surfaces");
    for (size_t ss=0; ss<in.surfaces.size(); ++ss) {
        const Fg3dSurface &     sin = in.surfaces[ss];
        Fg3dSurface &           sout = out.surfaces[ss];
        if ((sin.tris.size() != sout.tris.size()) ||
            (sin.quads.size() != sout.quads.size()))
            fgThrow("Incompatible facet counts");
        sout.tris.uvInds = sin.tris.uvInds;
        sout.quads.uvInds = sin.quads.uvInds;
    }
    fgSaveMeshAnyFormat(out,syntax.curr());
    return;
}

static
void
copyverts(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<ext0> <out>.<ext1>\n"
        "    <ext0> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <ext1> = " + fgMeshSaveFormatsString() + "\n"
        "    <out> is modified to have the vertex list from <in>"
        );
    Fg3dMesh        in = fgLoadMeshAnyFormat(syntax.next());
    Fg3dMesh        out = fgLoadMeshAnyFormat(syntax.next());
    if (in.verts.size() != out.verts.size())
        syntax.error("Incompatible vertex list sizes");
    out.verts = in.verts;
    fgSaveMeshAnyFormat(out,syntax.curr());
    return;
}

void
fgCmdEmboss(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<uvImage>.<img> <meshin>.<ext0> <val> <out>.<ext1>\n"
        "    <uvImage> = a UV-layout image whose grescale values will be used to emboss (0 - none, 255 - full)\n"
        "    <img>     = " + fgImgCommonFormatsDescription() + "\n"
        "    <ext0>    = " + fgLoadMeshFormatsDescription() + "\n"
        "    <val>     = Embossing factor as ratio of the max bounding box dimension.\n"
        "    <ext1>    = " + fgMeshSaveFormatsString()
        );
    FgImgUC         img;
    fgLoadImgAnyFormat(syntax.next(),img);      // Treat as greyscale
    Fg3dMesh        mesh = fgLoadMeshAnyFormat(syntax.next());
    if (mesh.uvs.empty())
        fgThrow("Mesh has no UVs",syntax.curr());
    if (mesh.surfaces.size() != 1)
        fgThrow("Only 1 surface currently supported",syntax.curr());
    float           val = fgFromString<float>(syntax.next());
    if (!(val > 0.0f))
        fgThrow("Emboss value must be > 0",fgToString(val));
    mesh.verts = fgEmboss(mesh,img,val);
    fgSaveMeshAnyFormat(mesh,syntax.next());
}

static
void
invWind(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <extOut> = " + fgMeshSaveFormatsString() + "\n"
        "    Inverts the winding of all facets in <in> and saves to <out>"
        );
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(syntax.next());
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        Fg3dSurface &   surf = mesh.surfaces[ss];
        for (size_t ii=0; ii<surf.tris.size(); ++ii)
            std::swap(surf.tris.vertInds[ii][1],surf.tris.vertInds[ii][2]);
        for (size_t ii=0; ii<surf.tris.uvInds.size(); ++ii)
            std::swap(surf.tris.uvInds[ii][1],surf.tris.uvInds[ii][2]);
        for (size_t ii=0; ii<surf.quads.size(); ++ii)
            std::swap(surf.quads.vertInds[ii][1],surf.quads.vertInds[ii][3]);
        for (size_t ii=0; ii<surf.quads.uvInds.size(); ++ii)
            std::swap(surf.quads.uvInds[ii][1],surf.quads.uvInds[ii][3]);
    }
    fgSaveMeshAnyFormat(mesh,syntax.next());
}

static
void
markVerts(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.tri <verts>.<ext> <out>.tri\n"
        "    <ext> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <out>.tri will be saved after marking a vertex in <in>.tri that is closest to each vertex in <verts>.<ext>."
        );
    Fg3dMesh    mesh = fgLoadTri(syntax.next());
    FgVerts     verts = fgLoadMeshAnyFormat(syntax.next()).verts;
    float       dim = fgMaxElem(fgDims(mesh.verts));
    uint        poorMatches = 0,
                totalMatches = 0;
    for (size_t vv=0; vv<verts.size(); ++vv) {
        FgVect3F        v = verts[vv];
        float           bestMag = numeric_limits<float>::max();
        uint            bestIdx = 0;
        for (size_t ii=0; ii<mesh.verts.size(); ++ii) {
            float       mag = fgMag(mesh.verts[ii]-v);
            if (mag < bestMag) {
                bestMag = mag;
                bestIdx = uint(ii);
            }
        }
        if (sqrt(bestMag) / dim > 0.00001f)
            ++poorMatches;
        if (!fgContains(mesh.markedVerts,bestIdx)) {
            mesh.markedVerts.push_back(FgMarkedVert(bestIdx));
            ++totalMatches;
        }
    }
    if (poorMatches > 0)
        fgout << fgnl << "WARNING: " << poorMatches << " poor matches.";
    if (totalMatches < verts.size())
        fgout << fgnl << "WARNING: duplicate matches.";
    fgout << fgnl << totalMatches << " vertices marked.";
    fgSaveTri(syntax.next(),mesh);
}

static
void
xformApply(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<similarity>.xml <in>.<ext0> <out>.<ext1>\n"
        "    <ext0> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <ext1> = " + fgMeshSaveFormatsString()
        );
    FgSimilarity    xform;
    fgLoadXml(syntax.next(),xform);
    Fg3dMesh        in = fgLoadMeshAnyFormat(syntax.next());
    Fg3dMesh        out(in);
    out.transform(FgAffine3F(xform.asAffine()));
    fgSaveMeshAnyFormat(out,syntax.next());
    return;
}

static
void
xformCreateMeshes(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<similarity>.xml <base>.<ex> <transformed>.<ex>\n"
        "    <ex> = " + fgLoadMeshFormatsDescription()
        );
    string      simFname = syntax.next();
    Fg3dMesh    base = fgLoadMeshAnyFormat(syntax.next());
    Fg3dMesh    targ = fgLoadMeshAnyFormat(syntax.next());
    if (base.verts.size() != targ.verts.size())
        fgThrow("Base and target mesh vertex counts are different");
    vector<FgVect3D>    bv = fgToDouble(base.verts),
                        tv = fgToDouble(targ.verts);
    FgSimilarity        sim = fgSimilarityApprox(bv,tv);
    double              ssd = fgSsd(fgTransform(bv,sim.asAffine()),tv),
                        sz = fgMaxElem(fgDims(tv));
    fgout << fgnl << "Transformed base to target relative RMS delta: " << sqrt(ssd / tv.size()) / sz;
    fgSaveXml(simFname,sim);
}

static
void
xformCreateIdentity(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<similarity>.xml\n"
        "    Edit the values in this file or apply subsequent transforms with other commands"
        );
    string      simFname = syntax.next();
    fgSaveXml(simFname,FgSimilarity());
}

static
void
xformCreateTrans(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<similarity>.xml <X> <Y> <Z>"
        );
    string          simFname = syntax.next();
    FgSimilarity    sim;
    if (fgExists(simFname))
        fgLoadXml(simFname,sim);
    FgVect3D    trans;
    trans[0] = fgFromString<double>(syntax.next());
    trans[1] = fgFromString<double>(syntax.next());
    trans[2] = fgFromString<double>(syntax.next());
    fgSaveXml(simFname,FgSimilarity(trans)*sim);
}

static
void
xformCreateRotate(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<similarity>.xml <axis> <degrees>\n"
        "    <axis> = (x | y | z)   - Right-hand-rule axis of rotation"
        );
    string          simFname = syntax.next();
    FgSimilarity    sim;
    if (fgExists(simFname))
        fgLoadXml(simFname,sim);
    string          axisStr = syntax.next();
    if (axisStr.empty())
        syntax.error("<axis> cannot be the empty string");
    char            axisName = std::tolower(axisStr[0]);
    double          degs = fgFromString<double>(syntax.next()),
                    rads = fgDegToRad(degs);
    int             axisNum = int(axisName) - int('x');
    if ((axisNum < 0) || (axisNum > 2))
        syntax.error("Invalid value for <axis>",axisStr);
    FgQuaternionD   rot(rads,uint(axisNum));
    fgSaveXml(simFname,FgSimilarity(rot)*sim);
}

static
void
xformCreateScale(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<similarity>.xml <scale>"
        );
    string          simFname = syntax.next();
    FgSimilarity    sim;
    if (fgExists(simFname))
        fgLoadXml(simFname,sim);
    double          scale = fgFromString<double>(syntax.next());
    fgSaveXml(simFname,FgSimilarity(scale)*sim);
}

static
void
xformCreate(const FgArgs & args)
{
    fgMenu(args,
        fgSvec(
            FgCmd(xformCreateMeshes,"meshes","Create similarity transform from base and transformed meshes with matching vertex lists"),
            FgCmd(xformCreateIdentity,"identity","Create the identity similarity transform"),
            FgCmd(xformCreateTrans,"translate","Apply a translation to a similarity transform"),
            FgCmd(xformCreateRotate,"rotate","Apply a rotation to a similarity transform"),
            FgCmd(xformCreateScale,"scale","Apply a scaling to a similarity transform")
            ));
}

static
void
xform(const FgArgs & args)
{
    fgMenu(args,
        fgSvec(
            FgCmd(xformApply,"apply","Apply a simiarlity transform (from .XML file) to a mesh"),
            FgCmd(xformCreate,"create","Create a similarity transform (to .XML file)")));
}

static
void
uvmask(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<meshIn>.<ext0> <imageIn>.<ext1> <meshOut>.<ext2>\n"
        "    <ext0> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <ext1> = " + fgImgCommonFormatsDescription() + "\n"
        "    <ext2> = " + fgMeshSaveFormatsString()
        );
    Fg3dMesh        mesh = fgLoadMeshAnyFormat(syntax.next());
    FgImgRgbaUb     img;
    fgLoadImgAnyFormat(syntax.next(),img);
    FgImage<FgBool> mask = FgImage<FgBool>(img.dims());
    for (FgIter2UI it(img.dims()); it.valid(); it.next()) {
        FgVect4UC   px = img[it()].m_c;
        mask[it()] = (px[0] > 0) || (px[1] > 0) || (px[2] > 0); }
    mask = fgAnd(mask,fgFlipHoriz(mask));
    mesh = fg3dMaskFromUvs(mesh,mask);
    fgSaveMeshAnyFormat(mesh,syntax.next());
}

static
void
mmerge(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "(<mesh>.<extIn>)+ <out>.<extOut>\n"
        "    <extIn> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <extOut> = " + fgMeshSaveFormatsString()
        );
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(syntax.next());
    while (syntax.more()) {
        string  name = syntax.next();
        if (syntax.more())
            mesh = fgMergeMeshes(mesh,fgLoadMeshAnyFormat(name));
        else
            fgSaveMeshAnyFormat(mesh,name);
    }
}

static
void
mergesurfs(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <extOut> = " + fgMeshSaveFormatsString()
        );
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(syntax.next());
    if (mesh.surfaces.size() == 1)
        fgout << "WARNING: Mesh only has one surface, no merging done.";
    mesh.mergeAllSurfaces();
    fgSaveMeshAnyFormat(mesh,syntax.next());
}

static
void
rdf(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.tri [<out>.tri]\n"
        "    <in>.tri       - Will be overwritten if <out>.tri is not specified\n"
        "NOTES:\n"
        "    Duplicates are determined by vertex index. To remove duplicates by vertex\n"
        "    value, first remove duplicate vertices."
        );
    FgString        fni(syntax.next()),
                    fno = fni;
    if (syntax.more())
        fno = syntax.next();
    fgSaveTri(fno,fgRemoveDuplicateFacets(fgLoadTri(fni)));
}

static
void
ruv(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <extOut> = " + fgMeshSaveFormatsString()
        );
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(syntax.next());
    mesh = fgRemoveUnusedVerts(mesh);
    fgSaveMeshAnyFormat(mesh,syntax.next());
}

static
void
mergenamedsurfs(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <extOut> = " + fgMeshSaveFormatsString()
        );
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(syntax.next());
    mesh = fgMergeSameNameSurfaces(mesh);
    fgSaveMeshAnyFormat(mesh,syntax.next());
}

static
void
splitObjByMtl(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<mesh>.[w]obj <base>\n"
        "    Creates a <base>_<name>.tri file for each 'usemtl' name referenced");
    Fg3dMesh    mesh = fgLoadWobj(syntax.next(),"usemtl");
    string      base = syntax.next();
    Fg3dMesh    m = mesh;
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        m.surfaces = fgSvec(mesh.surfaces[ii]);
        fgSaveTri(base+"_"+mesh.surfaces[ii].name+".tri",m);
    }
}

static
void
splitsurfsbyuvs(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <extOut> = " + fgMeshSaveFormatsString()
        );
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(syntax.next());
    mesh = fgSplitSurfsByUvs(mesh);
    fgSaveMeshAnyFormat(mesh,syntax.next());
}

static
void
splitsurface(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<extIn>\n"
        "    <extIn> = " + fgLoadMeshFormatsDescription() + "\n"
        "COMMENTS:\n"
        "    * Splits surfaces by connected vertex indices.\n"
        "    * Stores results to separate meshes with suffix '_<num>.tri'"
        );
    Fg3dMesh                mesh = fgLoadMeshAnyFormat(syntax.next());
    FgString                base = fgPathToBase(syntax.curr());
    uint                    idx = 0;
    Fg3dMesh                out = mesh;
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        vector<Fg3dSurface>     surfs = fgSplitSurface(mesh.surfaces[ss]);
        for (size_t ii=0; ii<surfs.size(); ++ii) {
            out.surfaces = fgSvec(surfs[ii]);
            fgSaveTri(base+"_"+fgToString(idx++)+".tri",out);
        }
    }
}

static
void
convert(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <extOut> = " + fgMeshSaveFormatsString()
        );
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(syntax.next());
    fgSaveMeshAnyFormat(mesh,syntax.next());
}

static
void
surfAdd(const FgArgs & args)
{
    FgSyntax    syn(args,
        "<in>.<ext> <name> <out>.fgmesh\n"
        "    <ext>  - " + fgLoadMeshFormatsDescription() + "\n"
        "    <name> - Surface name"
        );
    Fg3dMesh        mesh = fgLoadMeshAnyFormat(syn.next());
    Fg3dSurface     surf;
    surf.name = syn.next();
    mesh.surfaces.push_back(surf);
    fgSaveFgmesh(syn.next(),mesh);
}

static
void
surfCopy(const FgArgs & args)
{
    FgSyntax    syn(args,
        "<from>.fgmesh <to>.<ext> <out>.fgmesh\n"
        "    <ext>  - " + fgLoadMeshFormatsDescription() + "\n"
        " * tris only, uvs not preserved."
        );
    Fg3dMesh        from = fgLoadFgmesh(syn.next()),
                    to = fgLoadMeshAnyFormat(syn.next());
    fgSaveFgmesh(syn.next(),fgCopySurfaceStructure(from,to));
}

static
void
surfDel(const FgArgs & args)
{
    FgSyntax        syn(args,
        "<in>.<ext> <idx> <out>.<ext>\n"
        "    <ext> - " + fgLoadMeshFormatsDescription() + "\n"
        "    <idx> - Which surface index to delete"
        );
    Fg3dMesh        mesh = fgLoadMeshAnyFormat(syn.next());
    size_t          idx = syn.nextAs<uint>();
    if (idx >= mesh.surfaces.size())
        syn.error("Selected surface index out of range",fgToString(idx));
    mesh.surfaces.erase(mesh.surfaces.begin()+idx);
    fgSaveMeshAnyFormat(mesh,syn.next());
}

static
void
surfList(const FgArgs & args)
{
    FgSyntax    syn(args,
        "<in>.<ext>\n"
        "    <ext> - " + fgLoadMeshFormatsDescription()
        );
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(syn.next());
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        const Fg3dSurface & surf = mesh.surfaces[ss];
        fgout << fgnl << ss << ": " << surf.name;
    }
}

static
void
surfRen(const FgArgs & args)
{
    FgSyntax    syn(args,
        "<in>.fgmesh <idx> <name>\n"
        "   <idx>  - Which surface\n"
        "   <name> - Surface name"
        );
    FgString        meshFname = syn.next();
    Fg3dMesh        mesh = fgLoadFgmesh(meshFname);
    size_t          idx = fgFromString<size_t>(syn.next());
    if (idx >= mesh.surfaces.size())
        fgThrow("Index value larger than available surfaces");
    mesh.surfaces[idx].name = syn.next();
    fgSaveFgmesh(meshFname,mesh);
}

static
void
spCopy(const FgArgs & args)
{
    FgSyntax    syn(args,"<from>.fgmesh <to>.fgmesh <out>.fgmesh");
    Fg3dMesh    from = fgLoadFgmesh(syn.next()),
                to = fgLoadFgmesh(syn.next());
    if (from.surfaces.size() != to.surfaces.size())
        fgThrow("'from' and 'to' meshes have different surface counts");
    for (size_t ss=0; ss<to.surfaces.size(); ++ss)
        fgCat_(to.surfaces[ss].surfPoints,from.surfaces[ss].surfPoints);
    fgSaveFgmesh(syn.next(),to);
}

static
void
spDel(const FgArgs & args)
{
    FgSyntax    syn(args,
        "<in>.tri <ptIdx>\n"
        "   <spIdx>   - Which point on that surface to delete"
        );
    FgString        meshFname = syn.next();
    Fg3dMesh        mesh = fgLoadTri(meshFname);
    size_t          ii = fgFromString<size_t>(syn.next());
    Fg3dSurface &   surf = mesh.surfaces[0];
    if (ii >= surf.surfPoints.size())
        fgThrow("Point index value larger than availables points");
    surf.surfPoints.erase(surf.surfPoints.begin() + ii);
    fgSaveTri(meshFname,mesh);
}

static
void
spList(const FgArgs & args)
{
    FgSyntax    syn(args,"<in>.fgmesh");
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(syn.next());
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        const Fg3dSurface & surf = mesh.surfaces[ss];
        fgout << fgnl << "Surface " << ss << ": " << surf.name << fgpush;
        for (size_t ii=0; ii<surf.surfPoints.size(); ++ii)
            fgout << fgnl << ii << " : " << surf.surfPoints[ii].label;
        fgout << fgpop;
    }
}

static
void
spRen(const FgArgs & args)
{
    FgSyntax    syn(args,
        "<in>.fgmesh <surfIdx> <ptIdx> <name>\n"
        "   <surfIdx> - Which surface\n"
        "   <spIdx>   - Which point on that surface\n"
        "   <name>    - Name"
        );
    FgString        meshFname = syn.next();
    Fg3dMesh        mesh = fgLoadFgmesh(meshFname);
    size_t          ss = fgFromString<size_t>(syn.next()),
                    ii = fgFromString<size_t>(syn.next());
    if (ss >= mesh.surfaces.size())
        fgThrow("Surface index value larger than available surfaces");
    Fg3dSurface &   surf = mesh.surfaces[ss];
    if (ii >= surf.surfPoints.size())
        fgThrow("Point index value larger than availables points");
    surf.surfPoints[ii].label = syn.next();
    fgSaveFgmesh(meshFname,mesh);
}

static
void
spsToVerts(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.tri <out>.tri\n"
        "    <out>.tri will be appended with the new marked vertices."
        );
    Fg3dMesh    in = fgLoadTri(syntax.next());
    Fg3dMesh    out = fgLoadTri(syntax.next());
    fgSurfPointsToMarkedVerts(in,out);
    fgSaveTri(syntax.curr(),out);
}

static
void
surf(const FgArgs & args)
{
    vector<FgCmd>   ops;
    ops.push_back(FgCmd(surfAdd,"add","Add an empty surface to a mesh"));
    ops.push_back(FgCmd(surfCopy,"copy","Copy surface structure between aligned meshes"));
    ops.push_back(FgCmd(surfDel,"del","Delete a surface in a mesh (leaves verts unchanged)"));
    ops.push_back(FgCmd(surfList,"list","List surfaces in mesh"));
    ops.push_back(FgCmd(mergenamedsurfs,"mergeNamed","Merge surfaces with identical names"));
    ops.push_back(FgCmd(mergesurfs,"merge","Merge all surfaces in a mesh into one"));
    ops.push_back(FgCmd(surfRen,"ren","Rename a surface in a mesh"));
    ops.push_back(FgCmd(spCopy,"spCopy","Copy surf points between meshes with identical surface topology"));
    ops.push_back(FgCmd(spDel,"spDel","Delete a surface point"));
    ops.push_back(FgCmd(spList,"spList","List surface points in each surface"));
    ops.push_back(FgCmd(spRen,"spRen","Rename a surface point"));
    ops.push_back(FgCmd(spsToVerts,"spVert","Convert surface points to marked vertices"));
    fgMenu(args,ops);
}

static
void
toTris(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <extOut> = " + fgMeshSaveFormatsString()
        );
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(syntax.next());
    mesh.convertToTris();
    fgSaveMeshAnyFormat(mesh,syntax.next());
}

static
void
unifyuvs(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <extOut> = " + fgMeshSaveFormatsString()
        );
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(syntax.next());
    mesh = fgUnifyIdenticalUvs(mesh);
    fgSaveMeshAnyFormat(mesh,syntax.next());
}

static
void
unifyverts(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<extIn> <out>.<extOut>\n"
        "    <extIn> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <extOut> = " + fgMeshSaveFormatsString()
        );
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(syntax.next());
    mesh = fgUnifyIdenticalVerts(mesh);
    fgSaveMeshAnyFormat(mesh,syntax.next());
}

static
void
uvimg(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<mesh>.<extIm> <img>.<extImg>\n"
        "    <extIn> = " + fgLoadMeshFormatsDescription() + "\n"
        "    <extImg> = " + fgImgCommonFormatsDescription()
        );
    Fg3dMesh        mesh = fgLoadMeshAnyFormat(syntax.next());
    FgImgRgbaUb     img;
    if (fgExists(syntax.peekNext()))
        fgLoadImgAnyFormat(syntax.peekNext(),img);
    fgSaveImgAnyFormat(syntax.next(),fgUvImage(mesh,img));
}

static
void
meshops(const FgArgs & args)
{
    vector<FgCmd>   ops;
    ops.push_back(FgCmd(combinesurfs,"combinesurfs","Combine surfaces from meshes with identical vertex lists"));
    ops.push_back(FgCmd(convert,"convert","Convert the mesh between different formats"));
    ops.push_back(FgCmd(copyUvList,"copyUvList","Copy UV list from one mesh to another with same UV count"));
    ops.push_back(FgCmd(copyUvs,"copyUvs","Copy UVs from one mesh to another with identical facet structure"));
    ops.push_back(FgCmd(copyverts,"copyverts","Copy verts from one mesh to another with same vertex count"));
    ops.push_back(FgCmd(fgCmdEmboss,"emboss","Emboss a mesh based on greyscale values of a UV image"));
    ops.push_back(FgCmd(invWind,"invWind","Invert facet winding of a mesh"));
    ops.push_back(FgCmd(markVerts,"markVerts","Mark vertices in a .TRI file from a given list"));
    ops.push_back(FgCmd(mmerge,"merge","Merge multiple meshes into one. No optimization is done"));
    ops.push_back(FgCmd(rdf,"rdf","Remove Duplicate Facets within each surface"));
    ops.push_back(FgCmd(ruv,"ruv","Remove vertices and uvs not referenced by a surface or marked vertex"));
    ops.push_back(FgCmd(splitObjByMtl,"splitObjByMtl","Split up an OBJ mesh by 'usemtl' name"));
    ops.push_back(FgCmd(splitsurface,"splitSurface","Split up surface by connected vertex indices"));
    ops.push_back(FgCmd(splitsurfsbyuvs,"splitSurfsByUvs","Split up surfaces with discontiguous UV mappings"));
    ops.push_back(FgCmd(surf,"surf","Operations on mesh surface structure"));
    ops.push_back(FgCmd(toTris,"toTris","Convert all facets to tris"));
    ops.push_back(FgCmd(unifyuvs,"unifyUVs","Unify identical UV coordinates"));
    ops.push_back(FgCmd(unifyverts,"unifyVerts","Unify identical vertices"));
    ops.push_back(FgCmd(uvclamp,"uvclamp","Clamp UV coords to the range [0,1]"));
    ops.push_back(FgCmd(uvimg,"uvimg","Save an image of the  the UV map of the mesh"));
    ops.push_back(FgCmd(uvmask,"uvmask","Mask out geometry for any black areas of a texture image (auto symmetrized)"));
    ops.push_back(FgCmd(uvunwrap,"uvunwrap","Unwrap wrap-around UV coords to the range [0,1]"));
    ops.push_back(FgCmd(xform,"xform","Create or apply similarity transforms from/to meshes"));
    fgMenu(args,ops);
}

FgCmd
fgCmdMeshopsInfo()
{return FgCmd(meshops,"meshops","Operations on 3D meshes"); }

// */
