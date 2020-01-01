//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgSoftRender.hpp"
#include "FgSampler.hpp"
#include "Fg3dMesh.hpp"
#include "FgAffineCwC.hpp"
#include "FgGridTriangles.hpp"
#include "FgRayCaster.hpp"
#include "FgMath.hpp"
#include "FgTestUtils.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dCamera.hpp"
#include "FgTime.hpp"
#include "FgMain.hpp"
#include "FgCommand.hpp"
#include "FgImgDisplay.hpp"

using namespace std;
using namespace std::placeholders;

namespace Fg {

ImgC4UC
renderSoft(
    Vec2UI                  pxSz,
    Meshes const &          meshes,
    Affine3D                modelview,
    AffineEw2D              itcsToIucs,
    RenderOptions const &   options)
{
    ImgC4UC             img;
    VecF2               colorBounds = cBounds(options.backgroundColor.m_c.m);
    FGASSERT((colorBounds[0] >= 0.0f) && (colorBounds[1] <= 255.0f));
    RayCaster         rc(meshes,modelview,itcsToIucs,options.lighting,options.backgroundColor);
    // The 'cref' for the 'rc' arg is critical; otherwise 'rc' gets copied on every call:
    img = sampleAdaptive(pxSz,bind(&RayCaster::cast,cref(rc),_1),options.antiAliasBitDepth);

    // Calculate where the surface points land:
    FgProjSurfPoints    spps;
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        const Mesh &        mesh = meshes[mm];
        const Vec3Fs &      verts = rc.vertss[mm];
        const Normals & norms = rc.normss[mm];
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            const Surf &        surf = mesh.surfaces[ss];
            for (size_t ii=0; ii<surf.surfPoints.size(); ++ii) {
                const SurfPoint &   sp = surf.surfPoints[ii];
                FgProjSurfPoint     spp;
                spp.label = sp.label;
                Vec3F               spOecs = surf.surfPointPos(verts,ii),
                                    spNorm = norms.facet[ss].triEquiv(sp.triEquivIdx);
                spp.visible = (cDot(spOecs,spNorm) < 0);           // Point is camera-facing
                Vec3F               spIucs = rc.oecsToIucs(spOecs);
                spp.posIucs = Vec2F(spIucs[0],spIucs[1]);
                if (spIucs[2] > 0) {                                // Point is in front of the camera
                    FgBestN<float,RayCaster::Intersect,4>  intscts = rc.closestIntersects(Vec2F(spIucs[0],spIucs[1]));
                    if (!intscts.empty()) {                         // Point is in view of camaera
                        RayCaster::Intersect  intsct = intscts[0].second;     // First is closest
                        if ((intsct.triInd.meshIdx != mm) ||
                            (intsct.triInd.surfIdx != ss) ||
                            (intsct.triInd.triIdx != sp.triEquivIdx)) {         // Point is occluded
                            spp.visible = false;
                        }
                    }
                    else
                        spp.visible = false;
                }
                else
                    spp.visible = false;
                spps.push_back(spp);
            }
        }
    }
    if (options.projSurfPoints)
        *options.projSurfPoints = spps;
    
    // Paint surface points on image:
    if (options.renderSurfPoints != FgRenderSurfPoints::never) {
        for (const FgProjSurfPoint & spp : spps) {
            if (spp.visible || (options.renderSurfPoints == FgRenderSurfPoints::always)) {
                Vec2F        p = spp.posIucs;
                p[0] *= img.width();
                p[1] *= img.height();
                Vec2I        posIrcs = Vec2I(p);
                img.paint(posIrcs,RgbaUC(0,255,0,255));     // Ingores out-of-bounds points
            }
        }
    }

    return img;
}

void
fgSoftRenderTest(CLArgs const &)
{
    PushDir       pd(dataDir()+"base/test/render/");

    // Set up structures required for rendering:
    Meshes      meshes(1);
    Mesh &      mesh = meshes[0];
    mesh.surfaces.resize(1);
    Surf &   surf = mesh.surfaces[0];
    Affine3D      modelview;      // Default is identity
    AffineEw2D    itcsToIucs(Vec2D(0.5),Vec2D(0.5));
    RenderOptions ro;

    // Model a single triangle of equal width and height intersected by the optical axis in OECS at the barycentric centre:
    mesh.verts = { {-1,1.5,-4}, {-1,-1.5,-4}, {2,0,-4} };
    surf.tris.vertInds = {{0,1,2}};         // CC winding
    surf.surfPoints = fgSvec(SurfPoint(0,Vec3F(1.0f/3.0f)));
    // Test that the point is visible in the current configuration:
    ro.renderSurfPoints = FgRenderSurfPoints::whenVisible;
    ImgC4UC     img = renderSoft(Vec2UI(64),meshes,modelview,itcsToIucs,ro);
    fgRegress<ImgC4UC>(img,"t0.png",bind(fgImgApproxEqual,_1,_2,2U));
    // Flip the winding to test the surface point is not visible from behind:
    surf.tris.vertInds.back() = {1,0,2};
    img = renderSoft(Vec2UI(64),meshes,modelview,itcsToIucs,ro);
    fgRegress<ImgC4UC>(img,"t4.png",bind(fgImgApproxEqual,_1,_2,2U));
    surf.tris.vertInds.back() = {0,1,2};    // Restore
    // Place a triangle just in front to test occlusion of the surface point:
    mesh.verts.push_back( {2,0,-3.9} );
    surf.tris.vertInds.push_back( {0,1,3} );
    img = renderSoft(Vec2UI(64),meshes,modelview,itcsToIucs,ro);
    fgRegress<ImgC4UC>(img,"t3.png",bind(fgImgApproxEqual,_1,_2,2U));


    // Model 2 right angle triangles making a sqaure with a checkerboard color map (preserving aspect ratio):
    mesh.verts = {{-1,1.5,-4}, {-1,-1.5,-4}, {2,1.5,-4}, {2,-1.5,-4}};
    mesh.uvs = {{0,1}, {0,0}, {1,1}, {1,0}};
    surf.tris.vertInds = {{0,1,2}, {2,1,3}};
    surf.tris.uvInds = {{0,1,2}, {2,1,3}};
    ImgC4UC     map(128,128);
    for (Iter2UI it(map.dims()); it.valid(); it.next()) {
        bool        alternate = (it()[0] & 16) != (it()[1] & 16);
        map[it()] = alternate ? RgbaUC(0,0,0,255) : RgbaUC(255,255,255,255);
    }
    surf.material.albedoMap = make_shared<ImgC4UC>(map);
    // View undistorted checkerboard flat on:
    img = renderSoft(Vec2UI(256),meshes,modelview,itcsToIucs,ro);
    fgRegress<ImgC4UC>(img,"t1.png",bind(fgImgApproxEqual,_1,_2,2U));
    // View at an angle to see perspective distortion:
    modelview = Affine3D(Vec3D(0,0,-4)) * Affine3D(matRotateY(1.0)) * Affine3D(Vec3D(0,0,4));
    img = renderSoft(Vec2UI(256),meshes,modelview,itcsToIucs,ro);
    fgRegress<ImgC4UC>(img,"t2.png",bind(fgImgApproxEqual,_1,_2,2U));
}

Cmd
fgSoftRenderTestInfo()
{return Cmd(fgSoftRenderTest,"rend","renderSoft function"); }

}

// */
