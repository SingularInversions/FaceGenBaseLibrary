//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
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

using namespace std;

namespace Fg {

void                cmdViewMesh(CLArgs const & args)
{
    Syntax            syn {args,
        R"([-c] [-r] (<mesh>.<ext> [<color>.<img> [-t <transparency>.<img>] [-s <specular>.<img>]]+ )+
    -c         - Compare meshes rather than view all at once (use 'Select' tab to toggle)
    -r         - Remove unused vertices for viewing
    <mesh>     - Mesh to view
    <ext>      - )" + getMeshLoadExtsCLDescription() + R"(
    <color>    - Color / albedo map (can contain transparency in alpha channel). Can specify one for each surface.
    <transparency> - Transparency map
    <specular> - Specularity map
    <img>      - " + clOptionsStr(getImgExts())
NOTES:
    * If only one mesh is selected, the Edit tab will allow selection of surface points and
      marked vertices, along with a Save option.)"
    };
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
            if (mesh.name.empty()) {            // multi-mesh files can load mesh names
                if (meshes.size() > 1)          // but in this case didn't
                    mesh.name = path.base + "-" + toStr(idx);
                else
                    mesh.name = path.base;
            }
            if (removeUnused) {
                size_t          origVerts = mesh.verts.size();
                mesh = removeUnusedVerts(mesh);
                if (mesh.verts.size() < origVerts)
                    fgout << fgnl << origVerts-mesh.verts.size() << " unused vertices removed for viewing";
            }
            fgout << fgnl << toStrDigits(idx++,2) << fgpush << mesh << fgpop;
        }
        if (syn.more() && hasImgFileExt(syn.peekNext())) {
            size_t              meshIdx = 0,
                                surfIdx = 0;
            while (syn.more() && hasImgFileExt(syn.peekNext())) {
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

void                cmdViewImage(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<image>.<ext> [<points>.txt]
    <ext>      - ()" + cat(getImgExts(),",") + R"()
    <points>   - optional points annotation file in simple YOLO format
OUTPUT:
    displays the image [and landmarks] in a window, allowing for zoom
NOTES:
    if the file <image>.lms.txt exists (and no <points> is specified),
    it will be automatically used to load landmarks.)"
    };
    String8             imgFname = syn.next(),
                        lmsFname = pathToDirBase(imgFname)+".lms.txt";
    ImgRgba8            img = loadImage(imgFname);
    fgout << img;
    NameVec2Fs          lmsIrcs;
    if (syn.more())
        lmsIrcs = loadImageLandmarks(syn.next());
    else if (fileExists(lmsFname))
        lmsIrcs = loadImageLandmarks(lmsFname);
    Vec2Fs              pts;
    if (!lmsIrcs.empty()) {
        PushIndent          pind {"Landmarks"};
        AffineEw2F          xf = cIrcsToIucsXf(img.dims());
        Mat22F              bnds {0,1,0,1};
        for (NameVec2F const & lm : lmsIrcs) {
            fgout << fgnl << lm.name << ": " << lm.vec;
            Vec2F               iucs = xf * lm.vec;
            if (!isInBounds(bnds,iucs))
                fgout << " WARNING point not within image boundaries";
            pts.push_back(iucs);
        }
    }
    viewImage(img,pts);
}

void                cmdViewUvs(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<mesh>.<ext> [<texImage>]+
    <ext> = )" + getMeshLoadExtsCLDescription()
    };
    Meshes              meshes = loadMeshes(syn.next());
    String8s            names;
    ImgRgba8s           images;
    Rgba8               color {0,255,0,255};
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        Mesh const &        mesh = meshes[mm];
        String              meshName = mesh.name.empty() ? toStrDigits(mm,2) : mesh.name.m_str;
        PushIndent          pind {"Mesh "+meshName};
        if (mesh.uvs.empty())
            fgout << fgnl << "WARNING: mesh has no UVs";
        Mat22F              bounds = cBounds(mesh.uvs);
        fgout << fgnl << syn.curr() << " UV Bounds: " << bounds;
        if ((cMinElem(bounds) < 0) || (cMaxElem(bounds) > 1))
            fgout << fgnl << "WARNING: UVs outside [0,1] were not drawn";
        fgout << fgnl << "Surfaces: " << mesh.surfaces.size();
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            Surf const &        surf = mesh.surfaces[ss];
            String              surfName = surf.name.empty() ? toStrDigits(ss,2) : surf.name.m_str;
            ImgRgba8            wi = cUvWireframeImage(mesh.uvs,surf.tris.uvInds,surf.quads.uvInds,color);
            names.push_back(meshName + " - " + surfName);
            if (syn.more())
                images.push_back(composite(wi,loadImage(syn.next())));
            else
                images.push_back(wi);
        }
    }
    viewImages(images,names);
}

Cmds                getViewCmds()
{
    Cmds            cmds {
        {cmdViewMesh,"mesh","Interactively view 3D meshes"},
        {cmdViewImage,"image","Basic image viewer"},
        {cmdViewUvs,"uvs","View the UV layout of a 3D mesh"},
    };
    return cmds;
}

}
