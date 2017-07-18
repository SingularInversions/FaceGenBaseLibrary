//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: June 23, 2009
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dDisplay.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dMeshOps.hpp"
#include "FgSyntax.hpp"
#include "FgImgDisplay.hpp"
#include "FgDraw.hpp"
#include "FgAffineCwC.hpp"
#include "FgBuild.hpp"

using namespace std;

static
void
viewMesh(const FgArgs & args)
{
    FgSyntax            syntax(args,
        "[-c] [-r] (<mesh>.<ext> [<texImage> [<transparencyImage>]])+\n"
        "    -c    - Compare meshes rather than view all at once\n"
        "    -r    - Remove unused vertices for viewing\n"
        "    <ext> - " + fgLoadMeshFormatsDescription());
    bool                compare = false,
                        ru = false;
    while (syntax.peekNext()[0] == '-') {
        if (syntax.next() == "-c")
            compare = true;
        else if (syntax.curr() == "-r")
            ru = true;
        else
            syntax.error("Unrecognized option: ",syntax.curr());
    }
    vector<Fg3dMesh>    meshes;
    while (syntax.more()) {
        FgString        fname = syntax.next(),
                        ext = fgToLower(fgPathToExt(fname));
        Fg3dMesh        mesh = fgLoadMeshAnyFormat(fname);
        fgout << fgnl << "Mesh " << meshes.size() << ": " << fgpush << mesh;
        size_t          origVerts = mesh.verts.size();
        if (ru) {
            mesh = fgRemoveUnusedVerts(mesh);
            if (mesh.verts.size() < origVerts)
                fgout << fgnl << origVerts-mesh.verts.size() << " unused vertices removed for viewing";
        }
        if(syntax.more() && fgIsImgFilename(syntax.peekNext())) {
            if (mesh.uvs.empty())
                fgout << fgnl << "WARNING: " << syntax.curr() << " has no UVs, texture image "
                    << syntax.peekNext() << " will not be seen.";
            FgImgRgbaUb         texture;
            fgLoadImgAnyFormat(syntax.next(),texture);
            fgout << fgnl << "Texture image: " << texture;
            if(syntax.more() && fgIsImgFilename(syntax.peekNext())) {
                FgImgRgbaUb     trans;
                fgout << fgnl << "Transparency image:" << trans;
                fgLoadImgAnyFormat(syntax.next(),trans);
                mesh.surfaces[0].setAlbedoMap(fgImgApplyTransparencyPow2(texture,trans));
            }
            else {
                fgResizePow2Ceil(texture,mesh.surfaces[0].albedoMapRef());
            }
        }
        meshes.push_back(mesh);
        fgout << fgpop;
    }
    FgViewMeshes(meshes,compare);
}

void
fgViewImage(const FgArgs & args)
{
    FgSyntax    syntax(args,"<imageFileName>");
    if (args.size() > 2)
        syntax.incorrectNumArgs();
    FgImgRgbaUb     img;
    fgLoadImgAnyFormat(syntax.next(),img);
    fgout << fgnl << img;
    fgImgDisplay(img);
}

void
fgViewImagef(const FgArgs & args)
{
    FgSyntax    syntax(args,"<imageFileName> [<saveName>]");
    FgImgF      img;
    if (fgToLower(fgPathToExt(syntax.next())) == "fgpbn")
        fgLoadPBin(syntax.curr(),img);
    else {
        if (fgCurrentOS() != "win")
            fgout << "WARNING: This functionality currently only works properly under windows";
        fgLoadImgAnyFormat(syntax.curr(),img);
    }
    if (syntax.more())
        fgSavePBin(syntax.next(),img);
    fgout << fgnl << img;
    fgImgDisplay(img);
}

void
fgCmdViewUvs(const FgArgs & args)
{
    FgSyntax            syntax(args,
        "(<mesh>.<ext>)+ [<texImage>]\n"
        "     <ext> = " + fgLoadMeshFormatsDescription());
    Fg3dMesh            mesh;
    FgImgRgbaUb         img;
    while (syntax.more()) {
        string          fname = syntax.next(),
                        ext = fgToLower(fgPathToExt(fname));
        if (fgContains(fgLoadMeshFormats(),ext)) {
            Fg3dMesh    tmp = fgLoadMeshAnyFormat(fname);
            if (tmp.uvs.empty())
                syntax.error("Mesh has no UVs",fname);
            mesh = fgMergeMeshes(mesh,tmp);
        }
        else if (fgIsImgFilename(fname)) {
            if (!img.empty())
                syntax.error("Only one image allowed");
            img = fgLoadImgAnyFormat(fname);
        }
        else
            syntax.error("Unknown file type",fname);
    }
    FgMat22F            uvb = fgBounds(mesh.uvs);
    fgout << fgnl << "UV Bounds: " << uvb;
    FgVectF2            uvbb = fgBounds(uvb);
    if ((uvbb[0] < 0.0f) || (uvbb[1] > 1.0f))
        fgout << fgnl << "WARNING: wraparound UV bounds, mapping domain expanded";
    fgImgDisplay(fgUvImage(mesh,img));
}

vector<FgCmd>
fgCmdViewInfos()
{
    vector<FgCmd>   cmds;
    cmds.push_back(FgCmd(viewMesh,"mesh","Interactively view 3D meshes"));
    cmds.push_back(FgCmd(fgViewImage,"image","Basic image viewer"));
    cmds.push_back(FgCmd(fgViewImagef,"imagef","Floating point image viewer"));
    cmds.push_back(FgCmd(fgCmdViewUvs,"uvs","View the UV layout of a 3D mesh"));
    return cmds;
}
