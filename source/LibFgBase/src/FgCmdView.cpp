//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
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

namespace Fg {

static
void
viewMesh(CLArgs const & args)
{
    Syntax            syn(args,
        "[-c] [-r] (<mesh>.<ext> [<color>.<img> [-t <transparency>.<img>] [-s <specular>.<img>]])+\n"
        "    -c         - Compare meshes rather than view all at once\n"
        "    -r         - Remove unused vertices for viewing\n"
        "    <mesh>     - Mesh to view\n"
        "    <ext>      - " + meshLoadFormatsCLDescription() +
        "    <color>    - Color / albedo map (can contain transparency in alpha channel)\n"
        "    <transparency> - Transparency map\n"
        "    <specular> - Specularity map\n"
        "    <img>      - " + imgFileExtensionsDescription()
    );
    bool            compare = false,
                    removeUnused = false;
    while (syn.peekNext()[0] == '-') {
        if (syn.next() == "-c")
            compare = true;
        else if (syn.curr() == "-r")
            removeUnused = true;
        else
            syn.error("Unrecognized option: ",syn.curr());
    }
    Meshes           meshes;
    while (syn.more()) {
        Path            path(syn.next());
        Mesh            mesh = meshLoadAnyFormat(path.str());
        mesh.name = path.base;
        if (removeUnused) {
            size_t          origVerts = mesh.verts.size();
            mesh = meshRemoveUnusedVerts(mesh);
            if (mesh.verts.size() < origVerts)
                fgout << fgnl << origVerts-mesh.verts.size() << " unused vertices removed for viewing";
        }
        fgout << fgnl << mesh;
        if (syn.more() && hasImgExtension(syn.peekNext())) {
            if (mesh.uvs.empty())
                fgout << fgnl << "WARNING: " << syn.curr() << " has no UVs, texture image "
                    << syn.peekNext() << " will not be seen.";
            ImgC4UC         albedo = imgLoadAnyFormat(syn.next());
            fgout << fgnl << "Albedo map: " << albedo;
            albedo = fgResizePow2Ceil(albedo);
            if (syn.more() && (syn.peekNext()[0] == '-')) {
                if(syn.next() == "-t") {
                    ImgC4UC         trans = imgLoadAnyFormat(syn.next());
                    fgout << fgnl << "Transparency map: " << trans;
                    albedo = fgImgApplyTransparencyPow2(albedo,trans);
                }
                else if (syn.curr() == "-s") {
                    ImgC4UC         spec;
                    imgLoadAnyFormat(syn.next(),spec);
                    fgout << fgnl << "Specularity map: " << spec;
                    auto                specPtr = make_shared<ImgC4UC>(spec);
                    for (Surf & surf : mesh.surfaces)
                        surf.material.specularMap = specPtr;
                }
                else
                    syn.error("Unrecognized image map option",syn.curr());
            }
            auto        albPtr = make_shared<ImgC4UC>(albedo);
            for (Surf & surf : mesh.surfaces)
                surf.material.albedoMap = albPtr;
        }
        meshes.push_back(mesh);
    }
    if (meshes.empty())
        syn.error("No meshes specified");
    Mesh        ignoreModified = meshView(meshes,compare);
}

void
fgViewImage(CLArgs const & args)
{
    Syntax    syntax(args,"<imageFileName>");
    if (args.size() > 2)
        syntax.incorrectNumArgs();
    ImgC4UC     img;
    imgLoadAnyFormat(syntax.next(),img);
    fgout << fgnl << img;
    imgDisplay(img);
}

void
fgViewImagef(CLArgs const & args)
{
    Syntax    syntax(args,"<imageFileName> [<saveName>]");
    ImgF      img;
    if (fgToLower(fgPathToExt(syntax.next())) == "fgpbn")
        fgLoadPBin(syntax.curr(),img);
    else {
        if (fgCurrentBuildOS() != FgBuildOS::win)
            fgout << "WARNING: This functionality currently only works properly under windows";
        imgLoadAnyFormat(syntax.curr(),img);
    }
    if (syntax.more())
        fgSavePBin(syntax.next(),img);
    fgout << fgnl << img;
    imgDisplay(img);
}

void
fgCmdViewUvs(CLArgs const & args)
{
    Syntax            syntax(args,
        "(<mesh>.<ext>)+ [<texImage>]\n"
        "     <ext> = " + meshLoadFormatsCLDescription());
    Mesh            mesh;
    ImgC4UC         img;
    while (syntax.more()) {
        string          fname = syntax.next(),
                        ext = fgToLower(fgPathToExt(fname));
        if (fgContains(meshLoadFormats(),ext)) {
            Mesh    tmp = meshLoadAnyFormat(fname);
            if (tmp.uvs.empty())
                syntax.error("Mesh has no UVs",fname);
            mesh = fgMergeMeshes(mesh,tmp);
        }
        else if (hasImgExtension(fname)) {
            if (!img.empty())
                syntax.error("Only one image allowed");
            img = imgLoadAnyFormat(fname);
        }
        else
            syntax.error("Unknown file type",fname);
    }
    Mat22F            uvb = cBounds(mesh.uvs);
    fgout << fgnl << "UV Bounds: " << uvb;
    VecF2            uvbb = cBounds(uvb.m);
    if ((uvbb[0] < 0.0f) || (uvbb[1] > 1.0f))
        fgout << fgnl << "WARNING: wraparound UV bounds, mapping domain expanded";
    imgDisplay(fgUvWireframeImage(mesh,img));
}

vector<Cmd>
fgCmdViewInfos()
{
    vector<Cmd>   cmds;
    cmds.push_back(Cmd(viewMesh,"mesh","Interactively view 3D meshes"));
    cmds.push_back(Cmd(fgViewImage,"image","Basic image viewer"));
    cmds.push_back(Cmd(fgViewImagef,"imagef","Floating point image viewer"));
    cmds.push_back(Cmd(fgCmdViewUvs,"uvs","View the UV layout of a 3D mesh"));
    return cmds;
}

}
