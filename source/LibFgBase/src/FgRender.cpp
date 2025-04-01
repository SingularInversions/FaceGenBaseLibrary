//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgRender.hpp"
#include "FgGeometry.hpp"
#include "Fg3dMesh.hpp"
#include "FgTransform.hpp"
#include "FgGridIndex.hpp"
#include "FgMath.hpp"
#include "FgTestUtils.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgCamera.hpp"
#include "FgTime.hpp"
#include "FgMain.hpp"
#include "FgCommand.hpp"
#include "FgImgDisplay.hpp"
#include "FgBestN.hpp"
#include "FgGridIndex.hpp"

using namespace std;
using namespace std::placeholders;

namespace Fg {

Strings const &    rspInfo()
{
    static Strings  ret
    {
        "never",
        "whenVisible",
        "always",
    };
    return ret;
}

std::any            toReflect(RenderSurfPoints r)
{
    return rspInfo().at(scast<size_t>(r));
}

void                fromReflect_(std::any const & a,RenderSurfPoints & r)
{
    String              str = fromAny<String>(a);
    size_t              idx = findFirstIdx(rspInfo(),str);
    if (idx < rspInfo().size())
        r = scast<RenderSurfPoints>(idx);
    else
        fgThrow("fromReflect_ for RenderSurfPoints invalid string",str);
}

static RgbaF        sampleRecurse(
    Vec2UI                  ircs,
    SampleFn const &        sampleFn,
    Vec2F                   lc,             // Lower corner sample point
    Vec2F                   uc,             // Upper corner sample point (IUCS so not square)
    Mat<RgbaF,2,2>          bs,             // Bounds samples. No speedup at all from making this const ref
    float                   maxDiff,
    // Invalid if first channel is lims<float>::max(), othwerise gives sl/st from previous call.
    // Returned values must be sr/sb resp. for use in raster order next calls.
    // This optimization only saved 5% on time (granted for very simple raycaster)
    // so likely not worth saving values across deeper recursion boundaries.
    RgbaF &                 slo,
    RgbaF &                 sto)
{
    Vec2F           del = uc-lc,
                    hdel = del*0.5f,
                    centre = lc + hdel,
                    hdelx {hdel[0],0.0f},
                    hdely {0.0f,hdel[1]};
    RgbaF           sc {sampleFn(ircs,centre)};
    // recurse on all or none of the quads is easier since they share boundary samples.
    // if recursion from the top level happens 'r' times in all branches of the quadtree, and n=r+1,
    // the ratio of border to total sample weights is (2^n - 1) / 4^n, which becomes small with large r.
    // Also note that the highest such ratio only occurs when there is a sufficiently small difference
    // between the sample values, so blurring is minimized.
    if (
        (isApproxEqual(sc,bs[0],maxDiff)) &&
        (isApproxEqual(sc,bs[1],maxDiff)) &&
        (isApproxEqual(sc,bs[2],maxDiff)) &&
        (isApproxEqual(sc,bs[3],maxDiff)))
    {
        slo[0] = lims<float>::max();    // These invalidations are only needed at the top level, not in recursed calls
        sto[0] = lims<float>::max();
        return (bs[0]+bs[1]+bs[2]+bs[3]) * 0.125f + sc * 0.5f;
    }
    RgbaF           sst {lims<float>::max()},
                    ssl {lims<float>::max()},
                    ssr {lims<float>::max()},
                    ssb {lims<float>::max()};
    float           md2 = maxDiff * 2.0f;
    RgbaF           sl = (slo[0] == lims<float>::max()) ? sampleFn(ircs,centre-hdelx) : slo,
                    sr = sampleFn(ircs,centre+hdelx),
                    st = (sto[0] == lims<float>::max()) ? sampleFn(ircs,centre-hdely) : sto,
                    sb = sampleFn(ircs,centre+hdely),
                    stl = sampleRecurse(ircs,sampleFn,lc,centre,{bs[0],st,sl,sc},md2,sst,ssl),
                    str = sampleRecurse(ircs,sampleFn,lc+hdelx,centre+hdelx,{st,bs[1],sc,sr},md2,sst,ssr),
                    sbl = sampleRecurse(ircs,sampleFn,lc+hdely,centre+hdely,{sl,sc,bs[2],sb},md2,ssb,ssl),
                    sbr = sampleRecurse(ircs,sampleFn,centre,uc,{sc,sr,sb,bs[3]},md2,ssb,ssr);
    slo = sr;
    sto = sb;
    return (stl+str+sbl+sbr) * 0.25f;
}

ImgC4F              sampleAdaptiveF(
    Vec2UI              dims,
    SampleFn const &    sampleFn,
    float               channelBound,
    uint                antiAliasBitDepth)
{
    FGASSERT(dims.elemsProduct() > 0);
    FGASSERT(sampleFn);
    FGASSERT((antiAliasBitDepth > 0) && (antiAliasBitDepth <= 16));
    ImgC4F              img {dims};
    float               maxDiff = channelBound / float(1 << antiAliasBitDepth);
    ImgC4F              sampleLines {img.width()+1,2};
    for (uint xx=0; xx<dims[0]; ++xx)
        sampleLines.xy(xx,0) = sampleFn(Vec2UI{xx,0},Vec2F(xx,0));
    // last sample is for the raster coordinate 1 less:
    sampleLines.xy(dims[0],0) = sampleFn(Vec2UI{dims[0]-1,0},Vec2F(dims[0],0));
    RgbaFs              ssts (img.width(),RgbaF{lims<float>::max()});
    for (uint yy=0; yy<img.height(); ++yy) {
        float               yyf0 = yy,
                            yyf1 = yy+1;
        uint                fbit = yy%2,
                            sbit = 1-fbit;
        for (uint xx=0; xx<dims[0]; ++xx)
            sampleLines.xy(xx,sbit) = sampleFn(Vec2UI{xx,yy},Vec2F(xx,yyf1));
        // last sample is for the raster coordinate 1 less:
        sampleLines.xy(dims[0],sbit) = sampleFn(Vec2UI{dims[0]-1,yy},Vec2F(dims[0],yyf1));
        RgbaF               ssl {lims<float>::max()};
        for (uint xx=0; xx<img.width(); ++xx) {
            Vec2F               lc (xx,yyf0),
                                uc (xx+1,yyf1);
            Mat<RgbaF,2,2>      bs {
                sampleLines.xy(xx,fbit),
                sampleLines.xy(xx+1,fbit),
                sampleLines.xy(xx,sbit),
                sampleLines.xy(xx+1,sbit)
            };
            img.xy(xx,yy) = sampleRecurse(Vec2UI{xx,yy},sampleFn,lc,uc,bs,maxDiff,ssl,ssts[xx]);
        }
    }
    return img;
}

// Ray-casting requires caching the projected coordinates as well as their mesh and surface indices:
struct  RayCaster
{
    Svec<TriIndss>          trisss;         // By mesh, by surface
    Materialss              materialss;     // By mesh, by surface
    Vec3Fss                 vertss;         // By mesh, in OECS
    Svec<Vec2Fs const *>    uvsPtrs;        // By mesh, in OTCS
    SurfNormalss            normss;         // By mesh, in OECS
    AxAffine2D              itcsToIucs;
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
        AxAffine2D          itcsToIucs_,
        Lighting const &    lighting_,          // in OECS
        RgbaF               background_,        // must be alpha-weighted
        Vec2UI              dims,
        bool                useMaps_=true,
        bool                allShiny_=true)
        :
        itcsToIucs(itcsToIucs_),
        // TODO: set up grid only after seeing how many verts fall in frustum, possibly use smaller grid size,
        // and what their bounding box is for setting client to grid transform:
        grid {Rect2F{{0,0},{1,1}},cNumTriEquivs(meshes)},
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
            verts = mapMulR(Affine3F{modelview.asAffine()},mesh.verts);
            uvsPtrs[mm] = &mesh.uvs;
            normss[mm] = cNormals(mesh.surfaces,verts);
            Vec3Fs &           iucsVerts = iucsVertss[mm];
            iucsVerts.reserve(verts.size());
            for (Vec3F v : verts)
                iucsVerts.push_back(oecsToIucs(v));
            for (size_t ss=0; ss<triss.size(); ++ss) {
                TriInds const &  tris = triss[ss];
                for (size_t tt=0; tt<tris.vertInds.size(); ++tt) {
                    Arr3UI       t = tris.vertInds[tt];
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
    RgbaF               cast(Vec2F pacs) const
    {
        Vec2F               posIucs {pacs[0]/imgDims[0],pacs[1]/imgDims[1]};
        BestN<float,Intersect,8> best = closestIntersects(posIucs);
        // Compute ray color:
        RgbaF               color = background;
        for (size_t ii=best.size(); ii>0; --ii) {             // Render back to front
            Intersect           isct = best[ii-1].object;
            TriInds const &     tris = trisss[isct.triInd.meshIdx][isct.triInd.surfIdx];
            Material            material = materialss[isct.triInd.meshIdx][isct.triInd.surfIdx];
            SurfNormals const & norms = normss[isct.triInd.meshIdx];
            Arr3UI              vis = tris.vertInds[isct.triInd.triIdx];
            Arr<Vec3F,3>        triNorms = mapIndex(vis,norms.vert);
            // TODO: Use perspective-correct normal and UV interpolation (makes very little difference for small tris):
            Arr3F               bc = mapCast<float>(isct.barycentric);

            Vec3F               norm = normalize(multAcc(triNorms,bc));
            RgbaF               albedo {0.9,0.9,0.9,1};
            Vec2Fs const &      uvs = *uvsPtrs[isct.triInd.meshIdx];
            Vec2F               uv {lims<float>::max()};
            if ((!tris.uvInds.empty()) && (!uvs.empty()) && (material.albedoMap) &&
                (!material.albedoMap->empty()) && useMaps) {
                Arr3UI              uvInds = tris.uvInds[isct.triInd.triIdx];
                uv = bc[0]*uvs[uvInds[0]] + bc[1]*uvs[uvInds[1]] + bc[2]*uvs[uvInds[2]];
                uv[1] = 1.0f - uv[1];   // OTCS to IUCS
                albedo = RgbaF(sampleClampIucs(*material.albedoMap,uv)/255.0f);
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
                        RgbaF           s = sampleClampIucs(*material.specularMap,uv);
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
        Arr3D               barycentric;        // model space

        Intersect() {}
        Intersect(TriIdxSM ti,Arr3D bc) : triInd(ti), barycentric(bc) {}
    };

    // Return closest tri intersects for given ray:
    BestN<float,RayCaster::Intersect,8> closestIntersects(Vec2F posIucs) const
    {
        TriIdxSMs const &   triInds = grid[posIucs];
        BestN<float,Intersect,8> best;
        for (TriIdxSM ti : triInds) {
            TriInds const &     tris = trisss[ti.meshIdx][ti.surfIdx];
            Arr3UI              vis = tris.vertInds[ti.triIdx];
            Vec3Fs const &      iucsVerts = iucsVertss[ti.meshIdx];
            Arr<Vec3F,3>        verts = mapIndex(vis,iucsVerts);
            Arr<Vec2D,3>        vts = mapCall(verts,[](Vec3F v){return Vec2D{v[0],v[1]}; });
            Opt<Arr3D>          bco = cBarycentricCoord(Vec2D(posIucs),vts);
            if (bco.has_value()) {       // TODO: filter out degenerate projected tris during cache setup
                Arr3D               bc = bco.value();
                if (allGteZero(bc)) {   // sample on triangle
                    // convert from screen space barycentrics to model space barycentrics:
                    // https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf
                    Arr3D           invDepths = mapCall(verts,[](Vec3F v){return scast<double>(v[2]); });
                    double          invDepth = multAcc(bc,invDepths);
                    Arr3D           bcm = mapMul(bc,invDepths) / invDepth;
                    best.update(scast<float>(invDepth),Intersect(ti,bcm));
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
    AxAffine2D              itcsToIucs,
    RenderOptions const &   options)
{
    Arr2F                   colorBounds = cBounds(options.backgroundColor.m_c);
    FGASSERT((colorBounds[0] >= 0.0f) && (colorBounds[1] <= 255.0f));
    RayCaster               rc {meshes,modelview,itcsToIucs,
        options.lighting,
        options.backgroundColor / 255.0f,
        pxSz,
        options.useMaps,options.allShiny
    };
    auto                    rendFn = [&](Vec2UI,Vec2F pacs)
    {
        return rc.cast(pacs);
    };
    ImgC4F                  rend = sampleAdaptiveF(pxSz,rendFn);
    ImgRgba8                img = toRgba8(rend);
    // Calculate where the surface points land:
    ProjectedSurfPoints     spps;
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        Mesh const &            mesh = meshes[mm];
        Vec3Fs const &          verts = rc.vertss[mm];
        SurfNormals const &     norms = rc.normss[mm];
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
                        RayCaster::Intersect  intsct = intscts[0].object;     // First is closest
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
    CameraParams        camPrms {Mat32D(catH(updateVertBounds2(meshes)))};
    Camera              camera = camPrms.camera(pixelSize);
    RenderOptions       ro;
    ro.backgroundColor = bgColor / 255.0f;
    return renderSoft(pixelSize,meshes,camera.modelview,camera.itcsToIucs,ro);
}

namespace {

void                testHalfMoon(CLArgs const & args)
{
    size_t const        dim = 128,
                        hd = dim/2,
                        td = dim/3;
    auto                sampFn = [=](Vec2UI,Vec2F pacs)
    {
        if ((pacs[1] > hd) && (cLenD(pacs-Vec2F(hd,hd)) < td))
            return RgbaF{1,1,1,1};
        else
            return RgbaF{0,0,0,1};
    };
    ImgRgba8            img = toRgba8(mapGamma(sampleAdaptiveF(Vec2UI{dim},sampFn,1,4),0.5f));
    if (!isAutomated(args))
        viewImage(img);
}

void                testMandelbrot(CLArgs const & args)
{
    size_t const        dim = 1024;
    auto                sampFn = [](Vec2UI,Vec2F pacs)
    {
        complex<double>     z {0,0},
                            c {(pacs[0]-750.0)/512.0,(pacs[1]-512.0)/512.0};
        size_t              ii {0};
        for (; ii<255; ++ii) {
            z = z*z + c;
            if (cMagD(z) > 4)            // guaranteed to diverge
                break;
        }
        float               v = scast<float>(ii) / 255.0f;
        return RgbaF(v,v,v,1);
    };
    Timer               time;
    ImgRgba8            img = toRgba8(mapGamma(sampleAdaptiveF(Vec2UI{dim},sampFn,1,4),0.5f));
    fgout << " time: " << time.elapsedSeconds() << "s";
    if (!isAutomated(args))
        viewImage(img);
}

void                testRendTris(CLArgs const & args)
{
    String              relPath = "base/test/render/";
    PushDir             pd {dataDir()+relPath};
    // Set up structures required for rendering:
    Meshes              meshes(1);
    Mesh &              mesh = meshes[0];
    mesh.surfaces.resize(1);
    Surf &              surf = mesh.surfaces[0];
    SimilarityD         modelview;      // Default is identity
    AxAffine2D          itcsToIucs(Vec2D(0.5),Vec2D(0.5));
    RenderOptions       ro;
    // Model a single triangle of equal width and height intersected by the optical axis
    // in OECS at the barycentric centre:
    mesh.verts = { {-1,1.5,-4}, {-1,-1.5,-4}, {2,0,-4} };
    surf.tris.vertInds = {{0,1,2}};         // CC winding
    surf.surfPoints = {{0,Arr3F{1.0f/3.0f}}};
    // Test that the point is visible in the current configuration:
    ro.renderSurfPoints = RenderSurfPoints::whenVisible;
    auto                isApproxEqualFn = [](ImgRgba8 const & l,ImgRgba8 const & r)
    {
        return isApproxEqual(l,r,20);   // TODO: why is macos result so different ???
    };
    ImgRgba8            img = renderSoft(Vec2UI(64),meshes,modelview,itcsToIucs,ro);
    if (!isAutomated(args))
        viewImage(img);

    testRegressApprox<ImgRgba8>(img,relPath+"t0.png",isApproxEqualFn);
    // Flip the winding to test the surface point is not visible from behind:
    surf.tris.vertInds.back() = {1,0,2};
    img = renderSoft(Vec2UI(64),meshes,modelview,itcsToIucs,ro);
    testRegressApprox<ImgRgba8>(img,relPath+"t4.png",isApproxEqualFn);
    if (!isAutomated(args))
        viewImage(img);

    surf.tris.vertInds.back() = {0,1,2};    // Restore
    // Place a triangle just in front to test occlusion of the surface point:
    mesh.verts.emplace_back(2,0,-3.9f);
    surf.tris.vertInds.emplace_back(0,1,3);
    img = renderSoft(Vec2UI(64),meshes,modelview,itcsToIucs,ro);
    testRegressApprox<ImgRgba8>(img,relPath+"t3.png",isApproxEqualFn);
    if (!isAutomated(args))
        viewImage(img);
}

void                testRendChecker(CLArgs const & args)
{
    // Model 2 right angle triangles making a sqaure with a checkerboard color map (preserving aspect ratio):
    float constexpr     Z = -4;
    Vec3Fs              verts {{-1,1.5,Z}, {-1,-1.5,Z}, {2,1.5,Z}, {2,-1.5,Z}};
    Vec2Fs              uvs {{0,1}, {0,0}, {1,1}, {1,0}};
    Arr3UIs             triVinds {{0,1,2}, {2,1,3}};
    Arr3UIs             triUinds {{0,1,2}, {2,1,3}};
    ImgRgba8            map(128,128);
    for (Iter2UI it(map.dims()); it.valid(); it.next()) {
        bool                black = (it()[0] & 16) != (it()[1] & 16);
        map[it()] = black ? Rgba8(0,0,0,255) : Rgba8(255,255,255,255);
    }
    Surf                surf {TriInds{triVinds,triUinds}};
    surf.material.albedoMap = make_shared<ImgRgba8>(map);
    Mesh                mesh {verts,uvs,{surf}};
    // View undistorted checkerboard flat on:
    AxAffine2D          itcsToIucs(Vec2D{0.5},Vec2D{0.5});
    ImgRgba8            img = renderSoft(Vec2UI(256),{mesh},{},itcsToIucs);
    String              relPath = "base/test/render/";
    auto                isApproxEqualFn = [](ImgRgba8 const & l,ImgRgba8 const & r) {return isApproxEqual(l,r,20); };
    testRegressApprox<ImgRgba8>(img,relPath+"t1.png",isApproxEqualFn);
    if (!isAutomated(args))
        viewImage(img);
    // View at an angle to see perspective distortion:
    SimilarityD         modelview = SimilarityD{Vec3D{0,0,-4}} * SimilarityD{cRotateY(1.0)} * SimilarityD{Vec3D{0,0,4}};
    img = renderSoft(Vec2UI(256),{mesh},modelview,itcsToIucs);
    testRegressApprox<ImgRgba8>(img,relPath+"t2.png",isApproxEqualFn);
    if (!isAutomated(args))
        viewImage(img);
}

void                testRendMesh(CLArgs const &)
{
    String8             dd = dataDir()+"base/Jane";
    Mesh                mesh = loadMesh(dd+".tri");
    Arr3UIs             tris = mesh.surfaces[0].tris.vertInds;
    ImgRgba8            map = loadImage(dd+".jpg");
    uint constexpr      renderDim = 256;
    Vec2UI              renderDims {renderDim};
    double              halfFovTanMax = 0.125;
    Rigid3D             toFhcsAngle = cRotateY(pi*0.3) * Trans3D{Vec3D{-cMean(mesh.verts)}};
    mesh.verts = mapMulR(Affine3F{toFhcsAngle.asAffine()},mesh.verts);
    Rigid3D             toFccs = moveFmcsToFccs(Mat32D{catH(cBounds(mesh.verts))},halfFovTanMax*.75);
    ScaleTrans2D        itcsToPacs = cItcsToPacs(halfFovTanMax,renderDims);
    Vec3Ds              shapeFccs = mapMulR(toFccs.asAffine(),mapCast<Vec3D>(mesh.verts));
    ProjVerts           pvs = projectVerts(shapeFccs,itcsToPacs);

}

}

void                testRender(CLArgs const & args)
{
    Cmds                cmds {
        {testMandelbrot,"mand","Mandelbrot set"},
        {testHalfMoon,"moon","half moon"},
        {testRendMesh,"head","render a head mesh using sampleAdaptive"},
        {testRendTris,"tris","colored triangles and checkerboard"},
        {testRendChecker,"check","checkerboard frontal and perspective"},
    };
    doMenu(args,cmds,true);
}

}

// */
