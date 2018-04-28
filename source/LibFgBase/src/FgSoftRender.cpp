//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:		Andrew Beatty
// Created:		April 7, 2010
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

FgImgRgbaUb
fgRenderSoft(
    FgVect2UI                   pxSz,
    const Fg3dMeshes &          meshes,
    FgAffine3D                  modelview,
    FgAffineCw2D                itcsToIucs,
    const FgRenderOptions &     options)
{
    FgImgRgbaUb             img;
    FgVectF2                colorBounds = fgBounds(options.backgroundColor.m_c);
    FGASSERT((colorBounds[0] >= 0.0f) && (colorBounds[1] <= 255.0f));
    FgRayCaster             rc(meshes,modelview,itcsToIucs,options.lighting,options.backgroundColor);
    // The 'boost::cref' for the 'rc' arg is critical; otherwise 'rc' gets copied on every call:
    img = fgSampler(pxSz,boost::bind(&FgRayCaster::cast,boost::cref(rc),_1),options.antiAliasBitDepth);

    // Calculate where the surface points land:
    FgProjSurfPoints          spps;
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        const Fg3dMesh &    mesh = meshes[mm];
        const FgVerts &     verts = rc.vertss[mm];
        const Fg3dNormals & norms = rc.normss[mm];
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            const Fg3dSurface &     surf = mesh.surfaces[ss];
            for (size_t ii=0; ii<surf.surfPoints.size(); ++ii) {
                const FgSurfPoint &     sp = surf.surfPoints[ii];
                FgProjSurfPoint         spp;
                spp.label = sp.label;
                FgVect3F                spOecs = surf.surfPointPos(verts,ii),
                                        spNorm = norms.facet[ss].triEquiv(sp.triEquivIdx);
                spp.visible = (fgDot(spOecs,spNorm) < 0);           // Point is camera-facing
                FgVect3F                spIucs = rc.oecsToIucs(spOecs);
                spp.posIucs = FgVect2F(spIucs[0],spIucs[1]);
                if (spIucs[2] > 0) {                                // Point is in front of the camera
                    FgBestN<float,FgRayCaster::Intersect,4>  intscts = rc.closestIntersects(FgVect2F(spIucs[0],spIucs[1]));
                    if (!intscts.empty()) {                         // Point is in view of camaera
                        FgRayCaster::Intersect  intsct = intscts[0].second;     // First is closest
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
                FgVect2F        p = spp.posIucs;
                p[0] *= img.width();
                p[1] *= img.height();
                FgVect2I        posIrcs = FgVect2I(p);
                img.paint(posIrcs,FgRgbaUB(0,255,0,255));     // Ingores out-of-bounds points
            }
        }
    }

    return img;
}

void
fgSoftRenderTest(const FgArgs &)
{
    FgPushDir       pd(fgDataDir()+"base/test/render/");

    // Set up structures required for rendering:
    Fg3dMeshes      meshes(1);
    Fg3dMesh &      mesh = meshes[0];
    mesh.surfaces.resize(1);
    Fg3dSurface &   surf = mesh.surfaces[0];
    FgAffine3D      modelview;      // Default is identity
    FgAffineCw2D    itcsToIucs(FgVect2D(0.5),FgVect2D(0.5));
    FgRenderOptions ro;

    // Model a single triangle of equal width and height intersected by the optical axis in OECS at the barycentric centre:
    mesh.verts = { {-1,1.5,-4}, {-1,-1.5,-4}, {2,0,-4} };
    surf.tris.vertInds = {{0,1,2}};         // CC winding
    surf.surfPoints = fgSvec(FgSurfPoint(0,FgVect3F(1.0f/3.0f)));
    // Test that the point is visible in the current configuration:
    ro.renderSurfPoints = FgRenderSurfPoints::whenVisible;
    FgImgRgbaUb     img = fgRenderSoft(FgVect2UI(64),meshes,modelview,itcsToIucs,ro);
    fgRegress<FgImgRgbaUb>(img,"t0.png",boost::bind(fgImgApproxEqual,_1,_2,2U));
    // Flip the winding to test the surface point is not visible from behind:
    surf.tris.vertInds.back() = {1,0,2};
    img = fgRenderSoft(FgVect2UI(64),meshes,modelview,itcsToIucs,ro);
    fgRegress<FgImgRgbaUb>(img,"t4.png",boost::bind(fgImgApproxEqual,_1,_2,2U));
    surf.tris.vertInds.back() = {0,1,2};    // Restore
    // Place a triangle just in front to test occlusion of the surface point:
    mesh.verts.push_back( {2,0,-3.9} );
    surf.tris.vertInds.push_back( {0,1,3} );
    img = fgRenderSoft(FgVect2UI(64),meshes,modelview,itcsToIucs,ro);
    fgRegress<FgImgRgbaUb>(img,"t3.png",boost::bind(fgImgApproxEqual,_1,_2,2U));


    // Model 2 right angle triangles making a sqaure with a checkerboard color map (preserving aspect ratio):
    mesh.verts = {{-1,1.5,-4}, {-1,-1.5,-4}, {2,1.5,-4}, {2,-1.5,-4}};
    mesh.uvs = {{0,1}, {0,0}, {1,1}, {1,0}};
    surf.tris.vertInds = {{0,1,2}, {2,1,3}};
    surf.tris.uvInds = {{0,1,2}, {2,1,3}};
    FgImgRgbaUb     map(128,128);
    for (FgIter2UI it(map.dims()); it.valid(); it.next()) {
        bool        alternate = (it()[0] & 16) != (it()[1] & 16);
        map[it()] = alternate ? FgRgbaUB(0,0,0,255) : FgRgbaUB(255,255,255,255);
    }
    surf.material.albedoMap = boost::make_shared<FgImgRgbaUb>(map);
    // View undistorted checkerboard flat on:
    img = fgRenderSoft(FgVect2UI(256),meshes,modelview,itcsToIucs,ro);
    fgRegress<FgImgRgbaUb>(img,"t1.png",boost::bind(fgImgApproxEqual,_1,_2,2U));
    // View at an angle to see perspective distortion:
    modelview = FgAffine3D(FgVect3D(0,0,-4)) * FgAffine3D(fgMatRotateY(1.0)) * FgAffine3D(FgVect3D(0,0,4));
    img = fgRenderSoft(FgVect2UI(256),meshes,modelview,itcsToIucs,ro);
    fgRegress<FgImgRgbaUb>(img,"t2.png",boost::bind(fgImgApproxEqual,_1,_2,2U));
}

FgCmd
fgSoftRenderTestInfo()
{return FgCmd(fgSoftRenderTest,"rend","fgRenderSoft function"); }

// */
