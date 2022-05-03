//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dSurface.hpp"
#include "FgBounds.hpp"
#include "FgOpt.hpp"
#include "FgStdString.hpp"
#include "FgImageDraw.hpp"

using namespace std;

namespace Fg {

Vec3UI              getTriEquivalent(size_t tt,Vec3UIs const & tris,Vec4UIs const & quads)
{
    if (tt < tris.size())
        return tris[tt];
    else {
        tt -= tris.size();
        size_t              qq = tt / 2;
        FGASSERT(qq < quads.size());
        Vec4UI const &      quad = quads[qq];
        if (tt & 0x01U)
            return Vec3UI{quad[2],quad[3],quad[0]};
        else
            return Vec3UI{quad[0],quad[1],quad[2]};
	}
}
Vec3F               BaryPoint::pos(Vec3UIs const & tris,Vec4UIs const & quads,Vec3Fs const & verts) const
{
    Vec3UI              tri = getTriEquivalent(triEquivIdx,tris,quads);
    return interpolate(tri,weights,verts);
}
NameVec3Fs          toNameVecs(
    SurfPoints const &  sps,
    Vec3UIs const &     tris,
    Vec4UIs const &     quads,
    Vec3Fs const &      verts)
{
    NameVec3Fs          ret; ret.reserve(sps.size());
    for (SurfPoint const & sp : sps)
        ret.emplace_back(sp.label,sp.point.pos(tris,quads,verts));
    return ret;
}


Vec3UIs             asTris(Vec4UIs const & quads)
{
    Vec3UIs             ret; ret.reserve(quads.size()*2);
    for (Vec4UI const & q : quads) {
        // Ordering must match triEquiv ordering for surface point to remain valid:
        ret.emplace_back(q[0],q[1],q[2]);
        ret.emplace_back(q[2],q[3],q[0]);
    }
    return ret;
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
Vec3F               Surf::surfPointPos(Vec3Fs const & verts,BaryPoint const & bp) const
{
    Vec3UI              tri = getTriEquivVertInds(bp.triEquivIdx);
    return interpolate(tri,bp.weights,verts);
}
Vec3F               Surf::surfPointPos(Vec3Fs const & verts,size_t surfPointIdx) const
{
    FGASSERT(surfPointIdx < surfPoints.size());
    SurfPoint const &   sp = surfPoints[surfPointIdx];
    return surfPointPos(verts,sp.point);
}
Vec3F               Surf::surfPointPos(Vec3Fs const & verts,String const & label) const
{
    SurfPoint const &   sp = findFirst(surfPoints,label);
    return surfPointPos(verts,sp.point);
}
Vec3Fs              Surf::surfPointPositions(Vec3Fs const & verts) const
{
    Vec3Fs              ret;
    for (SurfPoint const & sp : surfPoints) {
        ret.push_back(sp.point.pos(tris.vertInds,quads.vertInds,verts));
    }
    return ret;
}
void                Surf::merge(Tris const & ts,Quads const & qs,SurfPoints const & sps)
{
    for (SurfPoint sp : sps) {
        if (sp.point.triEquivIdx < ts.size())
            sp.point.triEquivIdx += uint(tris.size());
        else
            sp.point.triEquivIdx += uint(tris.size() + 2*quads.size());
        surfPoints.push_back(sp);
    }
    tris = concat(tris,ts);
    quads = concat(quads,qs);
}
void                Surf::checkMeshConsistency(uint coordsSize,uint uvsSize) const
{
    if (tris.size() > 0)
        {FGASSERT(cMaxElem(cBounds(tris.vertInds)) < coordsSize); }
    if (quads.size() > 0)
        {FGASSERT(cMaxElem(cBounds(quads.vertInds)) < coordsSize); }
    if (tris.uvInds.size() > 0)
        {FGASSERT(cMaxElem(cBounds(tris.uvInds)) < uvsSize); }
    if (quads.uvInds.size() > 0)
        {FGASSERT(cMaxElem(cBounds(quads.uvInds)) < uvsSize); }
}
void                Surf::checkInternalConsistency()
{
    // UVs are either not present or per-facet:
    if (tris.uvInds.size() > 0)
        {FGASSERT(tris.uvInds.size() == tris.size()); }
    if (quads.uvInds.size() > 0)
        {FGASSERT(quads.uvInds.size() == quads.size()); }
}
template<uint N>
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
NameVec3Fs          surfPointsToNameVecs(Surfs const & surfs,Vec3Fs const & verts)
{
    NameVec3Fs          ret;
    for (Surf const & surf : surfs)
        cat_(ret,surf.surfPointsAsNameVecs(verts));
    return ret;
}
Surfs               splitByUvTile_(Surf const & surf,Vec2Fs & uvs)
{
    set<Vec2I>              domains;
    map<Vec2I,Uints>        domainToQuadInds,
                            domainToTriInds;
    bool                    mixed = false;
    for (uint tt=0; tt<surf.tris.uvInds.size(); ++tt) {
        Vec3UI                  uvInds = surf.tris.uvInds[tt];
        Vec2I                   domain(mapFloor(uvs[uvInds[0]]));
        domains.insert(domain);
        for (uint ii=1; ii<3; ++ii)
            if (Vec2I(mapFloor(uvs[uvInds[ii]])) != domain)
                mixed = true;
        domainToTriInds[domain].push_back(tt);
    }
    for (uint qq=0; qq<surf.quads.uvInds.size(); ++qq) {
        Vec4UI                  uvInds = surf.quads.uvInds[qq];
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
        Tris                    tris {
            select(surf.tris.vertInds,triSels),
            select(surf.tris.uvInds,triSels),
        };
        Uints const &           quadSels = domainToQuadInds[domain];
        Quads                   quads {
            select(surf.quads.vertInds,quadSels),
            select(surf.quads.uvInds,quadSels),
        };
        String8                 name = surf.name +nameSep + toStrDigits(ret.size(),2);
        ret.emplace_back(name,tris,quads);
    }
    for (Vec2F & uv : uvs)
        uv -= mapFloor(uv);
    return ret;
}
static inline void  swapLt(uint & a,uint & b)
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
        Vec3UI       inds = s.tris.vertInds[ii];
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
        Vec4UI       inds = s.quads.vertInds[ii];
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
Surf                mergeSurfaces(Surfs const & surfs)
{
    Surf                ret;
    if (surfs.empty())
        return ret;
    ret = surfs[0];                 // Retain name & material of first surface
    for (size_t ii=1; ii<surfs.size(); ++ii)
        ret.merge(surfs[ii]);
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
        Vec3UI          vertInds = surf.getTriEquivVertInds(tt);
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
            Vec3UI          vertInds = surf.getTriEquivVertInds(tt);
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
        for (SurfPoint const & sp : surf.surfPoints) {
            if (groupLut[sp.point.triEquivIdx] == groupVal) {
                SurfPoint       nsp = sp;
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
    SurfPoints        nsps;
    for (size_t ii=0; ii<surfPoints.size(); ++ii) {
        SurfPoint     sp = surfPoints[ii];
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
    SurfPoints        nsps;
    size_t              idx0 = 2*(quadIdx/2) + tris.size(),
                        idx1 = idx0 + 1;
    for (size_t ii=0; ii<surfPoints.size(); ++ii) {
        SurfPoint     sp = surfPoints[ii];
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
Vec3Fs              cVertsUsed(Vec3UIs const & tris,Vec3Fs const & verts)
{
    vector<bool>    used(verts.size(),false);
    for (Vec3UI const & tri : tris)
        for (size_t xx=0; xx<3; ++xx)
            used.at(tri[xx]) = true;
    Vec3Fs         ret;
    for (size_t ii=0; ii<verts.size(); ++ii)
        if (used[ii])
            ret.push_back(verts[ii]);
    return ret;
}
Vec3Ds              cVertsUsed(Vec3UIs const & tris,Vec3Ds const & verts)
{
    vector<bool>        used (verts.size(),false);
    for (Vec3UI const & tri : tris)
        for (size_t xx=0; xx<3; ++xx)
            used.at(tri[xx]) = true;
    Vec3Ds              ret;
    for (size_t ii=0; ii<verts.size(); ++ii)
        if (used[ii])
            ret.push_back(verts[ii]);
    return ret;
}
bool                hasUnusedVerts(Vec3UIs const & tris,Vec3Fs const & verts)
{
    vector<bool>    unused(verts.size(),true);
    for (Vec3UI const & tri : tris)
        for (size_t xx=0; xx<3; ++xx)
            unused.at(tri[xx]) = false;
    for (vector<bool>::const_iterator it=unused.begin(); it != unused.end(); ++it)
        if (*it)
            return true;
    return false;
}
Uints               removeUnusedVertsRemap(Vec3UIs const & tris,Vec3Fs const & verts)
{
    Uints               remap (verts.size(),numeric_limits<uint>::max());
    for (Vec3UI const & tri : tris)
        for (uint xx=0; xx<3; ++xx)
            remap.at(tri[xx]) = 0;
    uint                cnt = 0;
    for (uint & idx : remap)
        if (idx == 0)
            idx = cnt++;
    return remap;
}
Vec3UIs             reverseWinding(Vec3UIs const & tris)
{
    Vec3UIs             ret; ret.reserve(tris.size());
    for (Vec3UI const & t : tris)
        ret.emplace_back(t[0],t[2],t[1]);
    return ret;
}
Vec4UIs             reverseWinding(Vec4UIs const & quads)
{
    Vec4UIs             ret; ret.reserve(quads.size());
    for (Vec4UI const & q : quads)
        ret.emplace_back(q[0],q[3],q[2],q[1]);
    return ret;
}
BaryPoint           reverseWinding(BaryPoint const & bp,size_t numTris)
{
    Vec3F               w = bp.weights;
    if (bp.triEquivIdx < numTris) {
        Vec3F               wr {w[0],w[2],w[1]};
        return BaryPoint{bp.triEquivIdx,wr};
    }
    else {
        // quad was transformed [0123] -> [0321] thus the equivalent tris were transformed:
        // [012],[230] -> [032],[210] thus both tris have order reversed (and tris are swapped):
        Vec3F               wr {w[2],w[1],w[0]};
        if ((bp.triEquivIdx-numTris) & 0x01)        // second tri of quad -> first tri
            return BaryPoint{bp.triEquivIdx-1,wr};
        else                                        // first tri of quad -> second tri
            return BaryPoint{bp.triEquivIdx+1,wr};
    }
}
SurfPoints          reverseWinding(SurfPoints const & sps,size_t numTris)
{
    SurfPoints          ret; ret.reserve(sps.size());
    for (SurfPoint const & sp : sps)
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
TriSurf             removeUnusedVerts(Vec3Fs const & verts,Vec3UIs const & tris)
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
Vec3F               cQuadNorm(Vec4UI const & quad,Vec3Fs const & verts)
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
    double          crossMag = cMag(cross);
    if (crossMag == 0.0)
        return Vec3F{0};
    else
        return cross * (1.0 / sqrt(crossMag));
}
// calculate facet norm, accumulate it weighted by subtended angle to each vertex, return facet norm:
template<typename T>
Mat<T,3,1>          accNorm_(Svec<Mat<T,3,1>> const & verts,Vec3UI tri,Svec<Mat<T,3,1>> & acc)
{
    Mat<T,3,1>      v0 = verts[tri[0]],
                    v1 = verts[tri[1]],
                    v2 = verts[tri[2]],
                    v01 = v1-v0,
                    v12 = v2-v1,
                    v20 = v0-v2,
                    cross = crossProduct(v01,-v20);         // CC winding
    T               crossMag = cMag(cross);
    if (crossMag > 0) {                                     // non-degenerate
        // This methods weights the contribution of each tri norm to the vertex norm by the angle
        // subtended by that tri. This may give a more reasonable value than averaging (according
        // to Keenan Crane), and avoids problems caused by degenerate tris (with arbitrary normal):
        Mat<T,3,1>      norm = cross / sqrt(crossMag);
        T               len01 = cLen(v01),
                        len12 = cLen(v12),
                        len20 = cLen(v20),
                        // acos maps [1,-1] -> [0,PI] and input is (1,-1) due to non-degeneracy:
                        wgt0 = acos(-cDot(v01,v20)/(len01*len20)),
                        wgt1 = acos(-cDot(v12,v01)/(len12*len01)),
                        wgt2 = acos(-cDot(v20,v12)/(len20*len12));
        acc[tri[0]] += norm * wgt0;
        acc[tri[1]] += norm * wgt1;
        acc[tri[2]] += norm * wgt2;
        return norm;
    }
    return {0,0,1};     // arbitrary for degenerate tri
}
Vec3Ds              cVertNorms(Vec3Ds const & verts,Vec3UIs const & tris)
{
    Vec3Ds          ret (verts.size(),Vec3D{0});
    for (Vec3UI const & tri : tris)
        accNorm_(verts,tri,ret);
    for (Vec3D & norm : ret) {
        double          len = cLen(norm);
        if(len > 0.0)
            norm /= len;
        else
            norm = Vec3D{0,0,1};        // arbitrary when not part of a non-denerate surface
    }
    return ret;
}
MeshNormals         cNormals(Surfs const & surfs,Vec3Fs const & verts)
{
    MeshNormals         norms;
    norms.facet.resize(surfs.size());
    norms.vert.resize(verts.size(),Vec3F{0});
    // Calculate facet normals and accumulate weighted vertex normals:
    for (size_t ss=0; ss<surfs.size(); ss++) {
        Surf const &        surf = surfs[ss];
        FacetNormals &      fnorms = norms.facet[ss];
        fnorms.tri.reserve(surf.numTris());
        fnorms.quad.reserve(surf.numQuads());
        // TRIs
        for (Vec3UI const & tri : surf.tris.vertInds)
            fnorms.tri.push_back(accNorm_(verts,tri,norms.vert));
        // QUADs
        for (Vec4UI const & q : surf.quads.vertInds) {
            fnorms.quad.push_back(cQuadNorm(q,verts));      // use the least squares approx for facet
            // accumulate tri vertex normal contributions but ignore tri facet norms:
            accNorm_(verts,{q[0],q[1],q[2]},norms.vert),
            accNorm_(verts,{q[2],q[3],q[0]},norms.vert);
        }
    }
    // normalize accumulated weighted vertex normals:
    for (Vec3F & norm : norms.vert) {
        float               mag = cMag(norm);
        if (mag > 0)
            norm *= 1.0f / sqrt(mag);
        else
            norm = Vec3F{0,0,1};        // arbitrary. effectively only happens for unused verts.
    }
    return norms;
}
ImgRgba8            cUvWireframeImage(Vec2Fs const & uvs,Vec3UIs const & tris,Vec4UIs const & quads,Rgba8 color)
{
    uint constexpr      dim {2048};
    // Domain is (0,1,1,0) because we have to invert Y to go from OTCS to IPCS:
    Mat22F              domain {0,1,1,0},
                        range (0,dim,0,dim);
    AffineEw2F          xf {domain,range};
    Vec2Is              uvsIrcs; uvsIrcs.reserve(uvs.size());
    for (Vec2F const & uv : uvs)
        uvsIrcs.push_back(Vec2I{mapFloor(xf*uv)});      // floor(IPCS) == IRCS
    ImgRgba8            img {dim,dim,Rgba8{0,0,0,0}};
    for (Vec3UI tri : tris)
        for (uint vv=0; vv<3; ++vv)
            drawLineIrcs(img,uvsIrcs[tri[vv]],uvsIrcs[tri[(vv+1)%3]],color);
    for (Vec4UI quad : quads)
        for (size_t vv=0; vv<4; ++vv)
            drawLineIrcs(img,uvsIrcs[quad[vv]],uvsIrcs[quad[(vv+1)%4]],color);
    return shrink2(img);
}

}

// */
