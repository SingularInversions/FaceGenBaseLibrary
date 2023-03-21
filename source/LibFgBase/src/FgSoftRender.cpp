//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgSoftRender.hpp"
#include "FgGeometry.hpp"
#include "FgSampler.hpp"
#include "Fg3dMesh.hpp"
#include "FgAffine.hpp"
#include "FgGridTriangles.hpp"
#include "FgMath.hpp"
#include "FgTestUtils.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dCamera.hpp"
#include "FgTime.hpp"
#include "FgMain.hpp"
#include "FgCommand.hpp"
#include "FgImgDisplay.hpp"
#include "FgBestN.hpp"
#include "FgGridIndex.hpp"

using namespace std;
using namespace std::placeholders;

namespace Fg {

// Ray-casting requires caching the projected coordinates as well as their mesh and surface indices:
struct  RayCaster
{
    Svec<TriIndss>          trisss;         // By mesh, by surface
    Materialss              materialss;     // By mesh, by surface
    Vec3Fss                 vertss;         // By mesh, in OECS
    Svec<Vec2Fs const *>    uvsPtrs;        // By mesh, in OTCS
    MeshNormalss            normss;         // By mesh, in OECS
    AffineEw2D              itcsToIucs;
    Vec3Fss                 iucsVertss;     // By mesh, X,Y in IUCS, Z component is inverse FCCS depth
    GridIndex<TriIdxSM>     grid;           // Index from IUCS to bin of TriIdxSMs
    Lighting                lighting;
    RgbaF                   background;     // channels [0,1], alpha-weighted
    Vec2UI                  imgDims;
    bool                    useMaps = true;
    bool                    allShiny = false;

    RayCaster(
        Meshes const &      meshes,
        SimilarityD         modelview,          // to OECS
        AffineEw2D          itcsToIucs_,
        Lighting const &    lighting_,          // in OECS
        RgbaF               background_,        // must be alpha-weighted
        Vec2UI              dims,
        bool                useMaps_=true,
        bool                allShiny_=true)
        :
        itcsToIucs(itcsToIucs_),
        // TODO: set up grid only after seeing how many verts fall in frustum, possibly use smaller grid size,
        // and what their bounding box is for setting client to grid transform:
        grid {Mat22F{0,1,0,1},cNumTriEquivs(meshes)},
        lighting(lighting_),
        background(background_),
        imgDims {dims},
        useMaps(useMaps_),
        allShiny(allShiny_)
    {
        trisss.resize(meshes.size());
        materialss.resize(meshes.size());
        vertss.resize(meshes.size());
        uvsPtrs.resize(meshes.size());
        normss.resize(meshes.size());
        iucsVertss.resize(meshes.size());
        for (size_t mm=0; mm<meshes.size(); ++mm) {
            Mesh const &    mesh = meshes[mm];
            TriIndss &           triss = trisss[mm];
            Materials &       materials = materialss[mm];
            triss.reserve(mesh.surfaces.size());
            materials.reserve(mesh.surfaces.size());
            for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
                triss.push_back(mesh.surfaces[ss].getTriEquivs());
                materials.push_back(mesh.surfaces[ss].material);
            }
            Vec3Fs &           verts = vertss[mm];
            verts = mapMul(Affine3F{modelview.asAffine()},mesh.verts);
            uvsPtrs[mm] = &mesh.uvs;
            normss[mm] = cNormals(mesh.surfaces,verts);
            Vec3Fs &           iucsVerts = iucsVertss[mm];
            iucsVerts.reserve(verts.size());
            for (Vec3F v : verts)
                iucsVerts.push_back(oecsToIucs(v));
            for (size_t ss=0; ss<triss.size(); ++ss) {
                TriInds const &  tris = triss[ss];
                for (size_t tt=0; tt<tris.vertInds.size(); ++tt) {
                    Vec3UI       t = tris.vertInds[tt];
                    Vec3F        v0 = iucsVerts[t[0]],
                                    v1 = iucsVerts[t[1]],
                                    v2 = iucsVerts[t[2]];
                    if ((v0[2] > 0.0f) && (v1[2] > 0.0f) && (v2[2] > 0.0f)) {   // Only render tris fully in front of camera
                        Mat22F    bnds;
                        bnds[0] = cMin(v0[0],v1[0],v2[0]);
                        bnds[1] = cMax(v0[0],v1[0],v2[0]);
                        bnds[2] = cMin(v0[1],v1[1],v2[1]);
                        bnds[3] = cMax(v0[1],v1[1],v2[1]);
                        grid.add(TriIdxSM(tt,ss,mm),bnds);
                    }
                }
            }
        }
    }

    // Values in [0,1] within precision:
    RgbaF               cast(Vec2F ipcs) const
    {
        Vec2F               posIucs {ipcs[0]/imgDims[0],ipcs[1]/imgDims[1]};
        BestN<float,Intersect,8> best = closestIntersects(posIucs);
        // Compute ray color:
        RgbaF               color = background;
        for (size_t ii=best.size(); ii>0; --ii) {             // Render back to front
            Intersect           isct = best[ii-1].second;
            TriInds const &     tris = trisss[isct.triInd.meshIdx][isct.triInd.surfIdx];
            Material            material = materialss[isct.triInd.meshIdx][isct.triInd.surfIdx];
            MeshNormals const & norms = normss[isct.triInd.meshIdx];
            Vec3UI              vis = tris.vertInds[isct.triInd.triIdx];
            // TODO: Use perspective-correct normal and UV interpolation (makes very little difference for small tris):
            Vec3F               n0 = norms.vert[vis[0]],
                                n1 = norms.vert[vis[1]],
                                n2 = norms.vert[vis[2]],
                                bc = Vec3F(isct.barycentric),
                                norm = normalize(bc[0]*n0 + bc[1]*n1 + bc[2]*n2);
            RgbaF               albedo {0.9,0.9,0.9,1};
            Vec2Fs const &      uvs = *uvsPtrs[isct.triInd.meshIdx];
            Vec2F               uv {lims<float>::max()};
            if ((!tris.uvInds.empty()) && (!uvs.empty()) && (material.albedoMap) &&
                (!material.albedoMap->empty()) && useMaps) {
                Vec3UI              uvInds = tris.uvInds[isct.triInd.triIdx];
                uv = bc[0]*uvs[uvInds[0]] + bc[1]*uvs[uvInds[1]] + bc[2]*uvs[uvInds[2]];
                uv[1] = 1.0f - uv[1];   // OTCS to IUCS
                albedo = RgbaF(sampleClipIucs(*material.albedoMap,uv)/255.0f);
            }
            Vec3F               acc(0.0f);
	        float	            aw = albedo.alpha();
            Vec3F               surfColour = Vec3F(cHead<3>(albedo.m_c)) * aw;
            for (size_t ll=0; ll<lighting.lights.size(); ++ll) {
                Light               lgt = lighting.lights[ll];
                float               fac = cDot(norm,lgt.direction);
                if (fac > 0.0f) {
                    acc += mapMul(surfColour,lgt.colour) * fac;
                    float               shininess = material.shiny ? 1.0f : 0.0f;
                    if ((uv[0] != lims<float>::max()) && material.specularMap && !material.specularMap->empty()) {
                        RgbaF           s = sampleClipIucs(*material.specularMap,uv);
                        shininess = scast<float>(s.red()) / 255.0f;
                    }
                    if (allShiny)
                        shininess = 1.0f;
                    if (shininess > 0.0f) {
                        Vec3F           reflectDir = norm * fac * 2.0f - lgt.direction;
                        if (reflectDir[2] > 0.0f) {
                            float       deltaSqr = sqr(reflectDir[0]) + sqr(reflectDir[1]),
                                        val = exp(-deltaSqr * 32.0f);
                            acc += Vec3F(val);
                        }
                    }
                }
            }
            acc += mapMul(surfColour,lighting.ambient);
            RgbaF           isctColor = RgbaF(acc[0],acc[1],acc[2],albedo.alpha());
            float           cfac = (1.0f - isctColor.alpha());
            color = isctColor + color * cfac;
         }
         return color;
    }

    // Return value depth component is inverse depth if visible and >0, negative otherwise:
    Vec3F           oecsToIucs(Vec3F posOecs) const;

    struct      Intersect
    {
        TriIdxSM            triInd;
        Vec3D               barycentric;

        Intersect() {}
        Intersect(TriIdxSM ti,Vec3D bc) : triInd(ti), barycentric(bc) {}
    };

    // Return closest tri intersects for given ray:
    BestN<float,RayCaster::Intersect,8> closestIntersects(Vec2F posIucs) const
    {
        TriIdxSMs const &     triInds = grid[posIucs];
        BestN<float,Intersect,8> best;
        for (TriIdxSM ti : triInds) {
            TriInds const &        tris = trisss[ti.meshIdx][ti.surfIdx];
            Vec3UI              vis = tris.vertInds[ti.triIdx];
            Vec3Fs const &      iucsVerts = iucsVertss[ti.meshIdx];
            Vec3F               v0 = iucsVerts[vis[0]],
                                v1 = iucsVerts[vis[1]],
                                v2 = iucsVerts[vis[2]];
            Vec2D               u0(v0[0],v0[1]),
                                u1(v1[0],v1[1]),
                                u2(v2[0],v2[1]);
            // TODO: make a float version of fgBarycentriCoords:
            Opt<Vec3D>          bco = cBarycentricCoord(Vec2D(posIucs),u0,u1,u2);
            if (bco.valid()) {       // TODO: filter out degenerate projected tris during cache setup
                Vec3D               bc = bco.val();
                // TODO: Use a consistent intersection policy to ensure only 1 tri of an edge-connected pair
                // is ever intersected:
                if ((bc[0] >= 0) && (bc[1] >= 0) && (bc[2] >= 0)) {     // Point landed on triangle:
                    double          id = bc[0]*v0[2] + bc[1]*v1[2] + bc[2]*v2[2];   // Interpolate inverse depth
                    best.update(id,Intersect(ti,bc));
                }
            }
        }
        return best;
    }

};

Vec3F
RayCaster::oecsToIucs(Vec3F posOecs) const
{
    // Inverse depth value (map to -1 for points behind camera (OECS)):
    double          id = (posOecs[2] == 0.0f) ? -1.0 : (-1.0 / posOecs[2]);
    Vec2D           itcs(posOecs[0]*id,-posOecs[1]*id),     // Both Y and Z change sign in OECS -> ITCS
                    iucs = itcsToIucs * itcs;
    return Vec3F(iucs[0],iucs[1],id);
}

ImgRgba8            renderSoft(
    Vec2UI                  pxSz,
    Meshes const &          meshes,
    SimilarityD             modelview,
    AffineEw2D              itcsToIucs,
    RenderOptions const &   options)
{
    VecF2                   colorBounds = cBounds(options.backgroundColor.m_c);
    FGASSERT((colorBounds[0] >= 0.0f) && (colorBounds[1] <= 255.0f));
    RayCaster               rc {meshes,modelview,itcsToIucs,
        options.lighting,
        options.backgroundColor / 255.0f,
        pxSz,
        options.useMaps,options.allShiny
    };
    // The 'cref' for the 'rc' arg is critical; otherwise 'rc' gets copied on every call:
    ImgC4F      rend = sampleAdaptiveF(pxSz,bind(&RayCaster::cast,cref(rc),_1));
    ImgRgba8    img = toRgba8(rend);
    // Calculate where the surface points land:
    ProjectedSurfPoints     spps;
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        Mesh const &            mesh = meshes[mm];
        Vec3Fs const &          verts = rc.vertss[mm];
        MeshNormals const &     norms = rc.normss[mm];
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            Surf const &            surf = mesh.surfaces[ss];
            for (size_t ii=0; ii<surf.surfPoints.size(); ++ii) {
                SurfPointName const &       sp = surf.surfPoints[ii];
                ProjectedSurfPoint      spp;
                spp.label = sp.label;
                Vec3F               spOecs = surf.surfPointPos(verts,ii),
                                    spNorm = norms.facet[ss].triEquiv(sp.point.triEquivIdx);
                spp.visible = (cDot(spOecs,spNorm) < 0);           // Point is camera-facing
                Vec3F               spIucs = rc.oecsToIucs(spOecs);
                spp.posIucs = Vec2F(spIucs[0],spIucs[1]);
                if (spIucs[2] > 0) {                                // Point is in front of the camera
                    BestN<float,RayCaster::Intersect,8>  intscts = rc.closestIntersects(Vec2F(spIucs[0],spIucs[1]));
                    if (!intscts.empty()) {                         // Point is in view of camaera
                        RayCaster::Intersect  intsct = intscts[0].second;     // First is closest
                        if ((intsct.triInd.meshIdx != mm) ||
                            (intsct.triInd.surfIdx != ss) ||
                            (intsct.triInd.triIdx != sp.point.triEquivIdx)) {         // Point is occluded
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
    // Composite surface points:
    if (options.renderSurfPoints != RenderSurfPoints::never) {
        for (const ProjectedSurfPoint & spp : spps) {
            if (spp.visible || (options.renderSurfPoints == RenderSurfPoints::always)) {
                Vec2F               p = spp.posIucs;
                p[0] *= img.width();
                p[1] *= img.height();
                Vec2I               posIrcs = Vec2I(p);
                Rgba8               color {mapCast<uchar>(options.surfPointColor.m_c * 255.f)};
                paintDot(img,posIrcs,color,2);
                img.paint(posIrcs,color);           // Ignores out-of-bounds points
            }
        }
    }
    return img;
}

ImgRgba8            renderSoft(Vec2UI pixelSize,Meshes const & meshes,RgbaF bgColor)
{
    CameraParams        camPrms {Mat32D(cBounds(meshes))};
    Camera              camera = camPrms.camera(pixelSize);
    RenderOptions       ro;
    ro.backgroundColor = bgColor / 255.0f;
    return renderSoft(pixelSize,meshes,camera.modelview,camera.itcsToIucs,ro);
}

static void         testSoftRender(CLArgs const &)
{
    String          relPath = "base/test/render/";
    PushDir         pd {dataDir()+relPath};

    // Set up structures required for rendering:
    Meshes          meshes(1);
    Mesh &          mesh = meshes[0];
    mesh.surfaces.resize(1);
    Surf &          surf = mesh.surfaces[0];
    SimilarityD     modelview;      // Default is identity
    AffineEw2D      itcsToIucs(Vec2D(0.5),Vec2D(0.5));
    RenderOptions   ro;

    // Model a single triangle of equal width and height intersected by the optical axis
    // in OECS at the barycentric centre:
    mesh.verts = { {-1,1.5,-4}, {-1,-1.5,-4}, {2,0,-4} };
    surf.tris.vertInds = {{0,1,2}};         // CC winding
    surf.surfPoints = {{0,Vec3F{1.0f/3.0f}}};
    // Test that the point is visible in the current configuration:
    ro.renderSurfPoints = RenderSurfPoints::whenVisible;
    auto            isApproxEqualFn = [](ImgRgba8 const & l,ImgRgba8 const & r)
    {
        return isApproxEqual(l,r,20);   // TODO: why is macos result so different ???
    };
    ImgRgba8        img = renderSoft(Vec2UI(64),meshes,modelview,itcsToIucs,ro);
    testRegressApprox<ImgRgba8>(img,relPath+"t0.png",isApproxEqualFn);
    // Flip the winding to test the surface point is not visible from behind:
    surf.tris.vertInds.back() = {1,0,2};
    img = renderSoft(Vec2UI(64),meshes,modelview,itcsToIucs,ro);
    testRegressApprox<ImgRgba8>(img,relPath+"t4.png",isApproxEqualFn);
    surf.tris.vertInds.back() = {0,1,2};    // Restore
    // Place a triangle just in front to test occlusion of the surface point:
    mesh.verts.emplace_back(2,0,-3.9f);
    surf.tris.vertInds.emplace_back(0,1,3);
    img = renderSoft(Vec2UI(64),meshes,modelview,itcsToIucs,ro);
    testRegressApprox<ImgRgba8>(img,relPath+"t3.png",isApproxEqualFn);


    // Model 2 right angle triangles making a sqaure with a checkerboard color map (preserving aspect ratio):
    mesh.verts = {{-1,1.5,-4}, {-1,-1.5,-4}, {2,1.5,-4}, {2,-1.5,-4}};
    mesh.uvs = {{0,1}, {0,0}, {1,1}, {1,0}};
    surf.tris.vertInds = {{0,1,2}, {2,1,3}};
    surf.tris.uvInds = {{0,1,2}, {2,1,3}};
    ImgRgba8     map(128,128);
    for (Iter2UI it(map.dims()); it.valid(); it.next()) {
        bool        alternate = (it()[0] & 16) != (it()[1] & 16);
        map[it()] = alternate ? Rgba8(0,0,0,255) : Rgba8(255,255,255,255);
    }
    surf.material.albedoMap = make_shared<ImgRgba8>(map);
    // View undistorted checkerboard flat on:
    img = renderSoft(Vec2UI(256),meshes,modelview,itcsToIucs,ro);
    testRegressApprox<ImgRgba8>(img,relPath+"t1.png",isApproxEqualFn);
    // View at an angle to see perspective distortion:
    modelview = SimilarityD(Vec3D(0,0,-4)) * SimilarityD(cRotateY(1.0)) * SimilarityD(Vec3D(0,0,4));
    img = renderSoft(Vec2UI(256),meshes,modelview,itcsToIucs,ro);
    testRegressApprox<ImgRgba8>(img,relPath+"t2.png",isApproxEqualFn);
}

Cmd                 testSoftRenderInfo()
{
    return Cmd(testSoftRender,"rend","renderSoft function");
}

}

// */
