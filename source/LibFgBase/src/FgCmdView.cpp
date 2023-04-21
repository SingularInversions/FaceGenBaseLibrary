//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dDisplay.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dMesh.hpp"

#include "FgImgDisplay.hpp"
#include "FgImageDraw.hpp"
#include "FgAffine.hpp"
#include "FgBuild.hpp"
#include "FgApproxEqual.hpp"

using namespace std;

namespace Fg {

namespace {

void                cmdViewImage(CLArgs const & args)
{
    Syntax              syn {args,
        R"((<image>.<ext> [<points>.txt])+
    <ext>      - ()" + cat(getImgExts(),",") + R"()
    <points>   - optional points annotation file in simple YOLO format
OUTPUT:
    displays the image(s) [and landmarks] in a window, allowing for zoom
NOTES:
    if the file <image>.lms.txt exists (and no <points> is specified),
    it will be automatically used to load landmarks.)"
    };
    AnnotatedImgs       ais;
    do {
        AnnotatedImg        ai;
        String8             imgFname = syn.next(),
                            lmsFname = pathToDirBase(imgFname)+".lms.txt";
        ai.name = pathToBase(imgFname);
        ai.img = loadImage(imgFname);
        NameVec2Fs          lmsIrcs;
        if (syn.more() && (toLower(pathToExt(syn.peekNext())) == "txt"))
            lmsIrcs = loadLandmarks(syn.next());
        else if (fileExists(lmsFname))
            lmsIrcs = loadLandmarks(lmsFname);
        if (!lmsIrcs.empty()) {
            PushIndent          pind {imgFname.m_str+" Landmarks:"};
            Mat22F              bnds {0,scast<float>(ai.img.width()),0,scast<float>(ai.img.height())};
            for (NameVec2F const & lm : lmsIrcs) {
                fgout << fgnl << lm.name << ": " << lm.vec;
                if (!isInBounds(bnds,lm.vec))
                    fgout << " WARNING point not within image boundaries";
                ai.ptsIrcs.push_back(lm.vec);
            }
        }
        ais.push_back(ai);
    } while (syn.more());
    if (ais.size() == 1)
        viewImage(ais[0].img,ais[0].ptsIrcs);
    else
        viewImages(ais);
}

void                cmdViewDeltas(CLArgs const & args)
{
    Syntax              syn {args,R"(<base>.<ext> <targ>.<ext> [<epsBits>]
    <base>          - base shape mesh and vertex list. Must have UVs.
    <targ>          - comparison vertex list. Must be same size as base.
    <ext>           - )" + getMeshLoadExtsCLDescription() + R"(
    <epsBits>       - bit depth relative to max verts spread axis to consider the delta insignificant.
                      defaults to 17 (ie. 1 part in 131,072).
NOTES:
    * Displays the mesh with significantly modified vertices (position) in red
    * TRIS ONLY !)"
    };
    Mesh                base = loadMesh(syn.next()),
                        targ = loadMesh(syn.next());
    size_t              bits = 17;
    if (syn.more())
        bits = syn.nextAs<size_t>();
    size_t              V = base.verts.size();
    if (targ.verts.size() != V)
        fgThrow("<base> and <targ> vertex lists are different sizes");
    float               maxDim = cMaxElem(cDims(base.verts)),
                        thresh = epsBits(bits) * maxDim;
    TriInds             triInds = merge(sliceMember(base.surfaces,&Surf::tris));
    ImgRgba8            map {1024,1024,Rgba8{200,200,200,255}};
    Rgba8               color {255,0,0,255};
    size_t              sigDels {0},
                        epsDels {0},
                        exacts {0};
    for (size_t vv=0; vv<V; ++vv) {
        float               del = cMaxElem(mapAbs(targ.verts[vv]-base.verts[vv]));
        if (del == 0.0f)
            ++exacts;
        else if (del < thresh)
            ++epsDels;
        else {
            ++sigDels;
            markVertOnMap(base.uvs,triInds,vv,color,map);
        }
    }
    for (Surf & surf : base.surfaces)
        surf.setAlbedoMap(map);
    fgout
        << fgnl << "Identical vertices:     " << exacts
        << fgnl << "Approx identical verts: " << epsDels
        << fgnl << "Changed verts:          " << sigDels;
    viewMesh(base);
}

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
                    ruFlag = false;
    while (syn.peekNext()[0] == '-') {
        if (syn.next() == "-c")
            compare = true;
        else if (syn.curr() == "-r")
            ruFlag = true;
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
            if (ruFlag) {
                size_t          origVerts = mesh.verts.size();
                mesh = removeUnused(mesh);
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
                        albedo = applyTransparencyPow2(albedo,trans);
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
    viewMesh(all,compare);
}

void                cmdViewUvs(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<mesh>.<ext> [<texImage>]+
    <ext> = )" + getMeshLoadExtsCLDescription()
    };
    Meshes              meshes = loadMeshes(syn.next());
    String8s            names;
    AnnotatedImgs       ais;
    Rgba8               color {0,255,0,255};
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        Mesh const &        mesh = meshes[mm];
        String              meshName = mesh.name.empty() ? toStrDigits(mm,2) : mesh.name.m_str;
        PushIndent          pind {"Mesh "+meshName};
        if (mesh.uvs.empty()) {
            fgout << fgnl << "Mesh has no UVs";
            return;         // errors will result otherwise
        }
        Mat22F              bounds = cBounds(mesh.uvs);
        fgout << fgnl << syn.curr() << " UV Bounds: " << bounds;
        if ((cMinElem(bounds) < 0) || (cMaxElem(bounds) > 1))
            fgout << fgnl << "WARNING: UVs outside [0,1] were not drawn";
        fgout << fgnl << "Surfaces: " << mesh.surfaces.size();
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            Surf const &        surf = mesh.surfaces[ss];
            String              surfName = surf.name.empty() ? toStrDigits(ss,2) : surf.name.m_str;
            ImgRgba8            wi = cUvWireframeImage(mesh.uvs,surf.tris.uvInds,surf.quads.uvInds,color);
            String8             name = meshName + " - " + surfName;
            if (syn.more())
                ais.push_back({composite(wi,loadImage(syn.next())),Vec2Fs{},name});
            else
                ais.push_back({wi,Vec2Fs{},name});
        }
    }
    viewImages(ais);
}

}

Cmds                getViewCmds()
{
    Cmds            cmds {
        {cmdViewDeltas,"deltas","View vertex deltas between meshes with 1-1 vertices"},
        {cmdViewImage,"image","Basic image viewer"},
        {cmdViewMesh,"mesh","Interactively view 3D meshes"},
        {cmdViewUvs,"uvs","View the UV layout of a 3D mesh"},
    };
    return cmds;
}

}
