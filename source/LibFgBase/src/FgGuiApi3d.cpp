//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApi3d.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgTopology.hpp"
#include "FgTransform.hpp"
#include "FgGeometry.hpp"
#include "FgMatrixSolver.hpp"

using namespace std;


namespace Fg {

String8s            cAlbedoModeLabels()
{
    return {
        "albedo map",
        "none",
        "by mesh",
        "by surf",
    };
}

AxAffine2F          cD3psToRcs(Vec2UI viewportSize)
{
    // x: [-1,1] -> [-0.5,X-0.5]
    // y: [1,-1] -> [-0.5,Y-0.5]  (reverses)
    Mat22F              d3ps {-1,1,1,-1};
    Mat22F              rcs {-0.5f,viewportSize[0]-0.5f,-0.5f,viewportSize[1]-0.5f};
    return {d3ps,rcs};
}

OPT<bool>           linkUsesAlpha(NPT<ImgRgba8> const & imgN)
{
    return link1(imgN,[](ImgRgba8 const & img){return usesAlpha(img);});     // returns false if image empty
}

RendSurf::RendSurf(NPT<ImgRgba8> const & s) :
    smoothMapN{s},
    albedoHasTransparencyN{linkUsesAlpha(s)}
{}
RendSurf::RendSurf(NPT<ImgRgba8> const & s,NPT<ImgRgba8> const & m,NPT<ImgRgba8> const & p) :
    smoothMapN{s},
    albedoHasTransparencyN{linkUsesAlpha(s)},
    modulationMapN{m},
    specularMapN{p}
{}

namespace {

struct      ProjPnt {
    Vec2D           rcs {0};
    double          invDepth {0};
    bool            valid {false};

    ProjPnt() {}
    ProjPnt(Vec2F const & r,float i) : rcs{r}, invDepth{i}, valid{true} {}
};
typedef Svec<ProjPnt>   ProjPnts;

}

Opt<MeshesIsectPoint> intersectMeshesPoint(
    Vec2UI              winSize,
    Vec2I               pos,
    Mat44F              worldToD3ps,
    RendMeshes const &  rendMeshes)
{
    MeshesIsectPoint    ret;
    Valid<float>        minDepth;
    Vec2D               pnt {pos};
    AxAffine2F          d3psToRcs = cD3psToRcs(winSize);
    for (size_t mm=0; mm<rendMeshes.size(); ++mm) {
        RendMesh const &    rendMesh = rendMeshes[mm];
        Mesh const &        mesh = rendMesh.origMeshN.val();
        Vec3Fs const &      verts = rendMesh.shapeVertsN.val();
        auto                projectFn = [worldToD3ps,d3psToRcs](Vec3F vert) -> ProjPnt
        {
            Vec4F               d3psH = worldToD3ps * append(vert,1.0f);
            if (d3psH[3] == 0)
                return {};
            else {
                Vec3F               d3ps = projectHomog(d3psH);
                return {d3psToRcs * Vec2F{d3ps[0],d3ps[1]},d3ps[2]};
            }
        };
        ProjPnts            pntss = mapCall(verts,projectFn);
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            Surf const & surf = mesh.surfaces[ss];
            size_t              numTriEquivs = surf.numTriEquivs();
            for (size_t tt=0; tt<numTriEquivs; ++tt) {
                Arr3UI              tri = surf.getTriEquivVertInds(tt);
                Arr<ProjPnt,3>      pnts = mapIndex(tri,pntss);
                if (pnts[0].valid && pnts[1].valid && pnts[2].valid) {
                    Arr<Vec2D,3>    v = mapMember(pnts,&ProjPnt::rcs);
                    if (pointInTriangle(pnt,v[0],v[1],v[2]) == -1) {     // CC winding
                        Opt<Arr3D>      vbc = cBarycentricCoord(pnt,v);
                        if (vbc.has_value()) {
                            Arr3D           bc = vbc.value();
                            // Depth value range for unclipped polys is [-1,1]. These correspond to the
                            // negative inverse depth values of the frustum.
                            // Only an approximation to the depth value but who cares:
                            Arr3D           invDepth {pnts[0].invDepth,pnts[1].invDepth,pnts[2].invDepth};
                            double          dep = cDot(bc,invDepth);
                            if (!minDepth.valid() || (dep < minDepth.val())) {    // projection inverts depth
                                minDepth = dep;
                                ret.isect = {mm,ss,{uint(tt),mapCast<float>(bc)}};
                                ret.pos = multAcc(mapCast<float>(bc),mapIndex(tri,verts));
                            }
                        }
                    }
                }
            }
        }
    }
    if (minDepth.valid())
        return ret;
    return {};
}

Opt<MeshesIntersect> intersectMeshes(
    Vec2UI              winSize,
    Vec2I               pos,
    Mat44F              worldToD3ps,
    RendMeshes const &  rendMeshes)
{
    Opt<MeshesIsectPoint>   mip = intersectMeshesPoint(winSize,pos,worldToD3ps,rendMeshes);
    if (mip.has_value())
        return mip.value().isect;
    else
        return {};
}

Gui3d::Gui3d(NPT<RendMeshes> rmN,bool tft) :
    rendMeshesN {rmN},
    tryForTransparency {tft}
{
}

void                Gui3d::panTilt(Vec2I delta)
{
    size_t          mode = panTiltMode.val();
    if (mode == 0) {
        Vec2D    panTilt;
        panTilt[0] = panDegrees.val();
        panTilt[1] = tiltDegrees.val();
        panTilt += Vec2D(delta) / 3.0;
        if (panTiltLimits) {
            for (uint dd=0; dd<2; ++dd)
                panTilt[dd] = clamp(panTilt[dd],-90.0,90.0);
        }
        else {
            for (uint dd=0; dd<2; ++dd) {
                if (panTilt[dd] < -180)
                    panTilt[dd] += 360;
                if (panTilt[dd] > 180)
                    panTilt[dd] -= 360;
            }
        }
        panDegrees.set(panTilt[0]);
        tiltDegrees.set(panTilt[1]);
    }
    else {
        // Convert from pixels to half the tangent rotation in radians:
        Vec2D        del = Vec2D(delta) * 0.005;
        QuaternionD   poseVal = pose.val();
        poseVal = QuaternionD(1.0,del[1],del[0],0.0) * poseVal;
        pose.set(poseVal);
    }
}

void                Gui3d::roll(int delta)
{
    size_t          mode = panTiltMode.val();
    if (mode == 1) {
        // Convert from pixels to half the tangent rotation in radians:
        double          del = double(delta) * 0.005;
        QuaternionD   poseVal = pose.val();
        poseVal = QuaternionD(1.0,0,0,del) * poseVal;
        pose.set(poseVal);
    }
}

void                Gui3d::scale(int delta)
{
    double          rs = logRelSize.val();
    rs += double(delta)/200.0;
    static double   rsMin = std::log(0.05);
    // The effective limit is due to avoiding clipping in creation of the MVM and projection matrices,
    // not due to this value which well exceeds it:
    static double   rsMax = std::log(20.0);
    if (rs < rsMin)
        rs = rsMin;
    if (rs > rsMax)
        rs = rsMax;
    logRelSize.set(rs);
}

void                Gui3d::translate(Vec2I delta)
{
    delta[1] *= -1;
    Vec2D    tr = trans.val();
    double      scale = std::exp(logRelSize.val());
    tr += Vec2D(delta) / (400.0 * scale);
    trans.set(tr);
}

void                markSurfacePoint(
    NPT<RendMeshes> const & rendMeshesN,
    NPT<String8> const &    pointLabelN,
    Vec2UI          winSize,
    Vec2I           viewportPos,
    Mat44F          worldToD3ps)         // Transforms frustum to [-1,1] cube (depth & y inverted)
{
    RendMeshes const &      rms = rendMeshesN.val();
    Opt<MeshesIntersect>    vpt = intersectMeshes(winSize,viewportPos,worldToD3ps,rms);
    if (vpt.has_value()) {
        MeshesIntersect         pt = vpt.value();
        SurfPoint const &       sp = pt.surfPnt;
        FGASSERT(pt.meshIdx < rms.size());
        RendMesh const &        rm = rms[pt.meshIdx];
        Mesh *                  origMeshPtr = rm.origMeshN.valPtr();
        FGASSERT(pt.surfIdx < origMeshPtr->surfaces.size());
        Surf &                  surf = origMeshPtr->surfaces[pt.surfIdx];
        Vec3F                   pos = surf.surfPointPos(origMeshPtr->verts,sp);
        fgout << fgnl << "Surf point selected: surf " << pt.surfIdx
            << " tri equiv: " << sp.triEquivIdx
            << " coord: " << pos;
        FGASSERT(pointLabelN.ptr);
        String                  label = pointLabelN.val().as_ascii();
        if (label.empty())
            fgout << " NOT ADDED; empty label";
        else if (origMeshPtr) {      // If original mesh is an input node (ie. modifiable):
            SurfPointNames &        surfPoints =  surf.surfPoints;
            for (size_t ii=0; ii<surfPoints.size(); ++ii) {
                if (surfPoints[ii].label == label) {            // Replace SPs of same name:
                    surfPoints[ii].point = pt.surfPnt;
                    fgout << " REPLACED existing point";
                    return;
                }
            }
            // Add new SP only if there is a non-empty name:
            surfPoints.emplace_back(pt.surfPnt,label);
            fgout << " ADDED";
        }
        else
            fgout << " NOT ADDED; mesh cannot be modified";
    }
}

void                markMeshVertex(
    NPT<RendMeshes> const & rendMeshesN,
    NPT<size_t> const &     vertMarkModeN,
    Vec2UI                  winSize,
    Vec2I                   pos,
    Mat44F                  worldToD3ps)     // Transforms frustum to [-1,1] cube (depth & y inverted)
{
    RendMeshes const &          rms = rendMeshesN.val();
    Opt<MeshesIntersect>        vpt = intersectMeshes(winSize,pos,worldToD3ps,rms);
    if (vpt.has_value() && vertMarkModeN.ptr) {
        MeshesIntersect         pt = vpt.value();
        RendMesh const &        rm = rms[pt.meshIdx];
        Mesh *                  origMeshPtr = rm.origMeshN.valPtr();
        if (origMeshPtr) {
            Mesh &                  meshIn = *origMeshPtr;
            Surf const &            surf = meshIn.surfaces[pt.surfIdx];
            size_t                  facetIdx = cMaxIdx(pt.surfPnt.weights);
            Arr3UI                  vertInds = surf.getTriEquivVertInds(pt.surfPnt.triEquivIdx);
            uint                    vertIdx = vertInds[facetIdx];
            size_t                  vertMarkMode = vertMarkModeN.val();
            if (vertMarkMode == 0) {
                if (!contains(meshIn.markedVerts,vertIdx))
                    meshIn.markedVerts.push_back(MarkedVert(vertIdx));
            }
            else if (vertMarkMode < 4) {
                Arr3UIs             tris = surf.getTriEquivs().vertInds;
                SurfTopo        topo(meshIn.verts.size(),tris);
                set<uint>           seam;
                if (vertMarkMode == 1) {
                    auto                boundary = topo.boundaryContainingVert(vertIdx);
                    for (auto const & be : boundary)
                        seam.insert(be.vertIdx);
                }
                else if (vertMarkMode == 2) {
                    Surf                tmpSurf;
                    tmpSurf.tris.vertInds = tris;
                    vector<FatBool>      done(meshIn.verts.size(),false);
                    seam = topo.traceFold(cNormals({tmpSurf},meshIn.verts),done,vertIdx);
                }
                else if (vertMarkMode == 3)
                    seam = cFillMarkedVertRegion(meshIn,topo,vertIdx);
                for (uint idx : seam)
                    if (!contains(meshIn.markedVerts,idx))
                        meshIn.markedVerts.push_back(MarkedVert{idx});
            }
        }
    }
}

void                Gui3d::buttonDown(size_t buttonIdx,Vec2UI winSz,Vec2I pos,Mat44F worldToD3ps)
{
    FGASSERT(buttonIdx < 3);
    RendMeshes const &      rms = rendMeshesN.val();
    Opt<MeshesIsectPoint>   isect = intersectMeshesPoint(winSz,pos,worldToD3ps,rms);
    lastButtonDown[buttonIdx] = {pos,worldToD3ps,isect};
}

void                Gui3d::ctlDrag(bool left, Vec2UI winSize,Vec2I delta,Mat44F worldToD3ps)
{
    size_t              buttonIdx = left ? 0 : 2;
    if (lastButtonDown[buttonIdx].has_value()) {
        LastClick const &   vpc = lastButtonDown[buttonIdx].value();
        if (vpc.isect.has_value()) {
            MeshesIntersect         isect = vpc.isect.value().isect;
            RendMeshes const &      rms = rendMeshesN.val();
            size_t                  mi = cMaxIdx(isect.surfPnt.weights);
            RendMesh const &        rm = rms[isect.meshIdx];
            Mesh const &            mesh = rm.origMeshN.val();
            Surf const &            surf = mesh.surfaces[isect.surfIdx];
            uint                    vertIdx = surf.getTriEquivVertInds(isect.surfPnt.triEquivIdx)[mi];
            Vec3Fs const &          verts = rms[isect.meshIdx].shapeVertsN.val();
            // The concept of a delta vector doesn't work in projective space since it's not distance
            // preserving; a homogeneous component equal to zero is a direction. Conceptually, we can't
            // transform a delta in D3PS to FHCS without knowing it's absolute position.
            // Hence we transform the end point back into FHCS and take the difference:
            Vec3F                   vertPos0Hcs = verts[vertIdx];
            Vec4F                   vertPos0d3ps = worldToD3ps * append(vertPos0Hcs,1.0f);
            // Convert delta to D3PS. Y inverted and Viewport aspect (compensated for in frustum)
            // is ratio to largest dimension:
            Vec2F                   delD3ps2 = 2.0f * Vec2F(delta) / float(cMaxElem(winSize));
            Vec4F                   delD3ps(delD3ps2[0],-delD3ps2[1],0,0),
                                    // Normalize vector for valid addition of delta:
                                    vertPos1d3ps = vertPos0d3ps / vertPos0d3ps[3] + delD3ps,
                                    // We don't expect worldToD3ps to be singular:
                                    vertPos1HcsH = Vec4F(solveLinear(Mat44D(worldToD3ps),Vec4D(vertPos1d3ps)));
            Vec3F                   vertPos1Hcs = vertPos1HcsH.subMatrix<3,1>(0,0) / vertPos1HcsH[3],
                                    delHcs = vertPos1Hcs - vertPos0Hcs;
            ctlDragAction(left,{true,isect.meshIdx,vertIdx},delHcs);
        }
    }
}

void                Gui3d::translateBgImage(Vec2UI winSize,Vec2I delta)
{
    if (bgImg.imgN.ptr) {
        ImgRgba8 const & img = bgImg.imgN.val();
        if (!img.empty()) {
            Vec2F        del = mapDiv(Vec2F(delta),Vec2F(winSize));
            Vec2F &      offset = bgImg.offset.ref();
            offset += del;
        }
    }
}

void                Gui3d::scaleBgImage(Vec2UI winSize,Vec2I delta)
{
    if (bgImg.imgN.ptr) {
        ImgRgba8 const & img = bgImg.imgN.val();
        if (!img.empty()) {
            double      lnScale = bgImg.lnScale.val();
            lnScale += double(delta[1]) / double(winSize[1]);
            bgImg.lnScale.set(lnScale);
        }
    }
}

}

// */
