//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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

Vec3Fs
selectVerts(const LabelledVerts & labVerts,Strings const & labels)
{
    Vec3Fs          ret; ret.reserve(labels.size());
    for (String const & lab : labels)
        ret.push_back(findFirstByMember(labVerts,&LabelledVert::label,lab).pos);
    return ret;
}

Vec3UIs
quadsToTris(const vector<Vec4UI> & quads)
{
    Vec3UIs   ret;
    for (size_t ii=0; ii<quads.size(); ++ii) {
        const Vec4UI &   quad = quads[ii];
        // Ordering must match triEquiv ordering for surface point to remain valid:
        ret.push_back(Vec3UI(quad[0],quad[1],quad[2]));
        ret.push_back(Vec3UI(quad[2],quad[3],quad[0]));
    }
    return ret;
}

TriUv
cTriEquiv(Tris const & tris,Quads const & quads,size_t tt)
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
cTriEquivPosInds(Tris const & tris,Quads const & quads,size_t tt)
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
cTriEquivs(Tris const & tris,Quads const & quads)
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
cSurfPointPos(
    uint                    triEquivIdx,
    Vec3F const &           barycentricCoord,
    Tris const &            tris,
    Quads const &           quads,
    Vec3Fs const &          verts)
{
    Vec3UI           vertInds = cTriEquivPosInds(tris,quads,triEquivIdx);
    return (verts[vertInds[0]] * barycentricCoord[0] +
            verts[vertInds[1]] * barycentricCoord[1] +
            verts[vertInds[2]] * barycentricCoord[2]);
}

uint
Surf::vertIdxMax() const
{
    uint        ret = 0;
    for(size_t qq=0; qq<quads.size(); ++qq)
        for(uint ii=0; ii<4; ++ii)
            updateMax_(ret,quads.posInds[qq][ii]);
    for(size_t tt=0; tt<tris.size(); ++tt)
        for(uint ii=0; ii<3; ++ii)
            updateMax_(ret,tris.posInds[tt][ii]);
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
Surf::surfPointPos(Vec3Fs const & verts,size_t surfPointIdx) const
{
    FGASSERT(surfPointIdx < surfPoints.size());
    SurfPoint const &       sp = surfPoints[surfPointIdx];
    return cSurfPointPos(sp.triEquivIdx,sp.weights,tris,quads,verts);
}

Vec3F
Surf::surfPointPos(Vec3Fs const & verts,string const & label) const
{
    SurfPoint const & sp = findFirst(surfPoints,label);
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
Surf::surfPointsAsLabelledVerts(Vec3Fs const & verts) const
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

Polygons<3>
Surf::asTris() const
{
    Polygons<3>      ret;
    ret.posInds = cat(tris.posInds,quadsToTris(quads.posInds));
    ret.uvInds = cat(tris.uvInds,quadsToTris(quads.uvInds));
    return ret;
}

void
Surf::merge(Tris const & ts,Quads const & qs,SurfPoints const & sps)
{
    for (SurfPoint sp : sps) {
        if (sp.triEquivIdx < ts.size())
            sp.triEquivIdx += uint(tris.size());
        else
            sp.triEquivIdx += uint(tris.size() + 2*quads.size());
        surfPoints.push_back(sp);
    }
    tris.merge(ts);
    quads.merge(qs);
}

void
Surf::checkMeshConsistency(
    uint    coordsSize,
    uint    uvsSize)
    const
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

template<uint N>
ostream & operator<<(ostream & os,Polygons<N> const & f)
{
    os << "vertInds: " << f.posInds.size() << " uvInds: " << f.uvInds.size();
    if (!f.uvInds.empty() && (f.uvInds.size() != f.posInds.size()))
        os << " (MISMATCH)";
    return os;
}

ostream &
operator<<(ostream & os,Surf const & surf)
{
    os
        << fgnl << "Name: " << surf.name
        << fgnl << "Tris: " << surf.tris
        << fgnl << "Quads: " << surf.quads
        << fgnl << "Surf Points: " << surf.surfPoints.size() << fgpush;
        for (size_t ii=0; ii<surf.surfPoints.size(); ++ii)
            os << fgnl << ii << ": " << surf.surfPoints[ii].label;
        os << fgpop;
    return os;
}

Surfs           splitByUvDomain_(Surf const & surf,Vec2Fs & uvs)
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
            permute(surf.tris.posInds,triSels),
            permute(surf.tris.uvInds,triSels),
        };
        Uints const &           quadSels = domainToQuadInds[domain];
        Quads                   quads {
            permute(surf.quads.posInds,quadSels),
            permute(surf.quads.uvInds,quadSels),
        };
        String8                 name = surf.name +nameSep + toStrDigits(ret.size(),2);
        ret.emplace_back(name,tris,quads);
    }
    for (Vec2F & uv : uvs)
        uv -= mapFloor(uv);
    return ret;
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
removeDuplicateFacets(Surf const & s)
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
mergeSurfaces(Surfs const & surfs)
{
    Surf                ret;
    if (surfs.empty())
        return ret;
    ret = surfs[0];                 // Retain name & material of first surface
    for (size_t ii=1; ii<surfs.size(); ++ii)
        ret.merge(surfs[ii]);
    return ret;
}

Surfs
splitByContiguous(Surf const & surf)
{
    Surfs           ret;
    FGASSERT(!surf.empty());
    // Construct a lookup from vert inds back to triEquivs (SurfTopo is overkill for this):
    uint            idxBnd = surf.vertIdxMax() + 1;
    Uintss          vertIdxToTriIdx(idxBnd);
    for (uint tt=0; tt<surf.numTriEquivs(); ++tt) {
        Vec3UI          vertInds = surf.getTriEquivPosInds(tt);
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
            Vec3UI          vertInds = surf.getTriEquivPosInds(tt);
            for (uint jj=0; jj<3; ++jj) {
                Uints const &   triInds = vertIdxToTriIdx[vertInds[jj]];
                for (uint kk=0; kk<triInds.size(); ++kk) {
                    uint            ntt = triInds[kk];
                    if (groupLut[tt] != groupLut[ntt]) {
                        fgReplace_(groupLut,groupLut[ntt],groupLut[tt]);
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
                sub.tris.posInds.push_back(surf.tris.posInds[ii]);
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
                sub.quads.posInds.push_back(surf.quads.posInds[ii]);
                if (!surf.quads.uvInds.empty())
                    sub.quads.uvInds.push_back(surf.quads.uvInds[ii]);
            }
        }
        for (SurfPoint const & sp : surf.surfPoints) {
            if (groupLut[sp.triEquivIdx] == groupVal) {
                SurfPoint       nsp = sp;
                nsp.triEquivIdx = uint(surfIdxToSubIdx[sp.triEquivIdx]);
                sub.surfPoints.push_back(nsp);
            }
        }
        ret.push_back(sub);
    }
    return ret;
}

Surfs
fgEnsureNamed(Surfs const & surfs,String8 const & baseName)
{
    Surfs     ret = surfs;
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
cVertsUsed(Vec3UIs const & tris,Vec3Fs const & verts)
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

Vec3Ds
cVertsUsed(Vec3UIs const & tris,Vec3Ds const & verts)
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

bool
hasUnusedVerts(Vec3UIs const & tris,Vec3Fs const & verts)
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

Uints
removeUnusedVertsRemap(Vec3UIs const & tris,Vec3Fs const & verts)
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

Vec3UIs
reverseWinding(Vec3UIs const & tris)
{
    Vec3UIs             ret; ret.reserve(tris.size());
    for (Vec3UI const & t : tris)
        ret.emplace_back(t[0],t[2],t[1]);
    return ret;
}

Vec4UIs
reverseWinding(Vec4UIs const & quads)
{
    Vec4UIs             ret; ret.reserve(quads.size());
    for (Vec4UI const & q : quads)
        ret.emplace_back(q[0],q[3],q[2],q[1]);
    return ret;
}

SurfPoints
reverseWinding(SurfPoints const & sps,size_t numTris)
{
    SurfPoints          ret; ret.reserve(sps.size());
    for (SurfPoint const & sp : sps) {
        Vec3F const &       w = sp.weights;
        if (sp.triEquivIdx < numTris) {
            Vec3F               wr {w[0],w[2],w[1]};
            ret.emplace_back(sp.triEquivIdx,wr,sp.label);
        }
        else {
            Vec3F           wr {w[2],w[1],w[0]};
            if ((sp.triEquivIdx-numTris) & 0x01)        // On second tri of quad
                ret.emplace_back(sp.triEquivIdx-1,wr,sp.label);
            else                                        // On first tri of quad
                ret.emplace_back(sp.triEquivIdx+1,wr,sp.label);
        }
    }
    return ret;
}

Surf
reverseWinding(Surf const & in)
{
    return              Surf {
        in.name,
        {
            reverseWinding(in.tris.posInds),
            reverseWinding(in.tris.uvInds),
        },
        {
            reverseWinding(in.quads.posInds),
            reverseWinding(in.quads.uvInds),
        },
        reverseWinding(in.surfPoints,in.tris.posInds.size()),
        in.material,
    };
}

TriSurf
reverseWinding(TriSurf const & ts)
{
    return TriSurf {ts.verts,reverseWinding(ts.tris)};
}

TriSurf
removeUnusedVerts(Vec3Fs const & verts,Vec3UIs const & tris)
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

Vec3D
cTriNorm(Vec3UI const & tri,Vec3Ds const & verts)
{
    Vec3D               v0 = verts[tri[0]],
                        v1 = verts[tri[1]],
                        v2 = verts[tri[2]],
                        cross = crossProduct(v1-v0,v2-v0);      // CC winding
    double              mag = cMag(cross);
    return (mag == 0.0) ? Vec3D{0} : cross * (1.0 / sqrt(mag));
}

Vec3F
cTriNorm(Vec3UI const & tri,Vec3Fs const & verts)
{
    Vec3F               v0 = verts[tri[0]],
                        v1 = verts[tri[1]],
                        v2 = verts[tri[2]],
                        cross = crossProduct(v1-v0,v2-v0);      // CC winding
    float               mag = cMag(cross);
    return (mag == 0.0f) ? Vec3F{0} : cross * (1.0f / sqrt(mag));
}

Vec3Ds
cVertNorms(Vec3Ds const & verts,Vec3UIs const & tris)
{
    Vec3Ds          vertNorms(verts.size(),Vec3D(0));
    for (Vec3UI tri : tris) {
        Vec3D       norm = cTriNorm(tri,verts);
        vertNorms[tri[0]] += norm;
        vertNorms[tri[1]] += norm;
        vertNorms[tri[2]] += norm;
    }
    Vec3Ds          ret;
    ret.reserve(verts.size());
    for (Vec3D norm : vertNorms) {
        double          len = cLen(norm);
        if(len > 0.0)
            ret.push_back(norm/len);
        else
            ret.push_back(Vec3D(0,0,1));     // Arbitrary
    }
    return ret;
}

// Vertex normals are just approximated by a simple average of the facet normals of all
// facets containing the vertex:
MeshNormals
cNormals(Surfs const & surfs,Vec3Fs const & verts)
{
    MeshNormals         norms;
    norms.facet.resize(surfs.size());
    Vec3Fs              vertNorms(verts.size(),Vec3F{0});
    // Calculate facet normals and accumulate unnormalized vertex normals:
    for (size_t ss=0; ss<surfs.size(); ss++) {
        Surf const &        surf = surfs[ss];
        FacetNormals &      fnorms = norms.facet[ss];
        fnorms.tri.reserve(surf.numTris());
        fnorms.quad.reserve(surf.numQuads());
        // TRIs
        for (Vec3UI tri : surf.tris.posInds) {
            Vec3F       norm = cTriNorm(tri,verts);
            vertNorms[tri[0]] += norm;
            vertNorms[tri[1]] += norm;
            vertNorms[tri[2]] += norm;
            Vec3F       normf(norm);
            fnorms.tri.push_back(normf);
        }
        // QUADs
        // This least squares surface normal is taken from [Mantyla 87]:
        for (Vec4UI quad : surf.quads.posInds) {
            Vec3F       v0(verts[quad[0]]),
                        v1(verts[quad[1]]),
                        v2(verts[quad[2]]),
                        v3(verts[quad[3]]);
            Vec3F       cross,norm;
            cross[0] =  (v0[1]-v1[1]) * (v0[2]+v1[2]) +
                        (v1[1]-v2[1]) * (v1[2]+v2[2]) +
                        (v2[1]-v3[1]) * (v2[2]+v3[2]) +
                        (v3[1]-v0[1]) * (v3[2]+v0[2]);
            cross[1] =  (v0[2]-v1[2]) * (v0[0]+v1[0]) +
                        (v1[2]-v2[2]) * (v1[0]+v2[0]) +
                        (v2[2]-v3[2]) * (v2[0]+v3[0]) +
                        (v3[2]-v0[2]) * (v3[0]+v0[0]);
            cross[2] =  (v0[0]-v1[0]) * (v0[1]+v1[1]) +
                        (v1[0]-v2[0]) * (v1[1]+v2[1]) +
                        (v2[0]-v3[0]) * (v2[1]+v3[1]) +
                        (v3[0]-v0[0]) * (v3[1]+v0[1]);
            double      crossMag = cross.len();
            if (crossMag == 0.0)
                norm = Vec3F(0);
            else
                norm = cross * (1.0 / crossMag);
            vertNorms[quad[0]] += norm;
            vertNorms[quad[1]] += norm;
            vertNorms[quad[2]] += norm;
            vertNorms[quad[3]] += norm;
            Vec3F       normf(norm);
            fnorms.quad.push_back(normf);
        }
    }
    // Normalize vertex normals:
    norms.vert.reserve(verts.size());
    for (Vec3F const & norm : vertNorms) {
        float           len = cLen(norm);
        if(len > 0.0f)
            norms.vert.push_back(Vec3F(norm/len));
        else
            norms.vert.push_back(Vec3F(0,0,1));     // Arbitrary
    }
    return norms;
}

ImgRgba8
cUvWireframeImage(Vec2Fs const & uvs,Vec3UIs const & tris,Vec4UIs const & quads,Rgba8 color)
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
