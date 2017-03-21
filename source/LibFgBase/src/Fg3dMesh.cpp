//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 22, 2005
//

#include "stdafx.h"

#include "FgMatrix.hpp"
#include "Fg3dMesh.hpp"
#include "FgException.hpp"
#include "FgStdStream.hpp"
#include "FgBounds.hpp"
#include "FgMath.hpp"
#include "Fg3dTopology.hpp"
#include "FgStdSet.hpp"

using namespace std;

void
fgReadp(std::istream & is,FgMarkedVert & m)
{
    fgReadp(is,m.idx);
    fgReadp(is,m.label);
}

void
fgWritep(std::ostream & os,const FgMarkedVert & m)
{
    fgWritep(os,m.idx);
    fgWritep(os,m.label);
}

size_t
Fg3dMesh::totNumVerts() const
{
    size_t  ret = verts.size();
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        ret += targetMorphs[ii].verts.size();
    return ret;
}

FgVerts
Fg3dMesh::allVerts() const
{
    FgVerts     ret = verts;
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        fgAppend(ret,targetMorphs[ii].verts);
    return ret;
}

void
Fg3dMesh::updateAllVerts(const FgVerts & avs)
{
    FGASSERT(avs.size() == totNumVerts());
    size_t          idx = 0;
    for (; idx<verts.size(); ++idx)
        verts[idx] = avs[idx];
    for (size_t ii=0; ii<targetMorphs.size(); ++ii) {
        FgIndexedMorph & tm = targetMorphs[ii];
        for (size_t jj=0; jj<tm.verts.size(); ++jj)
            tm.verts[jj] = avs[idx++];
    }
}

uint
Fg3dMesh::numFacets() const
{
    uint    tot = 0;
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        tot += surfaces[ss].numFacets();
    return tot;
}

uint        Fg3dMesh::numTriEquivs() const
{
    uint        tot = 0;
    for (uint ss=0; ss<surfaces.size(); ss++)
        tot += surfaces[ss].numTriEquivs();
    return tot;
}

FgVect3UI
Fg3dMesh::getTriEquiv(uint idx) const
{
    for (size_t ss=0; ss<surfaces.size(); ++ss) {
        uint        num = surfaces[ss].numTriEquivs();
        if (idx < num)
            return surfaces[ss].getTriEquiv(idx);
        else
            idx -= num;
    }
    FGASSERT_FALSE;
    return FgVect3UI();
}

FgFacetInds<3>
Fg3dMesh::getTriEquivs() const
{
    FgFacetInds<3>      ret;
    if (surfaces.empty())
        return ret;
    ret = surfaces[0].getTriEquivs();
    for (size_t ii=1; ii<surfaces.size(); ++ii)
        fgAppend(ret,surfaces[ii].getTriEquivs());
    return ret;
}

size_t
Fg3dMesh::numTris() const
{
    size_t      ret = 0;
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        ret += surfaces[ss].tris.vertInds.size();
    return ret;
}

size_t
Fg3dMesh::numQuads() const
{
    size_t      ret = 0;
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        ret += surfaces[ss].quads.vertInds.size();
    return ret;
}

size_t
Fg3dMesh::surfPointNum() const
{
    size_t          tot = 0;
    for (size_t ss=0; ss<surfaces.size(); ss++)
        tot += surfaces[ss].surfPoints.size();
    return tot;
}

FgVect3F
Fg3dMesh::surfPointPos(const FgVerts & verts_,size_t num) const
{
    for (size_t ss=0; ss<surfaces.size(); ss++) {
        if (num < surfaces[ss].surfPoints.size())
            return surfaces[ss].surfPointPos(verts_,num);
        else
            num -= surfaces[ss].surfPoints.size();
    }
    FGASSERT_FALSE;
    return FgVect3F(0);        // Avoid warning.
}

FgOpt<FgVect3F>
Fg3dMesh::surfPointPos(const string & label) const
{
    FgOpt<FgVect3F>    ret;
    for (size_t ss=0; ss<surfaces.size(); ++ss) {
        size_t  idx = fgFindFirstIdx(surfaces[ss].surfPoints,label);
        if (idx < surfaces[ss].surfPoints.size()) {
            ret = surfaces[ss].surfPointPos(verts,idx);
            break;
        }
    }
    return ret;
}

vector<FgVertLabel>
Fg3dMesh::surfPointsAsVertLabels() const
{
    vector<FgVertLabel>     ret;
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        fgAppend(ret,surfaces[ss].surfPointsAsVertLabels(verts));
    return ret;
}

FgString
Fg3dMesh::morphName(size_t idx) const
{
    if (idx < deltaMorphs.size())
        return deltaMorphs[idx].name;
    idx -= deltaMorphs.size();
    return targetMorphs[idx].name;
}

FgStrings
Fg3dMesh::morphNames() const
{
    FgStrings    ret;
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        ret.push_back(deltaMorphs[ii].name);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        ret.push_back(targetMorphs[ii].name);
    return ret;
}

FgValid<size_t>
Fg3dMesh::findDeltaMorph(const FgString & name_) const
{
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        if (FgString(deltaMorphs[ii].name) == name_)
            return FgValid<size_t>(ii);
    return FgValid<size_t>();
}

FgValid<size_t>
Fg3dMesh::findTargMorph(const FgString & name_) const
{
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        if (targetMorphs[ii].name == name_)
            return FgValid<size_t>(ii);
    return FgValid<size_t>();
}

FgValid<size_t>
Fg3dMesh::findMorph(const FgString & name_) const
{
    for (size_t ii=0; ii<numMorphs(); ++ii)
        if (morphName(ii) == name_)
            return FgValid<size_t>(ii);
    return FgValid<size_t>();
}

void
Fg3dMesh::morph(
    const FgFlts &      morphCoord,
    FgVerts &           outVerts) const
{
    FGASSERT(morphCoord.size() == numMorphs());
    outVerts = verts;
    size_t      cnt = 0;
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        deltaMorphs[ii].applyAsDelta(outVerts,morphCoord[cnt++]);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        targetMorphs[ii].applyAsTarget_(verts,morphCoord[cnt++],outVerts);
}

void
Fg3dMesh::morph(
    const FgVerts &     allVerts,
    const FgFlts &      coord,
    FgVerts &           outVerts) const
{
    outVerts = fgHead(allVerts,verts.size());
    size_t          ndms = deltaMorphs.size();
    fgAccDeltaMorphs(deltaMorphs,fgHead(coord,ndms),outVerts);
    fgAccTargetMorphs(allVerts,targetMorphs,fgRest(coord,ndms),outVerts);
}

FgVerts
Fg3dMesh::morph(
    const FgFlts &      deltaMorphCoord,
    const FgFlts &      targMorphCoord) const
{
    FgVerts     ret = verts;
    FGASSERT(deltaMorphCoord.size() == deltaMorphs.size());
    FGASSERT(targMorphCoord.size() == targetMorphs.size());
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        if (deltaMorphCoord[ii] != 0.0f)
            deltaMorphs[ii].applyAsDelta(ret,deltaMorphCoord[ii]);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        if (targMorphCoord[ii] != 0.0f)
            targetMorphs[ii].applyAsTarget_(verts,targMorphCoord[ii],ret);
    return ret;
}

FgVerts
Fg3dMesh::morphSingle(size_t idx,float val) const
{
    FgVerts     ret = verts;
    if (idx < deltaMorphs.size())
        deltaMorphs[idx].applyAsDelta(ret,val);
    else {
        idx -= deltaMorphs.size();
        FGASSERT(idx < targetMorphs.size());
        targetMorphs[idx].applyAsTarget_(verts,val,ret);
    }
    return ret;
}

FgIndexedMorph
Fg3dMesh::getMorphAsIndexedDelta(size_t idx) const
{
    float               tol = fgSqr(fgMaxElem(fgDims(verts)) * 0.0001);
    FgIndexedMorph      ret;
    if (idx < deltaMorphs.size()) {
        const FgMorph & dm = deltaMorphs[idx];
        ret.name = dm.name;
        for (size_t ii=0; ii<dm.verts.size(); ++ii) {
            if (dm.verts[ii].mag() > tol) {
                ret.baseInds.push_back(uint32(ii));
                ret.verts.push_back(dm.verts[ii]);
            }
        }
    }
    else {
        idx -= deltaMorphs.size();
        FGASSERT(idx < targetMorphs.size());
        const FgIndexedMorph &  tm = targetMorphs[idx];
        ret.name = tm.name;
        ret.baseInds = tm.baseInds;
        ret.verts = tm.verts;
        for (size_t ii=0; ii<ret.verts.size(); ++ii)
            ret.verts[ii] -= verts[ret.baseInds[ii]];
    }
    return ret;
}

void
Fg3dMesh::addDeltaMorph(const FgMorph & morph)
{
    FgValid<size_t>         idx = findDeltaMorph(morph.name);
    if (idx.valid()) {
        fgout << fgnl << "WARNING: Overwriting existing morph " << morph.name;
        deltaMorphs[idx.val()] = morph;
    }
    else
        deltaMorphs.push_back(morph);
}

void
Fg3dMesh::addDeltaMorphFromTarget(const FgString & name_,const FgVerts & targetShape)
{
    FgMorph                 dm;
    dm.name = name_;
    dm.verts = targetShape - verts;
    addDeltaMorph(dm);
}

void
Fg3dMesh::addTargMorph(const FgIndexedMorph & morph)
{
    FGASSERT(fgMax(morph.baseInds) < verts.size());
    FgValid<size_t>     idx = findTargMorph(morph.name);
    if (idx.valid()) {
        fgout << fgnl << "WARNING: Overwriting existing morph " << morph.name;
        targetMorphs[idx.val()] = morph;
    }
    else
        targetMorphs.push_back(morph);
}

void
Fg3dMesh::addTargMorph(const FgString & name_,const FgVerts & targetShape)
{
    FGASSERT(targetShape.size() == verts.size());
    FgIndexedMorph       tm;
    tm.name = name_;
    FgVerts             deltas = targetShape - verts;
    float               maxMag = 0.0f;
    for (size_t ii=0; ii<deltas.size(); ++ii)
        fgSetIfGreater(maxMag,deltas[ii].mag());
    if (maxMag == 0.0f)
        fgThrow("Attempt to create empty target morph");
    maxMag *= fgSqr(0.001f);
    for (size_t ii=0; ii<deltas.size(); ++ii) {
        if (deltas[ii].mag() > maxMag) {
            tm.baseInds.push_back(uint(ii));
            tm.verts.push_back(targetShape[ii]);
        }
    }
    addTargMorph(tm);
}

FgVerts
Fg3dMesh::poseShape(const FgVerts & allVerts,const std::map<FgString,float> & poseVals) const
{
    FgVerts     ret = fgHead(allVerts,verts.size()),
                base = ret;
    fgPoseDeltas(poseVals,deltaMorphs,ret);
    fgPoseDeltas(poseVals,targetMorphs,base,fgRest(allVerts,verts.size()),ret);
    return ret;
}

void
Fg3dMesh::addSurfaces(
    const std::vector<Fg3dSurface> & surfs)
{
    for (size_t ss=0; ss<surfs.size(); ++ss)
        surfaces.push_back(surfs[ss]);
    checkValidity();
}

void
Fg3dMesh::transform(FgMat33F xform)
{
    fgTransform_(verts,verts,xform);
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        fgTransform_(deltaMorphs[ii].verts,deltaMorphs[ii].verts,xform);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        fgTransform_(targetMorphs[ii].verts,targetMorphs[ii].verts,xform);
}

void
Fg3dMesh::transform(FgAffine3F xform)
{
    fgTransform_(verts,verts,xform);
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        fgTransform_(deltaMorphs[ii].verts,deltaMorphs[ii].verts,xform.linear);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        fgTransform_(targetMorphs[ii].verts,targetMorphs[ii].verts,xform);
}

void
Fg3dMesh::mergeAllSurfaces()
{
    if (surfaces.size() > 1) {
        for (uint ss=1; ss<surfaces.size(); ++ss)
            surfaces[0].merge(surfaces[ss]);
        surfaces.resize(1);
    }
}

void
Fg3dMesh::convertToTris()
{
    for (size_t ii=0; ii<surfaces.size(); ++ii)
        surfaces[ii] = surfaces[ii].convertToTris();
}

void
Fg3dMesh::checkValidity()
{
    uint    numVerts = uint(verts.size());
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        surfaces[ss].checkMeshConsistency(numVerts,uint(uvs.size()));
    for (size_t mm=0; mm<markedVerts.size(); ++mm)
        FGASSERT(markedVerts[mm].idx < numVerts);
}

void
fgReadp(std::istream & is,Fg3dMesh & mesh)
{
    fgReadp(is,mesh.verts);
    fgReadp(is,mesh.uvs);
    fgReadp(is,mesh.surfaces);
    fgReadp(is,mesh.deltaMorphs);
    fgReadp(is,mesh.targetMorphs);
    fgReadp(is,mesh.markedVerts);
}

void
fgWritep(std::ostream & os,const Fg3dMesh & mesh)
{
    fgWritep(os,mesh.verts);
    fgWritep(os,mesh.uvs);
    fgWritep(os,mesh.surfaces);
    fgWritep(os,mesh.deltaMorphs);
    fgWritep(os,mesh.targetMorphs);
    fgWritep(os,mesh.markedVerts);
}

Fg3dMesh
fg3dMesh(const FgVect3UIs & tris,const FgVerts & verts)
{
    Fg3dMesh        ret;
    ret.surfaces.resize(1);
    ret.surfaces[0].tris.vertInds = tris;
    ret.verts = verts;
    return ret;
}

std::set<FgString>
fgMorphs(const Fg3dMeshes & meshes)
{
    std::set<FgString>  ret;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        fgAppend(ret,meshes[ii].morphNames());
    return ret;
}

FgPoses
fgPoses(const Fg3dMeshes & meshes)
{
    std::set<FgPose>    ps;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        fgAppend(ps,fgPoses(meshes[ii]));
    return vector<FgPose>(ps.begin(),ps.end());
}

static
bool
vertLt(const FgVect3F & v0,const FgVect3F & v1)
{
    if (v0[0] == v1[0]) {
        if (v0[1] == v1[1])
            return (v0[2] < v1[2]);
        else
            return (v0[1] < v1[1]);
    }
    else
        return (v0[0] < v1[0]);
}

std::ostream &
operator<<(std::ostream & os,const Fg3dMesh & m)
{
    FgMat32F bounds = fgBounds(m.verts);
    os << fgnl
        << "Name: " << m.name
        << fgnl << "Verts: " << m.verts.size() << "  "
        << fgnl << "UVs: " << m.uvs.size() << "  "
        << fgnl << "Bounding Box: " << bounds
        << fgnl << "Delta Morphs: " << m.deltaMorphs.size()
        << fgnl << "Target Morphs: " << m.targetMorphs.size()
        << fgnl << "Marked Verts: " << m.markedVerts.size()
        << fgpush;
    for (size_t ii=0; ii<m.markedVerts.size(); ++ii)
        if (!m.markedVerts[ii].label.empty())
            os << fgnl << m.markedVerts[ii].label;
    os << fgpop;
    for (size_t ss=0; ss<m.surfaces.size(); ss++) {
        const Fg3dSurface &     surf = m.surfaces[ss];
        os << fgnl << "Surface " << ss << ": " << surf.name << fgpush << surf << fgpop;
        if (surf.albedoMap)
            os << fgnl << "Albedo: " << *surf.albedoMap;
    }
    FgVerts             sortVerts = m.verts;
    std::sort(sortVerts.begin(),sortVerts.end(),vertLt);
    size_t              numDups = 0;
    for (size_t ii=1; ii<sortVerts.size(); ++ii)
        if (sortVerts[ii] == sortVerts[ii-1])
            ++numDups;
    os << fgnl << "Duplicate vertices: " << numDups;
    Fg3dTopology        topo(m.verts,m.getTriEquivs().vertInds);
    FgVect3UI           te = topo.isManifold();
    os << fgnl << "Watertight: ";
    if (te == FgVect3UI(0))
        os << "YES";
    else
        os << "NO (" << te[0] << " boundary edges)";
    os << fgnl << "Manifold: ";
    if ((te[1] == 0) && (te[2] == 0))
        os << "YES";
    else
        os << "NO (" << te[1] << " intersection edges, " << te[2] << " reversed edges)";
    os << fgnl << "Seams: " << topo.seams().size()
        << fgnl << "Unused verts: " << topo.unusedVerts();
    return os;
}

std::ostream &
operator<<(std::ostream & os,const Fg3dMeshes & ms)
{
    for (size_t ii=0; ii<ms.size(); ++ii)
        os << fgnl << "Mesh " << ii << ":" << fgpush << ms[ii] << fgpop;
    return os;
}

static
Fg3dSurface
subdivideTris(
    const vector<FgVect3UI> &       tris,
    const vector<FgSurfPoint> &     sps,
    const vector<Fg3dTopology::Tri> &   topoTris,
    uint                            newVertsBaseIdx)
{
    Fg3dSurface     ret;
    for (size_t ii=0; ii<tris.size(); ii++) {
        FgVect3UI   vertInds = tris[ii];
        FgVect3UI   edgeInds = topoTris[ii].edgeInds;
        uint        ni0 = newVertsBaseIdx + edgeInds[0],
                    ni1 = newVertsBaseIdx + edgeInds[1],
                    ni2 = newVertsBaseIdx + edgeInds[2];
        ret.tris.vertInds.push_back(FgVect3UI(vertInds[0],ni0,ni2));
        ret.tris.vertInds.push_back(FgVect3UI(vertInds[1],ni1,ni0));
        ret.tris.vertInds.push_back(FgVect3UI(vertInds[2],ni2,ni1));
        ret.tris.vertInds.push_back(FgVect3UI(ni0,ni1,ni2));
    }
    // Set up surface point weight transforms:
    FgMat33F        wgtXform(1),
                    wgtXform0(0),
                    wgtXform1(0),
                    wgtXform2(0);
    wgtXform.cr(2,0) = -1.0;
    wgtXform.cr(0,1) = -1.0;
    wgtXform.cr(1,2) = -1.0;
    wgtXform0[0] = 1.0f;
    wgtXform0[1] = -1.0f;
    wgtXform0[2] = -1.0f;
    wgtXform0.cr(1,1) = 2;
    wgtXform0.cr(2,2) = 2;
    wgtXform1[0] = -1.0f;
    wgtXform1[1] = 1.0f;
    wgtXform1[2] = -1.0f;
    wgtXform1.cr(2,1) = 2;
    wgtXform1.cr(0,2) = 2;
    wgtXform2[0] = -1.0f;
    wgtXform2[1] = -1.0f;
    wgtXform2[2] = 1.0f;
    wgtXform2.cr(0,1) = 2;
    wgtXform2.cr(1,2) = 2;
    // Update surface points:
    for (size_t ii=0; ii<sps.size(); ++ii) {
        uint        facetIdx = sps[ii].triEquivIdx * 4;
        FgVect3F    weights = sps[ii].weights,
                    wgtCentre = wgtXform * weights;
        if (wgtCentre[0] < 0.0)
            ret.surfPoints.push_back(FgSurfPoint(facetIdx+2,wgtXform2*weights));
        else if (wgtCentre[1] < 0.0)
            ret.surfPoints.push_back(FgSurfPoint(facetIdx,wgtXform0*weights));
        else if (wgtCentre[2] < 0.0)
            ret.surfPoints.push_back(FgSurfPoint(facetIdx+1,wgtXform1*weights));
        else
            ret.surfPoints.push_back(FgSurfPoint(facetIdx+3,wgtCentre));
    }
    return ret;
}

Fg3dMesh
fgSubdivide(const Fg3dMesh & in,bool loop)
{
    Fg3dMesh            ret;
    vector<FgVect3UI>   allTris;
    vector<FgSurfPoint> allSps;
    for (size_t ss=0; ss<in.surfaces.size(); ++ss) {
        const Fg3dSurface & surf = in.surfaces[ss];
        for (size_t ii=0; ii<surf.surfPoints.size(); ++ii) {
            FgSurfPoint     sp = surf.surfPoints[ii];
            sp.triEquivIdx += uint(allTris.size());
            allSps.push_back(sp);
        }
        fgAppend(allTris,surf.tris.vertInds);
    }
    ret.verts = in.verts;   // Modified later in case of Loop:
    ret.surfaces.resize(in.surfaces.size());
    Fg3dTopology        topo(in.verts,allTris);
    uint                newVertsBaseIdx = uint(in.verts.size());
    if (loop) {
        // Add the edge-split "odd" verts:
        for (uint ii=0; ii<topo.m_edges.size(); ++ii) {
            FgVect2UI       vertInds0 = topo.m_edges[ii].vertInds;
            if (topo.m_edges[ii].triInds.size() == 1) {     // Boundary
                ret.verts.push_back((
                    in.verts[vertInds0[0]] + 
                    in.verts[vertInds0[1]])*0.5f);
            }
            else {
                FgVect2UI   vertInds1 = topo.edgeFacingVertInds(ii);
                ret.verts.push_back((
                    in.verts[vertInds0[0]] * 3.0f +
                    in.verts[vertInds0[1]] * 3.0f +
                    in.verts[vertInds1[0]] +
                    in.verts[vertInds1[1]]) * 0.125f);
            }
        }
        // Modify the original "even" verts:
        for (uint ii=0; ii<newVertsBaseIdx; ++ii) {
            if (topo.vertOnBoundary(ii)) {
                vector<uint>    vertInds = topo.vertBoundaryNeighbours(ii);
                if (vertInds.size() != 2)
                    fgThrow("Cannot subdivide non-manifold mesh at vert index",fgToString(ii));
                ret.verts[ii] = (in.verts[ii] * 6.0 + in.verts[vertInds[0]] + in.verts[vertInds[1]]) * 0.125f;
            }
            else {
                // Note that there will always be at least 3 neighbours since 
                // this is not a boundary vertex:
                const vector<uint> &    neighbours = topo.vertNeighbours(ii);
                FgVect3F    acc;
                for (size_t jj=0; jj<neighbours.size(); ++jj)
                    acc += in.verts[neighbours[jj]];
                if (neighbours.size() == 3)
                    ret.verts[ii] = in.verts[ii] * 0.4375f + acc * 0.1875f;
                else if (neighbours.size() == 4)
                    ret.verts[ii] = in.verts[ii] * 0.515625f + acc * 0.12109375f;
                else if (neighbours.size() == 5)
                    ret.verts[ii] = in.verts[ii] * 0.579534f + acc * 0.0840932f;
                else
                    ret.verts[ii] = in.verts[ii] * 0.625f + acc * 0.375f / float(neighbours.size());
            }
        }
    }
    else {
        for (size_t ii=0; ii<topo.m_edges.size(); ++ii) {
            FgVect2UI       vertInds = topo.m_edges[ii].vertInds;
            ret.verts.push_back((in.verts[vertInds[0]]+in.verts[vertInds[1]])*0.5);
        }
    }
    Fg3dSurface     ssurf = subdivideTris(allTris,allSps,topo.m_tris,newVertsBaseIdx);
    size_t          sidx = 0;
    size_t          spidx = 0;
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss) {
        Fg3dSurface & surf = ret.surfaces[ss];
        const Fg3dSurface & inSurf = in.surfaces[ss];
        size_t      num = inSurf.tris.vertInds.size() * 4;
        fgAppend(surf.tris.vertInds,fgSubvec(ssurf.tris.vertInds,sidx,num));    // Clearer (and slower) than iterators
        for (size_t ii=0; ii<inSurf.surfPoints.size(); ++ii) {
            FgSurfPoint     sp = ssurf.surfPoints[spidx+ii];
            sp.triEquivIdx -= uint(sidx);
            surf.surfPoints.push_back(sp);
        }
        sidx += num;
        spidx += inSurf.surfPoints.size();
    }
    return ret;
}

