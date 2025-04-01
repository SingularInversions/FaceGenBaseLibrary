//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgTopology.hpp"
#include "FgImageDraw.hpp"
#include "FgCommand.hpp"
#include "FgMatrixV.hpp"

using namespace std;

namespace Fg {

bool                hasUnusedVerts(Arr3UIs const & tris,Vec3Fs const & verts)
{
    vector<bool>    unused(verts.size(),true);
    for (Arr3UI const & tri : tris)
        for (size_t xx=0; xx<3; ++xx)
            unused.at(tri[xx]) = false;
    for (vector<bool>::const_iterator it=unused.begin(); it != unused.end(); ++it)
        if (*it)
            return true;
    return false;
}

Arr3UIs             reverseWinding(Arr3UIs const & tris)
{
    Arr3UIs             ret; ret.reserve(tris.size());
    for (Arr3UI const & t : tris)
        ret.emplace_back(t[0],t[2],t[1]);
    return ret;
}

Arr3UI              getTriEquivalent(size_t tt,Arr3UIs const & tris,Arr4UIs const & quads)
{
    if (tt < tris.size())
        return tris[tt];
    else {
        tt -= tris.size();
        size_t              qq = tt / 2;
        FGASSERT(qq < quads.size());
        Arr4UI const &      quad = quads[qq];
        if (tt & 0x01U)
            return Arr3UI{quad[2],quad[3],quad[0]};
        else
            return Arr3UI{quad[0],quad[1],quad[2]};
	}
}

Vec3F               SurfPoint::pos(Arr3UIs const & tris,Arr4UIs const & quads,Vec3Fs const & verts) const
{
    Arr3UI              tri = getTriEquivalent(triEquivIdx,tris,quads);
    return indexInterp(tri,weights,verts);
}

Vec3D               SurfPoint::pos(Arr3UIs const & tris,Arr4UIs const & quads,Vec3Ds const & verts) const
{
    Arr3UI              tri = getTriEquivalent(triEquivIdx,tris,quads);
    return indexInterp(tri,weights,verts);
}

NameVec3Fs          toNameVecs(
    SurfPointNames const & sps,
    Arr3UIs const &     tris,
    Arr4UIs const &     quads,
    Vec3Fs const &      verts)
{
    NameVec3Fs          ret; ret.reserve(sps.size());
    for (SurfPointName const & sp : sps)
        ret.emplace_back(sp.label,sp.point.pos(tris,quads,verts));
    return ret;
}


Arr3UIs             asTris(Arr4UIs const & quads)
{
    Arr3UIs             ret; ret.reserve(quads.size()*2);
    for (Arr4UI const & q : quads) {
        // Ordering must match triEquiv ordering for surface point to remain valid:
        ret.emplace_back(q[0],q[1],q[2]);
        ret.emplace_back(q[2],q[3],q[0]);
    }
    return ret;
}

void                markVertOnMap(Vec2Fs const & uvs,TriInds const & triInds,size_t vertIdx,Rgba8 color,ImgRgba8 & map_)
{
    FGASSERT(triInds.hasUvs());
    // find each use of this vertex index by a tri and color the corresponding UV point:
    size_t              I = triInds.size();
    for (size_t ii=0; ii<I; ++ii) {
        Arr3UI              vis = triInds.vertInds[ii];
        for (size_t jj=0; jj<3; ++jj) {
            uint                vi = vis[jj];
            if (vertIdx == vi) {
                Vec2F               uv = uvs[triInds.uvInds[ii][jj]];
                uv[1] = 1.0f - uv[1];       // OTCS to IUCS
                Vec2UI              crd (mapMul(uv,Vec2F(map_.dims())));
                map_[crd] = color;
            }
        }
    }
}

uint                Surf::vertIdxMax() const
{
    uint        ret = 0;
    for(size_t qq=0; qq<quads.size(); ++qq)
        for(uint ii=0; ii<4; ++ii)
            updateMax_(ret,quads.vertInds[qq][ii]);
    for(size_t tt=0; tt<tris.size(); ++tt)
        for(uint ii=0; ii<3; ++ii)
            updateMax_(ret,tris.vertInds[tt][ii]);
    return ret;
}
set<uint>           Surf::vertsUsed() const
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
Vec3F               Surf::surfPointPos(Vec3Fs const & verts,SurfPoint const & bp) const
{
    Arr3UI              tri = getTriEquivVertInds(bp.triEquivIdx);
    return indexInterp(tri,bp.weights,verts);
}
Vec3F               Surf::surfPointPos(Vec3Fs const & verts,size_t surfPointIdx) const
{
    FGASSERT(surfPointIdx < surfPoints.size());
    SurfPointName const &   sp = surfPoints[surfPointIdx];
    return surfPointPos(verts,sp.point);
}
Vec3F               Surf::surfPointPos(Vec3Fs const & verts,String const & label) const
{
    SurfPointName const &   sp = findFirst(surfPoints,label);
    return surfPointPos(verts,sp.point);
}
Vec3Fs              Surf::surfPointPositions(Vec3Fs const & verts) const
{
    Vec3Fs              ret;
    for (SurfPointName const & sp : surfPoints) {
        ret.push_back(sp.point.pos(tris.vertInds,quads.vertInds,verts));
    }
    return ret;
}
void                Surf::validate(uint numVerts,uint numUvs) const
{
    tris.validate(numVerts,numUvs);
    quads.validate(numVerts,numUvs);
}
void                Surf::checkInternalConsistency()
{
    // UVs are either not present or per-facet:
    if (tris.uvInds.size() > 0)
        {FGASSERT(tris.uvInds.size() == tris.size()); }
    if (quads.uvInds.size() > 0)
        {FGASSERT(quads.uvInds.size() == quads.size()); }
}
template<size_t N>
ostream &           operator<<(ostream & os,NPolys<N> const & f)
{
    os << "vertInds: " << f.vertInds.size() << " uvInds: " << f.uvInds.size();
    if (!f.uvInds.empty() && (f.uvInds.size() != f.vertInds.size()))
        os << " (MISMATCH)";
    return os;
}
ostream &           operator<<(ostream & os,Surf const & surf)
{
    os
        << fgnl << "Name: " << surf.name
        << fgnl << "Tris: " << surf.tris
        << fgnl << "Quads: " << surf.quads
        << fgnl << "Surf Points: " << surf.surfPoints.size() << fgpush;
        for (size_t ii=0; ii<surf.surfPoints.size(); ++ii)
            os << fgnl << toStrDigits(ii,2) << ": " << surf.surfPoints[ii].label;
        os << fgpop;
    return os;
}

Arr3UIs         asTriVertInds(Surf const & surf)
{
    return cat(surf.tris.vertInds,asTris(surf.quads.vertInds));
}

Arr3UIs         asTriVertInds(Surfs const & surfs)
{
    Arr3UIs             ret;
    for (Surf const & surf : surfs) {
        cat_(ret,surf.tris.vertInds);
        cat_(ret,asTris(surf.quads.vertInds));
    }
    return ret;
}

NameVec3Fs          surfPointsToNameVecs(Surfs const & surfs,Vec3Fs const & verts)
{
    NameVec3Fs          ret;
    for (Surf const & surf : surfs)
        cat_(ret,surf.surfPointsAsNameVecs(verts));
    return ret;
}

Vec3D           surfPointPos(Surfs const & surfs,Vec3Ds const & verts,String const & name)
{
    for (Surf const & surf : surfs)
        for (auto const & spn : surf.surfPoints)
            if (spn.label == name)
                return spn.point.pos(surf.tris.vertInds,surf.quads.vertInds,verts);
    FGASSERT_FALSE;
    return Vec3D{0};        // avoid warning
}

Vec3Ds          surfPointPoss(Surfs const & surfs,Vec3Ds const & verts,Strings const & names)
{
    return mapCall(names,[&](String const & name){return surfPointPos(surfs,verts,name); });
}

Surfs               splitByUvTile_(Surf const & surf,Vec2Fs & uvs)
{
    set<Vec2I>              domains;
    map<Vec2I,Uints>        domainToQuadInds,
                            domainToTriInds;
    bool                    mixed = false;
    for (uint tt=0; tt<surf.tris.uvInds.size(); ++tt) {
        Arr3UI                  uvInds = surf.tris.uvInds[tt];
        Vec2I                   domain(mapFloor(uvs[uvInds[0]]));
        domains.insert(domain);
        for (uint ii=1; ii<3; ++ii)
            if (Vec2I(mapFloor(uvs[uvInds[ii]])) != domain)
                mixed = true;
        domainToTriInds[domain].push_back(tt);
    }
    for (uint qq=0; qq<surf.quads.uvInds.size(); ++qq) {
        Arr4UI                  uvInds = surf.quads.uvInds[qq];
        Vec2I                   domain(mapFloor(uvs[uvInds[0]]));
        domains.insert(domain);
        for (uint ii=1; ii<4; ++ii)
            if (Vec2I(mapFloor(uvs[uvInds[ii]])) != domain)
                mixed = true;
        domainToQuadInds[domain].push_back(qq);
    }
    if (domains.size() < 2)
        return {surf};
    fgout << fgnl << "WARNING: OBJ UV domains detected and converted to " << domains.size() << " surfaces: ";
    if (mixed)
        fgout << fgnl << "WARNING: some facet(s) span multiple UV domains";
    Surfs                   ret; ret.reserve(domains.size());
    String                  nameSep = surf.name.empty() ? "" : "-";
    for (Vec2I domain : domains) {
        fgout << domain << " ";
        Uints const &           triSels = domainToTriInds[domain];
        TriInds                 tris {
            mapIndex(triSels,surf.tris.vertInds),
            mapIndex(triSels,surf.tris.uvInds),
        };
        Uints const &           quadSels = domainToQuadInds[domain];
        QuadInds                quads {
            mapIndex(quadSels,surf.quads.vertInds),
            mapIndex(quadSels,surf.quads.uvInds),
        };
        String8                 name = surf.name +nameSep + toStrDigits(ret.size(),2);
        ret.emplace_back(name,tris,quads);
    }
    for (Vec2F & uv : uvs)
        uv -= mapFloor(uv);
    return ret;
}

Surfs               splitSurf(Surf const & surf,Uints const & triIdxToSurf,Uints const & quadIdxToSurf)
{
    FGASSERT(triIdxToSurf.size() == surf.tris.vertInds.size());
    FGASSERT(quadIdxToSurf.size() == surf.quads.vertInds.size());
    uint                numSurfs = cMax(cMaxElem(triIdxToSurf),cMaxElem(quadIdxToSurf)) + 1;
    Uints               oldToNewTriIdx,
                        oldToNewQuadIdx;
    Surfs               ret (numSurfs);
    for (size_t tt=0; tt<surf.tris.vertInds.size(); ++tt) {
        size_t              surfIdx = triIdxToSurf[tt];
        oldToNewTriIdx.push_back(scast<uint>(ret[surfIdx].tris.vertInds.size()));
        ret[surfIdx].tris.vertInds.push_back(surf.tris.vertInds[tt]);
        if (!surf.tris.uvInds.empty())
            ret[surfIdx].tris.uvInds.push_back(surf.tris.uvInds[tt]);
    }
    for (size_t qq=0; qq<surf.quads.vertInds.size(); ++qq) {
        size_t              surfIdx = quadIdxToSurf[qq];
        oldToNewQuadIdx.push_back(scast<uint>(ret[surfIdx].quads.vertInds.size()));
        ret[surfIdx].quads.vertInds.push_back(surf.quads.vertInds[qq]);
        if (!surf.quads.uvInds.empty())
            ret[surfIdx].quads.uvInds.push_back(surf.quads.uvInds[qq]);
    }
    for (SurfPointName const & spn : surf.surfPoints) {
        uint                teIdx = spn.point.triEquivIdx,
                            surfIdx;
        SurfPoint           sp = spn.point;
        if (teIdx < surf.tris.vertInds.size()) {
            surfIdx = triIdxToSurf[teIdx];
            sp.triEquivIdx = oldToNewTriIdx[teIdx];
        }
        else {
            surfIdx = quadIdxToSurf[(teIdx-surf.tris.size())/2];
            uint                quadTeIdx = teIdx - scast<uint>(surf.tris.vertInds.size()),
                                quadIdx = quadTeIdx / 2,
                                remaind = quadTeIdx % 2;
            sp.triEquivIdx = scast<uint>(ret[surfIdx].tris.size()) + oldToNewQuadIdx[quadIdx]*2 + remaind;
        }
        ret[surfIdx].surfPoints.emplace_back(sp,spn.label);
    }
    return ret;
}

Surfs               splitSurfContiguousUvs(Surf const & surf)
{
    FGASSERT(surf.hasUvIndices());
    Uints               uvIdxToSurf = cContiguousVertsMap(surf.tris.uvInds,surf.quads.uvInds);
    Uints               triIdxToSurf; triIdxToSurf.reserve(surf.tris.size());
    Uints               quadIdxToSurf; quadIdxToSurf.reserve(surf.quads.size());
    for (Arr3UI tri : surf.tris.uvInds)
        triIdxToSurf.push_back(uvIdxToSurf[tri[0]]);        // all uvs used by facet will have the same map since contiguous
    for (Arr4UI quad : surf.quads.uvInds)
        quadIdxToSurf.push_back(uvIdxToSurf[quad[0]]);      // "
    return splitSurf(surf,triIdxToSurf,quadIdxToSurf);
}

Surfs               splitSurfContiguousVerts(Surf const & surf)
{
    FGASSERT(!surf.empty());
    Uints               vertIdxToSurf = cContiguousVertsMap(surf.tris.vertInds,surf.quads.vertInds);
    Uints               triIdxToSurf; triIdxToSurf.reserve(surf.tris.size());
    Uints               quadIdxToSurf; quadIdxToSurf.reserve(surf.quads.size());
    for (Arr3UI tri : surf.tris.vertInds)
        triIdxToSurf.push_back(vertIdxToSurf[tri[0]]);        // all verts used by facet will have the same map since contiguous
    for (Arr4UI quad : surf.quads.vertInds)
        quadIdxToSurf.push_back(vertIdxToSurf[quad[0]]);      // "
    return splitSurf(surf,triIdxToSurf,quadIdxToSurf);
}

static inline void  swapLt(uint & a,uint & b)
{
    if (b < a)
        std::swap(a,b);
}

struct  STri
{
    Arr3UI   inds;

    STri(Arr3UI i)
    {
        // Bubble sort:
        swapLt(i[0],i[1]);
        swapLt(i[1],i[2]);
        swapLt(i[0],i[1]);
        inds = i;
    }

    bool            operator<(const STri & rhs) const
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
    Arr4UI   inds;

    SQuad(Arr4UI i)
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

    bool            operator<(const SQuad & rhs) const
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

Surf                removeDuplicateFacets(Surf const & s)
{
    if (!s.surfPoints.empty())
        fgThrow("Duplicate facet removal with surface points not implemented");
    Surf     ret = s;
    uint            numTris = 0,
                    numQuads = 0;
    ret.tris.vertInds.clear();
    ret.tris.uvInds.clear();
    set<STri>            ts;
    for (size_t ii=0; ii<s.tris.size(); ++ii) {
        Arr3UI       inds = s.tris.vertInds[ii];
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
        Arr4UI       inds = s.quads.vertInds[ii];
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

void                merge_(Surf & l,Surf const & r)
{
    uint                TL = scast<uint>(l.tris.size()),
                        TR = scast<uint>(r.tris.size()),
                        QL = scast<uint>(l.quads.size());
    for (SurfPointName & sp : l.surfPoints)
        if (sp.point.triEquivIdx > TL)
            sp.point.triEquivIdx += TR;
    for (SurfPointName sp : r.surfPoints) {
        if (sp.point.triEquivIdx < TR)
            sp.point.triEquivIdx += TL;
        else
            sp.point.triEquivIdx += TL + 2*QL;
        l.surfPoints.push_back(sp);
    }
    merge_(l.tris,r.tris);
    merge_(l.quads,r.quads);
}
Surf                merge(Surfs const & surfs)
{
    Surf                ret;
    if (surfs.empty())
        return ret;
    ret = surfs[0];                 // Retain name & material of first surface
    for (size_t ii=1; ii<surfs.size(); ++ii)
        merge_(ret,surfs[ii]);
    return ret;
}

bool                areConnected(Arr3UI lt,Arr3UI rt)               // connected through 1 or more vertices
{
    for (uint ll : lt)
        for (uint rr : rt)
            if (ll == rr)
                return true;
    return false;
}

bool                areConnected(Arr3UIs const & tris,Arr3UI rt)    // connected through 1 or more vertices
{
    for (Arr3UI lt : tris) {
        if (areConnected(lt,rt))
            return true;
    }
    return false;
}

Sizess              cContiguousInds(Arr3UIs const & tris)
{
    size_t              T = tris.size();
    Uints               labels; labels.reserve(T);          // 1-1 with tris
    // set up the initial labels by comparison with the first tri:
    uint                cnt {0};
    labels.push_back(0);
    for (size_t jj=1; jj<T; ++jj)
        labels.push_back(areConnected(tris[0],tris[jj]) ? 0 : ++cnt);
    if (cnt > 0)        // if not done, compare remaining with second tri, etc:
        // we're done once we've updated the labels for every tri-tri comparison:
        for (size_t ii=1; ii+1<T; ++ii)
            for (size_t jj=ii+1; jj<T; ++jj)
                if (labels[ii] != labels[jj])                   // no need to compare if already in same group
                    if (areConnected(tris[ii],tris[jj]))        // groups are connected
                        replaceAll_(labels,labels[jj],labels[ii]);
    // list unique group labels:
    Uints               labList = cUnique(sortAll(labels));
    auto                fn = [&](uint lab) {return selectEqualInds(labels,lab); };
    return mapCall(labList,fn);
}
static void         testContiguousInds(CLArgs const &)
{
    Arr3UIs             tris {
        {0,1,2},
        {3,4,5},{5,6,7},
        {8,9,10},{10,11,9},{11,12,8},
    };
    Sizess              ref = {{0},{1,2},{3,4,5},},
                        inds = cContiguousInds(tris),
                        order = sortAll(inds,[](Sizes const & l,Sizes const & r){return l.size() < r.size(); }),
                        tst = mapCall(order,[](Sizes const & l){return sortAll(l); });
    FGASSERT(tst == ref);
}

template<size_t R>
bool                sweepContig_(Svec<Arr<uint,R>> const & inds,uint & nextGroup,Uints & idxToLabel)
{
    bool            changed = false;
    for (Arr<uint,R> tinds : inds) {
        uint            idx0 = tinds[0]; 
        uint            label = idxToLabel[idx0];
        if (label == 0) {
            label = nextGroup++;
            idxToLabel[idx0] = label;
            changed = true;
        }
        for (uint ii=1; ii<R; ++ii) {
            uint            idx = tinds[ii];
            uint            lab = idxToLabel[idx];
            if (lab != label) {
                if (lab == 0)
                    idxToLabel[idx] = label;
                else
                    replaceAll_(idxToLabel,lab,label);
                changed = true;
            }
        }
    }
    return changed;
}

Uints               cContiguousVertsMap(Arr3UIs const & triInds,Arr4UIs const & quadInds)
{
    uint                maxIdx = cMax(cMaxElmMaxElm(triInds),cMaxElmMaxElm(quadInds));
    Uints               idxToLabel (maxIdx+1,0);
    uint                nextGroup = 1;
    bool                changed = false;
    do {
        bool                c0 = sweepContig_(triInds,nextGroup,idxToLabel),
                            c1 = sweepContig_(quadInds,nextGroup,idxToLabel);
        changed = (c0 || c1);
    } while (changed);
    Uints               labels = cUnique(sortAll(idxToLabel));
    Uints               ret;  ret.reserve(idxToLabel.size());
    for (uint label : idxToLabel)
        ret.push_back(scast<uint>(findFirstIdx(labels,label)));
    return ret;
}

Surfs               splitByContiguous(Surf const & surf)
{
    Surfs           ret;
    FGASSERT(!surf.empty());
    // Construct a lookup from vert inds back to triEquivs (SurfTopo is overkill for this):
    uint            idxBnd = surf.vertIdxMax() + 1;
    Uintss          vertIdxToTriIdx(idxBnd);
    for (uint tt=0; tt<surf.numTriEquivs(); ++tt) {
        Arr3UI          vertInds = surf.getTriEquivVertInds(tt);
        for (uint jj=0; jj<3; ++jj)
            vertIdxToTriIdx[vertInds[jj]].push_back(tt);
    }
    // Start with separate group for each tri, then merge until no change:
    Uints           groupLut;
    for (uint ii=0; ii<surf.numTriEquivs(); ++ii)
        groupLut.push_back(ii);
    bool            done = false;
    while (!done) {
        done = true;
        for (uint tt=0; tt<surf.numTriEquivs(); ++tt) {
            Arr3UI          vertInds = surf.getTriEquivVertInds(tt);
            for (uint jj=0; jj<3; ++jj) {
                Uints const &   triInds = vertIdxToTriIdx[vertInds[jj]];
                for (uint kk=0; kk<triInds.size(); ++kk) {
                    uint            ntt = triInds[kk];
                    if (groupLut[tt] != groupLut[ntt]) {
                        replaceAll_(groupLut,groupLut[ntt],groupLut[tt]);
                        done = false;
                    }
                }
            }
        }
    }
    set<uint>       groups = svecToSet(groupLut);
    for (uint groupVal : groups) {
        Surf                sub;
        map<size_t,size_t>  surfIdxToSubIdx;        // Where indices are both tri equiv
        for (size_t ii=0; ii<surf.tris.size(); ++ii) {
            if (groupLut[ii] == groupVal) {
                surfIdxToSubIdx[ii] = sub.tris.size();
                sub.tris.vertInds.push_back(surf.tris.vertInds[ii]);
                if (!surf.tris.uvInds.empty())
                    sub.tris.uvInds.push_back(surf.tris.uvInds[ii]);
            }
        }
        for (size_t ii=0; ii<surf.quads.size(); ++ii) {
            size_t          triEquiv = surf.tris.size() + 2*ii;
            if (groupLut[triEquiv] == groupVal) {
                size_t          subTriEquiv = sub.tris.size() + 2*sub.quads.size();
                surfIdxToSubIdx[triEquiv] = subTriEquiv;
                surfIdxToSubIdx[triEquiv+1] = subTriEquiv+1;
                sub.quads.vertInds.push_back(surf.quads.vertInds[ii]);
                if (!surf.quads.uvInds.empty())
                    sub.quads.uvInds.push_back(surf.quads.uvInds[ii]);
            }
        }
        for (SurfPointName const & sp : surf.surfPoints) {
            if (groupLut[sp.point.triEquivIdx] == groupVal) {
                SurfPointName       nsp = sp;
                nsp.point.triEquivIdx = uint(surfIdxToSubIdx[sp.point.triEquivIdx]);
                sub.surfPoints.push_back(nsp);
            }
        }
        ret.push_back(sub);
    }
    return ret;
}

void                Surf::removeTri(size_t triIdx)
{
    SurfPointNames      nsps;
    for (size_t ii=0; ii<surfPoints.size(); ++ii) {
        SurfPointName     sp = surfPoints[ii];
        if (sp.point.triEquivIdx < triIdx)
            nsps.push_back(sp);
        else if (sp.point.triEquivIdx > triIdx) {
            --sp.point.triEquivIdx;
            nsps.push_back(sp);
        }
    }
    surfPoints = nsps;
    tris.erase(triIdx);
}
void                Surf::removeQuad(size_t quadIdx)
{
    SurfPointNames      nsps;
    size_t              idx0 = 2*(quadIdx/2) + tris.size(),
                        idx1 = idx0 + 1;
    for (size_t ii=0; ii<surfPoints.size(); ++ii) {
        SurfPointName     sp = surfPoints[ii];
        if (sp.point.triEquivIdx < idx0)
            nsps.push_back(sp);
        else if (sp.point.triEquivIdx > idx1) {
            sp.point.triEquivIdx -= 2;
            nsps.push_back(sp);
        }
    }
    surfPoints = nsps;
    quads.erase(quadIdx);
}
Vec3Fs              cVertsUsed(Arr3UIs const & tris,Vec3Fs const & verts)
{
    vector<bool>    used(verts.size(),false);
    for (Arr3UI const & tri : tris)
        for (size_t xx=0; xx<3; ++xx)
            used.at(tri[xx]) = true;
    Vec3Fs         ret;
    for (size_t ii=0; ii<verts.size(); ++ii)
        if (used[ii])
            ret.push_back(verts[ii]);
    return ret;
}
Vec3Ds              cVertsUsed(Arr3UIs const & tris,Vec3Ds const & verts)
{
    vector<bool>        used (verts.size(),false);
    for (Arr3UI const & tri : tris)
        for (size_t xx=0; xx<3; ++xx)
            used.at(tri[xx]) = true;
    Vec3Ds              ret;
    for (size_t ii=0; ii<verts.size(); ++ii)
        if (used[ii])
            ret.push_back(verts[ii]);
    return ret;
}
Uints               removeUnusedVertsRemap(Arr3UIs const & tris,Vec3Fs const & verts)
{
    Uints               remap (verts.size(),numeric_limits<uint>::max());
    for (Arr3UI const & tri : tris)
        for (uint xx=0; xx<3; ++xx)
            remap.at(tri[xx]) = 0;
    uint                cnt = 0;
    for (uint & idx : remap)
        if (idx == 0)
            idx = cnt++;
    return remap;
}
Arr4UIs             reverseWinding(Arr4UIs const & quads)
{
    Arr4UIs             ret; ret.reserve(quads.size());
    for (Arr4UI const & q : quads)
        ret.emplace_back(q[0],q[3],q[2],q[1]);
    return ret;
}
SurfPoint           reverseWinding(SurfPoint const & bp,size_t numTris)
{
    Arr3F               w = bp.weights;
    if (bp.triEquivIdx < numTris) {
        Arr3F               wr {w[0],w[2],w[1]};
        return SurfPoint{bp.triEquivIdx,wr};
    }
    else {
        // quad was transformed [0123] -> [0321] thus the equivalent tris were transformed:
        // [012],[230] -> [032],[210] thus both tris have order reversed (and tris are swapped):
        Arr3F               wr {w[2],w[1],w[0]};
        if ((bp.triEquivIdx-numTris) & 0x01)        // second tri of quad -> first tri
            return SurfPoint{bp.triEquivIdx-1,wr};
        else                                        // first tri of quad -> second tri
            return SurfPoint{bp.triEquivIdx+1,wr};
    }
}
SurfPointNames      reverseWinding(SurfPointNames const & sps,size_t numTris)
{
    SurfPointNames      ret; ret.reserve(sps.size());
    for (SurfPointName const & sp : sps)
        ret.emplace_back(reverseWinding(sp.point,numTris),sp.label);
    return ret;
}
Surf                reverseWinding(Surf const & in)
{
    return              Surf {
        in.name,
        reverseWinding(in.tris),
        reverseWinding(in.quads),
        reverseWinding(in.surfPoints,in.tris.vertInds.size()),
        in.material,
    };
}

void                merge_(TriSurf & acc,TriSurf const & ts)
{
    cat_(acc.tris,mapAdd(ts.tris,Arr3UI{scast<uint>(acc.verts.size())}));
    cat_(acc.verts,ts.verts);
}

TriSurf             merge(TriSurfs const & ts)
{
    TriSurf             ret;
    for (TriSurf const & t : ts)
        merge_(ret,t);
    return ret;
}

TriSurf             removeUnused(Vec3Fs const & verts,Arr3UIs const & tris)
{
    TriSurf             ret;
    Uints               remap = removeUnusedVertsRemap(tris,verts);
    for (size_t ii=0; ii<verts.size(); ++ii)
        if (remap[ii] != numeric_limits<uint>::max())
            ret.verts.push_back(verts[ii]);
    ret.tris.resize(tris.size());
    for (size_t ii=0; ii<tris.size(); ++ii)
        for (uint xx=0; xx<3; ++xx)
            ret.tris[ii][xx] = remap[tris[ii][xx]];
    return ret;
}
Vec3F               cQuadNorm(Arr4UI const & quad,Vec3Fs const & verts)
{
    // This least squares surface normal is taken from [Mantyla 87]:
    Vec3F           v0 = verts[quad[0]],
                    v1 = verts[quad[1]],
                    v2 = verts[quad[2]],
                    v3 = verts[quad[3]];
    Vec3F           cross {
        (v0[1]-v1[1]) * (v0[2]+v1[2]) +
        (v1[1]-v2[1]) * (v1[2]+v2[2]) +
        (v2[1]-v3[1]) * (v2[2]+v3[2]) +
        (v3[1]-v0[1]) * (v3[2]+v0[2]),

        (v0[2]-v1[2]) * (v0[0]+v1[0]) +
        (v1[2]-v2[2]) * (v1[0]+v2[0]) +
        (v2[2]-v3[2]) * (v2[0]+v3[0]) +
        (v3[2]-v0[2]) * (v3[0]+v0[0]),

        (v0[0]-v1[0]) * (v0[1]+v1[1]) +
        (v1[0]-v2[0]) * (v1[1]+v2[1]) +
        (v2[0]-v3[0]) * (v2[1]+v3[1]) +
        (v3[0]-v0[0]) * (v3[1]+v0[1])
    };
    double          crossMag = cMagD(cross);
    if (crossMag == 0.0)
        return Vec3F{0};
    else
        return cross * scast<float>(1.0 / sqrt(crossMag));
}

// calculate facet norm, accumulate it weighted by subtended angle to each vertex, return facet norm:
template<class T,class U>
Mat<T,3,1>          accNorm_(
    Svec<Mat<T,3,1>> const & verts, // double precision useful here due to tri edge vectors being small reltive to abs pos
    Arr3UI                  tri,
    Svec<Mat<U,3,1>> &      acc)    // typically don't need double precision for the resulting normals for rendering
{
    Arr<Mat<T,3,1>,3>   vs = mapIndex(tri,verts);
    Mat<T,3,1>          v01 = vs[1]-vs[0],
                        v12 = vs[2]-vs[1],
                        v20 = vs[0]-vs[2],
                        cross = crossProduct(v01,-v20);     // CC winding
    T                   crossMag = cMagD(cross);
    if (crossMag > 0) {                                     // non-degenerate
        // This methods weights the contribution of each tri norm to the vertex norm by the angle
        // subtended by that tri. This may give a more reasonable value than averaging (according
        // to Keenan Crane), and avoids problems caused by degenerate tris (with arbitrary normal):
        Mat<T,3,1>      norm = cross / sqrt(crossMag);
        T               len01 = cLenD(v01),
                        len12 = cLenD(v12),
                        len20 = cLenD(v20),
                        // acos maps [1,-1] -> [0,PI] and input is (1,-1) due to non-degeneracy:
                        wgt0 = acos(-cDot(v01,v20)/(len01*len20)),
                        wgt1 = acos(-cDot(v12,v01)/(len12*len01)),
                        wgt2 = acos(-cDot(v20,v12)/(len20*len12));
        acc[tri[0]] += Mat<U,3,1>{norm * wgt0};
        acc[tri[1]] += Mat<U,3,1>{norm * wgt1};
        acc[tri[2]] += Mat<U,3,1>{norm * wgt2};
        return norm;
    }
    return {0,0,1};     // arbitrary for degenerate tri
}
Vec3Ds              cVertNorms(Vec3Ds const & verts,Arr3UIs const & tris)
{
    Vec3Ds              ret (verts.size(),Vec3D{0});
    for (Arr3UI const & tri : tris)
        accNorm_(verts,tri,ret);
    for (Vec3D & norm : ret) {
        double          len = cLenD(norm);
        if(len > 0.0)
            norm /= len;
        else
            norm = Vec3D{0,0,1};        // arbitrary when not part of a non-degenerate surface
    }
    return ret;
}

SurfNormals         cNormals(Surfs const & surfs,Vec3Fs const & verts)
{
    SurfNormals         norms;
    norms.facet.resize(surfs.size());
    norms.vert.resize(verts.size(),Vec3F{0});
    // Calculate facet normals and accumulate weighted vertex normals:
    for (size_t ss=0; ss<surfs.size(); ss++) {
        Surf const &        surf = surfs[ss];
        FacetNormals &      fnorms = norms.facet[ss];
        fnorms.tri.reserve(surf.tris.size());
        fnorms.quad.reserve(surf.quads.size());
        // TRIs
        for (Arr3UI const & tri : surf.tris.vertInds)
            fnorms.tri.push_back(accNorm_(verts,tri,norms.vert));
        // QUADs
        for (Arr4UI const & q : surf.quads.vertInds) {
            fnorms.quad.push_back(cQuadNorm(q,verts));      // use the least squares approx for facet
            // accumulate tri vertex normal contributions but ignore tri facet norms:
            accNorm_(verts,{q[0],q[1],q[2]},norms.vert),
            accNorm_(verts,{q[2],q[3],q[0]},norms.vert);
        }
    }
    // normalize accumulated weighted vertex normals:
    for (Vec3F & norm : norms.vert) {
        float               mag = cMagD(norm);
        if (mag > 0)
            norm *= 1.0f / sqrt(mag);
        else
            norm = Vec3F{0,0,1};        // arbitrary. effectively only happens for unused verts.
    }
    return norms;
}
ImgRgba8            cUvWireframeImage(Vec2Fs const & uvs,Arr3UIs const & tris,Arr4UIs const & quads,Rgba8 mfldClr,Rgba8 bndClr)
{
    // TODO: detect boundaries including quads:
    Arr3Bs              triBnds = cBoundFlags(cSharedEdges(tris));
    uint constexpr      dim {2048};
    AxAffine2F          xf = cOtcsToPacs<float>(Vec2UI{dim});
    Vec2Is              uvsIrcs = mapCall(uvs,[=](Vec2F uv){return Vec2I{mapFloor(xf*uv)}; });
    ImgRgba8            img {dim,dim,Rgba8{0,0,0,0}};
    for (size_t tt=0; tt<tris.size(); ++tt) {
        Arr3UI              triInds = tris[tt];
        Arr<Vec2I,3>        triIrcs = mapIndex(triInds,uvsIrcs);
        for (uint ee=0; ee<3; ++ee) {
            Rgba8           color = (triBnds[tt][ee] ? bndClr : mfldClr);
            Arr<Vec2I,2>    edge = getEdge(triIrcs,ee);
            drawLineIrcs(img,edge[0],edge[1],color);
        }
    }
    for (Arr4UI quad : quads)
        for (size_t vv=0; vv<4; ++vv)
            drawLineIrcs(img,uvsIrcs[quad[vv]],uvsIrcs[quad[(vv+1)%4]],mfldClr);
    return shrink2(img);
}

template<size_t N>
void                updateUvToVertPermutation_(Vec2Fs const & uvs,NPolys<N> const & inds,Uints & perm)
{
    size_t              U = uvs.size(),
                        V = perm.size(),
                        P = inds.vertInds.size();
    if (inds.uvInds.size() != P)
        fgThrow("Surface has different vert/uv poly counts",toStr(N)+"-polys: "+toStr(P)+"/"+toStr(inds.uvInds.size()));
    size_t              dupCnt {0};
    for (size_t ii=0; ii<P; ++ii) {
        Arr<uint,N>         vinds = inds.vertInds[ii],
                            uinds = inds.uvInds[ii];
        for (size_t jj=0; jj<N; ++jj) {
            uint                vidx = vinds[jj],
                                uidx = uinds[jj];
            FGASSERT(vidx < V);
            FGASSERT(uidx < U);
            uint                pidx = perm[vidx];
            if (pidx == lims<uint>::max())
                perm[vidx] = uidx;
            else if (pidx != uidx) {        // uv index / vert index topology difference. Check for duplication
                float constexpr         tol = lims<float>::epsilon() * 16;
                if (!isApproxEqual(uvs[pidx],uvs[uidx],tol))
                    ++dupCnt;
            }
        }
    }
    if (dupCnt > 0)
        fgout << fgnl << "WARNING: " << dupCnt << " duplicated UV indices vs. vertex topology";
}

Uints               cUvToVertPermutation(Vec3Fs const & verts,Vec2Fs const & uvs,Surfs const & surfs)
{
    size_t              V = verts.size(),
                        U = uvs.size();
    if (U < V)
        fgout << fgnl << "WARNING: More verts than uvs (" << V << " > " << U << ")";
    if (U > V)
        fgout << fgnl << "WARNING: More uvs than verts (" << U << " > " << V << ")";
    Uints               ret (V,lims<uint>::max());
    for (Surf const & surf : surfs) {
        updateUvToVertPermutation_(uvs,surf.tris,ret);
        updateUvToVertPermutation_(uvs,surf.quads,ret);
    }
    // set any unused indices to map to themselves:
    for (uint ii=0; ii<V; ++ii)
        if (ret[ii] == lims<uint>::max())
            ret[ii] = ii;
    return ret;
}

SurfPointIdx        findSurfPoint(Surfs const & surfs,String const & name)
{
    for (size_t ss=0; ss<surfs.size(); ++ss) {
        SurfPointNames const &  spns = surfs[ss].surfPoints;
        for (size_t pp=0; pp<spns.size(); ++pp) {
            if (spns[pp].label == name)
                return {ss,pp};
        }
    }
    return {};
}

SurfsPoint          findSurfsPoint(Surfs const & surfs,String const & name)
{
    SurfPointIdx        spi = findSurfPoint(surfs,name);
    if (!spi.valid())
        fgThrow("surface point not found",name);
    return {spi.surfIdx,surfs[spi.surfIdx].surfPoints[spi.pointIdx].point};
}

Vec3F               getSurfsPointPos(SurfsPoint sp,Surfs const & surfs,Vec3Fs const & verts)
{
    Surf const &        surf = surfs.at(sp.surfIdx);
    return sp.surfPoint.pos(surf.tris.vertInds,surf.quads.vertInds,verts);
}

TriSurf             cIsosurf(MatS3D const & precision,Vec3D const & centre)
{
    EigsRsm3            eigs = cRsmEigs(precision);
    // RSM solver does not preserve sign of determinant but we need +1 to preserve tri winding:
    if (cDeterminant(eigs.vecs) < 0)
        eigs.vecs *= -1.0;
    Vec3D               invSqrt = mapCall(eigs.vals,[](double e){return 1.0/sqrt(e); });
    Mat33D              mhlbsToWorld = eigs.vecs * cMatDiag(invSqrt);
    Affine3F            xf {Mat33F{mhlbsToWorld},Vec3F{centre}};
    TriSurf             ret = cSphere(2);
    mapMul_(xf,ret.verts);
    return ret;
}

void                testContiguousInds(CLArgs const & args);

void                testSurf(CLArgs const & args)
{
    Cmds                cmds {
        {testContiguousInds,"contig","cContiguousInds()"},
    };
    return doMenu(args,cmds,true);
}

}

// */
