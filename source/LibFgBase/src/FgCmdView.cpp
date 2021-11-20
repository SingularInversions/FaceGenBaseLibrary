//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dDisplay.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dMesh.hpp"
#include "FgSyntax.hpp"
#include "FgImgDisplay.hpp"
#include "FgImageDraw.hpp"
#include "FgAffine.hpp"
#include "FgBuild.hpp"
#include "FgImagePoint.hpp"

using namespace std;

namespace Fg {

void
cmdViewMesh(CLArgs const & args)
{
    Syntax            syn(args,
        "[-c] [-r] (<mesh>.<ext> [<color>.<img> [-t <transparency>.<img>] [-s <specular>.<img>]]+ )+\n"
        "    -c         - Compare meshes rather than view all at once (use 'Select' tab to toggle)\n"
        "    -r         - Remove unused vertices for viewing\n"
        "    <mesh>     - Mesh to view\n"
        "    <ext>      - " + getMeshLoadExtsCLDescription() +
        "    <color>    - Color / albedo map (can contain transparency in alpha channel). Can specify one for each surface.\n"
        "    <transparency> - Transparency map\n"
        "    <specular> - Specularity map\n"
        "    <img>      - " + getImageFileExtCLDescriptions()
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
    Meshes           all;
    while (syn.more()) {
        Path            path(syn.next());
        PushIndent      pind {path.baseExt().m_str};
        Meshes          meshes = loadMeshes(path.str());
        size_t          idx {0};
        for (Mesh & mesh : meshes) {
            if (mesh.name.empty())
                mesh.name = path.base;
            if (removeUnused) {
                size_t          origVerts = mesh.verts.size();
                mesh = removeUnusedVerts(mesh);
                if (mesh.verts.size() < origVerts)
                    fgout << fgnl << origVerts-mesh.verts.size() << " unused vertices removed for viewing";
            }
            fgout << fgnl << toStrDigits(idx++,2) << fgpush << mesh << fgpop;
        }
        if (syn.more() && hasImageFileExt(syn.peekNext())) {
            size_t              meshIdx = 0,
                                surfIdx = 0;
            while (syn.more() && hasImageFileExt(syn.peekNext())) {
                ImgRgba8            albedo = loadImage(syn.next()),
                                    specular;
                if (meshes[meshIdx].uvs.empty())
                    fgout << fgnl << "WARNING: " << syn.curr() << " has no UVs, color maps will not be seen.";
                fgout << fgnl << syn.curr() << " mesh " << meshIdx << " surf " << surfIdx
                    << fgpush << albedo << fgpop;
                if (syn.more() && (syn.peekNext()[0] == '-')) {
                    if(syn.next() == "-t") {
                        ImgRgba8         trans = loadImage(syn.next());
                        fgout << fgnl << syn.curr() << fgpush << trans << fgpop;
                        albedo = fgImgApplyTransparencyPow2(albedo,trans);
                    }
                    else if (syn.curr() == "-s") {
                        loadImage_(syn.next(),specular);
                        fgout << fgnl << syn.curr() << fgpush << specular << fgpop;
                    }
                    else
                        syn.error("Unrecognized image map option",syn.curr());
                }
                if ((meshIdx < meshes.size()) && (surfIdx < meshes[meshIdx].surfaces.size())) {
                    Surf &              surf = meshes[meshIdx].surfaces[surfIdx];
                    surf.setAlbedoMap(albedo);
                    if (!specular.empty())
                        surf.material.specularMap = make_shared<ImgRgba8>(specular);
                }
                else
                    fgout << fgnl << "WARNING: more maps than surfaces";
                ++surfIdx;
                if (surfIdx >= meshes[meshIdx].surfaces.size()) {
                    ++meshIdx;
                    surfIdx = 0;
                }
            }
        }
        cat_(all,meshes);
    }
    if (all.empty())
        syn.error("No meshes specified");
    Mesh        ignoreModified = viewMesh(all,compare);
}

void
cmdViewImage(CLArgs const & args)
{
    Syntax          syn {args,"<image>.<ext> [<points>.txt]\n"
        "    <ext>      - (" + cat(getImageFileExts(),",") + ")\n"
        "    <points>   - optional points annotation file in simple YOLO format"
    };
    ImgRgba8         img = loadImage(syn.next());
    fgout << img;
    ImagePoints     ips;
    if (syn.more()) {
        ips = loadImagePoints(syn.next());
        fgout << ips;
    }
    AffineEw2F      xf = cIrcsToIucsXf(img.dims());
    Mat22F          bnds {0,1,0,1};
    Vec2Fs          pts;
    for (ImagePoint const & ip : ips) {
        Vec2F           iucs = xf * ip.posIrcs;
        if (!isInBounds(bnds,iucs))
            fgout << fgnl << "WARNING point is not on image: " << ip.label;
        pts.push_back(iucs);
    }
    viewImage(img,pts);
}

void
cmdViewImageF(CLArgs const & args)
{
    Syntax    syn(args,"<imageFileName> [<saveName>]");
    ImgF      img;
    if (toLower(pathToExt(syn.next())) == "fgpbn")
        loadBsaPBin(syn.curr(),img);
    if (syn.more())
        saveBsaPBin(syn.next(),img);
    fgout << fgnl << img;
    viewImage(img);
}

void
cmdViewUvs(CLArgs const & args)
{
    Syntax              syn(args,
        "<mesh>.<ext> [<texImage>]+\n"
        "     <ext> = " + getMeshLoadExtsCLDescription());
    Mesh                mesh = loadMesh(syn.next());
    if (mesh.uvs.empty())
        syn.error("Mesh has no UVs",syn.curr());
    Mat22F              bounds = cBounds(mesh.uvs);
    fgout << fgnl << syn.curr() << " UV Bounds: " << bounds;
    if ((cMinElem(bounds) < 0) || (cMaxElem(bounds) > 1))
        fgout << fgnl << "WARNING: UVs outside [0,1] were not drawn";
    String8s            names;
    ImgRgba8s           images;
    Rgba8               color {0,255,0,255};
    size_t              cnt {0};
    for (Surf const & surf : mesh.surfaces) {
        if (surf.name.empty())
            names.emplace_back("Unnamed-" + toStr(cnt++));
        else
            names.push_back(surf.name);
        ImgRgba8            wi = cUvWireframeImage(mesh.uvs,surf.tris.uvInds,surf.quads.uvInds,color);
        if (syn.more())
            images.push_back(composite(wi,loadImage(syn.next())));
        else
            images.push_back(wi);
    }
    viewImages(images,names);
}

Cmds
getViewCmds()
{
    Cmds            cmds {
        {cmdViewMesh,"mesh","Interactively view 3D meshes"},
        {cmdViewImage,"image","Basic image viewer"},
        {cmdViewImageF,"imagef","Floating point image viewer"},
        {cmdViewUvs,"uvs","View the UV layout of a 3D mesh"},
    };
    return cmds;
}

}
