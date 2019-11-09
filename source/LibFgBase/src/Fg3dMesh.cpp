//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//

#include "stdafx.h"

#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"
#include "Fg3dMesh.hpp"
#include "FgException.hpp"
#include "FgStdStream.hpp"
#include "FgBounds.hpp"
#include "FgMath.hpp"
#include "Fg3dTopology.hpp"
#include "FgStdSet.hpp"

using namespace std;

namespace Fg {

void
fgReadp(std::istream & is,MarkedVert & m)
{
    fgReadp(is,m.idx);
    fgReadp(is,m.label);
}

void
fgWritep(std::ostream & os,const MarkedVert & m)
{
    fgWritep(os,m.idx);
    fgWritep(os,m.label);
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
Mesh::updateAllVerts(const Vec3Fs & avs)
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
Mesh::surfPointPos(const Vec3Fs & verts_,size_t num) const
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
Mesh::surfPointPos(const string & label) const
{
    Opt<Vec3F>    ret;
    for (size_t ss=0; ss<surfaces.size(); ++ss) {
        size_t  idx = fgFindFirstIdx(surfaces[ss].surfPoints,label);
        if (idx < surfaces[ss].surfPoints.size()) {
            ret = surfaces[ss].surfPointPos(verts,idx);
            break;
        }
    }
    return ret;
}

LabelledVerts
Mesh::surfPointsAsVertLabels() const
{
    LabelledVerts     ret;
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        cat_(ret,surfaces[ss].surfPointsAsVertLabels(verts));
    return ret;
}

Vec3Fs
Mesh::surfPointPositions(const Strings & labels) const
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
Mesh::markedVertPositions() const
{
    Vec3Fs     ret;
    ret.reserve(markedVerts.size());
    for (const MarkedVert & m : markedVerts)
        ret.push_back(verts[m.idx]);
    return ret;
}

TriSurf
Mesh::asTriSurf() const
{
    TriSurf   ret;
    ret.verts = verts;
    for (const Surf & surf : surfaces) {
        cat_(ret.tris,surf.tris.vertInds);
        if (!surf.quads.vertInds.empty())
            cat_(ret.tris,fgQuadsToTris(surf.quads.vertInds));
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
Mesh::findDeltaMorph(const Ustring & name_) const
{
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        if (Ustring(deltaMorphs[ii].name) == name_)
            return Valid<size_t>(ii);
    return Valid<size_t>();
}

Valid<size_t>
Mesh::findTargMorph(const Ustring & name_) const
{
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        if (targetMorphs[ii].name == name_)
            return Valid<size_t>(ii);
    return Valid<size_t>();
}

Valid<size_t>
Mesh::findMorph(const Ustring & name_) const
{
    for (size_t ii=0; ii<numMorphs(); ++ii)
        if (morphName(ii) == name_)
            return Valid<size_t>(ii);
    return Valid<size_t>();
}

void
Mesh::morph(
    const Floats &      morphCoord,
    Vec3Fs &           outVerts) const
{
    FGASSERT(morphCoord.size() == numMorphs());
    outVerts = verts;
    size_t      cnt = 0;
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        deltaMorphs[ii].applyAsDelta(outVerts,morphCoord[cnt++]);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        targetMorphs[ii].applyAsTarget_(verts,morphCoord[cnt++],outVerts);
}

void
Mesh::morph(
    const Vec3Fs &     allVerts,
    const Floats &      coord,
    Vec3Fs &           outVerts) const
{
    outVerts = fgHead(allVerts,verts.size());
    size_t          ndms = deltaMorphs.size();
    fgAccDeltaMorphs(deltaMorphs,fgHead(coord,ndms),outVerts);
    fgAccTargetMorphs(allVerts,targetMorphs,fgRest(coord,ndms),outVerts);
}

Vec3Fs
Mesh::morph(
    const Floats &      deltaMorphCoord,
    const Floats &      targMorphCoord) const
{
    Vec3Fs     ret = verts;
    FGASSERT(deltaMorphCoord.size() == deltaMorphs.size());
    FGASSERT(targMorphCoord.size() == targetMorphs.size());
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        if (deltaMorphCoord[ii] != 0.0f)
            deltaMorphs[ii].applyAsDelta(ret,deltaMorphCoord[ii]);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        if (targMorphCoord[ii] != 0.0f)
            targetMorphs[ii].applyAsTarget_(verts,targMorphCoord[ii],ret);
    return ret;
}

Vec3Fs
Mesh::morphSingle(size_t idx,float val) const
{
    Vec3Fs     ret = verts;
    if (idx < deltaMorphs.size())
        deltaMorphs[idx].applyAsDelta(ret,val);
    else {
        idx -= deltaMorphs.size();
        FGASSERT(idx < targetMorphs.size());
        targetMorphs[idx].applyAsTarget_(verts,val,ret);
    }
    return ret;
}

IndexedMorph
Mesh::getMorphAsIndexedDelta(size_t idx) const
{
    float               tol = sqr(fgMaxElem(fgDims(verts)) * 0.0001);
    IndexedMorph      ret;
    if (idx < deltaMorphs.size()) {
        const Morph & dm = deltaMorphs[idx];
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
Mesh::addDeltaMorph(const Morph & morph)
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
Mesh::addDeltaMorphFromTarget(const Ustring & name_,const Vec3Fs & targetShape)
{
    Morph                 dm;
    dm.name = name_;
    dm.verts = targetShape - verts;
    addDeltaMorph(dm);
}

void
Mesh::addTargMorph(const IndexedMorph & morph)
{
    FGASSERT(maxEl(morph.baseInds) < verts.size());
    Valid<size_t>     idx = findTargMorph(morph.name);
    if (idx.valid()) {
        fgout << fgnl << "WARNING: Overwriting existing morph " << morph.name;
        targetMorphs[idx.val()] = morph;
    }
    else
        targetMorphs.push_back(morph);
}

void
Mesh::addTargMorph(const Ustring & name_,const Vec3Fs & targetShape)
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
Mesh::poseShape(const Vec3Fs & allVerts,const std::map<Ustring,float> & poseVals) const
{
    Vec3Fs     ret = fgHead(allVerts,verts.size()),
                base = ret;
    fgPoseDeltas(poseVals,deltaMorphs,ret);
    fgPoseDeltas(poseVals,targetMorphs,base,fgRest(allVerts,verts.size()),ret);
    return ret;
}

void
Mesh::addSurfaces(
    const std::vector<Surf> & surfs)
{
    for (size_t ss=0; ss<surfs.size(); ++ss)
        surfaces.push_back(surfs[ss]);
    checkValidity();
}

void
Mesh::transform(Mat33F xform)
{
    mapXf_(verts,xform);
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        mapXf_(deltaMorphs[ii].verts,xform);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        mapXf_(targetMorphs[ii].verts,xform);
}

void
Mesh::transform(Affine3F xform)
{
    mapXf_(verts,xform);
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        mapXf_(deltaMorphs[ii].verts,xform.linear);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        mapXf_(targetMorphs[ii].verts,xform);
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

void
fgReadp(std::istream & is,Mesh & mesh)
{
    fgReadp(is,mesh.verts);
    fgReadp(is,mesh.uvs);
    fgReadp(is,mesh.surfaces);
    fgReadp(is,mesh.deltaMorphs);
    fgReadp(is,mesh.targetMorphs);
    fgReadp(is,mesh.markedVerts);
}

void
fgWritep(std::ostream & os,const Mesh & mesh)
{
    fgWritep(os,mesh.verts);
    fgWritep(os,mesh.uvs);
    fgWritep(os,mesh.surfaces);
    fgWritep(os,mesh.deltaMorphs);
    fgWritep(os,mesh.targetMorphs);
    fgWritep(os,mesh.markedVerts);
}

Mesh
fg3dMesh(const Vec3UIs & tris,const Vec3Fs & verts)
{
    Mesh        ret;
    ret.surfaces.resize(1);
    ret.surfaces[0].tris.vertInds = tris;
    ret.verts = verts;
    return ret;
}

size_t
fgNumTriEquivs(const Meshs & meshes)
{
    size_t      ret = 0;
    for (const Mesh & m : meshes)
        ret += m.numTriEquivs();
    return ret;
}

std::set<Ustring>
fgMorphs(const Meshs & meshes)
{
    std::set<Ustring>  ret;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        fgUnion_(ret,meshes[ii].morphNames());
    return ret;
}

PoseVals
fgPoses(const Meshs & meshes)
{
    std::set<PoseVal>    ps;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        fgUnion_(ps,fgPoses(meshes[ii]));
    return vector<PoseVal>(ps.begin(),ps.end());
}

static
bool
vertLt(const Vec3F & v0,const Vec3F & v1)
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
operator<<(std::ostream & os,const Mesh & m)
{
    os << fgnl << "Name: " << m.name << fgpush
        << fgnl << "Verts: " << m.verts.size() << "  "
        << fgnl << "UVs: " << m.uvs.size() << "  "
        << fgnl << "Bounding Box: " << getBounds(m.verts)
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
        const Surf &     surf = m.surfaces[ss];
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
    Fg3dTopology        topo(m.verts,m.getTriEquivs().vertInds);
    Vec3UI           te = topo.isManifold();
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
        << fgnl << "Unused verts: " << topo.unusedVerts()
        << fgpop;
    return os;
}

std::ostream &
operator<<(std::ostream & os,const Meshs & ms)
{
    for (size_t ii=0; ii<ms.size(); ++ii)
        os << fgnl << "Mesh " << ii << ":" << fgpush << ms[ii] << fgpop;
    return os;
}

static
Surf
subdivideTris(
    const vector<Vec3UI> &       tris,
    const vector<SurfPoint> &     sps,
    const vector<Fg3dTopology::Tri> &   topoTris,
    uint                            newVertsBaseIdx)
{
    Surf     ret;
    for (size_t ii=0; ii<tris.size(); ii++) {
        Vec3UI   vertInds = tris[ii];
        Vec3UI   edgeInds = topoTris[ii].edgeInds;
        uint        ni0 = newVertsBaseIdx + edgeInds[0],
                    ni1 = newVertsBaseIdx + edgeInds[1],
                    ni2 = newVertsBaseIdx + edgeInds[2];
        ret.tris.vertInds.push_back(Vec3UI(vertInds[0],ni0,ni2));
        ret.tris.vertInds.push_back(Vec3UI(vertInds[1],ni1,ni0));
        ret.tris.vertInds.push_back(Vec3UI(vertInds[2],ni2,ni1));
        ret.tris.vertInds.push_back(Vec3UI(ni0,ni1,ni2));
    }
    // Set up surface point weight transforms:
    Mat33F        wgtXform(1),
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
        Vec3F    weights = sps[ii].weights,
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
getBounds(const Meshs & meshes)
{
    FGASSERT(!meshes.empty());
    Mat32F    ret = getBounds(meshes[0].verts);
    for (size_t mm=1; mm<meshes.size(); ++mm)
        ret = boundsUnion(ret,getBounds(meshes[mm].verts));
    return ret;
}

Mesh
fgSubdivide(const Mesh & in,bool loop)
{
    Mesh            ret;
    vector<Vec3UI>   allTris;
    vector<SurfPoint> allSps;
    for (size_t ss=0; ss<in.surfaces.size(); ++ss) {
        const Surf & surf = in.surfaces[ss];
        for (size_t ii=0; ii<surf.surfPoints.size(); ++ii) {
            SurfPoint     sp = surf.surfPoints[ii];
            sp.triEquivIdx += uint(allTris.size());
            allSps.push_back(sp);
        }
        cat_(allTris,surf.tris.vertInds);
    }
    ret.verts = in.verts;   // Modified later in case of Loop:
    ret.surfaces.resize(in.surfaces.size());
    Fg3dTopology        topo(in.verts,allTris);
    uint                newVertsBaseIdx = uint(in.verts.size());
    if (loop) {
        // Add the edge-split "odd" verts:
        for (uint ii=0; ii<topo.m_edges.size(); ++ii) {
            Vec2UI       vertInds0 = topo.m_edges[ii].vertInds;
            if (topo.m_edges[ii].triInds.size() == 1) {     // Boundary
                ret.verts.push_back((
                    in.verts[vertInds0[0]] + 
                    in.verts[vertInds0[1]])*0.5f);
            }
            else {
                Vec2UI   vertInds1 = topo.edgeFacingVertInds(ii);
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
                vector<uint>    vertInds = topo.vertBoundaryNeighbours(ii);
                if (vertInds.size() != 2)
                    fgThrow("Cannot subdivide non-manifold mesh at vert index",toString(ii));
                ret.verts[ii] = (in.verts[ii] * 6.0 + in.verts[vertInds[0]] + in.verts[vertInds[1]]) * 0.125f;
            }
            else {
                // Note that there will always be at least 3 neighbours since 
                // this is not a boundary vertex:
                const vector<uint> &    neighbours = topo.vertNeighbours(ii);
                Vec3F    acc;
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
            Vec2UI       vertInds = topo.m_edges[ii].vertInds;
            ret.verts.push_back((in.verts[vertInds[0]]+in.verts[vertInds[1]])*0.5);
        }
    }
    Surf     ssurf = subdivideTris(allTris,allSps,topo.m_tris,newVertsBaseIdx);
    size_t          sidx = 0;
    size_t          spidx = 0;
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss) {
        Surf & surf = ret.surfaces[ss];
        const Surf & inSurf = in.surfaces[ss];
        size_t      num = inSurf.tris.size() * 4;
        cat_(surf.tris.vertInds,fgSubvec(ssurf.tris.vertInds,sidx,num));    // Clearer (and slower) than iterators
        for (size_t ii=0; ii<inSurf.surfPoints.size(); ++ii) {
            SurfPoint     sp = ssurf.surfPoints[spidx+ii];
            sp.triEquivIdx -= uint(sidx);
            surf.surfPoints.push_back(sp);
        }
        sidx += num;
        spidx += inSurf.surfPoints.size();
    }
    return ret;
}

}
