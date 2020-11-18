//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dMeshOps.hpp"
#include "FgException.hpp"
#include "FgStdStream.hpp"
#include "FgBounds.hpp"
#include "FgMath.hpp"
#include "Fg3dTopology.hpp"
#include "FgStdSet.hpp"

using namespace std;

namespace Fg {

Mesh::Mesh(TriSurfFids const & tsf) :
    verts {cat(tsf.surf.verts,tsf.fids)},
    surfaces {Surf{tsf.surf.tris}}
{
    size_t          V = tsf.surf.verts.size(),
                    F = tsf.fids.size();
    for (size_t ii=V; ii<V+F; ++ii)
        markedVerts.push_back(MarkedVert{ii});
}

size_t
Mesh::totNumVerts() const
{
    size_t  ret = verts.size();
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        ret += targetMorphs[ii].verts.size();
    return ret;
}

Vec3Fs
Mesh::allVerts() const
{
    Vec3Fs     ret = verts;
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        cat_(ret,targetMorphs[ii].verts);
    return ret;
}

void
Mesh::updateAllVerts(Vec3Fs const & avs)
{
    FGASSERT(avs.size() == totNumVerts());
    size_t          idx = 0;
    for (; idx<verts.size(); ++idx)
        verts[idx] = avs[idx];
    for (size_t ii=0; ii<targetMorphs.size(); ++ii) {
        IndexedMorph & tm = targetMorphs[ii];
        for (size_t jj=0; jj<tm.verts.size(); ++jj)
            tm.verts[jj] = avs[idx++];
    }
}

uint
Mesh::numFacets() const
{
    uint    tot = 0;
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        tot += surfaces[ss].numFacets();
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
Mesh::getTriEquivPosInds(uint idx) const
{
    for (size_t ss=0; ss<surfaces.size(); ++ss) {
        uint        num = surfaces[ss].numTriEquivs();
        if (idx < num)
            return surfaces[ss].getTriEquivPosInds(idx);
        else
            idx -= num;
    }
    FGASSERT_FALSE;
    return Vec3UI();
}

FacetInds<3>
Mesh::getTriEquivs() const
{
    FacetInds<3>      ret;
    if (surfaces.empty())
        return ret;
    ret = surfaces[0].getTriEquivs();
    for (size_t ii=1; ii<surfaces.size(); ++ii)
        cat_(ret,surfaces[ii].getTriEquivs());
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

size_t
Mesh::surfPointNum() const
{
    size_t          tot = 0;
    for (size_t ss=0; ss<surfaces.size(); ss++)
        tot += surfaces[ss].surfPoints.size();
    return tot;
}

Vec3F
Mesh::surfPointPos(Vec3Fs const & verts_,size_t num) const
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

Opt<Vec3F>
Mesh::surfPointPos(string const & label) const
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

LabelledVerts
Mesh::surfPointsAsLabelledVerts() const
{
    LabelledVerts     ret;
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        cat_(ret,surfaces[ss].surfPointsAsLabelledVerts(verts));
    return ret;
}

Vec3Fs
Mesh::surfPointPositions(Strings const & labels) const
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

Vec3Fs
Mesh::surfPointPositions() const
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

LabelledVerts
Mesh::markedVertsAsLabelledVerts() const
{
    LabelledVerts       ret;
    ret.reserve(markedVerts.size());
    for (MarkedVert const & mv : markedVerts)
        ret.push_back(LabelledVert{verts[mv.idx],mv.label});
    return ret;
}

TriSurf
Mesh::asTriSurf() const
{
    TriSurf   ret;
    ret.verts = verts;
    for (Surf const & surf : surfaces) {
        cat_(ret.tris,surf.tris.posInds);
        if (!surf.quads.posInds.empty())
            cat_(ret.tris,quadsToTris(surf.quads.posInds));
    }
    return ret;
}

Ustring
Mesh::morphName(size_t idx) const
{
    if (idx < deltaMorphs.size())
        return deltaMorphs[idx].name;
    idx -= deltaMorphs.size();
    return targetMorphs[idx].name;
}

Ustrings
Mesh::morphNames() const
{
    Ustrings    ret;
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        ret.push_back(deltaMorphs[ii].name);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        ret.push_back(targetMorphs[ii].name);
    return ret;
}

Valid<size_t>
Mesh::findDeltaMorph(Ustring const & name_) const
{
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        if (Ustring(deltaMorphs[ii].name) == name_)
            return Valid<size_t>(ii);
    return Valid<size_t>();
}

Valid<size_t>
Mesh::findTargMorph(Ustring const & name_) const
{
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        if (targetMorphs[ii].name == name_)
            return Valid<size_t>(ii);
    return Valid<size_t>();
}

Valid<size_t>
Mesh::findMorph(Ustring const & name_) const
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
    IndexedMorph      ret;
    if (idx < deltaMorphs.size()) {
        Morph const & dm = deltaMorphs[idx];
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
        const IndexedMorph &  tm = targetMorphs[idx];
        ret.name = tm.name;
        ret.baseInds = tm.baseInds;
        ret.verts = tm.verts;
        for (size_t ii=0; ii<ret.verts.size(); ++ii)
            ret.verts[ii] -= verts[ret.baseInds[ii]];
    }
    return ret;
}

void
Mesh::addDeltaMorph(Morph const & morph)
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
Mesh::addDeltaMorphFromTarget(Ustring const & name_,Vec3Fs const & targetShape)
{
    Morph                 dm;
    dm.name = name_;
    dm.verts = targetShape - verts;
    addDeltaMorph(dm);
}

void
Mesh::addTargMorph(const IndexedMorph & morph)
{
    FGASSERT(cMax(morph.baseInds) < verts.size());
    Valid<size_t>     idx = findTargMorph(morph.name);
    if (idx.valid()) {
        fgout << fgnl << "WARNING: Overwriting existing morph " << morph.name;
        targetMorphs[idx.val()] = morph;
    }
    else
        targetMorphs.push_back(morph);
}

void
Mesh::addTargMorph(Ustring const & name_,Vec3Fs const & targetShape)
{
    FGASSERT(targetShape.size() == verts.size());
    IndexedMorph       tm;
    tm.name = name_;
    Vec3Fs             deltas = targetShape - verts;
    double              maxMag = 0.0;
    for (size_t ii=0; ii<deltas.size(); ++ii)
        setIfGreater(maxMag,deltas[ii].mag());
    if (maxMag == 0.0f) {
        fgout << fgnl << "WARNING: skipping empty morph " << name_;
        return;
    }
    maxMag *= sqr(0.001f);
    for (size_t ii=0; ii<deltas.size(); ++ii) {
        if (deltas[ii].mag() > maxMag) {
            tm.baseInds.push_back(uint(ii));
            tm.verts.push_back(targetShape[ii]);
        }
    }
    addTargMorph(tm);
}

Vec3Fs
Mesh::poseShape(Vec3Fs const & allVerts,const std::map<Ustring,float> & poseVals) const
{
    Vec3Fs      ret = cHead(allVerts,verts.size()),
                base = ret;
    accPoseDeltas_(poseVals,deltaMorphs,ret);
    accPoseDeltas_(poseVals,targetMorphs,base,cRest(allVerts,verts.size()),ret);
    return ret;
}

void
Mesh::addSurfaces(const Surfs & surfs)
{
    for (size_t ss=0; ss<surfs.size(); ++ss)
        surfaces.push_back(surfs[ss]);
    checkValidity();
}

void
Mesh::scale(float val)
{
    mapMul_(val,verts);
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        mapMul_(val,deltaMorphs[ii].verts);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        mapMul_(val,targetMorphs[ii].verts);
}

void
Mesh::transform(Mat33F xform)
{
    mapMul_(xform,verts);
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        mapMul_(xform,deltaMorphs[ii].verts);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        mapMul_(xform,targetMorphs[ii].verts);
}

void
Mesh::transform(Affine3F xform)
{
    mapMul_(xform,verts);
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        mapMul_(xform.linear,deltaMorphs[ii].verts);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        mapMul_(xform,targetMorphs[ii].verts);
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
Mesh::checkValidity()
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
    ret.surfaces[0].tris.posInds = tris;
    ret.verts = verts;
    return ret;
}

size_t
fgNumTriEquivs(Meshes const & meshes)
{
    size_t      ret = 0;
    for (Mesh const & m : meshes)
        ret += m.numTriEquivs();
    return ret;
}

std::set<Ustring>
getMorphNames(Meshes const & meshes)
{
    std::set<Ustring>  ret;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        cUnion_(ret,meshes[ii].morphNames());
    return ret;
}

PoseVals
cPoseVals(Meshes const & meshes)
{
    std::set<PoseVal>    ps;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        cUnion_(ps,cPoseVals(meshes[ii]));
    return vector<PoseVal>(ps.begin(),ps.end());
}

static
bool
vertLt(Vec3F const & v0,Vec3F const & v1)
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
operator<<(std::ostream & os,Mesh const & m)
{
    os  << fgnl << "Name: " << m.name
        << fgnl << "Verts: " << m.verts.size();
    size_t      allVerts = m.allVerts().size();
    if (allVerts != m.verts.size())
        os << " (" << allVerts << " with targ morphs)";
    os  << fgnl << "UVs: " << m.uvs.size()
        << fgnl << "Bounding Box: " << cBounds(m.verts)
        << fgnl << "Vertex Mean:  " << cMean(m.verts)
        << fgnl << "Delta Morphs: " << m.deltaMorphs.size()
        << fgnl << "Target Morphs: " << m.targetMorphs.size()
        << fgnl << "Marked Verts: " << m.markedVerts.size()
        << fgpush;
    for (size_t ii=0; ii<m.markedVerts.size(); ++ii)
        if (!m.markedVerts[ii].label.empty())
            os << fgnl << m.markedVerts[ii].label;
    os << fgpop;
    os << fgnl << "Surfaces: " << m.surfaces.size();
    for (size_t ss=0; ss<m.surfaces.size(); ss++) {
        Surf const &     surf = m.surfaces[ss];
        os << fgnl << "Surface " << ss << ": " << surf.name << fgpush << surf << fgpop;
        if (surf.material.albedoMap)
            os << fgnl << "Albedo: " << *surf.material.albedoMap;
    }
    Vec3Fs             sortVerts = m.verts;
    std::sort(sortVerts.begin(),sortVerts.end(),vertLt);
    size_t              numDups = 0;
    for (size_t ii=1; ii<sortVerts.size(); ++ii)
        if (sortVerts[ii] == sortVerts[ii-1])
            ++numDups;
    os << fgnl << "Duplicate vertices: " << numDups;
    MeshTopology        topo(m.verts.size(),m.getTriEquivs().posInds);
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
    os << fgnl << "Seams: " << topo.seams().size()
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
    Svec<MeshTopology::Tri> const &   topoTris,
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
    Svec<MeshTopology::Tri> const &   topoTris,
    uint                        newVertsBaseIdx,
    SurfPoints const &          sps)
{
    Surf            ret;
    ret.tris.posInds = subdivideTris(tris,topoTris,newVertsBaseIdx);
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
        uint        facetIdx = sps[ii].triEquivIdx * 4;
        Vec3F       weights = sps[ii].weights,
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
            sp.triEquivIdx += uint(allTris.size());
            allSps.push_back(sp);
        }
        cat_(allTris,surf.tris);
    }
    Mesh                ret;
    ret.verts = in.verts;   // Modified later in case of Loop:
    ret.markedVerts = in.markedVerts;
    MeshTopology            topo {in.verts.size(),allTris.posInds};
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
    Surf                ssurf = subdivideTris(allTris.posInds,topo.m_tris,newVertsBaseIdx,allSps);
    // Can only carry over UVs if they exist and are defined for all tris (ie on all surfaces):
    if (!in.uvs.empty() && (allTris.uvInds.size() == allTris.posInds.size())) {
        ret.uvs = in.uvs;
        MeshTopology        topoUvs {in.uvs.size(),allTris.uvInds};
        uint                newUvsBaseIdx = uint(in.uvs.size());
        for (MeshTopology::Edge const & edge : topoUvs.m_edges) {
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
        tris.posInds = cSubvec(ssurf.tris.posInds,sidx,num);
        if (!ssurf.tris.uvInds.empty())
            tris.uvInds = cSubvec(ssurf.tris.uvInds,sidx,num);
        for (size_t ii=0; ii<surfIn.surfPoints.size(); ++ii) {
            SurfPoint           sp = ssurf.surfPoints[spidx+ii];
            sp.triEquivIdx -= uint(sidx);
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
    return TriSurf {mesh.verts,tris.posInds};
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
    MeshTopology        topo {triSurf.verts.size(),triSurf.tris};
    for (MeshTopology::Tri const & t : topo.m_tris) {
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
    return meshRemoveUnusedVerts(triSurf.verts,nTris);
}

}
