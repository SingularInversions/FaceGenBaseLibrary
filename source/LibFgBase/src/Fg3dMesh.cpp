//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 22, 2005
//

#include "stdafx.h"

#include "FgMatrix.hpp"
#include "Fg3dMesh.hpp"
#include "FgException.hpp"
#include "FgStdStream.hpp"
#include "FgBounds.hpp"
#include "FgMath.hpp"
#include "Fg3dTopology.hpp"
#include "FgStdSet.hpp"

using namespace std;

void
fgAccDeltaMorphs(
    const vector<FgMorph> &     deltaMorphs,
    const FgFloats &            coord,
    FgVerts &                   accVerts)
{
    FGASSERT(deltaMorphs.size() == coord.size());
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii) {
        const FgMorph &     morph = deltaMorphs[ii];
        FGASSERT(morph.verts.size() == accVerts.size());
        for (size_t jj=0; jj<accVerts.size(); ++jj)
            accVerts[jj] += morph.verts[jj] * coord[ii];
    }
}

void
fgAccTargetMorphs(
    const FgVerts &             allVerts,
    const vector<FgIndexedMorph> & targMorphs,
    const vector<float> &       coord,
    FgVerts &                   accVerts)
{
    FGASSERT(targMorphs.size() == coord.size());
    size_t          numTargVerts = 0;
    for (size_t ii=0; ii<targMorphs.size(); ++ii)
        numTargVerts += targMorphs[ii].baseInds.size();
    FGASSERT(accVerts.size() + numTargVerts == allVerts.size());
    size_t          idx = accVerts.size();
    for (size_t ii=0; ii<targMorphs.size(); ++ii) {
        const FgUints &     inds = targMorphs[ii].baseInds;
        for (size_t jj=0; jj<inds.size(); ++jj) {
            size_t          baseIdx = inds[jj];
            FgVect3F        del = allVerts[idx++] - allVerts[baseIdx];
            accVerts[baseIdx] += del * coord[ii];
        }
    }
}

Fg3dMesh::Fg3dMesh(
    const FgVerts &                     verts,
    const Fg3dSurface &                 surf)
    :
    verts(verts),
    surfaces(fgSvec(surf))
{
    checkConsistency();
}

Fg3dMesh::Fg3dMesh(
    const FgVerts &                     verts,
    const std::vector<Fg3dSurface> &    surfaces)
    :
    verts(verts)
{
    addSurfaces(surfaces);
    checkConsistency();
}

Fg3dMesh::Fg3dMesh(
    const FgVerts &                     verts,
    const std::vector<FgVect2F> &       uvs,
    const std::vector<Fg3dSurface> &    surfaces,
    const std::vector<FgMorph> &        morphs)
    : 
    verts(verts),
    uvs(uvs),
    deltaMorphs(morphs)
{
    addSurfaces(surfaces);
    checkConsistency();
}

size_t
Fg3dMesh::totNumVerts() const
{
    size_t  ret = verts.size();
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        ret += targetMorphs[ii].verts.size();
    return ret;
}

FgVerts
Fg3dMesh::allVerts() const
{
    FgVerts     ret = verts;
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        fgAppend(ret,targetMorphs[ii].verts);
    return ret;
}

void
Fg3dMesh::updateAllVerts(const FgVerts & avs)
{
    FGASSERT(avs.size() == totNumVerts());
    size_t          idx = 0;
    for (; idx<verts.size(); ++idx)
        verts[idx] = avs[idx];
    for (size_t ii=0; ii<targetMorphs.size(); ++ii) {
        FgIndexedMorph & tm = targetMorphs[ii];
        for (size_t jj=0; jj<tm.verts.size(); ++jj)
            tm.verts[jj] = avs[idx++];
    }
}

uint
Fg3dMesh::numFacets() const
{
    uint    tot = 0;
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        tot += surfaces[ss].numFacets();
    return tot;
}

uint        Fg3dMesh::numTriEquivs() const
{
    uint        tot = 0;
    for (uint ss=0; ss<surfaces.size(); ss++)
        tot += surfaces[ss].numTriEquivs();
    return tot;
}

FgVect3UI
Fg3dMesh::getTriEquiv(uint idx) const
{
    for (size_t ss=0; ss<surfaces.size(); ++ss) {
        uint        num = surfaces[ss].numTriEquivs();
        if (idx < num)
            return surfaces[ss].getTriEquiv(idx);
        else
            idx -= num;
    }
    FGASSERT_FALSE;
    return FgVect3UI();
}

FgFacetInds<3>
Fg3dMesh::getTriEquivs() const
{
    FgFacetInds<3>      ret;
    if (surfaces.empty())
        return ret;
    ret = surfaces[0].getTriEquivs();
    for (size_t ii=1; ii<surfaces.size(); ++ii)
        fgAppend(ret,surfaces[ii].getTriEquivs());
    return ret;
}

size_t
Fg3dMesh::numTris() const
{
    size_t      ret = 0;
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        ret += surfaces[ss].tris.vertInds.size();
    return ret;
}

size_t
Fg3dMesh::numQuads() const
{
    size_t      ret = 0;
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        ret += surfaces[ss].quads.vertInds.size();
    return ret;
}

uint        Fg3dMesh::numSurfPoints() const
{
    uint        tot = 0;
    for (uint ss=0; ss<surfaces.size(); ss++)
        tot += surfaces[ss].numSurfPoints();
    return tot;
}

FgOpt<FgVect3F>
Fg3dMesh::surfPoint(const string & label) const
{
    FgOpt<FgVect3F>    ret;
    for (size_t ss=0; ss<surfaces.size(); ++ss) {
        size_t  idx = fgFindFirstIdx(surfaces[ss].surfPoints,label);
        if (idx < surfaces[ss].surfPoints.size()) {
            ret = surfaces[ss].getSurfPoint(verts,idx);
            break;
        }
    }
    return ret;
}

vector<FgVertLabel>
Fg3dMesh::surfPointsAsVertLabels() const
{
    vector<FgVertLabel>     ret;
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        fgAppend(ret,surfaces[ss].surfPointsAsVertLabels(verts));
    return ret;
}

size_t
Fg3dMesh::numTargetMorphVerts() const
{
    size_t      ret = 0;
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        ret += targetMorphs[ii].verts.size();
    return ret;
}

FgString
Fg3dMesh::morphName(size_t idx) const
{
    if (idx < deltaMorphs.size())
        return deltaMorphs[idx].name;
    idx -= deltaMorphs.size();
    return targetMorphs[idx].name;
}

vector<FgString>
Fg3dMesh::morphNames() const
{
    vector<FgString>    ret;
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        ret.push_back(deltaMorphs[ii].name);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        ret.push_back(targetMorphs[ii].name);
    return ret;
}

FgValid<size_t>
Fg3dMesh::findDeltaMorph(const FgString & name) const
{
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        if (FgString(deltaMorphs[ii].name) == name)
            return FgValid<size_t>(ii);
    return FgValid<size_t>();
}

FgValid<size_t>
Fg3dMesh::findTargMorph(const FgString & name) const
{
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        if (targetMorphs[ii].name == name)
            return FgValid<size_t>(ii);
    return FgValid<size_t>();
}

FgValid<size_t>
Fg3dMesh::findMorph(const FgString & name) const
{
    for (size_t ii=0; ii<numMorphs(); ++ii)
        if (morphName(ii) == name)
            return FgValid<size_t>(ii);
    return FgValid<size_t>();
}

void
Fg3dMesh::morph(
    const vector<float> &   morphCoord,
    FgVerts &               outVerts) const
{
    FGASSERT(morphCoord.size() == numMorphs());
    outVerts = verts;
    size_t      cnt = 0;
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        deltaMorphs[ii].applyAsDelta(outVerts,morphCoord[cnt++]);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        targetMorphs[ii].applyAsTarget(verts,outVerts,morphCoord[cnt++]);
}

void
Fg3dMesh::morph(
    const FgVerts &     allVerts,
    const FgFloats &    coord,
    FgVerts &           outVerts) const
{
    outVerts = fgHead(allVerts,verts.size());
    size_t          ndms = deltaMorphs.size();
    fgAccDeltaMorphs(deltaMorphs,fgHead(coord,ndms),outVerts);
    fgAccTargetMorphs(allVerts,targetMorphs,fgRest(coord,ndms),outVerts);
}

void
Fg3dMesh::morph(
    const vector<float> &   deltaMorphCoord,
    const vector<float> &   targMorphCoord,
    FgVerts &               outVerts) const
{
    FGASSERT(deltaMorphCoord.size() == deltaMorphs.size());
    FGASSERT(targMorphCoord.size() == targetMorphs.size());
    outVerts = verts;
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        deltaMorphs[ii].applyAsDelta(outVerts,deltaMorphCoord[ii]);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        targetMorphs[ii].applyAsTarget(verts,outVerts,targMorphCoord[ii]);
}

FgVerts
Fg3dMesh::morphSingle(size_t idx,float val) const
{
    FgVerts     ret = verts;
    if (idx < deltaMorphs.size())
        deltaMorphs[idx].applyAsDelta(ret,val);
    else {
        idx -= deltaMorphs.size();
        FGASSERT(idx < targetMorphs.size());
        targetMorphs[idx].applyAsTarget(verts,ret,val);
    }
    return ret;
}

FgIndexedMorph
Fg3dMesh::getMorphAsIndexedDelta(size_t idx) const
{
    float               tol = fgSqr(fgMaxElem(fgDims(verts)) * 0.0001);
    FgIndexedMorph      ret;
    if (idx < deltaMorphs.size()) {
        const FgMorph & dm = deltaMorphs[idx];
        ret.name = dm.name;
        for (size_t ii=0; ii<dm.verts.size(); ++ii) {
            if (dm.verts[ii].lengthSqr() > tol) {
                ret.baseInds.push_back(uint32(ii));
                ret.verts.push_back(dm.verts[ii]);
            }
        }
    }
    else {
        idx -= deltaMorphs.size();
        FGASSERT(idx < targetMorphs.size());
        const FgIndexedMorph &  tm = targetMorphs[idx];
        ret.name = tm.name;
        ret.baseInds = tm.baseInds;
        ret.verts = tm.verts;
        for (size_t ii=0; ii<ret.verts.size(); ++ii)
            ret.verts[ii] -= verts[ret.baseInds[ii]];
    }
    return ret;
}

void
Fg3dMesh::addDeltaMorph(const FgMorph & morph)
{
    FgValid<size_t>         idx = findDeltaMorph(morph.name);
    if (idx.valid()) {
        fgout << fgnl << "WARNING: Overwriting existing morph " + morph.name.as_ascii();
        deltaMorphs[idx.val()] = morph;
    }
    else
        deltaMorphs.push_back(morph);
}

void
Fg3dMesh::addDeltaMorphFromTarget(const FgString & name,const FgVerts & targetShape)
{
    FgMorph                 dm;
    dm.name = name;
    dm.verts = targetShape - verts;
    addDeltaMorph(dm);
}

void
Fg3dMesh::addTargMorph(const FgIndexedMorph & morph)
{
    FGASSERT(fgMax(morph.baseInds) < verts.size());
    FgValid<size_t>     idx = findTargMorph(morph.name);
    if (idx.valid()) {
        fgout << fgnl << "WARNING: Overwriting existing morph " + morph.name.as_ascii();
        targetMorphs[idx.val()] = morph;
    }
    else
        targetMorphs.push_back(morph);
}

void
Fg3dMesh::addTargMorph(const FgString & name,const FgVerts & targetShape)
{
    FGASSERT(targetShape.size() == verts.size());
    FgIndexedMorph       tm;
    tm.name = name;
    FgVerts             deltas = targetShape - verts;
    float               maxMag = 0.0f;
    for (size_t ii=0; ii<deltas.size(); ++ii)
        fgSetIfGreater(maxMag,deltas[ii].lengthSqr());
    if (maxMag == 0.0f)
        fgThrow("Attempt to create empty target morph");
    maxMag *= fgSqr(0.001f);
    for (size_t ii=0; ii<deltas.size(); ++ii) {
        if (deltas[ii].lengthSqr() > maxMag) {
            tm.baseInds.push_back(uint(ii));
            tm.verts.push_back(targetShape[ii]);
        }
    }
    addTargMorph(tm);
}

void
Fg3dMesh::addSurfaces(
    const std::vector<Fg3dSurface> & surfs)
{
    for (size_t ss=0; ss<surfs.size(); ++ss)
        surfaces.push_back(surfs[ss]);
    checkConsistency();
}

void
Fg3dMesh::transform(FgMat33F xform)
{
    fgTransform_(verts,verts,xform);
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        fgTransform_(deltaMorphs[ii].verts,deltaMorphs[ii].verts,xform);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        fgTransform_(targetMorphs[ii].verts,targetMorphs[ii].verts,xform);
}

void
Fg3dMesh::transform(FgAffine3F xform)
{
    fgTransform_(verts,verts,xform);
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii)
        fgTransform_(deltaMorphs[ii].verts,deltaMorphs[ii].verts,xform.linear);
    for (size_t ii=0; ii<targetMorphs.size(); ++ii)
        fgTransform_(targetMorphs[ii].verts,targetMorphs[ii].verts,xform);
}

Fg3dMesh
Fg3dMesh::subdivideFlat() const
{
    // Subdividing multi-surface meshes is complex and unnecessary for our needs:
    FGASSERT(surfaces.size() == 1);
    FgVerts             newVerts = verts;
    return Fg3dMesh(newVerts,surfaces[0].subdivideFlat(newVerts));
}

Fg3dMesh
Fg3dMesh::subdivideLoop() const
{
    FGASSERT(surfaces.size() == 1);
    FgVerts             newVerts;
    return Fg3dMesh(newVerts,surfaces[0].subdivideLoop(verts,newVerts));
}

Fg3dSurface
Fg3dMesh::mergedSurfaces() const
{
    Fg3dSurface     ret;
    for (size_t ii=0; ii<surfaces.size(); ++ii)
        ret.merge(surfaces[ii]);
    return ret;
}

void
Fg3dMesh::mergeAllSurfaces()
{
    if (surfaces.size() > 1) {
        for (uint ss=1; ss<surfaces.size(); ++ss)
            surfaces[0].merge(surfaces[ss]);
        surfaces.resize(1);
    }
}

void
Fg3dMesh::convertToTris()
{
    for (size_t ii=0; ii<surfaces.size(); ++ii)
        surfaces[ii] = surfaces[ii].convertToTris();
}

void
Fg3dMesh::checkConsistency()
{
    uint    numVerts = uint(verts.size());
    for (size_t ss=0; ss<surfaces.size(); ++ss)
        surfaces[ss].checkMeshConsistency(numVerts,uint(uvs.size()));
    for (size_t mm=0; mm<markedVerts.size(); ++mm)
        FGASSERT(markedVerts[mm].idx < numVerts);
}

Fg3dMesh
fg3dMesh(const FgVerts & v)
{
    Fg3dMesh    ret;
    ret.verts = v;
    return ret;
}

std::set<FgString>
fgMorphs(const vector<Fg3dMesh> & meshes)
{
    std::set<FgString>  ret;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        fgAppend(ret,meshes[ii].morphNames());
    return ret;
}

std::ostream &
operator<<(std::ostream & os,const Fg3dMesh & m)
{
    FgMat32F bounds = fgBounds(m.verts);
    os << fgnl
        << "Name: " << m.name
        << fgnl << "Verts: " << m.verts.size() << "  "
        << fgnl << "UVs: " << m.uvs.size() << "  "
        << fgnl << "Bounding Box: " << bounds
        << fgnl << "Delta Morphs: " << m.deltaMorphs.size()
        << fgnl << "Target Morphs: " << m.targetMorphs.size()
        << fgnl << "Marked Verts: " << m.markedVerts.size()
        << fgpush;
    for (size_t ii=0; ii<m.markedVerts.size(); ++ii)
        if (!m.markedVerts[ii].label.empty())
            os << fgnl << m.markedVerts[ii].label;
    os << fgpop << fgnl << "Textures: " << m.texImages.size()
        << fgpush;
    for (size_t ii=0; ii<m.texImages.size(); ++ii)
        os << fgnl << m.texImages[ii];
    os << fgpop;
    for (size_t ss=0; ss<m.surfaces.size(); ss++) {
        const Fg3dSurface &     surf = m.surfaces[ss];
        os << fgnl << "Surface " << ss << ": " << fgpush << fgnl
           << "Tris: " << surf.numTris() << "  Quads: " << surf.numQuads() << fgnl
           << "Surf Points: " << surf.numSurfPoints() << fgpush;
        for (uint ii=0; ii<surf.numSurfPoints(); ++ii)
            os << fgnl << ii << ": " << surf.surfPoints[ii].label << "  " << surf.getSurfPoint(m.verts,ii);
        os << fgpop;
        os << fgpop;
    }
    Fg3dTopology            topo(m.verts,m.getTriEquivs().vertInds);
    os << fgnl
        << "Manifold: " << (topo.isManifold() ? "yes" : "no")
        << " Seams: " << topo.seams().size()
        << " Unused verts: " << topo.unusedVerts();
    return os;
}
