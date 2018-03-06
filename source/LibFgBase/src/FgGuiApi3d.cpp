//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: April 5, 2011
//

#include "stdafx.h"

#include "FgGuiApi.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dTopology.hpp"
#include "FgAffineCwC.hpp"
#include "FgGeometry.hpp"

using namespace std;

void
FgGuiApi3d::panTilt(FgVect2I delta)
{
    size_t          mode = g_gg.getVal(panTiltMode);
    if (mode == 0) {
        FgVect2D    panTilt;
        panTilt[0] = g_gg.getVal(panDegrees);
        panTilt[1] = g_gg.getVal(tiltDegrees);
        panTilt += FgVect2D(delta) / 3.0;
        if (panTiltLimits) {
            for (uint dd=0; dd<2; ++dd)
                panTilt[dd] = fgClip(panTilt[dd],-90.0,90.0);
        }
        else {
            for (uint dd=0; dd<2; ++dd) {
                if (panTilt[dd] < -180)
                    panTilt[dd] += 360;
                if (panTilt[dd] > 180)
                    panTilt[dd] -= 360;
            }
        }
        g_gg.setVal(panDegrees,panTilt[0]);
        g_gg.setVal(tiltDegrees,panTilt[1]);
    }
    else {
        // Convert from pixels to half the tangent rotation in radians:
        FgVect2D        del = FgVect2D(delta) * 0.005;
        FgQuaternionD   poseVal = g_gg.getVal(pose);
        poseVal = FgQuaternionD(1.0,del[1],del[0],0.0) * poseVal;
        g_gg.setVal(pose,poseVal);
    }
}

void
FgGuiApi3d::roll(int delta)
{
    size_t          mode = g_gg.getVal(panTiltMode);
    if (mode == 1) {
        // Convert from pixels to half the tangent rotation in radians:
        double          del = double(delta) * 0.005;
        FgQuaternionD   poseVal = g_gg.getVal(pose);
        poseVal = FgQuaternionD(1.0,0,0,del) * poseVal;
        g_gg.setVal(pose,poseVal);
    }
}

void
FgGuiApi3d::scale(int delta)
{
    double          rs = g_gg.getVal(logRelSize);
    rs += double(delta)/200.0;
    static double   rsMin = std::log(0.05);
    // The effective limit is due to avoiding clipping in creation of the MVM and projection matrices,
    // not due to this value which well exceeds it:
    static double   rsMax = std::log(20.0);
    if (rs < rsMin)
        rs = rsMin;
    if (rs > rsMax)
        rs = rsMax;
    g_gg.setVal(logRelSize,rs);
}

void
FgGuiApi3d::translate(FgVect2I delta)
{
    delta[1] *= -1;
    FgVect2D    tr = g_gg.getVal(trans);
    double      scale = std::exp(g_gg.getVal(logRelSize));
    tr += FgVect2D(delta) / (400.0 * scale);
    g_gg.setVal(trans,tr);
}

FgOpt<FgMeshesIntersect>
fgMeshesIntersect(FgVect2UI winSize,FgVect2I pos,FgMat44F toOics,const vector<Fg3dMesh> & meshes,const FgVertss & vertss)
{
    FgMeshesIntersect   ret;
    FgValid<float>      minDepth;
    FgVect2D            pnt(pos);
    FgMat32F         oics(-1,1,1,-1,0,1);
    FgMat32F         rcs(-0.5f,winSize[0]-0.5f,-0.5f,winSize[1]-0.5f,0,1);
    FgAffineCw3F        oicsToRcs(oics,rcs);
    FgMat44F         invXform = oicsToRcs.asAffine().asHomogenous() * toOics;
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        const Fg3dMesh &    mesh = meshes[mm];
        const FgVerts &     verts = vertss[mm];
        FgVerts             pvs(verts.size());
        for (size_t ii=0; ii<pvs.size(); ++ii) {
            FgVect4F        v = invXform * fgAsHomogVec(verts[ii]);
            pvs[ii] = v.subMatrix<3,1>(0,0) / v[3];
        }
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            const Fg3dSurface & surf = mesh.surfaces[ss];
            for (size_t tt=0; tt<surf.numTriEquivs(); ++tt) {
                FgVect3UI   tri = surf.getTriEquiv(uint(tt));
                FgVect3F    t0 = pvs[tri[0]],
                            t1 = pvs[tri[1]],
                            t2 = pvs[tri[2]];
                FgVect2D    v0 = FgVect2D(t0.subMatrix<2,1>(0,0)),
                            v1 = FgVect2D(t1.subMatrix<2,1>(0,0)),
                            v2 = FgVect2D(t2.subMatrix<2,1>(0,0));
                if (fgPointInTriangle(pnt,v0,v1,v2) == -1) {     // CC winding
                    FgOpt<FgVect3D>    vbc = fgBarycentricCoords(pnt,v0,v1,v2);
                    if (vbc.valid()) {
                        FgVect3D    bc = vbc.val();
                        // Depth value range for unclipped polys is [-1,1]. These correspond to the
                        // negative inverse depth values of the frustum.
                        // Only an approximation to the depth value but who cares:
                        double  dep = fgDot(bc,FgVect3D(t0[2],t1[2],t2[2]));
                        if (!minDepth.valid() || (dep < minDepth.val())) {    // OGL prj inverts depth
                            minDepth = dep;
                            ret.meshIdx = mm;
                            ret.surfIdx = ss;
                            ret.surfPnt.triEquivIdx = uint(tt);
                            ret.surfPnt.weights = FgVect3F(bc);
                        }
                    }
                }
            }
        }
    }
    if (minDepth.valid())
        return FgOpt<FgMeshesIntersect>(ret);
    return FgOpt<FgMeshesIntersect>();
}

void
FgGuiApi3d::markSurfPoint(
    FgVect2UI           winSize,
    FgVect2I            pos,
    FgMat44F            toOics)         // Transforms frustum to [-1,1] cube (depth & y inverted)
{
    if (g_gg.dg.sinkNode(meshesN))      // feature disabled if node not modifiable
        return;
    vector<Fg3dMesh>            meshes = g_gg.getVal(meshesN);
    const FgVertss &            vertss = g_gg.getVal(vertssN);
    FgOpt<FgMeshesIntersect>    vpt = fgMeshesIntersect(winSize,pos,toOics,meshes,vertss);
    if (vpt.valid() && pointLabel.valid()) {
        FgMeshesIntersect       pt = vpt.val();
        pt.surfPnt.label = g_gg.getVal(pointLabel).as_ascii();
        vector<FgSurfPoint> &   surfPoints =  meshes[pt.meshIdx].surfaces[pt.surfIdx].surfPoints;
        if (!pt.surfPnt.label.empty()) {
            for (size_t ii=0; ii<surfPoints.size(); ++ii) {
                if (surfPoints[ii].label == pt.surfPnt.label) {     // Replace SPs of same name:
                    surfPoints[ii] = pt.surfPnt;
                    g_gg.setVal(meshesN,meshes);
                    return;
                }
            }
        }
        // Add new SP:
        surfPoints.push_back(pt.surfPnt);
        g_gg.setVal(meshesN,meshes);
    }
}

void
FgGuiApi3d::markVertex(
    FgVect2UI               winSize,
    FgVect2I                pos,
    FgMat44F             toOics)     // Transforms frustum to [-1,1] cube (depth & y inverted)
{
    if (g_gg.dg.sinkNode(meshesN))      // feature disabled if node not modifiable
        return;
    vector<Fg3dMesh>            meshes = g_gg.getVal(meshesN);
    const FgVertss &            vertss = g_gg.getVal(vertssN);
    FgOpt<FgMeshesIntersect>    vpt = fgMeshesIntersect(winSize,pos,toOics,meshes,vertss);
    if (vpt.valid() && vertMarkModeN.valid()) {
        FgMeshesIntersect   pt = vpt.val();
        uint                facetIdx = fgMaxIdx(pt.surfPnt.weights);
        Fg3dMesh &          mesh = meshes[pt.meshIdx];
        uint                vertIdx = mesh.surfaces[pt.surfIdx].getTriEquiv(pt.surfPnt.triEquivIdx)[facetIdx];
        size_t              vertMarkMode = g_gg.getVal(vertMarkModeN);
        if (vertMarkMode == 0) {
            if (!fgContains(mesh.markedVerts,vertIdx)) {
                mesh.markedVerts.push_back(FgMarkedVert(vertIdx));
                g_gg.setVal(meshesN,meshes);
            }
        }
        else if (vertMarkMode < 3) {
            const FgVerts & verts = vertss[pt.meshIdx];
            vector<FgVect3UI>   tris = mesh.getTriEquivs().vertInds;
            Fg3dTopology        topo(verts,tris);
            set<uint>           seam;
            if (vertMarkMode == 1)
                seam = topo.seamContaining(vertIdx);
            else {
                Fg3dSurface         surf;
                surf.tris.vertInds = tris;
                vector<FgBool>      done(verts.size(),false);
                seam = topo.traceFold(fgNormals(fgSvec(surf),verts),done,vertIdx);
            }
            for (set<uint>::const_iterator it=seam.begin(); it != seam.end(); ++it)
                if (!fgContains(mesh.markedVerts,*it))
                    mesh.markedVerts.push_back(FgMarkedVert(*it));
            if (!seam.empty())
                g_gg.setVal(meshesN,meshes);
        }
    }
}

void
FgGuiApi3d::ctlClick(FgVect2UI winSize,FgVect2I pos,FgMat44F toOics)
{
    const vector<Fg3dMesh> &    meshes = g_gg.getVal(meshesN);
    const FgVertss &            vertss = g_gg.getVal(vertssN);
    FgOpt<FgMeshesIntersect>    vpt = fgMeshesIntersect(winSize,pos,toOics,meshes,vertss);
    if (vpt.valid()) {
        FgMeshesIntersect       pt = vpt.val();
        uint                    mi = fgMaxIdx(pt.surfPnt.weights);
        lastCtlClick.meshIdx = pt.meshIdx;
        lastCtlClick.vertIdx = meshes[pt.meshIdx].surfaces[pt.surfIdx].getTriEquiv(pt.surfPnt.triEquivIdx)[mi];
        lastCtlClick.valid = true;
    }
    else
        lastCtlClick.valid = false;
}

void
FgGuiApi3d::ctlDrag(bool left, FgVect2UI winSize,FgVect2I delta,FgMat44F toOics)
{
    if (lastCtlClick.valid) {
        // Interestingly, the concept of a delta vector doesn't work in projective space;
        // a homogenous component equal to zero is a direction. Conceptually, we can't
        // transform a delta in OICS to FHCS without knowing it's absolute position since. Hence
        // we transform the end point back into FHCS and take the difference:
        const vector<Fg3dMesh> &    meshes = g_gg.getVal(meshesN);
        FgVect3F                    vertPos0Hcs = meshes[lastCtlClick.meshIdx].verts[lastCtlClick.vertIdx];
        FgVect4F                    vertPos0Oics = toOics * fgAsHomogVec(vertPos0Hcs);
        // Convert delta to OICS. Y inverted and Viewport aspect (compensated for in frustum)
        // is ratio to largest dimension:
        FgVect2F                    delOics2 = 2.0f * FgVect2F(delta) / float(fgMaxElem(winSize));
        FgVect4F                    delOics(delOics2[0],-delOics2[1],0,0),
                                    // Normalize vector for valid addition of delta:
                                    vertPos1Oics = vertPos0Oics / vertPos0Oics[3] + delOics,
                                    // We don't expect toOics to be singular:
                                    vertPos1HcsH = fgSolve(toOics,vertPos1Oics).val();
        FgVect3F                    vertPos1Hcs = vertPos1HcsH.subMatrix<3,1>(0,0) / vertPos1HcsH[3],
                                    delHcs = vertPos1Hcs - vertPos0Hcs;
        ctlDragAction(left,lastCtlClick,delHcs);
    }
}

void
FgGuiApi3d::ctrlShiftLeftDrag(FgVect2UI winSize,FgVect2I delta)
{
    if (bgImg.imgN.valid()) {
        const FgImgRgbaUb & img = g_gg.getVal(bgImg.imgN);
        if (!img.empty()) {
            FgVect2F    del = fgMapDiv(FgVect2F(delta),FgVect2F(winSize));
            FgVect2F &  offset = g_gg.getRef(bgImg.offset);
            offset += del;
        }
    }
}

void
FgGuiApi3d::ctrlShiftRightDrag(FgVect2UI winSize,FgVect2I delta)
{
    if (bgImg.imgN.valid()) {
        const FgImgRgbaUb & img = g_gg.getVal(bgImg.imgN);
        if (!img.empty()) {
            double      lnScale = g_gg.getVal(bgImg.lnScale);
            lnScale += double(delta[1]) / double(winSize[1]);
            g_gg.setVal(bgImg.lnScale,lnScale);
        }
    }
}

// */
