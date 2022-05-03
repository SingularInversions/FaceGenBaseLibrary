//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGui.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgTopology.hpp"
#include "FgAffine.hpp"
#include "FgGeometry.hpp"

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

AffineEw2F          cD3psToRcs(Vec2UI viewportSize)
{
    // x: [-1,1] -> [-0.5,X-0.5]
    // y: [1,-1] -> [-0.5,Y-0.5]  (reverses)
    Mat22F              d3ps {-1,1,1,-1};
    Mat22F              rcs {-0.5f,viewportSize[0]-0.5f,-0.5f,viewportSize[1]-0.5f};
    return {d3ps,rcs};
}

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

Opt<MeshesIntersect> intersectMeshes(
    Vec2UI              winSize,
    Vec2I               pos,
    Mat44F              worldToD3ps,
    RendMeshes const &  rendMeshes)
{
    MeshesIntersect     ret;
    Valid<float>        minDepth;
    Vec2D               pnt {pos};
    AffineEw2F          d3psToRcs = cD3psToRcs(winSize);
    for (size_t mm=0; mm<rendMeshes.size(); ++mm) {
        RendMesh const &    rendMesh = rendMeshes[mm];
        Mesh const &        mesh = rendMesh.origMeshN.cref();
        Vec3Fs const &      verts = rendMesh.posedVertsN.cref();
        ProjPnts            pntss; pntss.reserve(verts.size());
        for (Vec3F const & vert : verts) {
            Vec4F               d3psH = worldToD3ps * append(vert,1.0f);
            if (d3psH[3] == 0)
                pntss.emplace_back();
            else {
                Vec3F               d3ps = projectHomog(d3psH);
                pntss.emplace_back(d3psToRcs * Vec2F{d3ps[0],d3ps[1]},d3ps[2]);
            }
        }
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            Surf const & surf = mesh.surfaces[ss];
            size_t              numTriEquivs = surf.numTriEquivs();
            for (size_t tt=0; tt<numTriEquivs; ++tt) {
                Vec3UI              tri = surf.getTriEquivVertInds(tt);
                Arr<ProjPnt,3>      pnts {pntss[tri[0]],pntss[tri[1]],pntss[tri[2]]};
                if (pnts[0].valid && pnts[0].valid && pnts[0].valid) {
                    Vec2D           v0 = pnts[0].rcs,
                                    v1 = pnts[1].rcs,
                                    v2 = pnts[2].rcs;
                    if (pointInTriangle(pnt,v0,v1,v2) == -1) {     // CC winding
                        Opt<Vec3D>      vbc = cBarycentricCoord(pnt,v0,v1,v2);
                        if (vbc.valid()) {
                            Vec3D           bc = vbc.val();
                            // Depth value range for unclipped polys is [-1,1]. These correspond to the
                            // negative inverse depth values of the frustum.
                            // Only an approximation to the depth value but who cares:
                            Vec3D           invDepth {pnts[0].invDepth,pnts[1].invDepth,pnts[2].invDepth};
                            double          dep = cDot(bc,invDepth);
                            if (!minDepth.valid() || (dep < minDepth.val())) {    // OGL prj inverts depth
                                minDepth = dep;
                                ret.meshIdx = mm;
                                ret.surfIdx = ss;
                                ret.surfPnt.triEquivIdx = uint(tt);
                                ret.surfPnt.weights = Vec3F(bc);
                            }
                        }
                    }
                }
            }
        }
    }
    if (minDepth.valid())
        return Opt<MeshesIntersect>(ret);
    return Opt<MeshesIntersect>();
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
    RendMeshes const &      rms = rendMeshesN.cref();
    Opt<MeshesIntersect>    vpt = intersectMeshes(winSize,viewportPos,worldToD3ps,rms);
    if (vpt.valid() && pointLabelN.ptr) {
        MeshesIntersect         pt = vpt.val();
        BaryPoint const &       sp = pt.surfPnt;
        FGASSERT(pt.meshIdx < rms.size());
        RendMesh const &        rm = rms[pt.meshIdx];
        Mesh *                  origMeshPtr = rm.origMeshN.valPtr();
        FGASSERT(pt.surfIdx < origMeshPtr->surfaces.size());
        Surf &                  surf = origMeshPtr->surfaces[pt.surfIdx];
        Vec3F                   pos = surf.surfPointPos(origMeshPtr->verts,sp);
        fgout << fgnl << "Surf point placed on surf " << pt.surfIdx << " at coord: " << pos;
        if (origMeshPtr) {      // If original mesh is an input node (ie. modifiable):
            SurfPoints &            surfPoints =  surf.surfPoints;
            String                     label = pointLabelN.cref().as_ascii();
            if (!label.empty()) {
                for (size_t ii=0; ii<surfPoints.size(); ++ii) {
                    if (surfPoints[ii].label == label) {            // Replace SPs of same name:
                        surfPoints[ii].point = pt.surfPnt;
                        return;
                    }
                }
            }
            // Add new SP:
            surfPoints.emplace_back(pt.surfPnt,label);
        }
    }
}

void                markMeshVertex(
    NPT<RendMeshes> const & rendMeshesN,
    NPT<size_t> const &     vertMarkModeN,
    Vec2UI                  winSize,
    Vec2I                   pos,
    Mat44F                  worldToD3ps)     // Transforms frustum to [-1,1] cube (depth & y inverted)
{
    RendMeshes const &          rms = rendMeshesN.cref();
    Opt<MeshesIntersect>        vpt = intersectMeshes(winSize,pos,worldToD3ps,rms);
    if (vpt.valid() && vertMarkModeN.ptr) {
        MeshesIntersect         pt = vpt.val();
        RendMesh const &        rm = rms[pt.meshIdx];
        Mesh *                  origMeshPtr = rm.origMeshN.valPtr();
        if (origMeshPtr) {
            Mesh &                  meshIn = *origMeshPtr;
            Surf const &            surf = meshIn.surfaces[pt.surfIdx];
            size_t                  facetIdx = cMaxIdx(pt.surfPnt.weights);
            Vec3UI                  vertInds = surf.getTriEquivVertInds(pt.surfPnt.triEquivIdx);
            uint                    vertIdx = vertInds[facetIdx];
            size_t                  vertMarkMode = vertMarkModeN.val();
            if (vertMarkMode == 0) {
                if (!contains(meshIn.markedVerts,vertIdx))
                    meshIn.markedVerts.push_back(MarkedVert(vertIdx));
            }
            else if (vertMarkMode < 4) {
                Vec3UIs             tris = surf.getTriEquivs().vertInds;
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
                    seam = topo.traceFold(cNormals(svec(tmpSurf),meshIn.verts),done,vertIdx);
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

void                Gui3d::ctlClick(Vec2UI winSize,Vec2I pos,Mat44F worldToD3ps)
{
    RendMeshes const &      rms = rendMeshesN.cref();
    Opt<MeshesIntersect>    vpt = intersectMeshes(winSize,pos,worldToD3ps,rms);
    if (vpt.valid()) {
        MeshesIntersect         pt = vpt.val();
        size_t                  mi = cMaxIdx(pt.surfPnt.weights);
        RendMesh const &        rm = rms[pt.meshIdx];
        Mesh const &            mesh = rm.origMeshN.cref();
        Surf const &            surf = mesh.surfaces[pt.surfIdx];
        lastCtlClick.meshIdx = pt.meshIdx;
        lastCtlClick.vertIdx = surf.getTriEquivVertInds(pt.surfPnt.triEquivIdx)[mi];
        lastCtlClick.valid = true;
    }
    else
        lastCtlClick.valid = false;
}

void                Gui3d::ctlDrag(bool left, Vec2UI winSize,Vec2I delta,Mat44F worldToD3ps)
{
    if (lastCtlClick.valid) {
        // Interestingly, the concept of a delta vector doesn't work in projective space;
        // a homogeneous component equal to zero is a direction. Conceptually, we can't
        // transform a delta in D3PS to FHCS without knowing it's absolute position.
        // Hence we transform the end point back into FHCS and take the difference:
        RendMeshes const &      rms = rendMeshesN.cref();
        Vec3Fs const &          verts = rms[lastCtlClick.meshIdx].posedVertsN.cref();
        Vec3F                   vertPos0Hcs = verts[lastCtlClick.vertIdx];
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
        ctlDragAction(left,lastCtlClick,delHcs);
    }
}

void                Gui3d::translateBgImage(Vec2UI winSize,Vec2I delta)
{
    if (bgImg.imgN.ptr) {
        ImgRgba8 const & img = bgImg.imgN.cref();
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
        ImgRgba8 const & img = bgImg.imgN.cref();
        if (!img.empty()) {
            double      lnScale = bgImg.lnScale.val();
            lnScale += double(delta[1]) / double(winSize[1]);
            bgImg.lnScale.set(lnScale);
        }
    }
}

}

// */
