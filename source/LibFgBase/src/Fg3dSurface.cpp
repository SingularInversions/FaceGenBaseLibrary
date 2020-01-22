//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dSurface.hpp"
#include "FgBounds.hpp"
#include "FgOpt.hpp"
#include "FgStdString.hpp"

using namespace std;

namespace Fg {

Vec3Fs
fgSelectVerts(const LabelledVerts & labVerts,Strings const & labels)
{
    Vec3Fs         ret;
    ret.reserve(labels.size());
    for (std::string str : labels)
        ret.push_back(findFirst(labVerts,str).pos);
    return ret;
}

void
fgReadp(std::istream & is,SurfPoint & sp)
{
    fgReadp(is,sp.triEquivIdx);
    fgReadp(is,sp.weights);
    fgReadp(is,sp.label);
}

void
fgWritep(std::ostream & os,const SurfPoint & sp)
{
    fgWritep(os,sp.triEquivIdx);
    fgWritep(os,sp.weights);
    fgWritep(os,sp.label);
}

vector<Vec3UI>
fgQuadsToTris(const vector<Vec4UI> & quads)
{
    vector<Vec3UI>   ret;
    for (size_t ii=0; ii<quads.size(); ++ii) {
        const Vec4UI &   quad = quads[ii];
        // Ordering must match triEquiv ordering for surface point to remain valid:
        ret.push_back(Vec3UI(quad[0],quad[1],quad[2]));
        ret.push_back(Vec3UI(quad[2],quad[3],quad[0]));
    }
    return ret;
}

TriUv
fgTriEquiv(const Tris & tris,const Quads & quads,size_t tt)
{
    TriUv       ret;
    ret.uvInds = Vec3UI(0);
    if (tt < tris.posInds.size()) {
        ret.posInds = tris.posInds[tt];
        if (tt < tris.uvInds.size())
            ret.uvInds = tris.uvInds[tt];
    }
    else {
        tt -= tris.size();
        size_t      qq = tt >> 1;
        FGASSERT(qq < quads.posInds.size());
        if (tt & 0x01) {
            ret.posInds = Vec3UI(
                quads.posInds[qq][2],
                quads.posInds[qq][3],
                quads.posInds[qq][0]);
            if (qq < quads.uvInds.size())
                ret.uvInds = Vec3UI(
                    quads.uvInds[qq][2],
                    quads.uvInds[qq][3],
                    quads.uvInds[qq][0]);
        }
        else {
            ret.posInds = Vec3UI(
                quads.posInds[qq][0],
                quads.posInds[qq][1],
                quads.posInds[qq][2]);
            if (qq < quads.uvInds.size())
                ret.uvInds = Vec3UI(
                    quads.uvInds[qq][0],
                    quads.uvInds[qq][1],
                    quads.uvInds[qq][2]);
        }
	}
    return ret;
}

Vec3UI
fgTriEquivPosInds(const Tris & tris,const Quads & quads,size_t tt)
{
    if (tt < tris.size())
        return tris.posInds[tt];
    else
    {
        tt -= tris.size();
        size_t      qq = tt >> 1;
        FGASSERT(qq < quads.size());
        if (tt & 0x01)
            return 
                Vec3UI(
                    quads.posInds[qq][2],
                    quads.posInds[qq][3],
                    quads.posInds[qq][0]);
        else
            return
                Vec3UI(
                    quads.posInds[qq][0],
                    quads.posInds[qq][1],
                    quads.posInds[qq][2]);
	}
}

Tris
fgTriEquivs(const Tris & tris,const Quads & quads)
{
    Tris          ret = tris;
    for (size_t ii=0; ii<quads.size(); ++ii) {
        Vec4UI       quad = quads.posInds[ii];
        ret.posInds.push_back(Vec3UI(quad[0],quad[1],quad[2]));
        ret.posInds.push_back(Vec3UI(quad[2],quad[3],quad[0]));
    }
    for (size_t ii=0; ii<quads.uvInds.size(); ++ii) {
        Vec4UI       quad = quads.uvInds[ii];
        ret.uvInds.push_back(Vec3UI(quad[0],quad[1],quad[2]));
        ret.uvInds.push_back(Vec3UI(quad[2],quad[3],quad[0]));
    }
    return ret;
}

Vec3F
fgSurfPointPos(
    const SurfPoint &         sp,
    const Tris &              tris,
    const Quads &             quads,
    Vec3Fs const &             verts)
{
    Vec3UI           vertInds = fgTriEquivPosInds(tris,quads,sp.triEquivIdx);
    Vec3F            vertWeights = sp.weights;
    return (verts[vertInds[0]] * vertWeights[0] +
            verts[vertInds[1]] * vertWeights[1] +
            verts[vertInds[2]] * vertWeights[2]);
}

uint
Surf::vertIdxMax() const
{
    uint        ret = 0;
    for(size_t qq=0; qq<quads.size(); ++qq)
        for(uint ii=0; ii<4; ++ii)
            setIfGreater(ret,quads.posInds[qq][ii]);
    for(size_t tt=0; tt<tris.size(); ++tt)
        for(uint ii=0; ii<3; ++ii)
            setIfGreater(ret,tris.posInds[tt][ii]);
    return ret;
}

std::set<uint>
Surf::vertsUsed() const
{
    std::set<uint>  ret;
    for(size_t qq = 0; qq < quads.size(); ++qq)
        for(uint ii = 0; ii < 4; ++ii)
            ret.insert(quads.posInds[qq][ii]);
    for(size_t tt = 0; tt < tris.size(); ++tt)
        for(uint ii = 0; ii < 3; ++ii)
            ret.insert(tris.posInds[tt][ii]);
    return ret;
}

Vec3F
Surf::surfPointPos(Vec3Fs const & verts,string const & label) const
{
    const SurfPoint & sp = findFirst(surfPoints,label);
    Vec3UI           tri = getTriEquivPosInds(sp.triEquivIdx);
    return cBarycentricVert(tri,sp.weights,verts);
}

Vec3Fs
Surf::surfPointPositions(Vec3Fs const & verts) const
{
    Vec3Fs      ret;
    for (SurfPoint const & sp : surfPoints) {
        Vec3UI      tri = getTriEquivPosInds(sp.triEquivIdx);
        ret.push_back(cBarycentricVert(tri,sp.weights,verts));
    }
    return ret;
}

LabelledVerts
Surf::surfPointsAsVertLabels(Vec3Fs const & verts) const
{
    LabelledVerts     ret;
    ret.reserve(surfPoints.size());
    for (size_t ii=0; ii<surfPoints.size(); ++ii) {
        LabelledVert         vl;
        vl.label = surfPoints[ii].label;
        vl.pos = surfPointPos(verts,ii);
        ret.push_back(vl);
    }
    return ret;
}

FacetInds<3>
Surf::asTris() const
{
    FacetInds<3>      ret;
    ret.posInds = cat(tris.posInds,fgQuadsToTris(quads.posInds));
    ret.uvInds = cat(tris.uvInds,fgQuadsToTris(quads.uvInds));
    return ret;
}

Surf
Surf::convertToTris() const
{
    Surf     ret;
    ret.name = name;
    ret.tris = asTris();
    ret.surfPoints = surfPoints;
    return ret;
}

void
Surf::merge(Surf const & surf)
{
    for (size_t pp=0; pp<surf.surfPoints.size(); ++pp) {
        SurfPoint     sp = surf.surfPoints[pp];
        if (sp.triEquivIdx < surf.tris.size())
            sp.triEquivIdx += uint(tris.size());
        else
            sp.triEquivIdx += uint(tris.size() + 2*quads.size());
        surfPoints.push_back(sp);
    }
    cat_(tris.posInds,surf.tris.posInds);
    cat_(tris.uvInds,surf.tris.uvInds);
    cat_(quads.posInds,surf.quads.posInds);
    cat_(quads.uvInds,surf.quads.uvInds);
}

void
Surf::checkMeshConsistency(
    uint    coordsSize,
    uint    uvsSize)
{
    if (tris.size() > 0)
        {FGASSERT(cMaxElem(cBounds(tris.posInds)) < coordsSize); }
    if (quads.size() > 0)
        {FGASSERT(cMaxElem(cBounds(quads.posInds)) < coordsSize); }
    if (tris.uvInds.size() > 0)
        {FGASSERT(cMaxElem(cBounds(tris.uvInds)) < uvsSize); }
    if (quads.uvInds.size() > 0)
        {FGASSERT(cMaxElem(cBounds(quads.uvInds)) < uvsSize); }
}

void
Surf::checkInternalConsistency()
{
    // UVs are either not present or per-facet:
    if (tris.uvInds.size() > 0)
        {FGASSERT(tris.uvInds.size() == tris.size()); }
    if (quads.uvInds.size() > 0)
        {FGASSERT(quads.uvInds.size() == quads.size()); }
}

void
Surf::clear()
{
    *this = Surf();
}

void
fgReadp(std::istream & is,Surf & s)
{
    fgReadp(is,s.name);
    fgReadp(is,s.tris);
    fgReadp(is,s.quads);
    fgReadp(is,s.surfPoints);
}

void
fgWritep(std::ostream & os,Surf const & s)
{
    fgWritep(os,s.name);
    fgWritep(os,s.tris);
    fgWritep(os,s.quads);
    fgWritep(os,s.surfPoints);
}

ostream &
operator<<(ostream & os,Surf const & surf)
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
    Vec3UI   inds;

    STri(Vec3UI i)
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
    Vec4UI   inds;

    SQuad(Vec4UI i)
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

Surf
fgSelectTris(Surf const & surf,const vector<FgBool> & sel)
{
    Surf     ret;
    ret.name = surf.name;
    FGASSERT(surf.tris.size() == sel.size());
    vector<uint>    remap(surf.tris.size());
    uint            idx = 0;
    for (size_t ii=0; ii<surf.tris.size(); ++ii) {
        if (sel[ii]) {
            ret.tris.posInds.push_back(surf.tris.posInds[ii]);
            if (!surf.tris.uvInds.empty())
                ret.tris.uvInds.push_back(surf.tris.uvInds[ii]);
            remap[ii] = idx++;
        }
    }
    for (size_t ii=0; ii<surf.surfPoints.size(); ++ii) {
        SurfPoint     sp = surf.surfPoints[ii];
        if ((sp.triEquivIdx < sel.size()) && (sel[sp.triEquivIdx])) {
            sp.triEquivIdx = remap[sp.triEquivIdx];
            ret.surfPoints.push_back(sp);
        }
    }
    return ret;
}

Surf
fgRemoveDuplicateFacets(Surf const & s)
{
    if (!s.surfPoints.empty())
        fgThrow("Duplicate facet removal with surface points not implemented");
    Surf     ret = s;
    uint            numTris = 0,
                    numQuads = 0;
    ret.tris.posInds.clear();
    ret.tris.uvInds.clear();
    set<STri>            ts;
    for (size_t ii=0; ii<s.tris.size(); ++ii) {
        Vec3UI       inds = s.tris.posInds[ii];
        STri             tri(inds);
        if (ts.find(tri) == ts.end()) {
            ts.insert(tri);
            ret.tris.posInds.push_back(inds);
            ret.tris.uvInds.push_back(s.tris.uvInds[ii]);
        }
        else
            ++numTris;
    }
    ret.quads.posInds.clear();
    ret.quads.uvInds.clear();
    set<SQuad>           qs;
    for (size_t ii=0; ii<s.quads.size(); ++ii) {
        Vec4UI       inds = s.quads.posInds[ii];
        SQuad            quad(inds);
        if (qs.find(quad) == qs.end()) {
            qs.insert(quad);
            ret.quads.posInds.push_back(inds);
            ret.quads.uvInds.push_back(s.quads.uvInds[ii]);
        }
        else
            ++numQuads;
    }
    fgout << fgnl << numTris << " duplicate tris removed, "
        << numQuads << " duplicate quads removed.";
    return ret;
}

Surf
mergeSurfaces(const vector<Surf> & surfs)
{
    Surf     ret;
    for (size_t ii=0; ii<surfs.size(); ++ii)
        ret.merge(surfs[ii]);
    return ret;
}

vector<Surf>
fgSplitSurface(Surf const & surf)
{
    vector<Surf>     ret;
    FGASSERT(!surf.empty());
    // Construct a lookup from vert inds back to triEquivs (FgTopology is overkill for this):
    uint                    idxBnd = surf.vertIdxMax() + 1;
    vector<vector<uint> >   vertIdxToTriIdx(idxBnd);
    for (uint tt=0; tt<surf.numTriEquivs(); ++tt) {
        Vec3UI           vertInds = surf.getTriEquivPosInds(tt);
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
            Vec3UI       vertInds = surf.getTriEquivPosInds(tt);
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
        Surf     s;
        for (size_t ii=0; ii<surf.tris.size(); ++ii) {
            if (groups[ii] == groupVal) {
                s.tris.posInds.push_back(surf.tris.posInds[ii]);
                if (!surf.tris.uvInds.empty())
                    s.tris.uvInds.push_back(surf.tris.uvInds[ii]);
            }
        }
        for (size_t ii=0; ii<surf.quads.size(); ++ii) {
            uint        triEquiv = uint(surf.tris.size() + 2 * ii);
            if (groups[triEquiv] == groupVal) {
                s.quads.posInds.push_back(surf.quads.posInds[ii]);
                if (!surf.quads.uvInds.empty())
                    s.quads.uvInds.push_back(surf.quads.uvInds[ii]);
            }
        }
        ret.push_back(s);
    }
    return ret;
}

vector<Surf>
fgEnsureNamed(const vector<Surf> & surfs,Ustring const & baseName)
{
    vector<Surf>     ret = surfs;
    if ((ret.size() == 1) && (ret[0].name.empty()))
        ret[0].name = baseName;
    else {
        size_t                  cnt = 0;
        for (size_t ss=0; ss<ret.size(); ++ss)
            if (ret[ss].name.empty())
                ret[ss].name = baseName + toStr(cnt++);
    }
    return ret;
}

void
Surf::removeTri(size_t triIdx)
{
    SurfPoints        nsps;
    for (size_t ii=0; ii<surfPoints.size(); ++ii) {
        SurfPoint     sp = surfPoints[ii];
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
Surf::removeQuad(size_t quadIdx)
{
    SurfPoints        nsps;
    size_t              idx0 = 2*(quadIdx/2) + tris.size(),
                        idx1 = idx0 + 1;
    for (size_t ii=0; ii<surfPoints.size(); ++ii) {
        SurfPoint     sp = surfPoints[ii];
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

Vec3Fs
fgVertsUsed(Vec3UIs const & tris,Vec3Fs const & verts)
{
    vector<bool>    used(verts.size(),false);
    for (const Vec3UI & tri : tris)
        for (size_t xx=0; xx<3; ++xx)
            used.at(tri[xx]) = true;
    Vec3Fs         ret;
    for (size_t ii=0; ii<verts.size(); ++ii)
        if (used[ii])
            ret.push_back(verts[ii]);
    return ret;
}

bool
fgHasUnusedVerts(Vec3UIs const & tris,Vec3Fs const & verts)
{
    vector<bool>    unused(verts.size(),true);
    for (const Vec3UI & tri : tris)
        for (size_t xx=0; xx<3; ++xx)
            unused.at(tri[xx]) = false;
    for (vector<bool>::const_iterator it=unused.begin(); it != unused.end(); ++it)
        if (*it)
            return true;
    return false;
}

Uints
fgRemoveUnusedVertsRemap(Vec3UIs const & tris,Vec3Fs const & verts)
{
    Uints         remap(verts.size(),numeric_limits<uint>::max());
    for (size_t ii=0; ii<tris.size(); ++ii)
        for (uint xx=0; xx<3; ++xx)
            remap.at(tris[ii][xx]) = 0;
    uint            cnt = 0;
    for (size_t ii=0; ii<remap.size(); ++ii)
        if (remap[ii] == 0)
            remap[ii] = cnt++;
    return remap;
}

TriSurf
meshRemoveUnusedVerts(Vec3Fs const & verts,Vec3UIs const & tris)
{
    TriSurf       ret;
    Uints         remap = fgRemoveUnusedVertsRemap(tris,verts);
    for (size_t ii=0; ii<verts.size(); ++ii)
        if (remap[ii] != numeric_limits<uint>::max())
            ret.verts.push_back(verts[ii]);
    ret.tris.resize(tris.size());
    for (size_t ii=0; ii<tris.size(); ++ii)
        for (uint xx=0; xx<3; ++xx)
            ret.tris[ii][xx] = remap[tris[ii][xx]];
    return ret;
}

}

// */
