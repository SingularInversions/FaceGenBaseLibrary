//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 22, 2005
//

#include "stdafx.h"

#include "Fg3dSurface.hpp"
#include "Fg3dTopology.hpp"
#include "FgBounds.hpp"
#include "FgOpt.hpp"
#include "FgStdString.hpp"

using namespace std;

Fg3dSurface::Fg3dSurface(
    const std::vector<FgVect3UI> & tris_,
    const std::vector<FgVect4UI> & quads_,
    const std::vector<FgVect3UI> & tris_uvinds,
    const std::vector<FgVect4UI> & quads_uvinds,
    const std::vector<FgSurfPoint> & surfPoints_)
    :
    tris(tris_,tris_uvinds),
    quads(quads_,quads_uvinds),
    surfPoints(surfPoints_)
{
    checkInternalConsistency();
}

Fg3dSurface::Fg3dSurface(
    const std::vector<FgVect4UI> &  quads_,
    const std::vector<FgVect4UI> &  texs)
    :
    quads(quads_,texs)
{
    checkInternalConsistency();
}

std::set<uint>
Fg3dSurface::vertsUsed() const
{
    std::set<uint>  ret;
    for(size_t qq = 0; qq < quads.vertInds.size(); ++qq)
        for(uint ii = 0; ii < 4; ++ii)
            ret.insert(quads.vertInds[qq][ii]);
    for(size_t tt = 0; tt < tris.vertInds.size(); ++tt)
        for(uint ii = 0; ii < 3; ++ii)
            ret.insert(tris.vertInds[tt][ii]);
    return ret;
}

FgVect3UI
Fg3dSurface::getTriEquiv(uint tt) const
{
    if (tt < tris.vertInds.size())
        return tris.vertInds[tt];
    else
    {
        tt -= uint(tris.vertInds.size());
        uint    qq = tt >> 1;
        FGASSERT(qq < quads.vertInds.size());
        if (tt & 0x01)
            return 
                FgVect3UI(
                    quads.vertInds[qq][2],
                    quads.vertInds[qq][3],
                    quads.vertInds[qq][0]);
        else
            return
                FgVect3UI(
                    quads.vertInds[qq][0],
                    quads.vertInds[qq][1],
                    quads.vertInds[qq][2]);
	}
}

FgFacetInds<3>
Fg3dSurface::getTriEquivs() const
{
    FgFacetInds<3>      ret = tris;
    for (size_t ii=0; ii<quads.vertInds.size(); ++ii) {
        FgVect4UI       quad = quads.vertInds[ii];
        ret.vertInds.push_back(FgVect3UI(quad[0],quad[1],quad[2]));
        ret.vertInds.push_back(FgVect3UI(quad[2],quad[3],quad[0]));
    }
    for (size_t ii=0; ii<quads.uvInds.size(); ++ii) {
        FgVect4UI       quad = quads.uvInds[ii];
        ret.uvInds.push_back(FgVect3UI(quad[0],quad[1],quad[2]));
        ret.uvInds.push_back(FgVect3UI(quad[2],quad[3],quad[0]));
    }
    return ret;
}

static
vector<FgVect3UI>
convToTris(const vector<FgVect4UI> & quads)
{
    vector<FgVect3UI>   ret;
    for (size_t ii=0; ii<quads.size(); ++ii) {
        const FgVect4UI &   quad = quads[ii];
        // Ordering must match triEquiv ordering for surface point to remain valid:
        ret.push_back(FgVect3UI(quad[0],quad[1],quad[2]));
        ret.push_back(FgVect3UI(quad[2],quad[3],quad[0]));
    }
    return ret;
}

vector<FgVertLabel>
Fg3dSurface::surfPointsAsVertLabels(const FgVerts & verts) const
{
    vector<FgVertLabel>     ret;
    ret.reserve(surfPoints.size());
    for (size_t ii=0; ii<surfPoints.size(); ++ii) {
        FgVertLabel         vl;
        vl.label = surfPoints[ii].label;
        vl.vert = getSurfPoint(verts,ii);
        ret.push_back(vl);
    }
    return ret;
}

Fg3dSurface
Fg3dSurface::convertToTris() const
{
    Fg3dSurface     ret;
    ret.name = name;
    ret.material = material;
    ret.tris.vertInds = fgConcat(tris.vertInds,convToTris(quads.vertInds));
    ret.tris.uvInds = fgConcat(tris.uvInds,convToTris(quads.uvInds));
    ret.surfPoints = surfPoints;
    return ret;
}

Fg3dSurface
Fg3dSurface::subdivideFlat(
    FgVerts &       verts) const
{
    FGASSERT(numQuads() == 0);              // Quads not supported.
    Fg3dTopology    topo(verts,tris.vertInds);

    // Add the edge-split verts:
    uint        newVertsBaseIdx = uint(verts.size());
    for (size_t ii=0; ii<topo.m_edges.size(); ++ii) {
        FgVect2UI   vertInds = topo.m_edges[ii].vertInds;
        verts.push_back((verts[vertInds[0]]+verts[vertInds[1]])*0.5);
    }

    vector<FgVect3UI>       tris;
    vector<FgSurfPoint>     surfPoints;
    subdivideTris(topo,newVertsBaseIdx,tris,surfPoints);

    return Fg3dSurface(
        tris,
        std::vector<FgVect4UI>(),
        std::vector<FgVect3UI>(),
        std::vector<FgVect4UI>(),
        surfPoints);
}

Fg3dSurface
Fg3dSurface::subdivideLoop(
    const FgVerts &     vertsIn,
    FgVerts &           vertsOut) const
{
    FGASSERT(numQuads() == 0);
    vertsOut.resize(vertsIn.size());
    Fg3dTopology    topo(vertsIn,tris.vertInds);

    // Add the edge-split "odd" verts:
    uint        newVertsBaseIdx = uint(vertsIn.size());
    for (uint ii=0; ii<topo.m_edges.size(); ++ii) {
        FgVect2UI   vertInds0 = topo.m_edges[ii].vertInds;
        if (topo.m_edges[ii].triInds.size() == 1) {     // Boundary
            vertsOut.push_back((
                vertsIn[vertInds0[0]] + 
                vertsIn[vertInds0[1]])*0.5f);
        }
        else {
            FgVect2UI   vertInds1 = topo.edgeFacingVertInds(ii);
            vertsOut.push_back((
                vertsIn[vertInds0[0]] * 3.0f +
                vertsIn[vertInds0[1]] * 3.0f +
                vertsIn[vertInds1[0]] +
                vertsIn[vertInds1[1]]) * 0.125f);
        }
    }
    // Modify the original "even" verts:
    for (uint ii=0; ii<newVertsBaseIdx; ++ii) {
        if (topo.vertOnBoundary(ii)) {
            vector<uint>    vertInds = topo.vertBoundaryNeighbours(ii);
            if (vertInds.size() != 2)
                fgThrow(
                    "Cannot subdivide non-manifold mesh, invalid boundary vertex neighbour count at index",
                    fgToString(vertInds.size()) + fgToString(ii));
            vertsOut[ii] = (vertsIn[ii] * 6.0 +
                            vertsIn[vertInds[0]] +
                            vertsIn[vertInds[1]]) * 0.125f;
        }
        else {
            // Note that there will always be at least 3 neighbours since 
            // this is not a boundary vertex:
            const vector<uint> &    neighbours = topo.vertNeighbours(ii);
            FgVect3F    acc;
            for (size_t jj=0; jj<neighbours.size(); ++jj)
                acc += vertsIn[neighbours[jj]];
            if (neighbours.size() == 3)
                vertsOut[ii] = vertsIn[ii] * 0.4375f + acc * 0.1875f;
            else if (neighbours.size() == 4)
                vertsOut[ii] = vertsIn[ii] * 0.515625f + acc * 0.12109375f;
            else if (neighbours.size() == 5)
                vertsOut[ii] = vertsIn[ii] * 0.579534f + acc * 0.0840932f;
            else
                vertsOut[ii] = vertsIn[ii] * 0.625f + acc * 0.375f / float(neighbours.size());
        }
    }

    vector<FgVect3UI>       tris;
    vector<FgSurfPoint>     surfPoints;
    subdivideTris(topo,newVertsBaseIdx,tris,surfPoints);

    return Fg3dSurface(
        tris,
        std::vector<FgVect4UI>(),
        std::vector<FgVect3UI>(),
        std::vector<FgVect4UI>(),
        surfPoints);
}

void
Fg3dSurface::merge(const Fg3dSurface & surf)
{
    tris.vertInds.insert(
        tris.vertInds.end(),
        surf.tris.vertInds.begin(),
        surf.tris.vertInds.end());
    quads.vertInds.insert(
        quads.vertInds.end(),
        surf.quads.vertInds.begin(),
        surf.quads.vertInds.end());
    tris.uvInds.insert(
        tris.uvInds.end(),
        surf.tris.uvInds.begin(),
        surf.tris.uvInds.end());
    quads.uvInds.insert(
        quads.uvInds.end(),
        surf.quads.uvInds.begin(),
        surf.quads.uvInds.end());
    surfPoints.insert(
        surfPoints.end(),
        surf.surfPoints.begin(),
        surf.surfPoints.end());
}

void
Fg3dSurface::checkMeshConsistency(
    uint    coordsSize,
    uint    uvsSize)
{
    if (tris.vertInds.size() > 0)
        {FGASSERT(fgBounds(fgBounds(tris.vertInds))[1] < coordsSize); }
    if (quads.vertInds.size() > 0)
        {FGASSERT(fgBounds(fgBounds(quads.vertInds))[1] < coordsSize); }
    if (tris.uvInds.size() > 0)
        {FGASSERT(fgBounds(fgBounds(tris.uvInds))[1] < uvsSize); }
    if (quads.uvInds.size() > 0)
        {FGASSERT(fgBounds(fgBounds(quads.uvInds))[1] < uvsSize); }
}

void
Fg3dSurface::subdivideTris(
    const Fg3dTopology &    topo,
    uint                    newVertsBaseIdx,
    vector<FgVect3UI> &     newTris,           // RETURNED
    vector<FgSurfPoint> &   newSurfPoints)     // RETURNED
    const
{
    for (uint ii=0; ii<numTriEquivs(); ii++) {
        FgVect3UI   vertInds = getTriEquiv(ii);
        FgVect3UI   edgeInds = topo.m_tris[ii].edgeInds;
        uint        ni0 = newVertsBaseIdx + edgeInds[0],
                    ni1 = newVertsBaseIdx + edgeInds[1],
                    ni2 = newVertsBaseIdx + edgeInds[2];
        newTris.push_back(FgVect3UI(vertInds[0],ni0,ni2));
        newTris.push_back(FgVect3UI(vertInds[1],ni1,ni0));
        newTris.push_back(FgVect3UI(vertInds[2],ni2,ni1));
        newTris.push_back(FgVect3UI(ni0,ni1,ni2));
    }

    // Set up surface point weight transforms:
    FgMat33F     wgtXform(1.0),
                    wgtXform0,
                    wgtXform1,
                    wgtXform2;

    wgtXform.elm(2,0) = -1.0;
    wgtXform.elm(0,1) = -1.0;
    wgtXform.elm(1,2) = -1.0;

    wgtXform0[0] = 1.0f;
    wgtXform0[1] = -1.0f;
    wgtXform0[2] = -1.0f;
    wgtXform0.elm(1,1) = 2;
    wgtXform0.elm(2,2) = 2;

    wgtXform1[0] = -1.0f;
    wgtXform1[1] = 1.0f;
    wgtXform1[2] = -1.0f;
    wgtXform1.elm(2,1) = 2;
    wgtXform1.elm(0,2) = 2;

    wgtXform2[0] = -1.0f;
    wgtXform2[1] = -1.0f;
    wgtXform2[2] = 1.0f;
    wgtXform2.elm(0,1) = 2;
    wgtXform2.elm(1,2) = 2;

    // Update surface points:
    for (size_t ii=0; ii<surfPoints.size(); ++ii)
    {
        uint        facetIdx = surfPoints[ii].triEquivIdx * 4;
        FgVect3F    weights = surfPoints[ii].weights,
                    wgtCentre = wgtXform * weights;
        if (wgtCentre[0] < 0.0)
            newSurfPoints.push_back(FgSurfPoint(facetIdx+2,wgtXform2*weights));
        else if (wgtCentre[1] < 0.0)
            newSurfPoints.push_back(FgSurfPoint(facetIdx,wgtXform0*weights));
        else if (wgtCentre[2] < 0.0)
            newSurfPoints.push_back(FgSurfPoint(facetIdx+1,wgtXform1*weights));
        else
            newSurfPoints.push_back(FgSurfPoint(facetIdx+3,wgtCentre));
    }
}

void
Fg3dSurface::checkInternalConsistency()
{
    // UVs are either not present or per-facet:
    if (tris.uvInds.size() > 0)
        {FGASSERT(tris.uvInds.size() == tris.vertInds.size()); }
    if (quads.uvInds.size() > 0)
        {FGASSERT(quads.uvInds.size() == quads.vertInds.size()); }
}

void
Fg3dSurface::clear()
{
    *this = Fg3dSurface();
}

std::ostream &
operator<<(
    std::ostream &      os,
    const Fg3dSurface & surf)
{
    os << "Tris: " << surf.numTris()
       << "  Quads: " << surf.numQuads()
       << "  Surf Points: " << surf.numSurfPoints();
    return os;
}

static
inline
void
swapLt(uint & a,uint & b)
{
    if (b < a)
        std::swap(a,b);
}

struct  STri
{
    FgVect3UI   inds;

    STri(FgVect3UI i)
    {
        // Bubble sort:
        swapLt(i[0],i[1]);
        swapLt(i[1],i[2]);
        swapLt(i[0],i[1]);
        inds = i;
    }

    bool operator<(const STri & rhs) const
    {
        for (uint ii=0; ii<3; ++ii) {
            if (inds[ii] < rhs.inds[ii])
                return true;
            else if (inds[ii] == rhs.inds[ii])
                continue;
            else
                return false;
        }
        return false;
    }
};

struct  SQuad
{
    FgVect4UI   inds;

    SQuad(FgVect4UI i)
    {
        // Bubble sort:
        swapLt(i[0],i[1]);
        swapLt(i[1],i[2]);
        swapLt(i[2],i[3]);
        swapLt(i[0],i[1]);
        swapLt(i[1],i[2]);
        swapLt(i[0],i[1]);
        inds = i;
    }

    bool operator<(const SQuad & rhs) const
    {
        for (uint ii=0; ii<4; ++ii) {
            if (inds[ii] < rhs.inds[ii])
                return true;
            else if (inds[ii] == rhs.inds[ii])
                continue;
            else
                return false;
        }
        return false;
    }
};

Fg3dSurface
fgSelectTris(const Fg3dSurface & surf,const vector<FgBool> & sel)
{
    Fg3dSurface     ret;
    ret.name = surf.name;
    ret.material = surf.material;
    FGASSERT(surf.tris.vertInds.size() == sel.size());
    vector<uint>    remap(surf.tris.vertInds.size());
    uint            idx = 0;
    for (size_t ii=0; ii<surf.tris.vertInds.size(); ++ii) {
        if (sel[ii]) {
            ret.tris.vertInds.push_back(surf.tris.vertInds[ii]);
            if (!surf.tris.uvInds.empty())
                ret.tris.uvInds.push_back(surf.tris.uvInds[ii]);
            remap[ii] = idx++;
        }
    }
    for (size_t ii=0; ii<surf.surfPoints.size(); ++ii) {
        FgSurfPoint     sp = surf.surfPoints[ii];
        if ((sp.triEquivIdx < sel.size()) && (sel[sp.triEquivIdx])) {
            sp.triEquivIdx = remap[sp.triEquivIdx];
            ret.surfPoints.push_back(sp);
        }
    }
    return ret;
}

Fg3dSurface
fgRemoveDuplicateFacets(const Fg3dSurface & s)
{
    if (!s.surfPoints.empty())
        fgThrow("Duplicate facet removal with surface points not implemented");
    Fg3dSurface     ret = s;
    uint            numTris = 0,
                    numQuads = 0;
    ret.tris.vertInds.clear();
    ret.tris.uvInds.clear();
    set<STri>            ts;
    for (size_t ii=0; ii<s.tris.vertInds.size(); ++ii) {
        FgVect3UI       inds = s.tris.vertInds[ii];
        STri             tri(inds);
        if (ts.find(tri) == ts.end()) {
            ts.insert(tri);
            ret.tris.vertInds.push_back(inds);
            ret.tris.uvInds.push_back(s.tris.uvInds[ii]);
        }
        else
            ++numTris;
    }
    ret.quads.vertInds.clear();
    ret.quads.uvInds.clear();
    set<SQuad>           qs;
    for (size_t ii=0; ii<s.quads.vertInds.size(); ++ii) {
        FgVect4UI       inds = s.quads.vertInds[ii];
        SQuad            quad(inds);
        if (qs.find(quad) == qs.end()) {
            qs.insert(quad);
            ret.quads.vertInds.push_back(inds);
            ret.quads.uvInds.push_back(s.quads.uvInds[ii]);
        }
        else
            ++numQuads;
    }
    fgout << fgnl << numTris << " duplicate tris removed, "
        << numQuads << " duplicate quads removed.";
    return ret;
}

// */
