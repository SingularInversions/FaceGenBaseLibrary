//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dMesh.hpp"
#include "FgException.hpp"
#include "FgStdStream.hpp"
#include "FgBounds.hpp"
#include "FgMath.hpp"
#include "FgTopology.hpp"
#include "FgStdSet.hpp"
#include "FgQuaternion.hpp"

using namespace std;

namespace Fg {

void
IndexedMorph::accAsTarget_(Vec3Fs const & baseVerts,float coeff,Vec3Fs & accVerts) const
{
    FGASSERT(baseVerts.size() == accVerts.size());
    for (IdxVec3F const & iv : ivs) {
        Vec3F           del = iv.vec - baseVerts[iv.idx];
        accVerts[iv.idx] += del * coeff;
    }
}

IdxVec3Fs           offsetIndices(IdxVec3Fs const & ivs,uint offset)
{
    return mapCall(ivs,[=](IdxVec3F const & iv){return IdxVec3F{iv.idx+offset,iv.vec}; });
}

Vec3Fs              indexedToCorrMorph(IdxVec3Fs const & ivs,size_t baseSize)
{
    Vec3Fs              ret {baseSize,Vec3F{0}};
    for (IdxVec3F const & iv : ivs) {
        FGASSERT(iv.idx < baseSize);
        ret[iv.idx] = iv.vec;
    }
    return ret;
}

size_t
cNumVerts(IndexedMorphs const & ims)
{
    size_t          ret = 0;
    for (IndexedMorph const & im : ims)
        ret += im.ivs.size();
    return ret;
}

IndexedMorph        deltaToIndexedMorph(DirectMorph const & morph,float dmag)
{
    IndexedMorph            ret {morph.name,{}};
    for (size_t vv=0; vv<morph.verts.size(); ++vv) {
        Vec3F               del = morph.verts[vv];
        if (cMag(del) > dmag)
            ret.ivs.emplace_back(uint(vv),del);
    }
    return ret;
}

IndexedMorph        deltaToTargetMorph(Vec3Fs const & base,IndexedMorph const & morph)
{
    IndexedMorph            ret {morph.name,{}};
    ret.ivs.reserve(morph.ivs.size());
    for (IdxVec3F const & iv : morph.ivs)
        ret.ivs.emplace_back(iv.idx,base[iv.idx]+iv.vec);
    return ret;
}

void
accDeltaMorphs(
    DirectMorphs const &    deltaMorphs,
    Floats const &          coord,
    Vec3Fs &                accVerts)
{
    FGASSERT(deltaMorphs.size() == coord.size());
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii) {
        DirectMorph const &     morph = deltaMorphs[ii];
        FGASSERT(morph.verts.size() == accVerts.size());
        for (size_t jj=0; jj<accVerts.size(); ++jj)
            accVerts[jj] += morph.verts[jj] * coord[ii];
    }
}

void
accTargetMorphs(
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

Mesh::Mesh(TriSurfLms const & tsf) :
    verts {cat(tsf.surf.verts,tsf.landmarks)},
    surfaces {Surf{tsf.surf.tris}}
{
    size_t          V = tsf.surf.verts.size(),
                    F = tsf.landmarks.size();
    for (size_t ii=V; ii<V+F; ++ii)
        markedVerts.push_back(MarkedVert{uint(ii)});
}

size_t
Mesh::allVertsSize() const
{
    return verts.size() + cNumVerts(targetMorphs) + joints.size();
}

Vec3Fs
Mesh::allVerts() const
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

void
Mesh::updateAllVerts(Vec3Fs const & allVerts)
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

uint
Mesh::numPolys() const
{
    uint    tot = 0;
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        tot += surfaces[ss].numPolys();
    return tot;
}

uint        Mesh::numTriEquivs() const
{
    uint        tot = 0;
    for (uint ss=0; ss<surfaces.size(); ss++)
        tot += surfaces[ss].numTriEquivs();
    return tot;
}

Vec3UI
Mesh::getTriEquivVertInds(uint idx) const
{
    for (size_t ss=0; ss<surfaces.size(); ++ss) {
        uint        num = surfaces[ss].numTriEquivs();
        if (idx < num)
            return surfaces[ss].getTriEquivVertInds(idx);
        else
            idx -= num;
    }
    FGASSERT_FALSE;
    return Vec3UI();
}

NPolys<3>
Mesh::getTriEquivs() const
{
    NPolys<3>       ret;
    if (surfaces.empty())
        return ret;
    ret = surfaces[0].getTriEquivs();
    for (size_t ii=1; ii<surfaces.size(); ++ii)
        ret = concat(ret,surfaces[ii].getTriEquivs());
    return ret;
}

size_t
Mesh::numTris() const
{
    size_t      ret = 0;
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        ret += surfaces[ss].tris.size();
    return ret;
}

size_t
Mesh::numQuads() const
{
    size_t      ret = 0;
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        ret += surfaces[ss].quads.size();
    return ret;
}

size_t              Mesh::surfPointNum() const
{
    size_t          tot = 0;
    for (size_t ss=0; ss<surfaces.size(); ss++)
        tot += surfaces[ss].surfPoints.size();
    return tot;
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
Opt<Vec3F>          Mesh::surfPointPos(string const & label) const
{
    Opt<Vec3F>    ret;
    for (size_t ss=0; ss<surfaces.size(); ++ss) {
        size_t  idx = findFirstIdx(surfaces[ss].surfPoints,label);
        if (idx < surfaces[ss].surfPoints.size()) {
            ret = surfaces[ss].surfPointPos(verts,idx);
            break;
        }
    }
    return ret;
}
Vec3Fs              Mesh::surfPointPositions(Strings const & labels) const
{
    Vec3Fs         ret;
    ret.reserve(labels.size());
    for (size_t ss=0; ss<labels.size(); ++ss) {
        Opt<Vec3F>     pos = surfPointPos(labels[ss]);
        if (!pos.valid())
            fgThrow("Surface point not found",labels[ss]);
        ret.push_back(pos.val());
    }
    return ret;
}
Vec3Fs              Mesh::surfPointPositions() const
{
    Vec3Fs      ret;
    for (Surf const & surf : surfaces)
        cat_(ret,surf.surfPointPositions(verts));
    return ret;
}

Vec3Fs
Mesh::markedVertPositions() const
{
    Vec3Fs     ret;
    ret.reserve(markedVerts.size());
    for (const MarkedVert & m : markedVerts)
        ret.push_back(verts[m.idx]);
    return ret;
}

Vec3Fs
Mesh::markedVertPositions(Strings const & labels) const
{
    Vec3Fs              ret;
    ret.reserve(labels.size());
    for (String const & label : labels) {
        MarkedVerts         mvs = findAllByMember(markedVerts,&MarkedVert::label,label);
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

TriSurf
Mesh::asTriSurf() const
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

String8
Mesh::morphName(size_t idx) const
{
    if (idx < deltaMorphs.size())
        return deltaMorphs[idx].name;
    idx -= deltaMorphs.size();
    return targetMorphs[idx].name;
}

String8s
Mesh::morphNames() const
{
    String8s    ret;
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        ret.push_back(deltaMorphs[ii].name);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        ret.push_back(targetMorphs[ii].name);
    return ret;
}

Valid<size_t>
Mesh::findDeltaMorph(String8 const & name_) const
{
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        if (String8(deltaMorphs[ii].name) == name_)
            return Valid<size_t>(ii);
    return Valid<size_t>();
}

Valid<size_t>
Mesh::findTargMorph(String8 const & name_) const
{
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        if (targetMorphs[ii].name == name_)
            return Valid<size_t>(ii);
    return Valid<size_t>();
}

Valid<size_t>
Mesh::findMorph(String8 const & name_) const
{
    for (size_t ii=0; ii<numMorphs(); ++ii)
        if (morphName(ii) == name_)
            return Valid<size_t>(ii);
    return Valid<size_t>();
}

void
Mesh::morph(
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

void
Mesh::morph(
    Vec3Fs const &     allVerts,
    Floats const &      coord,
    Vec3Fs &           outVerts) const
{
    outVerts = cHead(allVerts,verts.size());
    size_t          ndms = deltaMorphs.size();
    accDeltaMorphs(deltaMorphs,cHead(coord,ndms),outVerts);
    accTargetMorphs(allVerts,targetMorphs,cRest(coord,ndms),outVerts);
}

Vec3Fs
Mesh::morph(
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

Vec3Fs
Mesh::morphSingle(size_t idx,float val) const
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

IndexedMorph
Mesh::getMorphAsIndexedDelta(size_t idx) const
{
    float               tol = sqr(cMaxElem(cDims(verts)) * 0.0001);
    IndexedMorph        ret;
    if (idx < deltaMorphs.size()) {
        DirectMorph const & dm = deltaMorphs[idx];
        ret.name = dm.name;
        for (size_t ii=0; ii<dm.verts.size(); ++ii)
            if (dm.verts[ii].mag() > tol)
                ret.ivs.emplace_back(uint(ii),dm.verts[ii]);
    }
    else {
        idx -= deltaMorphs.size();
        FGASSERT(idx < targetMorphs.size());
        const IndexedMorph &  tm = targetMorphs[idx];
        ret.name = tm.name;
        ret.ivs = tm.ivs;
        for (size_t ii=0; ii<ret.ivs.size(); ++ii)
            ret.ivs[ii].vec -= verts[ret.ivs[ii].idx];
    }
    return ret;
}

void
Mesh::addDeltaMorph(DirectMorph const & morph)
{
    Valid<size_t>         idx = findDeltaMorph(morph.name);
    if (idx.valid()) {
        fgout << fgnl << "WARNING: Overwriting existing morph " << morph.name;
        deltaMorphs[idx.val()] = morph;
    }
    else
        deltaMorphs.push_back(morph);
}

void
Mesh::addDeltaMorphFromTarget(String8 const & name_,Vec3Fs const & targetShape)
{
    DirectMorph                 dm;
    dm.name = name_;
    dm.verts = targetShape - verts;
    addDeltaMorph(dm);
}

void
Mesh::addTargMorph(const IndexedMorph & morph)
{
    FGASSERT(cMax(sliceMember(morph.ivs,&IdxVec3F::idx)) < verts.size());
    Valid<size_t>     idx = findTargMorph(morph.name);
    if (idx.valid()) {
        fgout << fgnl << "WARNING: Overwriting existing morph " << morph.name;
        targetMorphs[idx.val()] = morph;
    }
    else
        targetMorphs.push_back(morph);
}

void
Mesh::addTargMorph(String8 const & name_,Vec3Fs const & targetShape)
{
    FGASSERT(targetShape.size() == verts.size());
    IndexedMorph       tm;
    tm.name = name_;
    Vec3Fs              deltas = targetShape - verts;
    double              maxMag = 0.0;
    for (size_t ii=0; ii<deltas.size(); ++ii)
        updateMax_(maxMag,deltas[ii].mag());
    if (maxMag == 0.0f) {
        fgout << fgnl << "WARNING: skipping empty morph " << name_;
        return;
    }
    maxMag *= sqr(0.001f);
    for (size_t ii=0; ii<deltas.size(); ++ii)
        if (deltas[ii].mag() > maxMag)
            tm.ivs.emplace_back(uint(ii),targetShape[ii]);
    addTargMorph(tm);
}

Vec3Fs
Mesh::poseShape(Vec3Fs const & allVerts,map<String8,float> const & poseVals) const
{
    Vec3Fs                  ret = cHead(allVerts,verts.size());
    for (DirectMorph const & morph : deltaMorphs) {
        auto                    it = poseVals.find(morph.name);
        if (it != poseVals.end())
            mapMulAcc_(morph.verts,it->second,ret);
    }
    size_t                  targIdx = verts.size();
    for (IndexedMorph const & tm : targetMorphs) {
        auto                    it = poseVals.find(tm.name);
        if (it != poseVals.end()) {
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
        auto                    it = poseVals.find(dof.name);
        if (it != poseVals.end()) {
            FGASSERT(dof.jointIdx < jointAccs.size());
            float                   val = it->second / 2.0f;
            Vec3F                   a = dof.rotAxis * sin(val);
            jointAccs[dof.jointIdx] += Vec4F {cos(val),a[0],a[1],a[2]};
        }
    }
    // For the non-zero accumulants, interpolate and rotate:
    for (size_t pp=0; pp<joints.size(); ++pp) {
        Vec4F const &           acc = jointAccs[pp];
        if (cMag(acc) > 0) {
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

void
Mesh::addSurfaces(Surfs const & surfs)
{
    for (size_t ss=0; ss<surfs.size(); ++ss)
        surfaces.push_back(surfs[ss]);
    checkValidity();
}

void
Mesh::transform_(Affine3F const & aff)
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

void
Mesh::convertToTris()
{
    for (size_t ii=0; ii<surfaces.size(); ++ii)
        surfaces[ii] = surfaces[ii].convertToTris();
}

void
Mesh::removeUVs()
{
    for (Surf & surf : surfaces) {
        surf.tris.uvInds.clear();
        surf.quads.uvInds.clear();
    }
    uvs.clear();
}


void
Mesh::checkValidity() const
{
    uint    numVerts = uint(verts.size());
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        surfaces[ss].checkMeshConsistency(numVerts,uint(uvs.size()));
    for (size_t mm=0; mm<markedVerts.size(); ++mm)
        FGASSERT(markedVerts[mm].idx < numVerts);
}

Mesh
fg3dMesh(Vec3UIs const & tris,Vec3Fs const & verts)
{
    Mesh        ret;
    ret.surfaces.resize(1);
    ret.surfaces[0].tris.vertInds = tris;
    ret.verts = verts;
    return ret;
}

size_t
cNumTriEquivs(Meshes const & meshes)
{
    size_t      ret = 0;
    for (Mesh const & m : meshes)
        ret += m.numTriEquivs();
    return ret;
}

std::set<String8>
getMorphNames(Meshes const & meshes)
{
    std::set<String8>  ret;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        cUnion_(ret,meshes[ii].morphNames());
    return ret;
}

PoseDefs
cPoseDefs(Mesh const & mesh)
{
    PoseDefs         ret;
    for (DirectMorph const & morph : mesh.deltaMorphs)
        ret.emplace_back(morph.name,Vec2F{0,1},0);
    for (IndexedMorph const & morph : mesh.targetMorphs)
        ret.emplace_back(morph.name,Vec2F{0,1},0);
    for (JointDof const & dof : mesh.jointDofs)
        ret.emplace_back(dof.name,dof.angleBounds,0);
    return ret;
}

PoseDefs
cPoseDefs(Meshes const & meshes)
{
    unordered_set<String>   names;
    PoseDefs                ret;
    for (Mesh const & mesh : meshes) {
        PoseDefs            poseDefs = cPoseDefs(mesh);
        for (PoseDef const & pd : poseDefs)
            if (!contains(names,pd.name.m_str))
                ret.push_back(pd);
    }
    return ret;
}

Mesh
transform(Mesh const & mesh,SimilarityD const & sim)
{
    Mesh                ret = mesh;
    ret.transform_(sim);
    return ret;
}

std::ostream &
operator<<(std::ostream & os,Mesh const & m)
{
    os  << fgnl << "Name: " << m.name;
    Vec3Fs              allVerts = m.allVerts();
    if (allVerts.size() != m.verts.size())
        os << fgnl << "AllVerts: " << allVerts.size() << " with bounds: " << cBounds(allVerts);
    os  << fgnl << "Verts: " << m.verts.size();
    size_t              numUniqueVerts = getUniqueSorted(cSort(m.verts)).size();
    if (numUniqueVerts < m.verts.size())
        fgout << " unique: " << numUniqueVerts;
    os  << " with bounds: " << cBounds(m.verts)
        << fgnl << "UVs: " << m.uvs.size();
    if (!m.uvs.empty()) {
        size_t          numUnique = getUniqueSorted(cSort(m.uvs)).size();
        if (numUnique < m.uvs.size())
            os << " unique: " << numUnique;
        os << " with bounds: " << cBounds(m.uvs);
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
        PushIndent      pind {toStrDigits(ss,2)};
        Surf const &    surf = m.surfaces[ss];
        os << surf;
        if (surf.material.albedoMap)
            os << fgnl << "Albedo: " << *surf.material.albedoMap;
    }
    os << fgpop;
    SurfTopo            topo(m.verts.size(),m.getTriEquivs().vertInds);
    Vec3UI              te = topo.isManifold();
    os << fgnl << "Watertight: ";
    if (te == Vec3UI(0))
        os << "YES";
    else
        os << "NO (" << te[0] << " boundary edges)";
    os << fgnl << "Manifold: ";
    if ((te[1] == 0) && (te[2] == 0))
        os << "YES";
    else
        os << "NO (" << te[1] << " intersection edges, " << te[2] << " reversed edges)";
    os << fgnl << "Boundaries: " << topo.boundaries().size()
        << fgnl << "Unused verts: " << topo.unusedVerts();
    return os;
}

std::ostream &
operator<<(std::ostream & os,Meshes const & ms)
{
    for (size_t ii=0; ii<ms.size(); ++ii)
        os << fgnl << "Mesh " << ii << ":" << fgpush << ms[ii] << fgpop;
    return os;
}

Vec3UIs
subdivideTris(
    Vec3UIs const &             tris,
    Svec<SurfTopo::Tri> const &   topoTris,
    uint                        newVertsBaseIdx)
{
    Vec3UIs             ret;
    for (size_t ii=0; ii<tris.size(); ii++) {
        Vec3UI          vertInds = tris[ii];
        Vec3UI          edgeInds = topoTris[ii].edgeInds;
        uint            ni0 = newVertsBaseIdx + edgeInds[0],
                        ni1 = newVertsBaseIdx + edgeInds[1],
                        ni2 = newVertsBaseIdx + edgeInds[2];
        ret.push_back(Vec3UI(vertInds[0],ni0,ni2));
        ret.push_back(Vec3UI(vertInds[1],ni1,ni0));
        ret.push_back(Vec3UI(vertInds[2],ni2,ni1));
        ret.push_back(Vec3UI(ni0,ni1,ni2));
    }
    return ret;
}

Surf
subdivideTris(
    Vec3UIs const &             tris,
    Svec<SurfTopo::Tri> const &   topoTris,
    uint                        newVertsBaseIdx,
    SurfPoints const &          sps)
{
    Surf            ret;
    ret.tris.vertInds = subdivideTris(tris,topoTris,newVertsBaseIdx);
    // Set up surface point weight transforms:
    Mat33F          wgtXform(1),
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
        uint        facetIdx = sps[ii].point.triEquivIdx * 4;
        Vec3F       weights = sps[ii].point.weights,
                    wgtCentre = wgtXform * weights;
        if (wgtCentre[0] < 0.0)
            ret.surfPoints.push_back(SurfPoint(facetIdx+2,wgtXform2*weights));
        else if (wgtCentre[1] < 0.0)
            ret.surfPoints.push_back(SurfPoint(facetIdx,wgtXform0*weights));
        else if (wgtCentre[2] < 0.0)
            ret.surfPoints.push_back(SurfPoint(facetIdx+1,wgtXform1*weights));
        else
            ret.surfPoints.push_back(SurfPoint(facetIdx+3,wgtCentre));
    }
    return ret;
}

Mat32F
cBounds(Meshes const & meshes)
{
    FGASSERT(!meshes.empty());
    Mat32F          ret = cBounds(meshes[0].verts);
    for (size_t mm=1; mm<meshes.size(); ++mm)
        ret = cBoundsUnion(ret,cBounds(meshes[mm].verts));
    return ret;
}

Mesh
subdivide(Mesh const & in,bool loop)
{
    Tris                allTris;
    SurfPoints          allSps;
    for (Surf const & surf : in.surfaces) {
        for (SurfPoint sp : surf.surfPoints) {
            sp.point.triEquivIdx += uint(allTris.size());
            allSps.push_back(sp);
        }
        allTris = concat(allTris,surf.tris);
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
                Vec3F           acc;
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
        Tris                tris;
        SurfPoints          surfPoints;
        size_t              num = surfIn.tris.size() * 4;
        tris.vertInds = cSubvec(ssurf.tris.vertInds,sidx,num);
        if (!ssurf.tris.uvInds.empty())
            tris.uvInds = cSubvec(ssurf.tris.uvInds,sidx,num);
        for (size_t ii=0; ii<surfIn.surfPoints.size(); ++ii) {
            SurfPoint           sp = ssurf.surfPoints[spidx+ii];
            sp.point.triEquivIdx -= uint(sidx);
            surfPoints.push_back(sp);
        }
        sidx += num;
        spidx += surfIn.surfPoints.size();
        ret.surfaces.emplace_back(surfIn.name,tris,surfPoints,surfIn.material);
    }
    return ret;
}

// Hack this for now:
TriSurf
subdivide(TriSurf const & surf,bool loop)
{
    Mesh        mesh = subdivide(Mesh{surf},loop);
    Tris        tris = sliceMember(mesh.surfaces,&Surf::tris)[0];
    return TriSurf {mesh.verts,tris.vertInds};
}

TriSurf
cullVolume(TriSurf triSurf,Mat32F const & bounds)
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
        for (uint vertIdx : t.vertInds.m) {
            if (!vertInBounds[vertIdx])
                inBounds = false;
        }
        triInBounds.push_back(inBounds);
    }
    Vec3UIs             nTris;
    for (size_t tt=0; tt<triSurf.tris.size(); ++tt)
        if (triInBounds[tt])
            nTris.push_back(triSurf.tris[tt]);
    return removeUnusedVerts(triSurf.verts,nTris);
}

}
