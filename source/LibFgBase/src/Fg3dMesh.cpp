//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dDisplay.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgCamera.hpp"
#include "FgGridIndex.hpp"
#include "FgBestN.hpp"
#include "FgSerial.hpp"
#include "FgBounds.hpp"
#include "FgMath.hpp"
#include "FgTopology.hpp"
#include "FgMain.hpp"
#include "FgCommand.hpp"
#include "FgFileSystem.hpp"

using namespace std;

namespace Fg {

IdxVec3Fs           toIndexedDeltaMorph(Vec3Fs const & deltas,float distanceThreshold)
{
    IdxVec3Fs           ret;
    float               magThresh = sqr(distanceThreshold);
    for (size_t vv=0; vv<deltas.size(); ++vv) {
        Vec3F const &       delta = deltas[vv];
        if (cMagD(delta) > magThresh)
            ret.emplace_back(scast<uint>(vv),delta);
    }
    return ret;
}

IdxVec3Fs           toIndexedTargMorph(Vec3Fs const & base,Vec3Fs const & deltas,float diffMagThreshold)
{
    size_t              V = base.size();
    FGASSERT(deltas.size() == V);
    IdxVec3Fs           ret;
    for (size_t vv=0; vv<V; ++vv) {
        Vec3F               del = deltas[vv];
        if (cMagD(del) > diffMagThreshold)
            ret.emplace_back(scast<uint>(vv),base[vv]+del);
    }
    return ret;
}

IdxVec3Fs           offsetIndices(IdxVec3Fs const & ivs,uint offset)
{
    return mapCall(ivs,[=](IdxVec3F const & iv){return IdxVec3F{iv.idx+offset,iv.vec}; });
}

Vec3Fs              toDirectDeltaMorph(IdxVec3Fs const & ivs,size_t baseSize)
{
    Vec3Fs              ret {baseSize,Vec3F{0}};
    for (IdxVec3F const & iv : ivs) {
        FGASSERT(iv.idx < baseSize);
        ret[iv.idx] = iv.vec;
    }
    return ret;
}

void                accDeltaMorph_(IdxVec3Fs const & deltaMorph,Vec3Fs & verts)
{
    size_t              V = verts.size();
    for (IdxVec3F const & iv : deltaMorph) {
        FGASSERT(iv.idx < V);
        verts[iv.idx] += iv.vec;
    }
}

size_t              sumSizes(IndexedMorphs const & ims)
{
    size_t              ret {0};
    for (IndexedMorph const & im : ims)
        ret += im.ivs.size();
    return ret;
}

void                accDeltaMorphs_(DirectMorphs const & morphs,Floats const & coeffs,Vec3Fs & acc)
{
    size_t              M = morphs.size();
    FGASSERT(coeffs.size() == M);
    for (size_t mm=0; mm<M; ++mm)
        mapMulAcc_(morphs[mm].verts,coeffs[mm],acc);
}

void                accTargetMorphs_(
    Vec3Fs const &          allVerts,
    IndexedMorphs const &   targMorphs,
    Floats const &          coord,
    Vec3Fs &                accVerts)
{
    FGASSERT(targMorphs.size() == coord.size());
    size_t          numTargVerts = 0;
    for (IndexedMorph const & tm : targMorphs)
        numTargVerts += tm.ivs.size();
    FGASSERT(accVerts.size() + numTargVerts == allVerts.size());
    size_t          idx = accVerts.size();
    for (size_t ii=0; ii<targMorphs.size(); ++ii) {
        IdxVec3Fs const &   ivs = targMorphs[ii].ivs;
        for (size_t jj=0; jj<ivs.size(); ++jj) {
            size_t          baseIdx = ivs[jj].idx;
            Vec3F           del = allVerts[idx++] - allVerts[baseIdx];
            accVerts[baseIdx] += del * coord[ii];
        }
    }
}

Mesh::Mesh(Vec3Fs const & vts,Surf const & surf) : verts(vts), surfaces{surf}
{
    surfaces[0].tris.uvInds.clear();
    surfaces[0].quads.uvInds.clear();
}

Mesh::Mesh(Vec3Fs const & vts,Surfs const & surfs) : verts(vts), surfaces(surfs)
{
    for (Surf & surf : surfaces) {
        surf.tris.uvInds.clear();
        surf.quads.uvInds.clear();
    }
}

Mesh::Mesh(TriSurfLms const & tsf) :
    verts {cat(tsf.surf.verts,tsf.landmarks)},
    surfaces {Surf{tsf.surf.tris,Arr4UIs{}}}
{
    size_t          V = tsf.surf.verts.size(),
                    F = tsf.landmarks.size();
    for (size_t ii=V; ii<V+F; ++ii)
        markedVerts.push_back(MarkedVert{uint(ii)});
}

size_t              Mesh::allVertsSize() const
{
    return verts.size() + sumSizes(targetMorphs) + joints.size();
}

Vec3Fs              Mesh::allVerts() const
{
    Vec3Fs              ret; ret.reserve(allVertsSize());
    cat_(ret,verts);
    for (IndexedMorph const & tm : targetMorphs)
        for (IdxVec3F const & iv : tm.ivs)
            ret.push_back(iv.vec);
    for (Joint const & joint : joints)
        ret.push_back(joint.pos);
    return ret;
}

void                Mesh::updateAllVerts(Vec3Fs const & allVerts)
{
    FGASSERT(allVerts.size() == allVertsSize());
    size_t          idx = 0;
    for (Vec3F & v : verts)
        v =  allVerts[idx++];
    for (IndexedMorph & tm : targetMorphs)
        for (IdxVec3F & iv : tm.ivs)
            iv.vec = allVerts[idx++];
    for (Joint & joint : joints)
        joint.pos = allVerts[idx++];
}

Arr3UI              Mesh::getTriEquivVertInds(uint idx) const
{
    for (size_t ss=0; ss<surfaces.size(); ++ss) {
        uint        num = scast<uint>(surfaces[ss].numTriEquivs());
        if (idx < num)
            return surfaces[ss].getTriEquivVertInds(idx);
        else
            idx -= num;
    }
    FGASSERT_FALSE;
    return Arr3UI();
}

NPolys<3>           Mesh::getTriEquivs() const
{
    NPolys<3>       ret;
    if (surfaces.empty())
        return ret;
    ret = surfaces[0].getTriEquivs();
    for (size_t ii=1; ii<surfaces.size(); ++ii)
        ret = merge(ret,surfaces[ii].getTriEquivs());
    return ret;
}

Vec3F               Mesh::surfPointPos(Vec3Fs const & verts_,size_t num) const
{
    for (size_t ss=0; ss<surfaces.size(); ss++) {
        if (num < surfaces[ss].surfPoints.size())
            return surfaces[ss].surfPointPos(verts_,num);
        else
            num -= surfaces[ss].surfPoints.size();
    }
    FGASSERT_FALSE;
    return Vec3F(0);        // Avoid warning.
}
Vec3F               Mesh::surfPointPos(String const & label) const
{
    for (Surf const & surf : surfaces)
        for (SurfPointName const & spn : surf.surfPoints)
            if (spn.label == label)
                return surf.surfPointPos(verts,spn.point);
    fgThrow("Unable to find surface point",label);
    return {};
}
Vec3Fs              Mesh::surfPointPositions(Strings const & labels) const
{
    return mapCall(labels,[&](String const & label){return this->surfPointPos(label); });
}
Vec3Fs              Mesh::surfPointPositions() const
{
    Vec3Fs      ret;
    for (Surf const & surf : surfaces)
        cat_(ret,surf.surfPointPositions(verts));
    return ret;
}

Vec3Fs              Mesh::markedVertPositions() const
{
    Vec3Fs     ret;
    ret.reserve(markedVerts.size());
    for (const MarkedVert & m : markedVerts)
        ret.push_back(verts[m.idx]);
    return ret;
}

Vec3Fs              Mesh::markedVertPositions(Strings const & labels) const
{
    Vec3Fs              ret;
    ret.reserve(labels.size());
    for (String const & label : labels) {
        MarkedVerts         mvs = selectEqual(markedVerts,label);
        for (MarkedVert const & mv : mvs)
            ret.push_back(verts[mv.idx]);
    }
    return ret;
}

NameVec3Fs          Mesh::markedVertsAsNameVecs() const
{
    NameVec3Fs          ret; ret.reserve(markedVerts.size());
    for (MarkedVert const & mv : markedVerts)
        ret.emplace_back(mv.label,verts[mv.idx]);
    return ret;
}

void                Mesh::addMarkedVerts(NameVec3Fs const & nvs)
{
    for (NameVec3F const & nv : nvs) {
        markedVerts.emplace_back(scast<uint>(verts.size()),nv.name);
        verts.push_back(nv.vec);
    }
}

TriSurf             Mesh::asTriSurf() const
{
    TriSurf   ret;
    ret.verts = verts;
    for (Surf const & surf : surfaces) {
        cat_(ret.tris,surf.tris.vertInds);
        if (!surf.quads.vertInds.empty())
            cat_(ret.tris,asTris(surf.quads.vertInds));
    }
    return ret;
}

String8             Mesh::morphName(size_t idx) const
{
    if (idx < deltaMorphs.size())
        return deltaMorphs[idx].name;
    idx -= deltaMorphs.size();
    return targetMorphs[idx].name;
}

String8s            Mesh::morphNames() const
{
    String8s    ret;
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        ret.push_back(deltaMorphs[ii].name);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        ret.push_back(targetMorphs[ii].name);
    return ret;
}

Valid<size_t>       Mesh::findDeltaMorph(String8 const & name_) const
{
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        if (String8(deltaMorphs[ii].name) == name_)
            return Valid<size_t>(ii);
    return Valid<size_t>();
}

Valid<size_t>       Mesh::findTargMorph(String8 const & name_) const
{
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        if (targetMorphs[ii].name == name_)
            return Valid<size_t>(ii);
    return Valid<size_t>();
}

Valid<size_t>       Mesh::findMorph(String8 const & name_) const
{
    for (size_t ii=0; ii<numMorphs(); ++ii)
        if (morphName(ii) == name_)
            return Valid<size_t>(ii);
    return Valid<size_t>();
}

void                Mesh::morph(
    Floats const &      morphCoord,
    Vec3Fs &           outVerts) const
{
    FGASSERT(morphCoord.size() == numMorphs());
    outVerts = verts;
    size_t      cnt = 0;
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        deltaMorphs[ii].accAsDelta_(morphCoord[cnt++],outVerts);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        targetMorphs[ii].accAsTarget_(verts,morphCoord[cnt++],outVerts);
}

void                Mesh::morph(
    Vec3Fs const &     allVerts,
    Floats const &      coord,
    Vec3Fs &           outVerts) const
{
    outVerts = cHead(allVerts,verts.size());
    size_t          ndms = deltaMorphs.size();
    accDeltaMorphs_(deltaMorphs,cHead(coord,ndms),outVerts);
    accTargetMorphs_(allVerts,targetMorphs,cRest(coord,ndms),outVerts);
}

Vec3Fs              Mesh::morph(
    Floats const &      deltaMorphCoord,
    Floats const &      targMorphCoord) const
{
    Vec3Fs     ret = verts;
    FGASSERT(deltaMorphCoord.size() == deltaMorphs.size());
    FGASSERT(targMorphCoord.size() == targetMorphs.size());
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        if (deltaMorphCoord[ii] != 0.0f)
            deltaMorphs[ii].accAsDelta_(deltaMorphCoord[ii],ret);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        if (targMorphCoord[ii] != 0.0f)
            targetMorphs[ii].accAsTarget_(verts,targMorphCoord[ii],ret);
    return ret;
}

Vec3Fs              Mesh::morphSingle(size_t idx,float val) const
{
    Vec3Fs     ret = verts;
    if (idx < deltaMorphs.size())
        deltaMorphs[idx].accAsDelta_(val,ret);
    else {
        idx -= deltaMorphs.size();
        FGASSERT(idx < targetMorphs.size());
        targetMorphs[idx].accAsTarget_(verts,val,ret);
    }
    return ret;
}

IndexedMorph        Mesh::getMorphAsIndexedDelta(size_t idx) const
{
    IndexedMorph        ret;
    float               thresh = cMaxElem(cDims(verts)) * epsBits(15);
    if (idx < deltaMorphs.size()) {         // input is a per-vertx delta morph
        DirectMorph const &     dm = deltaMorphs[idx];
        ret = {dm.name,toIndexedDeltaMorph(dm.verts,thresh)};
    }
    else {                                  // input is an indexed target morph
        idx -= deltaMorphs.size();
        FGASSERT(idx < targetMorphs.size());
        IndexedMorph const &    tm = targetMorphs[idx];
        ret.name = tm.name;
        ret.ivs = tm.ivs;
        for (size_t ii=0; ii<ret.ivs.size(); ++ii)
            ret.ivs[ii].vec -= verts[ret.ivs[ii].idx];
    }
    return ret;
}

void                Mesh::addDeltaMorph(DirectMorph const & morph)
{
    Valid<size_t>         idx = findDeltaMorph(morph.name);
    if (idx.valid()) {
        fgout << fgnl << "WARNING: Overwriting existing morph " << morph.name;
        deltaMorphs[idx.val()] = morph;
    }
    else
        deltaMorphs.push_back(morph);
}

void                Mesh::addDeltaMorphFromTarget(String8 const & name_,Vec3Fs const & targetShape)
{
    DirectMorph                 dm;
    dm.name = name_;
    dm.verts = targetShape - verts;
    addDeltaMorph(dm);
}

void                Mesh::addTargMorph(const IndexedMorph & morph)
{
    FGASSERT(cMaxElem(mapMember(morph.ivs,&IdxVec3F::idx)) < verts.size());
    Valid<size_t>     idx = findTargMorph(morph.name);
    if (idx.valid()) {
        fgout << fgnl << "WARNING: Overwriting existing morph " << morph.name;
        targetMorphs[idx.val()] = morph;
    }
    else
        targetMorphs.push_back(morph);
}

void                Mesh::addTargMorph(String8 const & morphName,Vec3Fs const & targetShape)
{
    FGASSERT(targetShape.size() == verts.size());
    IndexedMorph        targMorph {morphName,{}};
    Vec3Fs              deltas = targetShape - verts;
    double              maxMag = 0.0;
    for (size_t ii=0; ii<deltas.size(); ++ii)
        updateMax_(maxMag,deltas[ii].magD());
    if (maxMag == 0.0f) {
        fgout << fgnl << "WARNING: skipping empty morph " << morphName;
        return;
    }
    maxMag *= sqr(0.001f);
    for (size_t ii=0; ii<deltas.size(); ++ii)
        if (deltas[ii].magD() > maxMag)
            targMorph.ivs.emplace_back(uint(ii),targetShape[ii]);
    addTargMorph(targMorph);
}

Vec3Fs              Mesh::applyMorphs(Vec3Fs const & allVerts,map<String8,float> const & morphVals) const
{
    Vec3Fs                  ret = cHead(allVerts,verts.size());
    for (DirectMorph const & morph : deltaMorphs) {
        auto                    it = morphVals.find(morph.name);
        if (it != morphVals.end())
            mapMulAcc_(morph.verts,it->second,ret);
    }
    size_t                  targIdx = verts.size();
    for (IndexedMorph const & tm : targetMorphs) {
        auto                    it = morphVals.find(tm.name);
        if (it != morphVals.end()) {
            FGASSERT(targIdx + tm.ivs.size() < allVerts.size()+1);
            size_t                  ti = targIdx;
            for (IdxVec3F const & iv : tm.ivs) {
                Vec3F           del = allVerts[ti++] - allVerts[iv.idx];
                ret[iv.idx] += del * it->second;
            }
        }
        targIdx += tm.ivs.size();
    }
    // Accumulate the quaternion component contributions from each joint DOF:
    Vec4Fs                  jointAccs (joints.size(),Vec4F{0});
    for (JointDof const & dof : jointDofs) {
        auto                    it = morphVals.find(dof.name);
        if (it != morphVals.end()) {
            FGASSERT(dof.jointIdx < jointAccs.size());
            float                   val = it->second / 2.0f;
            Vec3F                   a = dof.rotAxis * sin(val);
            jointAccs[dof.jointIdx] += Vec4F {cos(val),a[0],a[1],a[2]};
        }
    }
    // For the non-zero accumulants, interpolate and rotate:
    for (size_t pp=0; pp<joints.size(); ++pp) {
        Vec4F const &           acc = jointAccs[pp];
        if (cMagD(acc) > 0) {
            Joint const &           joint = joints[pp];
            QuaternionF             q {acc};        // normalizes
            Mat33F                  rot = q.asMatrix();
            // TODO: Ignore skinning and apply to all for now:
            // TODO: we'll need deformed bone & axis values:
            for (Vec3F & v : ret)
                v = rot * (v - joint.pos) + joint.pos;
        }
    }
    return ret;
}

void                Mesh::addSurface(TriSurf const & ts,String8 const & surfName)
{
    uint                offset = scast<uint>(verts.size());
    TriInds             tris {mapAdd(ts.tris,Arr3UI{offset})};
    surfaces.emplace_back(surfName,tris,QuadInds{});
    cat_(verts,ts.verts);
}

void                Mesh::transform_(Affine3F const & aff)
{
    Mat33F              lin = aff.linear;
    mapMul_(aff,verts);
    for (DirectMorph & morph : deltaMorphs)
        mapMul_(lin,morph.verts);
    for (IndexedMorph & tm : targetMorphs)
        for (IdxVec3F & iv : tm.ivs)
            iv.vec = aff * iv.vec;
    for (Joint & joint : joints)
        joint.pos = aff * joint.pos;
    for (JointDof & dof : jointDofs)
        dof.rotAxis = normalize(lin * dof.rotAxis);
}

void                Mesh::convertToTris()
{
    for (size_t ii=0; ii<surfaces.size(); ++ii)
        surfaces[ii] = surfaces[ii].convertToTris();
}

void                Mesh::removeUVs()
{
    for (Surf & surf : surfaces) {
        surf.tris.uvInds.clear();
        surf.quads.uvInds.clear();
    }
    uvs.clear();
}

void                Mesh::validate() const
{
    uint            numVerts = uint(verts.size());
    for (Surf const & surf : surfaces)
        surf.validate(numVerts,uint(uvs.size()));
    for (MarkedVert const & markedVert : markedVerts)
        FGASSERT(markedVert.idx < numVerts);
}

Mesh                fg3dMesh(Arr3UIs const & tris,Vec3Fs const & verts)
{
    Mesh        ret;
    ret.surfaces.resize(1);
    ret.surfaces[0].tris.vertInds = tris;
    ret.verts = verts;
    return ret;
}

size_t              cNumTriEquivs(Meshes const & meshes)
{
    size_t      ret = 0;
    for (Mesh const & m : meshes)
        ret += m.numTriEquivs();
    return ret;
}

set<String8>        getMorphNames(Meshes const & meshes)
{
    set<String8>  ret;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        cUnion_(ret,meshes[ii].morphNames());
    return ret;
}

MorphCtrls            cPoseDefs(Mesh const & mesh)
{
    MorphCtrls            ret;
    for (DirectMorph const & morph : mesh.deltaMorphs)
        ret.emplace_back(morph.name,Vec2F{0,1},0);
    for (IndexedMorph const & morph : mesh.targetMorphs)
        ret.emplace_back(morph.name,Vec2F{0,1},0);
    for (JointDof const & dof : mesh.jointDofs)
        ret.emplace_back(dof.name,dof.angleBounds,0);
    return ret;
}

MorphCtrls            cPoseDefs(Meshes const & meshes)
{
    MorphCtrls                poseDefs;
    for (Mesh const & mesh : meshes)
        cat_(poseDefs,cPoseDefs(mesh));
    return cUnique(sortAll(poseDefs));
}

Mesh                transform(Mesh const & mesh,SimilarityD const & sim)
{
    Mesh                ret = mesh;
    ret.transform_(sim);
    return ret;
}

ostream &      operator<<(ostream & os,Mesh const & m)
{
    os  << fgnl << "Name: " << m.name;
    Vec3Fs              allVerts = m.allVerts();
    if (allVerts.size() != m.verts.size())
        os << fgnl << "AllVerts: " << allVerts.size() << " with bounds: " << catH(cBounds(allVerts));
    os  << fgnl << "Verts: " << m.verts.size();
    size_t              numUniqueVerts = cUnique(sortAll(m.verts)).size();
    if (numUniqueVerts < m.verts.size())
        os << " (" << numUniqueVerts << " unique)";
    os  << " with bounds: " << catH(cBounds(m.verts))
        << fgnl << "UVs: " << m.uvs.size();
    if (!m.uvs.empty()) {
        size_t          numUnique = cUnique(sortAll(m.uvs)).size();
        if (numUnique < m.uvs.size())
            os << " (" << numUnique << " unique)";
        os << " with bounds: " << catH(cBounds(m.uvs));
    }
    os  << fgnl << "Delta Morphs: " << m.deltaMorphs.size()
        << fgnl << "Target Morphs: " << m.targetMorphs.size()
        << fgnl << "Marked Verts: " << m.markedVerts.size();
    {
        PushIndent          pind;
        for (MarkedVert const & mv : m.markedVerts)
            if (!mv.label.empty())
                os << fgnl << toStrDigits(mv.idx,2) << ": " << mv.label;
    }
    os << fgnl << "Surfaces: " << m.surfaces.size() << fgpush;
    for (size_t ss=0; ss<m.surfaces.size(); ss++) {
        os << fgnl << toStrDigits(ss,2) << fgpush;
        Surf const &    surf = m.surfaces[ss];
        os << surf;
        if (surf.material.albedoMap)
            os << fgnl << "Albedo: " << *surf.material.albedoMap;
        os << fgpop;
    }
    os << fgpop;
    SurfTopo            topo {m.verts.size(),m.getTriEquivs().vertInds};
    auto                boundaries = topo.boundaries();
    Arr3UI              te = topo.isManifold();
    os << fgnl << "Watertight: " << ((te == Arr3UI(0)) ? "YES" : "NO");
    os << fgnl << "Manifold: ";
    if ((te[1] == 0) && (te[2] == 0))
        os << "YES";
    else
        os << "NO (" << te[1] << " intersection edges, " << te[2] << " reversed edges)";
    os << fgnl << "Boundaries: " << boundaries.size();
    if (!boundaries.empty())
        os << " with edge sizes " << cSizes(boundaries);
    os << fgnl << "Unused verts: " << topo.unusedVerts();
    return os;
}

ostream &      operator<<(ostream & os,Meshes const & ms)
{
    for (size_t ii=0; ii<ms.size(); ++ii)
        os << fgnl << "Mesh " << ii << ":" << fgpush << ms[ii] << fgpop;
    return os;
}

Arr3UIs             subdivideTris(
    Arr3UIs const &             tris,
    Svec<SurfTopo::Tri> const &   topoTris,
    uint                        newVertsBaseIdx)
{
    Arr3UIs             ret;
    for (size_t ii=0; ii<tris.size(); ii++) {
        Arr3UI          vertInds = tris[ii];
        Arr3UI          edgeInds = topoTris[ii].edgeInds;
        uint            ni0 = newVertsBaseIdx + edgeInds[0],
                        ni1 = newVertsBaseIdx + edgeInds[1],
                        ni2 = newVertsBaseIdx + edgeInds[2];
        ret.push_back(Arr3UI(vertInds[0],ni0,ni2));
        ret.push_back(Arr3UI(vertInds[1],ni1,ni0));
        ret.push_back(Arr3UI(vertInds[2],ni2,ni1));
        ret.push_back(Arr3UI(ni0,ni1,ni2));
    }
    return ret;
}

Surf                subdivideTris(
    Arr3UIs const &             tris,
    Svec<SurfTopo::Tri> const & topoTris,
    uint                        newVertsBaseIdx,
    SurfPointNames const &      sps)
{
    Surf            ret;
    ret.tris.vertInds = subdivideTris(tris,topoTris,newVertsBaseIdx);
    // Set up surface point weight transforms:
    Mat33F          wgtXform(1),
                    wgtXform0(0),
                    wgtXform1(0),
                    wgtXform2(0);
    wgtXform.rc(0,2) = -1.0;
    wgtXform.rc(1,0) = -1.0;
    wgtXform.rc(2,1) = -1.0;
    wgtXform0[0] = 1.0f;
    wgtXform0[1] = -1.0f;
    wgtXform0[2] = -1.0f;
    wgtXform0.rc(1,1) = 2;
    wgtXform0.rc(2,2) = 2;
    wgtXform1[0] = -1.0f;
    wgtXform1[1] = 1.0f;
    wgtXform1[2] = -1.0f;
    wgtXform1.rc(1,2) = 2;
    wgtXform1.rc(2,0) = 2;
    wgtXform2[0] = -1.0f;
    wgtXform2[1] = -1.0f;
    wgtXform2[2] = 1.0f;
    wgtXform2.rc(1,0) = 2;
    wgtXform2.rc(2,1) = 2;
    // Update surface points:
    for (size_t ii=0; ii<sps.size(); ++ii) {
        uint        facetIdx = sps[ii].point.triEquivIdx * 4;
        Vec3F       weights {sps[ii].point.weights},
                    wgtCentre = wgtXform * weights;
        if (wgtCentre[0] < 0.0)
            ret.surfPoints.push_back(SurfPointName(facetIdx+2,(wgtXform2*weights).m));
        else if (wgtCentre[1] < 0.0)
            ret.surfPoints.push_back(SurfPointName(facetIdx,(wgtXform0*weights).m));
        else if (wgtCentre[2] < 0.0)
            ret.surfPoints.push_back(SurfPointName(facetIdx+1,(wgtXform1*weights).m));
        else
            ret.surfPoints.push_back(SurfPointName(facetIdx+3,wgtCentre.m));
    }
    return ret;
}

Arr<Vec3F,2>        updateVertBounds2(Meshes const & meshes)
{
    Arr<Vec3F,2>        ret = nullBounds<Vec3F>();
    for (Mesh const & mesh : meshes)
        ret = updateBounds(mesh.verts,ret);
    return ret;
}

Mesh                subdivide(Mesh const & in,bool loop)
{
    TriInds             allTris;
    SurfPointNames      allSps;
    for (Surf const & surf : in.surfaces) {
        for (SurfPointName sp : surf.surfPoints) {
            sp.point.triEquivIdx += uint(allTris.size());
            allSps.push_back(sp);
        }
        allTris = merge(allTris,surf.tris);
    }
    Mesh                ret;
    ret.verts = in.verts;   // Modified later in case of Loop:
    ret.markedVerts = in.markedVerts;
    SurfTopo            topo {in.verts.size(),allTris.vertInds};
    uint                    newVertsBaseIdx = uint(in.verts.size());
    if (loop) {
        // Add the edge-split "odd" verts:
        for (uint ii=0; ii<topo.m_edges.size(); ++ii) {
            Vec2UI              vertInds0 = topo.m_edges[ii].vertInds;
            if (topo.m_edges[ii].triInds.size() == 1) {     // Boundary
                ret.verts.push_back((
                    in.verts[vertInds0[0]] + 
                    in.verts[vertInds0[1]])*0.5f);
            }
            else {
                Vec2UI          vertInds1 = topo.edgeFacingVertInds(ii);
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
                Uints           vertInds = topo.vertBoundaryNeighbours(ii);
                if (vertInds.size() != 2)
                    fgThrow("Cannot subdivide non-manifold mesh at vert index",toStr(ii));
                ret.verts[ii] = (in.verts[ii] * 6.0 + in.verts[vertInds[0]] + in.verts[vertInds[1]]) * 0.125f;
            }
            else {
                // Note that there will always be at least 3 neighbours since 
                // this is not a boundary vertex:
                const Uints &   neighbours = topo.vertNeighbours(ii);
                Vec3F           acc {0};
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
            Vec2UI          vertInds = topo.m_edges[ii].vertInds;
            ret.verts.push_back((in.verts[vertInds[0]]+in.verts[vertInds[1]])*0.5);
        }
    }
    Surf                ssurf = subdivideTris(allTris.vertInds,topo.m_tris,newVertsBaseIdx,allSps);
    // Can only carry over UVs if they exist and are defined for all tris (ie on all surfaces):
    if (!in.uvs.empty() && (allTris.uvInds.size() == allTris.vertInds.size())) {
        ret.uvs = in.uvs;
        SurfTopo        topoUvs {in.uvs.size(),allTris.uvInds};
        uint                newUvsBaseIdx = uint(in.uvs.size());
        for (SurfTopo::Edge const & edge : topoUvs.m_edges) {
            Vec2UI              vi = edge.vertInds;
            ret.uvs.push_back((in.uvs[vi[0]]+in.uvs[vi[1]])*0.5);
        }
        ssurf.tris.uvInds = subdivideTris(allTris.uvInds,topoUvs.m_tris,newUvsBaseIdx);
    }
    size_t              sidx = 0,
                        spidx = 0;
    ret.surfaces.reserve(in.surfaces.size());
    for (Surf const & surfIn : in.surfaces) {
        TriInds             tris;
        SurfPointNames      surfPoints;
        size_t              num = surfIn.tris.size() * 4;
        tris.vertInds = cSubvec(ssurf.tris.vertInds,sidx,num);
        if (!ssurf.tris.uvInds.empty())
            tris.uvInds = cSubvec(ssurf.tris.uvInds,sidx,num);
        for (size_t ii=0; ii<surfIn.surfPoints.size(); ++ii) {
            SurfPointName           sp = ssurf.surfPoints[spidx+ii];
            sp.point.triEquivIdx -= uint(sidx);
            surfPoints.push_back(sp);
        }
        sidx += num;
        spidx += surfIn.surfPoints.size();
        ret.surfaces.emplace_back(surfIn.name,tris,QuadInds{},surfPoints,surfIn.material);
    }
    return ret;
}

// Hack this for now:
TriSurf             subdivide(TriSurf const & surf,bool loop)
{
    Mesh                mesh = subdivide(Mesh{"",surf},loop);
    return TriSurf {mesh.verts,mesh.surfaces[0].tris.vertInds};
}

TriSurf             subdivideN(TriSurf ts,size_t N,bool loop)
{
    for (size_t ii=0; ii<N; ++ii)
        ts = subdivide(ts,loop);
    return ts;
}

TriSurf             cullVolume(TriSurf triSurf,Mat32F const & bounds)
{
    Bools               vertInBounds;
    vertInBounds.reserve(triSurf.verts.size());
    for (Vec3F const & v : triSurf.verts)
        vertInBounds.push_back(isInBounds(bounds,v));
    Bools               triInBounds;
    triInBounds.reserve(triSurf.tris.size());
    SurfTopo        topo {triSurf.verts.size(),triSurf.tris};
    for (SurfTopo::Tri const & t : topo.m_tris) {
        bool            inBounds = true;
        for (uint vertIdx : t.vertInds) {
            if (!vertInBounds[vertIdx])
                inBounds = false;
        }
        triInBounds.push_back(inBounds);
    }
    Arr3UIs             nTris;
    for (size_t tt=0; tt<triSurf.tris.size(); ++tt)
        if (triInBounds[tt])
            nTris.push_back(triSurf.tris[tt]);
    return removeUnused(triSurf.verts,nTris);
}


Mesh                meshFromImage(const ImgD & img)
{
    Mesh                ret;
    FGASSERT((img.height() > 1) && (img.width() > 1));
    Mat22D              imgIdxBounds(0,img.width()-1,0,img.height()-1);
    AxAffine2D          pacsToIucs {mapDiv(1.0,mapCast<double>(img.dims())),Vec2D{0}},
                        pacsToOtcs = cIucsToOtcs<double>() * pacsToIucs;
    Arr2D               domBnds = cBounds(img.dataVec());
    Affine1D            imgValToSpace(domBnds[0],domBnds[1],0,1);
    for (Iter2UI it(img.dims()); it.valid(); it.next()) {
        Vec2D               imgCrd = Vec2D(it()),
                            xy = pacsToIucs * imgCrd;
        ret.verts.push_back(Vec3F(xy[0],xy[1],imgValToSpace * img[it()]));
        ret.uvs.push_back(Vec2F(pacsToOtcs * imgCrd));
    }
    Arr4UIs                 quads,
                            texInds;
    uint                    w = img.width();
    for (Iter2UI it(img.dims()-Vec2UI(1)); it.valid(); it.next()) {
        uint                x = it()[0],
                            x1 = x + 1,
                            y = it()[1] * w,
                            y1 = y + w;
        Arr4UI           inds(x1+y,x+y,x+y1,x1+y1);      // CC winding
        quads.push_back(inds);
        texInds.push_back(inds);
    }
    ret.surfaces.emplace_back(TriInds{},QuadInds{quads,texInds});
    return ret;
}

QuadSurf            cGrid(size_t szll)
{
    FGASSERT(szll > 0);
    uint                sz = scast<uint>(szll);
    QuadSurf            ret;
    AxAffine2D          itToCoord {Mat22D(0,sz,0,sz),Mat22D{-1,1,-1,1}};
    for (Iter2UI it(sz+1); it.valid(); it.next()) {
        Vec2D           p = itToCoord * Vec2D(it());
        ret.verts.push_back(Vec3F(p[0],p[1],0));
    }
    for (Iter2UI it(sz); it.valid(); it.next()) {
        uint            i0 = it()[0],
                        i1 = it()[1]*(sz+1),
                        v0 = i0+i1,
                        v1 = i0+i1+1,
                        v2 = i0+i1+1+(sz+1),
                        v3 = i0+i1+(sz+1);
        ret.quads.push_back(Arr4UI(v0,v1,v2,v3));
    }
    return ret;
}

TriSurf             cSphere4(size_t subdivisions)
{
    FGASSERT(subdivisions < 10);    // sanity check
    TriSurf             tet = cTetrahedron();
    for (uint ss=0; ss<subdivisions; ss++) {
        tet = subdivide(tet,true);
        for (Vec3F & v : tet.verts)
            v = normalize(v);
    }
    return tet;
}

TriSurf             cSphere(size_t subdivisions)
{
    FGASSERT(subdivisions < 8);    // sanity check
    TriSurf             ico = cIcosahedron();
    for (size_t ii=0; ii<subdivisions; ++ii) {
        ico = subdivide(ico,true);
        for (Vec3F & v : ico.verts)
            v = normalize(v);
    }
    return ico;
}

TriSurf         cSquarePrism(float sideLen,float height)
{
    FGASSERT(sideLen > 0);
    float           s = sideLen * 0.5f;
    Vec3Fs          verts {
        {0,0,0},
        {s,s,0},
        {s,-s,0},
        {-s,-s,0},
        {-s,s,0},
    };
    verts = cat(verts,mapAdd(verts,Vec3F{0,0,height}));
    Arr3UIs         base {
        {0,1,2},
        {0,2,3},
        {0,3,4},
        {0,4,1},
    },
                    top {
        {5,7,6},
        {5,8,7},
        {5,9,8},
        {5,6,9},
    },
                    sides {
        {1,7,2},
        {7,1,6},
        {2,8,3},
        {8,2,7},
        {3,9,4},
        {9,3,8},
        {4,6,1},
        {6,4,9},
    };
    return {verts,cat(base,top,sides)};
}

Mesh            cSpheres(Vec3Ds const & poss,double radius,Rgba8 color)
{
    Vec3Fs              verts;
    Arr3UIs             vinds;
    Vec2Fs              uvs {{0.5f,0.5f}};
    Arr3UIs             tinds;
    for (Vec3D const & pos : poss) {
        ScaleTrans3F        st {ScaleTrans3D{radius,pos}};
        TriSurf             ts = cIcosahedron();
        cat_(vinds,mapAdd(ts.tris,Arr3UI{scast<uint>(verts.size())}));
        cat_(verts,mapMulR(st,ts.verts));
        cat_(tinds,Arr3UIs(ts.tris.size(),Arr3UI{0}));
    }
    ImgRgba8            clrMap {2,2,color};
    Material            mtrl {false,make_shared<ImgRgba8>(clrMap)};
    Surf                surf {TriInds{vinds,tinds},{},{},mtrl};
    return {verts,uvs,{surf}};
}

Mesh                removeDuplicateFacets(Mesh const & mesh)
{
    Mesh                ret = mesh;
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss) {
        ret.surfaces[ss] = removeDuplicateFacets(ret.surfaces[ss]);
    }
    return ret;
}

Mesh                removeUnused(Mesh const & mesh)
{
    Mesh            ret;
    ret.name = mesh.name;
    ret.surfaces = mesh.surfaces;
    // Which vertices & uvs are referenced by a surface or marked vertex:
    vector<bool>        vertUsed(mesh.verts.size(),false);
    vector<bool>        uvsUsed(mesh.uvs.size(),false);
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        Surf const & surf = mesh.surfaces[ss];
        for (size_t tt=0; tt<surf.tris.size(); ++tt) {
            Arr3UI   v = surf.tris.vertInds[tt];
            for (uint ii=0; ii<3; ++ii)
                vertUsed[v[ii]] = true;
        }
        for (size_t tt=0; tt<surf.quads.size(); ++tt) {
            Arr4UI   v = surf.quads.vertInds[tt];
            for (uint ii=0; ii<4; ++ii)
                vertUsed[v[ii]] = true;
        }
        for (size_t tt=0; tt<surf.tris.uvInds.size(); ++tt) {
            Arr3UI   u = surf.tris.uvInds[tt];
            for (uint ii=0; ii<3; ++ii)
                uvsUsed[u[ii]] = true;
        }
        for (size_t tt=0; tt<surf.quads.uvInds.size(); ++tt) {
            Arr4UI   u = surf.quads.uvInds[tt];
            for (uint ii=0; ii<4; ++ii)
                uvsUsed[u[ii]] = true;
        }
    }
    for (size_t ii=0; ii<mesh.markedVerts.size(); ++ii)
        vertUsed[mesh.markedVerts[ii].idx] = true;
    // Create the new vertex list:
    Uints    mapVerts(mesh.verts.size(),numeric_limits<uint>::max());
    uint            cnt = 0;
    for (size_t ii=0; ii<vertUsed.size(); ++ii) {
        if (vertUsed[ii]) {
            ret.verts.push_back(mesh.verts[ii]);
            mapVerts[ii] = cnt++;
        }
    }
    // Create the new uv list:
    Uints    mapUvs(mesh.uvs.size(),numeric_limits<uint>::max());
    cnt = 0;
    for (size_t ii=0; ii<uvsUsed.size(); ++ii) {
        if (uvsUsed[ii]) {
            ret.uvs.push_back(mesh.uvs[ii]);
            mapUvs[ii] = cnt++;
        }
    }
    // Remap the surfaces and marked verts, which we know all map to valid indices in the new list:
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss) {
        Surf &           surf = ret.surfaces[ss];
        for (size_t ii=0; ii<surf.tris.size(); ++ii)
            for (uint jj=0; jj<3; ++jj)
                surf.tris.vertInds[ii][jj] = mapVerts[surf.tris.vertInds[ii][jj]];
        for (size_t ii=0; ii<surf.quads.size(); ++ii)
            for (uint jj=0; jj<4; ++jj)
                surf.quads.vertInds[ii][jj] = mapVerts[surf.quads.vertInds[ii][jj]];
        for (size_t ii=0; ii<surf.tris.uvInds.size(); ++ii)
            for (uint jj=0; jj<3; ++jj)
                surf.tris.uvInds[ii][jj] = mapUvs[surf.tris.uvInds[ii][jj]];
        for (size_t ii=0; ii<surf.quads.uvInds.size(); ++ii)
            for (uint jj=0; jj<4; ++jj)
                surf.quads.uvInds[ii][jj] = mapUvs[surf.quads.uvInds[ii][jj]];
    }
    ret.markedVerts = mesh.markedVerts;
    for (size_t ii=0; ii<ret.markedVerts.size(); ++ii)
        ret.markedVerts[ii].idx = mapVerts[ret.markedVerts[ii].idx];
    // Remap only those delta morphs which contain non-zero deltas:
    Vec3F            zero(0);
    for (size_t ii=0; ii<mesh.deltaMorphs.size(); ++ii) {
        DirectMorph const & src = mesh.deltaMorphs[ii];
        DirectMorph         dst;
        dst.name = src.name;
        bool            keep = false;
        for (size_t jj=0; jj<src.verts.size(); ++jj) {
            if (mapVerts[jj] != numeric_limits<uint>::max()) {
                dst.verts.push_back(src.verts[jj]);
                if (src.verts[jj] != zero)
                    keep = true;
            }
        }
        if (keep)
            ret.deltaMorphs.push_back(dst);
    }
    // Remap only those target morphs that reference retained vertices:
    for (size_t ii=0; ii<mesh.targetMorphs.size(); ++ii) {
        const IndexedMorph &      src = mesh.targetMorphs[ii];
        IndexedMorph              dst;
        dst.name = src.name;
        for (size_t jj=0; jj<src.ivs.size(); ++jj) {
            uint        idx = mapVerts[src.ivs[jj].idx];
            if (idx != numeric_limits<uint>::max())
                dst.ivs.emplace_back(idx,src.ivs[jj].vec);
        }
        if (!dst.ivs.empty())
            ret.targetMorphs.push_back(dst);
    }
    return  ret;
}

// returns a list with an element for each entry in 'keepFlags' which is:
//   false: uint::max()
//   true: the index in the new list containing only the kept values
Uints               flagsToIndexMap(Bools const & keepFlags)
{
    Uints               ret; ret.reserve(keepFlags.size());
    uint                cnt {0};
    for (bool flag : keepFlags) {
        if (flag)
            ret.push_back(cnt++);
        else
            ret.push_back(lims<uint>::max());
    }
    return ret;
}

template<size_t N>
Svec<Arr<uint,N>> removeVerts(
    Svec<Arr<uint,N>> const & polys,
    Bools const &           polyKeepFlags,
    Uints const &           vertIndsMap)
{
    size_t                  P = polys.size();
    FGASSERT(polyKeepFlags.size() == P);
    Svec<Arr<uint,N>>     ret;
    for (size_t pp=0; pp<P; ++pp) {
        if (polyKeepFlags[pp]) {
            Arr<uint,N>       in = polys[pp],
                              out {mapIndex(in,vertIndsMap)};
            ret.push_back(out);
        }
    }
    return ret;
}

template<size_t N>
NPolys<N>           removeVerts(
    NPolys<N> const &   in,
    Bools const &       polyKeepFlags,
    Uints const &       vertIndsMap)
{
    NPolys<N>           ret;
    ret.vertInds = removeVerts(in.vertInds,polyKeepFlags,vertIndsMap);
    if (!in.uvInds.empty())
        ret.uvInds = selectIf(in.uvInds,polyKeepFlags);       // UV list unchanged
    return ret;
}

Surf                removeVerts(
    Surf const &        orig,
    Uints const &       vertIndsMap)    // 1-1 with vert list, maps to new vert list, or lims<uint>::max() for removed
{
    Surf                ret;
    ret.name = orig.name;
    size_t              T = orig.tris.vertInds.size(),
                        Q = orig.quads.vertInds.size();
    Bools               keepTris (T,true);
    for (size_t tt=0; tt<T; ++tt) {
        Arr3UI              tri = orig.tris.vertInds[tt];
        for (uint idx : tri)
            if (vertIndsMap[idx] == lims<uint>::max())          // uses a removed vert
                keepTris[tt] = false;
    }
    ret.tris = removeVerts(orig.tris,keepTris,vertIndsMap);
    Bools               keepQuads (Q,true),
                        keepTriEquivs = cat(keepTris,Bools(Q*2,true));
    for (size_t qq=0; qq<Q; ++qq) {
        Arr4UI              quad = orig.quads.vertInds[qq];
        for (uint idx : quad) {
            if (vertIndsMap[idx] == lims<uint>::max()) {        // uses a removed vert
                keepQuads[qq] = false;
                keepTriEquivs[T+qq*2] = false;
                keepTriEquivs[T+qq*2+1] = false;
            }
        }
    }
    ret.quads = removeVerts(orig.quads,keepQuads,vertIndsMap);
    Uints               triEquivIndsMap = flagsToIndexMap(keepTriEquivs);
    for (SurfPointName const & sp : orig.surfPoints) {
        if (keepTriEquivs[sp.point.triEquivIdx]) {
            SurfPoint           bp {
                triEquivIndsMap[sp.point.triEquivIdx],
                sp.point.weights
            };
            ret.surfPoints.emplace_back(bp,sp.label);
        }
    }
    ret.material = orig.material;
    return ret;
}

Mesh            removeVerts(Mesh const & orig,Uints const & vertInds)
{
    size_t              V = orig.verts.size();
    Bools               vertKeepFlags (V,true);
    for (uint idx : vertInds) {
        FGASSERT(idx < V);
        vertKeepFlags[idx] = false;
    }
    Mesh                ret;
    ret.name = orig.name;
    ret.verts = selectIf(orig.verts,vertKeepFlags);
    ret.uvs = orig.uvs;
    Uints               vertIndsMap = flagsToIndexMap(vertKeepFlags);
    ret.surfaces.reserve(orig.surfaces.size());
    for (Surf const & surf : orig.surfaces)
        ret.surfaces.push_back(removeVerts(surf,vertIndsMap));
    for (DirectMorph const & dm : orig.deltaMorphs) {
        Vec3Fs          verts;
        for (size_t vv=0; vv<V; ++vv)
            if (vertKeepFlags[vv])
                verts.push_back(dm.verts[vertIndsMap[vv]]);
        if (!verts.empty())
            ret.deltaMorphs.emplace_back(dm.name,verts);
    }
    for (IndexedMorph const & im : orig.targetMorphs) {
        IdxVec3Fs           ivs;
        for (IdxVec3F const & iv : im.ivs)
            if (vertKeepFlags[iv.idx])
                ivs.emplace_back(vertIndsMap[iv.idx],iv.vec);
        if (!ivs.empty())
            ret.targetMorphs.emplace_back(im.name,ivs);
    }
    for (MarkedVert const & mv : orig.markedVerts)
        if (vertKeepFlags[mv.idx])
            ret.markedVerts.emplace_back(vertIndsMap[mv.idx],mv.label);
    return ret;
}

TriSurf             cTetrahedron(bool open)
{
    // Coordinates of a regular tetrahedron with edges of length 2*sqrt(2):
    Vec3Fs             verts {
        { 1, 1, 1},
        {-1,-1, 1},
        {-1, 1,-1},
        { 1,-1,-1},
    };
    Arr3UIs             tris {
        {0,1,3},
        {0,2,1},
        {2,0,3},
    };
    if (!open)
        tris.emplace_back(1,2,3);
    return TriSurf {verts,tris};
}

TriSurf             cPyramid(bool open)
{
    Vec3Fs         verts {
        {-1, 0,-1},
        { 1, 0,-1},
        {-1, 0, 1},
        { 1, 0, 1},
        { 0, 1, 0},
    };
    Arr3UIs         tris {
        {0, 4, 1},
        {0, 2, 4},
        {2, 3, 4},
        {1, 4, 3},
    };
    if (!open) {
        tris.emplace_back(0,1,3);
        tris.emplace_back(3,2,0);
    }
    return {verts,tris};
}

TriSurf             cCubeTris(bool open)
{
    Vec3Fs             verts;
    for (uint vv=0; vv<8; ++vv)
        verts.push_back(
            Vec3F(
                float(vv & 0x01),
                float((vv >> 1) & 0x01),
                float((vv >> 2) & 0x01)) * 2.0f -
            Vec3F(1.0f));
    Arr3UIs             tris {
    // X planes:
        {0,4,6},
        {6,2,0},
        {5,1,3},
        {3,7,5},
    // Z planes:
        {1,0,2},
        {2,3,1},
        {4,5,7},
        {7,6,4},
    // Y planes:
        {0,1,5},
        {5,4,0},
    };
    if (!open) {
        tris.emplace_back(3,2,6);
        tris.emplace_back(6,7,3);
    }
    return {verts,tris};
}

TriSurf             cOctahedron()
{
    // Visualize as a diamond with opposite vertices axially aligned:
    return TriSurf {
    {
        {-1, 0, 0},
        { 1, 0, 0},
        { 0,-1, 0},
        { 0, 1, 0},
        { 0, 0,-1},
        { 0, 0, 1},
    },
    {
        {0,2,5},
        {0,3,4},
        {0,4,2},
        {0,5,3},
        {1,2,4},
        {1,3,5},
        {1,4,3},
        {1,5,2},
    },
    };
}

TriSurf             cIcosahedron()
{
    // Data copied from github cginternals:
    float const         t = 0.5f * (1.0f + sqrt(5.0f)),
                        i = 1.0f / sqrt(sqr(t) + 1.0f),
                        a = t * i;
    return TriSurf {
        {
            // sqr(i) + sqr(a) = 1
            {-i, a, 0},
            { i, a, 0},
            {-i,-a, 0},
            { i,-a, 0},
            { 0,-i, a},
            { 0, i, a},
            { 0,-i,-a},
            { 0, i,-a},
            { a, 0,-i},
            { a, 0, i},
            {-a, 0,-i},
            {-a, 0, i},
        },
        {
            { 0,11, 5},
            { 0, 5, 1},
            { 0, 1, 7},
            { 0, 7,10},
            { 0,10,11},
            { 1, 5, 9},
            { 5,11, 4},
            {11,10, 2},
            {10, 7, 6},
            { 7, 1, 8},
            { 3, 9, 4},
            { 3, 4, 2},
            { 3, 2, 6},
            { 3, 6, 8},
            { 3, 8, 9},
            { 4, 9, 5},
            { 2, 4,11},
            { 6, 2,10},
            { 8, 6, 7},
            { 9, 8, 1},
        },
    };
}

TriSurf             cIcosahedron(float scale,Vec3F const & centre)
{
    TriSurf         ret;
    for (Vec3F & v : ret.verts)
        v = v * scale + centre;
    return ret;
}

TriSurf             cNTent(uint nn)
{
    FGASSERT(nn > 2);
    Vec3Fs              verts {{0,1,0}};
    float               step = 2.0f * float(pi) / float(nn);
    for (uint ii=0; ii<nn; ++ii) {
        float               angle = step * float(ii);
        verts.emplace_back(cos(angle),0.0f,-sin(angle));
    }
    Arr3UIs             tris;
    for (uint ii=0; ii<nn; ++ii)
        tris.emplace_back(0,ii+1,((ii+1)%nn+1));    // first vert is centre so must offset that
    return {verts,tris};
}

TriSurf             cTube()
{
    float               i = 1,          // inner radius
                        o = 2,          // outer radius
                        h = 4;          // half height
    Vec2Fs              crossSection {
        {i,i},{-i,i},{-i,-i},{i,-i},    // inner
        {o,o},{-o,o},{-o,-o},{o,-o},    // outer
    };
    auto                cLayer = [&](float z)
    {
        auto                fn = [z](Vec2F p){return Vec3F{p[0],p[1],z}; };
        return mapCall(crossSection,fn);
    };
    Vec3Fs              verts = cat(cLayer(-h),cLayer(0),cLayer(h));
    uint                v = scast<uint>(crossSection.size());
    Arr4UIs             quads {{0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7},},   // cap
                        innerLo {{0,v,1+v,1},{1,1+v,2+v,2},{2,2+v,3+v,3},{3,3+v,v,0},},
                        outerLo {{4+v,4,5,5+v},{5+v,5,6,6+v},{6+v,6,7,7+v},{7+v,7,4,4+v},};
    cat_(quads,innerLo);
    cat_(quads,outerLo);
    cat_(quads,mapAdd(innerLo,Arr4UI{v}));          // innerHi
    cat_(quads,mapAdd(outerLo,Arr4UI{v}));          // outerHi
    return subdivide({verts,asTris(quads)});
}

Mesh                fuseIdenticalVerts(Mesh const & mesh)
{
    Mesh            ret;
    ret.name = mesh.name;
    ret.uvs = mesh.uvs;
    ret.joints = mesh.joints;
    ret.jointDofs = mesh.jointDofs;
    Uints           map; map.reserve(mesh.verts.size());
    for (Vec3F const & vert : mesh.verts) {
        size_t          idx = findFirstIdx(ret.verts,vert);
        if (idx < ret.verts.size())         // duplicate found
            map.push_back(scast<uint>(idx));
        else {
            map.push_back(scast<uint>(ret.verts.size()));
            ret.verts.push_back(vert);
        }
    }
    for (Surf surf : mesh.surfaces) {
        for (Arr3UI & tri : surf.tris.vertInds)
            for (uint & idx : tri)
                idx = map[idx];
        for (Arr4UI & quad : surf.quads.vertInds)
            for (uint & idx : quad)
                idx = map[idx];
        ret.surfaces.push_back(surf);
    }
    return ret;
}

Mesh                fuseIdenticalUvs(Mesh const & in)
{
    Mesh            ret = in;
    ret.uvs.clear();
    Uints           map; map.reserve(in.uvs.size());
    for (Vec2F const & uv : in.uvs) {
        size_t          idx = findFirstIdx(ret.uvs,uv);
        if (idx < ret.uvs.size())         // duplicate found
            map.push_back(scast<uint>(idx));
        else {
            map.push_back(scast<uint>(ret.uvs.size()));
            ret.uvs.push_back(uv);
        }
    }
    for (Surf & surf : ret.surfaces) {
        for (Arr3UI & tri : surf.tris.uvInds)
            for (uint & idx : tri)
                idx = map[idx];
        for (Arr4UI & quad : surf.quads.uvInds)
            for (uint & idx : quad)
                idx = map[idx];
    }
    return ret;
}

Mesh                splitSurfsContiguousUvs(Mesh mesh)
{
    mesh.surfaces = splitSurfContiguousUvs(merge(mesh.surfaces));
    return mesh;
}

Mesh                selectSurfs(Mesh const & mesh,Strings const & surfNames)
{
    Mesh                ret = mesh;
    Surfs               surfs; surfs.reserve(surfNames.size());
    for (String const & name : surfNames)
        surfs.push_back(findFirst(mesh.surfaces,name));
    ret.surfaces = surfs;
    return ret;
}

Mesh                mergeMeshes(Ptrs<Mesh> meshPtrs)
{
    Mesh                ret;
    Ptrs<String8>       namePtrs = mapPtrsMember(meshPtrs,&Mesh::name);
    Ptrs<String8>       neNamePtrs = select(namePtrs,[](String8 const * n){return !n->empty(); });
    ret.name = catDeref(neNamePtrs,"+");
    Ptrs<Vec3Fs>        vertsPtrs = mapPtrsMember(meshPtrs,&Mesh::verts);
    Sizes               voffsets = integrate(cSizes(vertsPtrs));
    ret.verts = catDeref(vertsPtrs);
    Ptrs<Vec2Fs>        uvsPtrs = mapPtrsMember(meshPtrs,&Mesh::uvs);
    Sizes               uoffsets = integrate(cSizes(uvsPtrs));
    ret.uvs = catDeref(uvsPtrs);
    Ptrs<Surfs>         surfsPtrs = mapPtrsMember(meshPtrs,&Mesh::surfaces);
    Sizes               soffsets = integrate(cSizes(surfsPtrs));
    ret.surfaces.reserve(soffsets.back());
    map<String8,IdxVec3Fs>  deltaMorphs,
                            targMorphs;
    for (size_t mm=0; mm<meshPtrs.size(); ++mm) {
        Mesh const &        mesh = *meshPtrs[mm];
        uint                voffset = scast<uint>(voffsets[mm]),
                            uoffset = scast<uint>(uoffsets[mm]);
        for (Surf const & surf : mesh.surfaces)
            ret.surfaces.emplace_back(
                surf.name,
                offsetIndices(surf.tris,voffset,uoffset),
                offsetIndices(surf.quads,voffset,uoffset),
                surf.surfPoints,
                surf.material
            );
        for (DirectMorph const & dm : mesh.deltaMorphs) {
            IdxVec3Fs           ivs = toIndexedDeltaMorph(dm.verts);
            auto                it = deltaMorphs.find(dm.name);
            if (it == deltaMorphs.end())
                deltaMorphs[dm.name] = ivs;
            else
                cat_(it->second,offsetIndices(ivs,scast<uint>(voffsets[mm])));
        }
        for (IndexedMorph const & m : mesh.targetMorphs) {
            auto                it = targMorphs.find(m.name);
            if (it == targMorphs.end())
                targMorphs[m.name] = m.ivs;
            else
                cat_(it->second,offsetIndices(m.ivs,scast<uint>(voffsets[mm])));
        }
        for (MarkedVert const & mv : mesh.markedVerts)
            ret.markedVerts.emplace_back(mv.idx + voffset,mv.label);
    }
    ret.deltaMorphs.reserve(deltaMorphs.size());
    for (auto const & dm : deltaMorphs)
        ret.deltaMorphs.emplace_back(dm.first,toDirectDeltaMorph(dm.second,ret.verts.size()));
    ret.targetMorphs.reserve(targMorphs.size());
    for (auto const & tm : targMorphs)
        ret.targetMorphs.emplace_back(tm.first,tm.second);
    return ret;
}

Mesh                fg3dMaskFromUvs(Mesh const & mesh,const Img<FatBool> & mask)
{
    // Make a list of which vertices have UVs that only fall in the excluded regions:
    vector<FatBool>     keep(mesh.verts.size(),false);
    AxAffine2F          otcsToPacs = cOtcsToPacs<float>(mask.dims());
    Mat22UI             clampVal(0,mask.width()-1,0,mask.height()-1);
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        Surf const & surf = mesh.surfaces[ii];
        if (!surf.quads.uvInds.empty())
            fgThrow("Quads not supported");
        if (surf.tris.uvInds.empty())
            fgThrow("No tri facet UVs");
        for (size_t jj=0; jj<surf.tris.uvInds.size(); ++jj) {
            Arr3UI   uvInd = surf.tris.uvInds[jj];
            Arr3UI   vtInd = surf.tris.vertInds[jj];
            for (uint kk=0; kk<3; ++kk) {
                bool    valid = mask[mapClamp(Vec2UI(otcsToPacs * mesh.uvs[uvInd[kk]]),clampVal)];
                keep[vtInd[kk]] = keep[vtInd[kk]] || valid;
            }
        }
    }
    // Remove the facets that use those vertices:
    Surfs nsurfs;
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        Surf const & surf = mesh.surfaces[ii];
        Surf         nsurf;
        for (size_t jj=0; jj<surf.tris.uvInds.size(); ++jj) {
            Arr3UI   vtInd = surf.tris.vertInds[jj];
            bool copy = false;
            for (uint kk=0; kk<3; ++kk)
                copy = copy || keep[vtInd[kk]];
            if (copy)
                nsurf.tris.vertInds.push_back(vtInd);
        }
        nsurfs.push_back(nsurf);
    }
    // Remove unused vertices:
    return removeUnused(Mesh(mesh.verts,nsurfs));
}

ImgV3F          pixelsToPositions(
    Vec3Fs const &      verts,
    Arr3UIs const &     vtIndss,
    Vec2Fs const &      uvs,
    Arr3UIs const &     uvIndss,
    Vec2UI              dims)
{
    float constexpr     invalid {lims<float>::max()};
    GridTriangles       grid {uvs,uvIndss};
    ImgV3F              ret {dims,{invalid,invalid,invalid}};
    AxAffine2F          ircsToOtcs {cIrcsToOtcs<float>(dims)};
    size_t              overlaps {0},
                        unused {0};
    for (Iter2UI it{dims}; it.valid(); it.next()) {
        Vec2F               otcs = ircsToOtcs * Vec2F{it()};
        TriPoints           tps = grid.intersects(otcs);
        if (tps.empty())
            ++unused;
        else {
            if (tps.size() > 1)
                ++overlaps;
            TriPoint const &    tp = tps[0];
            Arr3UI              vtInds = vtIndss[tp.triInd];
            ret[it()] = indexInterp(vtInds,tp.baryCoord,verts);
        }
    }
    double              numPix = Vec2D{dims}.elemsProduct();
    fgout << fgnl
        << "UV coverage: " << toStrPercent(1.0 - unused / numPix)
        << " UV overlaps: " << toStrPercent(overlaps /  numPix);
    return ret;
}

ImgUC               getUvCover(Mesh const & mesh,Vec2UI dims)
{
    ImgUC               ret {dims,uchar{0}};
    GridTriangles       grid {mesh.uvs,mesh.getTriEquivs().uvInds};
    AxAffine2F          ircsToOtcs {cIrcsToOtcs<float>(dims)};
    for (Iter2UI it{dims}; it.valid(); it.next())
        if (!grid.intersects(ircsToOtcs * Vec2F{it()}).empty())
            ret[it()] = uchar(255);
    return ret;
}

ImgRgba8s           cUvWireframeImages(Mesh const & mesh,Rgba8 clr)
{
    ImgRgba8s           ret; ret.reserve(mesh.surfaces.size());
    for (Surf const & surf : mesh.surfaces)
        ret.push_back(cUvWireframeImage(mesh.uvs,surf.tris.uvInds,surf.quads.uvInds,clr,clr));
    return ret;
}

Vec3Fs              embossMesh(Mesh const & mesh,const ImgUC & logoImg,double val)
{
    FGASSERT(!mesh.uvs.empty());
    Vec3Fs              ret;
    // Don't check for UV seams, just let the emboss value be the last one traversed:
    Vec3Fs              deltas(mesh.verts.size());
    Sizes               embossedVertInds;
    SurfNormals         norms = cNormals(mesh);
    for (Surf const & surf : mesh.surfaces) {
        for (size_t ii=0; ii<surf.tris.uvInds.size(); ++ii) {
            Arr3UI              uvInds = surf.tris.uvInds[ii],
                                vtInds = surf.tris.vertInds[ii];
            for (uint jj=0; jj<3; ++jj) {
                Vec2F               uv = mesh.uvs[uvInds[jj]];
                uv[1] = 1.0f - uv[1];       // Convert from OTCS to IUCS
                float               imgSamp = sampleClampIucs(logoImg,uv) / 255.0f;
                uint                vtIdx = vtInds[jj];
                deltas[vtIdx] = norms.vert[vtIdx] * imgSamp;
                if (imgSamp > 0)
                    embossedVertInds.push_back(vtIdx);
            }
        }
        for (size_t ii=0; ii<surf.quads.uvInds.size(); ++ii) {
            Arr4UI              uvInds = surf.quads.uvInds[ii],
                                vtInds = surf.quads.vertInds[ii];
            for (uint jj=0; jj<4; ++jj) {
                Vec2F               uv = mesh.uvs[uvInds[jj]];
                uv[1] = 1.0f - uv[1];       // Convert from OTCS to IUCS
                float               imgSamp = sampleClampIucs(logoImg,uv) / 255.0f;
                uint                vtIdx = vtInds[jj];
                deltas[vtIdx] = norms.vert[vtIdx] * imgSamp;
                if (imgSamp > 0)
                    embossedVertInds.push_back(vtIdx);
            }
        }
    }
    float               fac = cMaxElem(cDims(mapIndex(embossedVertInds,mesh.verts))) * val;
    ret.resize(mesh.verts.size());
    for (size_t ii=0; ii<deltas.size(); ++ii)
        ret[ii] = mesh.verts[ii] + deltas[ii] * fac;
    return ret;
}

Vec3Fs              cMirrorX(Vec3Fs const & verts)
{
    Vec3Fs              ret; ret.reserve(verts.size());
    for (Vec3F const & v : verts)
        ret.emplace_back(-v[0],v[1],v[2]);
    return ret;
}

static void         mirrorLabel_(String & l)
{
    if (l.back() == 'L')
        l.back() = 'R';
    else if (l.back() == 'R')
        l.back() = 'L';
}
static String       mirrorLabel(String lab)
{
    mirrorLabel_(lab);
    return lab;
}

Surf                cMirrorX(Surf const & surf)
{
    Surf                ret = reverseWinding(surf);
    for (SurfPointName & sp : ret.surfPoints)
        mirrorLabel_(sp.label);
    return ret;
}

Mesh                cMirrorX(Mesh const & m)
{
    Mesh                ret;
    ret.verts = cMirrorX(m.verts);
    ret.uvs = mapCall(m.uvs,[](Vec2F uv){return Vec2F{1-uv[0],uv[1]}; });
    ret.surfaces = mapCall(m.surfaces,[](Surf const & s){return cMirrorX(s);});
    ret.markedVerts = m.markedVerts;
    for (MarkedVert & mv : ret.markedVerts)
        mirrorLabel_(mv.label);
    return ret;
}

NameVec3Fs          mirrorXFuse(NameVec3Fs const & lvs)
{
    NameVec3Fs          ret;
    for (NameVec3F lv : lvs) {
        if (lv.name.back() == 'L') {
            ret.push_back(lv);
            lv.vec[0] *= -1;
            lv.name.back() = 'R';
            ret.push_back(lv);
        }
        else {
            lv.vec[0] = 0;
            ret.push_back(lv);
        }
    }
    return ret;
}

Mesh                mirrorXFuse(Mesh const & in)
{
    size_t              V = in.verts.size();
    Vec3Fs              verts = in.verts;   // new verts list
    Uints               mirrorInds(V);      // map each 'verts' idx to its mirror idx (which is itself if X=0)
    for (size_t ii=0; ii<V; ++ii) {
        Vec3F               pos = in.verts[ii];
        if (pos[0] == 0)
            mirrorInds[ii] = uint(ii);      // maps to itself
        else {
            mirrorInds[ii] = uint(verts.size());
            pos[0] *= -1;
            verts.push_back(pos);
            mirrorInds.push_back(uint(ii));
        }
    }
    size_t              U = in.uvs.size();
    Vec2Fs              uvs = in.uvs;           // new uvs list
    Uints               mirrorUvs(U);           // map each uv idx to its mirror idx
    for (size_t ii=0; ii<U; ++ii) {
        Vec2F               pos = in.uvs[ii];
        if (pos[0] == 0.5)
            mirrorUvs[ii] = uint(ii);           // maps to itself
        else {
            mirrorUvs[ii] = uint(uvs.size());
            pos[0] = 1 - pos[0];
            uvs.push_back(pos);
            mirrorUvs.push_back(uint(ii));
        }
    }
    Surfs               surfs; surfs.reserve(in.surfaces.size());
    for (Surf const & is : in.surfaces) {
        Surf                surf;
        surf.name = is.name;
        surf.tris.vertInds = cat(is.tris.vertInds,reverseWinding(remapInds(is.tris.vertInds,mirrorInds)));
        surf.tris.uvInds = cat(is.tris.uvInds,reverseWinding(remapInds(is.tris.uvInds,mirrorUvs)));
        surf.quads.vertInds = cat(is.quads.vertInds,reverseWinding(remapInds(is.quads.vertInds,mirrorInds)));
        surf.quads.uvInds = cat(is.quads.uvInds,reverseWinding(remapInds(is.quads.uvInds,mirrorUvs)));
        for (SurfPointName const & sp : is.surfPoints) {
            if (contains(Arr<char,2>{'L','R'},sp.label.back())) {                   // mirror this SP
                SurfPoint           bp = reverseWinding(sp.point,is.tris.size());
                if (sp.point.triEquivIdx < is.tris.size())                            // shift index
                    bp.triEquivIdx += scast<uint>(is.tris.size());
                else
                    bp.triEquivIdx += scast<uint>(is.numTriEquivs());
                surf.surfPoints.push_back(sp);
                surf.surfPoints.emplace_back(bp,mirrorLabel(sp.label));
            }
            else                                                // project this SP to saggital plane
            {
                Arr3F           bary = sp.point.weights;
                Arr3UI          vertInds = surf.getTriEquivVertInds(sp.point.triEquivIdx);
                for (size_t ii=0; ii<3; ++ii) {
                    uint            idx = vertInds[ii];
                    if (mirrorInds[idx] != idx)                 // non-saggital mirrored vert
                        bary[ii] = 0;
                }
                float           sum = cSum(bary);
                if (sum <= 0) {     // no vertices of this tri are on saggital plane or point is off-facet
                    fgout << fgnl << "WARNING: unable to project " << sp.label << " onto saggital plane";
                    surf.surfPoints.push_back(sp);
                }
                else {
                    SurfPoint           spProj {sp.point.triEquivIdx,bary*(1/sum)};
                    surf.surfPoints.emplace_back(spProj,sp.label);
                }
            }
        }
        surfs.push_back(surf);
    }
    DirectMorphs        dmorphs; dmorphs.reserve(in.deltaMorphs.size());
    for (DirectMorph const & morphIn : in.deltaMorphs) {
        DirectMorph         morph = morphIn;
        for (size_t vv=0; vv<V; ++vv) {
            uint                idxM = mirrorInds[vv];
            if (idxM != vv)                     // non-saggital mirrored vert
                morph.verts.push_back(cMirrorX(morphIn.verts[vv]));
        }
        dmorphs.push_back(morph);
    }
    IndexedMorphs       tmorphs; tmorphs.reserve(in.targetMorphs.size());
    for (IndexedMorph const & morphIn : in.targetMorphs) {
        IndexedMorph        imorph = morphIn;
        for (IdxVec3F iv : morphIn.ivs) {
            uint                idxM = mirrorInds[iv.idx];
            if (idxM != iv.idx)                 // non-saggital mirrored vert
                imorph.ivs.emplace_back(idxM,cMirrorX(iv.vec));
        }
        tmorphs.push_back(imorph);
    }
    return Mesh {verts,uvs,surfs,dmorphs,tmorphs};
}

Mesh                copySurfAssignment(Mesh const & from,Mesh const & to)
{
    Mesh            ret(to);
    ret.surfaces = {merge(ret.surfaces)};
    Surf            surf = ret.surfaces[0];
    if (!surf.quads.vertInds.empty())
        fgThrow("Quads not supported");
    Arr3UIs const & tris = surf.tris.vertInds;
    ret.surfaces.clear();
    ret.surfaces.resize(from.surfaces.size());
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss)
        ret.surfaces[ss].name = from.surfaces[ss].name;
    for (size_t ii=0; ii<tris.size(); ++ii) {
        Arr3UI   inds = tris[ii];
        Vec3F    tpos = (ret.verts[inds[0]] + ret.verts[inds[1]] + ret.verts[inds[2]]) / 3;
        Min<float,size_t>     minSurf;
        for (size_t ss=0; ss<from.surfaces.size(); ++ss) {
            Surf const &     fs = from.surfaces[ss];
            for (size_t jj=0; jj<fs.tris.size(); ++jj) {
                Arr3UI   fi = fs.tris.vertInds[jj];
                Vec3F    fpos = (from.verts[fi[0]] + from.verts[fi[1]] + from.verts[fi[2]]) / 3;
                float       mag = (fpos-tpos).magD();
                minSurf.update(mag,ss);
            }
        }
        ret.surfaces[minSurf.metric].tris.vertInds.push_back(inds);
    }
    return ret;
}

Arr3UIs             meshSurfacesAsTris(Mesh const & m)
{
    Arr3UIs   ret;
    for (size_t ss=0; ss<m.surfaces.size(); ++ss)
        cat_(ret,m.surfaces[ss].convertToTris().tris.vertInds);
    return ret;
}

void                testMeshImageMapRend(CLArgs const & args)
{
    Vec3Fs              verts {{0,0,0},{1,0,0},{1,1,0},{0,1,0}};
    Vec2Fs              uvs {{0,0},{1,0},{1,1},{0,1},};
    Arr4UI              quad {0,1,2,3};
    Surf                surf {{},QuadInds{{quad},{quad}}};
    Mesh                mesh {verts,uvs,{surf}};
    loadImage_(dataDir()+"base/test/TextureMapOrdering.jpg",mesh.surfaces[0].albedoMapRef());
    if (!isAutomated(args))
        viewMesh(mesh);
}

void                testSubdFace(CLArgs const & args)
{
    Mesh            base = loadMeshMaps(dataDir()+"base/Jane");
    base.surfaces[0] = base.surfaces[0].convertToTris();
    Mesh            subd = subdivide(base,true);
    if (!isAutomated(args))
        viewMesh({base,subd},true);
}

void                testSubdShapes(CLArgs const & args)
{
    Meshes          meshes;
    auto            addSubdivisions = [&](TriSurf const & ts,String const & name)
    {
        Mesh            mesh {name,ts};
        meshes.push_back(mesh);
        for (uint ii=0; ii<5; ++ii) {
            mesh = subdivide(mesh);
            mesh.name = name+toStr(ii+1);
            meshes.push_back(mesh);
        }
    };
    addSubdivisions(cTetrahedron(),"Tetrahedron");
    addSubdivisions(cTetrahedron(true),"TetrahedronOpen");
    addSubdivisions(cPyramid(),"Pyramid");
    addSubdivisions(cPyramid(true),"PyramidOpen");
    addSubdivisions(cCubeTris(),"Cube");
    addSubdivisions(cCubeTris(true),"CubeOpen");
    addSubdivisions(cOctahedron(),"Octahedron");
    addSubdivisions(cIcosahedron(),"Icosahedron");
    addSubdivisions(cNTent(5),"5-Tent");
    addSubdivisions(cNTent(6),"6-Tent");
    if (!isAutomated(args))
        viewMesh(meshes,true);
}

void                testSphere4(CLArgs const & args)
{
    Meshes          meshes;
    for (size_t ii=0; ii<5; ++ii)
        meshes.emplace_back("TetrahedronSphere",cSphere4(ii));
    if (!isAutomated(args))
        viewMesh(meshes,true);
}

void                testSphere(CLArgs const & args)
{
    Meshes          meshes;
    for (size_t ii=0; ii<4; ++ii)
        meshes.emplace_back("Sphere",cSphere(ii));
    if (!isAutomated(args))
        viewMesh(meshes,true);
}

void                testSquarePrism(CLArgs const & args)
{
    Mesh                mesh {"SquarePrism",cSquarePrism(1,4)};
    if (!isAutomated(args))
        viewMesh(mesh);
}

void                testTube(CLArgs const & args)
{
    Mesh                mesh {"Tube",cTube()};
    if (!isAutomated(args))
        viewMesh(mesh);
}

void                testInterp(CLArgs const & args)
{
    // interpolate colors at every discrete step between two pixel centres
    ImgRgba8            clrMap {2,1,{{0,255,0,255},{255,0,0,255}}};
    Vec3Fs              verts {{0,0,0},{0,32,0}};
    Vec2Fs              uvs;
    Arr3UIs             vinds;
    Arr3UIs             tinds;
    for (uint ii=0; ii<256; ++ii) {
        verts.emplace_back(ii+1,0,0);
        verts.emplace_back(ii+1,32,0);
        uint                i2 = 2*ii;
        vinds.emplace_back(i2,i2+2,i2+1);
        vinds.emplace_back(i2+1,i2+2,i2+3);
        uvs.emplace_back(ii/255.0f,0.5f);
        tinds.emplace_back(ii);
        tinds.emplace_back(ii);
    }
    Surf                surf {TriInds{vinds,tinds},{}};
    surf.setAlbedoMap(clrMap);
    Mesh                mesh {"Interpolated color gradient",verts,uvs,{surf}};
    if (!isAutomated(args))
        viewMesh(mesh);
}

static void         testRemoveVerts(CLArgs const &)
{
    {   // all tris
        Vec3Fs              origVerts {
            {0,0,0},
            {0,2,0},
            {1,3,0},
            {2,2,0},
            {2,0,0},
        };
        Arr3UIs             origTris {
            {0,1,3},
            {1,2,3},
            {3,4,0},
        };
        Mesh                orig {origVerts,Surf{origTris}},
                            test = removeVerts(orig,{2});
        Vec3Fs              refVerts {
            {0,0,0},
            {0,2,0},
            {2,2,0},
            {2,0,0},
        };
        Arr3UIs             refTris {
            {0,1,2},
            {2,3,0},
        };
        Mesh                ref {refVerts,Surf{refTris}};
        FGASSERT(test.verts == ref.verts);
        FGASSERT(test.surfaces[0].tris == ref.surfaces[0].tris);
    }
    {   // quad and tris
        Vec3Fs              origVerts {
            {0,0,0},
            {0,2,0},
            {1,3,0},
            {2,2,0},
            {3,1,0},
            {2,0,0},
        };
        Arr4UIs             origQuads {
            {0,1,3,5},
        };
        Arr3UIs             origTris {
            {1,2,3},
            {3,4,5},
        };
        Mesh                orig {origVerts,Surf{origTris}},
                            test = removeVerts(orig,{2});
        Vec3Fs              refVerts {
            {0,0,0},
            {0,2,0},
            {2,2,0},
            {3,1,0},
            {2,0,0},
        };
        Arr4UIs             refQuads {
            {0,1,2,4},
        };
        Arr3UIs             refTris {
            {2,3,4},
        };
        Mesh                ref {refVerts,Surf{refTris}};
        FGASSERT(test.verts == ref.verts);
        FGASSERT(test.surfaces[0].tris == ref.surfaces[0].tris);
    }
}

void                test3dMeshIo(CLArgs const &);

void                testMesh(CLArgs const & args)
{
    Cmds            cmds {
        {test3dMeshIo, "io", "3D mesh I/O"},
        {testInterp,"interp","interpolation of color maps"},
        {testSquarePrism,"prism","Square prism"},
        {testSubdShapes,"subd0","Loop subdivsion of simple shapes"},
        {testSubdFace,"subd1","Loop subdivision of textured face"},
        {testSphere4,"sphere4","Spheres created from tetrahedon"},
        {testSphere,"sphere","Spheres created from icosahedron"},
        {testTube,"tube"},
        {testRemoveVerts, "rvs", "remove vertices"},
        {testMeshImageMapRend,"texmap"},
    };
    doMenu(args,cmds,true,false);
}

}
