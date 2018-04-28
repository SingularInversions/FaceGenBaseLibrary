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

FgVerts
fgSelectVerts(const FgLabelledVerts & labVerts,const FgStrs & labels)
{
    FgVerts         ret;
    ret.reserve(labels.size());
    for (std::string str : labels)
        ret.push_back(fgFindFirst(labVerts,str).pos);
    return ret;
}

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

vector<FgVect3UI>
fgQuadsToTris(const vector<FgVect4UI> & quads)
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

uint
Fg3dSurface::vertIdxMax() const
{
    uint        ret = 0;
    for(size_t qq=0; qq<quads.size(); ++qq)
        for(uint ii=0; ii<4; ++ii)
            fgSetIfGreater(ret,quads.vertInds[qq][ii]);
    for(size_t tt=0; tt<tris.size(); ++tt)
        for(uint ii=0; ii<3; ++ii)
            fgSetIfGreater(ret,tris.vertInds[tt][ii]);
    return ret;
}

std::set<uint>
Fg3dSurface::vertsUsed() const
{
    std::set<uint>  ret;
    for(size_t qq = 0; qq < quads.size(); ++qq)
        for(uint ii = 0; ii < 4; ++ii)
            ret.insert(quads.vertInds[qq][ii]);
    for(size_t tt = 0; tt < tris.size(); ++tt)
        for(uint ii = 0; ii < 3; ++ii)
            ret.insert(tris.vertInds[tt][ii]);
    return ret;
}

FgVect3UI
Fg3dSurface::getTriEquiv(uint tt) const
{
    if (tt < tris.size())
        return tris.vertInds[tt];
    else
    {
        tt -= uint(tris.size());
        uint    qq = tt >> 1;
        FGASSERT(qq < quads.size());
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
    for (size_t ii=0; ii<quads.size(); ++ii) {
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

FgLabelledVerts
Fg3dSurface::surfPointsAsVertLabels(const FgVerts & verts) const
{
    FgLabelledVerts     ret;
    ret.reserve(surfPoints.size());
    for (size_t ii=0; ii<surfPoints.size(); ++ii) {
        FgLabelledVert         vl;
        vl.label = surfPoints[ii].label;
        vl.pos = surfPointPos(verts,ii);
        ret.push_back(vl);
    }
    return ret;
}

FgFacetInds<3>
Fg3dSurface::asTris() const
{
    FgFacetInds<3>      ret;
    ret.vertInds = fgCat(tris.vertInds,fgQuadsToTris(quads.vertInds));
    ret.uvInds = fgCat(tris.uvInds,fgQuadsToTris(quads.uvInds));
    return ret;
}

Fg3dSurface
Fg3dSurface::convertToTris() const
{
    Fg3dSurface     ret;
    ret.name = name;
    ret.tris = asTris();
    ret.surfPoints = surfPoints;
    return ret;
}

void
Fg3dSurface::merge(const Fg3dSurface & surf)
{
    for (size_t pp=0; pp<surf.surfPoints.size(); ++pp) {
        FgSurfPoint     sp = surf.surfPoints[pp];
        if (sp.triEquivIdx < surf.tris.size())
            sp.triEquivIdx += uint(tris.size());
        else
            sp.triEquivIdx += uint(tris.size() + 2*quads.size());
        surfPoints.push_back(sp);
    }
    fgCat_(tris.vertInds,surf.tris.vertInds);
    fgCat_(tris.uvInds,surf.tris.uvInds);
    fgCat_(quads.vertInds,surf.quads.vertInds);
    fgCat_(quads.uvInds,surf.quads.uvInds);
}

void
Fg3dSurface::checkMeshConsistency(
    uint    coordsSize,
    uint    uvsSize)
{
    if (tris.size() > 0)
        {FGASSERT(fgMaxElem(fgBounds(tris.vertInds)) < coordsSize); }
    if (quads.size() > 0)
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
        {FGASSERT(tris.uvInds.size() == tris.size()); }
    if (quads.uvInds.size() > 0)
        {FGASSERT(quads.uvInds.size() == quads.size()); }
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
    FGASSERT(surf.tris.size() == sel.size());
    vector<uint>    remap(surf.tris.size());
    uint            idx = 0;
    for (size_t ii=0; ii<surf.tris.size(); ++ii) {
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
    for (size_t ii=0; ii<s.tris.size(); ++ii) {
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
    for (size_t ii=0; ii<s.quads.size(); ++ii) {
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
    // Construct a lookup from vert inds back to triEquivs (FgTopology is overkill for this):
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
        for (size_t ii=0; ii<surf.tris.size(); ++ii) {
            if (groups[ii] == groupVal) {
                s.tris.vertInds.push_back(surf.tris.vertInds[ii]);
                if (!surf.tris.uvInds.empty())
                    s.tris.uvInds.push_back(surf.tris.uvInds[ii]);
            }
        }
        for (size_t ii=0; ii<surf.quads.size(); ++ii) {
            uint        triEquiv = uint(surf.tris.size() + 2 * ii);
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
    tris.erase(triIdx);
}

void
Fg3dSurface::removeQuad(size_t quadIdx)
{
    FgSurfPoints        nsps;
    size_t              idx0 = 2*(quadIdx/2) + tris.size(),
                        idx1 = idx0 + 1;
    for (size_t ii=0; ii<surfPoints.size(); ++ii) {
        FgSurfPoint     sp = surfPoints[ii];
        if (sp.triEquivIdx < idx0)
            nsps.push_back(sp);
        else if (sp.triEquivIdx > idx1) {
            sp.triEquivIdx -= 2;
            nsps.push_back(sp);
        }
    }
    surfPoints = nsps;
    quads.erase(quadIdx);
}

FgVerts
fgVertsUsed(const FgVect3UIs & tris,const FgVerts & verts)
{
    vector<bool>    used(verts.size(),false);
    for (const FgVect3UI & tri : tris)
        for (size_t xx=0; xx<3; ++xx)
            used.at(tri[xx]) = true;
    FgVerts         ret;
    for (size_t ii=0; ii<verts.size(); ++ii)
        if (used[ii])
            ret.push_back(verts[ii]);
    return ret;
}

bool
fgHasUnusedVerts(const FgVect3UIs & tris,const FgVerts & verts)
{
    vector<bool>    unused(verts.size(),true);
    for (const FgVect3UI & tri : tris)
        for (size_t xx=0; xx<3; ++xx)
            unused.at(tri[xx]) = false;
    for (vector<bool>::const_iterator it=unused.begin(); it != unused.end(); ++it)
        if (*it)
            return true;
    return false;
}

FgUints
fgRemoveUnusedVertsRemap(const FgVect3UIs & tris,const FgVerts & verts)
{
    FgUints         remap(verts.size(),numeric_limits<uint>::max());
    for (size_t ii=0; ii<tris.size(); ++ii)
        for (uint xx=0; xx<3; ++xx)
            remap.at(tris[ii][xx]) = 0;
    uint            cnt = 0;
    for (size_t ii=0; ii<remap.size(); ++ii)
        if (remap[ii] == 0)
            remap[ii] = cnt++;
    return remap;
}

FgTriSurf
fgRemoveUnusedVerts(const FgVerts & verts,const FgVect3UIs & tris)
{
    FgTriSurf       ret;
    FgUints         remap = fgRemoveUnusedVertsRemap(tris,verts);
    for (size_t ii=0; ii<verts.size(); ++ii)
        if (remap[ii] != numeric_limits<uint>::max())
            ret.verts.push_back(verts[ii]);
    ret.tris.resize(tris.size());
    for (size_t ii=0; ii<tris.size(); ++ii)
        for (uint xx=0; xx<3; ++xx)
            ret.tris[ii][xx] = remap[tris[ii][xx]];
    return ret;
}

// */
