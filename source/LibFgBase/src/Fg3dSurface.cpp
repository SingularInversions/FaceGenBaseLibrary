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

void
fgReadp(std::istream & is,FgSurfPoint & sp)
{
    fgReadp(is,sp.triEquivIdx);
    fgReadp(is,sp.weights);
    fgReadp(is,sp.label);
}

void
fgWritep(std::ostream & os,const FgSurfPoint & sp)
{
    fgWritep(os,sp.triEquivIdx);
    fgWritep(os,sp.weights);
    fgWritep(os,sp.label);
}

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

uint
Fg3dSurface::vertIdxMax() const
{
    uint        ret = 0;
    for(size_t qq=0; qq<quads.vertInds.size(); ++qq)
        for(uint ii=0; ii<4; ++ii)
            fgSetIfGreater(ret,quads.vertInds[qq][ii]);
    for(size_t tt=0; tt<tris.vertInds.size(); ++tt)
        for(uint ii=0; ii<3; ++ii)
            fgSetIfGreater(ret,tris.vertInds[tt][ii]);
    return ret;
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

FgVect3F
Fg3dSurface::surfPointPos(const FgVerts & verts,size_t idx) const
{
    FgVect3UI   vertInds = getTriEquiv(surfPoints[idx].triEquivIdx);
    FgVect3F    vertWeights = surfPoints[idx].weights;
    return (verts[vertInds[0]] * vertWeights[0] +
            verts[vertInds[1]] * vertWeights[1] +
            verts[vertInds[2]] * vertWeights[2]);
}

FgVect3F
Fg3dSurface::surfPointPos(const FgVerts & verts,const string & label) const
{
    const FgSurfPoint & sp = fgFindFirst(surfPoints,label);
    FgVect3UI           tri = getTriEquiv(sp.triEquivIdx);
    return fgBarycentricPos(tri,sp.weights,verts);
}

vector<FgVertLabel>
Fg3dSurface::surfPointsAsVertLabels(const FgVerts & verts) const
{
    vector<FgVertLabel>     ret;
    ret.reserve(surfPoints.size());
    for (size_t ii=0; ii<surfPoints.size(); ++ii) {
        FgVertLabel         vl;
        vl.label = surfPoints[ii].label;
        vl.vert = surfPointPos(verts,ii);
        ret.push_back(vl);
    }
    return ret;
}

Fg3dSurface
Fg3dSurface::convertToTris() const
{
    Fg3dSurface     ret;
    ret.name = name;
    ret.tris.vertInds = fgConcat(tris.vertInds,convToTris(quads.vertInds));
    ret.tris.uvInds = fgConcat(tris.uvInds,convToTris(quads.uvInds));
    ret.surfPoints = surfPoints;
    return ret;
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
        {FGASSERT(fgMaxElem(fgBounds(tris.vertInds)) < coordsSize); }
    if (quads.vertInds.size() > 0)
        {FGASSERT(fgMaxElem(fgBounds(quads.vertInds)) < coordsSize); }
    if (tris.uvInds.size() > 0)
        {FGASSERT(fgMaxElem(fgBounds(tris.uvInds)) < uvsSize); }
    if (quads.uvInds.size() > 0)
        {FGASSERT(fgMaxElem(fgBounds(quads.uvInds)) < uvsSize); }
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

void
fgReadp(std::istream & is,Fg3dSurface & s)
{
    fgReadp(is,s.name);
    fgReadp(is,s.tris);
    fgReadp(is,s.quads);
    fgReadp(is,s.surfPoints);
}

void
fgWritep(std::ostream & os,const Fg3dSurface & s)
{
    fgWritep(os,s.name);
    fgWritep(os,s.tris);
    fgWritep(os,s.quads);
    fgWritep(os,s.surfPoints);
}

ostream &
operator<<(ostream & os,const Fg3dSurface & surf)
{
    os << fgnl << "Tris: " << surf.numTris()
        << "  Quads: " << surf.numQuads()
        << "  UVs: " << (surf.hasUvIndices() ? "YES" : "NO")
        << fgnl << "Surf Points: " << surf.surfPoints.size() << fgpush;
        for (size_t ii=0; ii<surf.surfPoints.size(); ++ii)
            os << fgnl << ii << ": " << surf.surfPoints[ii].label;
        os << fgpop;
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

Fg3dSurface
fgMergeSurfaces(const vector<Fg3dSurface> & surfs)
{
    Fg3dSurface     ret;
    for (size_t ii=0; ii<surfs.size(); ++ii)
        ret.merge(surfs[ii]);
    return ret;
}

vector<Fg3dSurface>
fgSplitSurface(const Fg3dSurface & surf)
{
    vector<Fg3dSurface>     ret;
    FGASSERT(!surf.empty());
    // Construct a map from vert inds back to triEquivs (FgTopology is overkill for this):
    uint                    idxBnd = surf.vertIdxMax() + 1;
    vector<vector<uint> >   vertIdxToTriIdx(idxBnd);
    for (uint tt=0; tt<surf.numTriEquivs(); ++tt) {
        FgVect3UI           vertInds = surf.getTriEquiv(tt);
        for (uint jj=0; jj<3; ++jj)
            vertIdxToTriIdx[vertInds[jj]].push_back(tt);
    }
    // Start with separate group for each tri, then merge until no change:
    vector<uint>            groups(surf.numTriEquivs());
    for (uint ii=0; ii<groups.size(); ++ii)
        groups[ii] = ii;
    bool                    done = false;
    while (!done) {
        done = true;
        for (uint tt=0; tt<surf.numTriEquivs(); ++tt) {
            FgVect3UI       vertInds = surf.getTriEquiv(tt);
            for (uint jj=0; jj<3; ++jj) {
                const vector<uint> & triInds = vertIdxToTriIdx[vertInds[jj]];
                for (uint kk=0; kk<triInds.size(); ++kk) {
                    uint    ntt = triInds[kk];
                    if (groups[tt] != groups[ntt]) {
                        fgReplace_(groups,groups[ntt],groups[tt]);
                        done = false;
                    }
                }
            }
        }
    }
    set<uint>           surfGroups(groups.begin(),groups.end());
    for (set<uint>::const_iterator it(surfGroups.begin()); it != surfGroups.end(); ++it) {
        uint            groupVal = *it;
        Fg3dSurface     s;
        for (size_t ii=0; ii<surf.tris.vertInds.size(); ++ii) {
            if (groups[ii] == groupVal) {
                s.tris.vertInds.push_back(surf.tris.vertInds[ii]);
                if (!surf.tris.uvInds.empty())
                    s.tris.uvInds.push_back(surf.tris.uvInds[ii]);
            }
        }
        for (size_t ii=0; ii<surf.quads.vertInds.size(); ++ii) {
            uint        triEquiv = uint(surf.tris.vertInds.size() + 2 * ii);
            if (groups[triEquiv] == groupVal) {
                s.quads.vertInds.push_back(surf.quads.vertInds[ii]);
                if (!surf.quads.uvInds.empty())
                    s.quads.uvInds.push_back(surf.quads.uvInds[ii]);
            }
        }
        ret.push_back(s);
    }
    return ret;
}

vector<Fg3dSurface>
fgEnsureNamed(const vector<Fg3dSurface> & surfs,const FgString & baseName)
{
    vector<Fg3dSurface>     ret = surfs;
    if ((ret.size() == 1) && (ret[0].name.empty()))
        ret[0].name = baseName;
    else {
        size_t                  cnt = 0;
        for (size_t ss=0; ss<ret.size(); ++ss)
            if (ret[ss].name.empty())
                ret[ss].name = baseName + fgToString(cnt++);
    }
    return ret;
}

void
Fg3dSurface::removeTri(size_t triIdx)
{
    FGASSERT(quads.vertInds.empty());
    FgSurfPoints        nsps;
    for (size_t ii=0; ii<surfPoints.size(); ++ii) {
        FgSurfPoint     sp = surfPoints[ii];
        if (sp.triEquivIdx < triIdx)
            nsps.push_back(sp);
        else if (sp.triEquivIdx > triIdx) {
            --sp.triEquivIdx;
            nsps.push_back(sp);
        }
    }
    surfPoints = nsps;
    tris.vertInds.erase(tris.vertInds.begin()+triIdx);
}

// */
