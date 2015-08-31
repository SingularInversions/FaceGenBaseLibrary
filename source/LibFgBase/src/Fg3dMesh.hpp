//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 8, 2005
//
// Polygonal mesh with multiple surfaces sharing a vertex list.
//
// DESIGN
//
// Tris and quads only.
//
// Rendering with different vertex position arrays is supported, in which case
// the member vertex array can be a base or reference shape.
//
// There is a one-to-many relationship between the vertex array and surfaces,
// since several smoothly connected surfaces may be segemented for different
// texture images.
//
// Floats are used for vertex positions due to size but the design tries to stay agnostic
// in case we need other implementations (fixed point or double). Note that in a 1 metre 
// square volume, float precision gives at least 1 micron resolution. Since the thinnest human
// hairs are 17 microns (the thickest 180) float is adequate for most applications.
//

#ifndef FG3DMESH_HPP
#define FG3DMESH_HPP

#include "FgStdLibs.hpp"

#include "FgStdVector.hpp"
#include "FgStdString.hpp"
#include "FgMatrix.hpp"
#include "Fg3dSurface.hpp"
#include "FgImage.hpp"
#include "FgVariant.hpp"
#include "FgStdStream.hpp"
#include "FgValidVal.hpp"

// Delta morphs and target morphs will have the same effect when applied to the base
// face on which they were defined but will have very different effects when applied
// to a face created by an statistical shape model:

struct  FgMorph
{
    FgString            name;
    FgVerts             verts;      // 1-1 correspondence with base verts
    FG_SERIALIZE2(name,verts)

    void
    applyAsDelta(FgVerts & accVerts,float val) const
    {
        FGASSERT(verts.size() <= accVerts.size());
        for (size_t ii=0; ii<verts.size(); ++ii)
            accVerts[ii] += verts[ii] * val;
    }
};

struct  FgIndexedMorph
{
    FgString            name;
    FgUints             baseInds;   // Indices of base vertices to be morphed.
    // Can represent target position or delta depending on type of morph.
    // Must be same size() as baseInds.:
    FgVerts             verts;
    FG_SERIALIZE3(name,baseInds,verts)

    void
    applyAsTarget(const FgVerts & baseVerts,FgVerts & accVerts,float val) const
    {
        for (size_t ii=0; ii<baseInds.size(); ++ii) {
            uint        idx = baseInds[ii];
            FgVect3F    del = verts[ii] - baseVerts[idx];
            accVerts[idx] += del * val;
        }
    }
};

void
fgAccDeltaMorphs(
    const vector<FgMorph> &     deltaMorphs,
    const FgFloats &            coord,
    FgVerts &                   accVerts);  // MODIFIED: morphing delta accumualted here

// This version of target morph application is more suited to SSM dataflow, where the
// target positions have been transformed as part of the 'allVerts' array:
void
fgAccTargetMorphs(
    const FgVerts &             allVerts,   // Base verts plus all target morph verts
    const vector<FgIndexedMorph> & targMorphs,  // Only 'baseInds' is used.
    const FgFloats &            coord,      // morph coefficient for each target morph
    FgVerts &                   accVerts);  // MODIFIED: target morphing delta accumulated here

struct  FgMarkedVert
{
    uint        idx;
    string      label;
    FG_SERIALIZE2(idx,label)

    FgMarkedVert() : idx(0) {}

    explicit
    FgMarkedVert(uint i) : idx(i) {}

    FgMarkedVert(uint i,const string & l) : idx(i), label(l) {}

    bool operator==(uint rhs) const
    {return (idx == rhs); }

    bool operator==(const string & rhs) const
    {return (label == rhs); }
};

struct  FgMaterial 
{
    bool    shiny;

    FG_SERIALIZE1(shiny)

    FgMaterial()
    : shiny(false)
    {}
};

struct  Fg3dMesh
{
    FgString                    name;           // Optional
    FgVerts                     verts;          // Base shape
    FgUvs                       uvs;            // Texture coordinates in OTCS
    vector<Fg3dSurface>         surfaces;
    vector<FgImgRgbaUb>         texImages;      // Empty or 1-1 with 'surfaces'.
    vector<FgMorph>             deltaMorphs;
    vector<FgIndexedMorph>      targetMorphs;
    vector<FgMarkedVert>        markedVerts;
    FgMaterial                  material;
    FG_SERIALIZE8(verts,uvs,surfaces,texImages,deltaMorphs,targetMorphs,markedVerts,material)

    Fg3dMesh()
    {}

    Fg3dMesh(
        const FgVerts &                     verts,
        const Fg3dSurface &                 surf);

    Fg3dMesh(
        const FgVerts &                     verts,
        const vector<Fg3dSurface> &    surfaces);

    Fg3dMesh(
        const FgVerts &                     verts,
        const vector<FgVect2F> &       uvs,
        const vector<Fg3dSurface> &    surfaces,
        const vector<FgMorph> &        morphs=vector<FgMorph>());

    // Total number of verts including target morph verts:
    size_t
    totNumVerts() const;

    // Return base verts plus all target morph verts:
    FgVerts
    allVerts() const;

    // Update base verts and all target morph verts:
    void
    updateAllVerts(const FgVerts &);

    uint
    numFacets() const;                  // tris plus quads over all surfaces

    uint
    numTriEquivs() const;               // tris plus 2*quads over all surfaces

    FgVect3UI
    getTriEquiv(uint idx) const;

    FgFacetInds<3>
    getTriEquivs() const;               // Over all surfaces

    uint
    numSurfPoints() const;              // Over all surfaces

    FgVect3F
    getSurfPoint(uint num) const
    {return getSurfPoint(verts,num); }

    template<class T>
    FgMatrixC<T,3,1>
    getSurfPoint(const vector<FgMatrixC<T,3,1> > &verts,uint num) const;

    template<class T>
    vector<FgMatrixC<T,3,1> >
    getSurfPoints(const vector<FgMatrixC<T,3,1> > & verts) const;

    vector<FgVect3F>
    getSurfPoints() const
    {return getSurfPoints(verts); }

    FgValidVal<FgVect3F>
    surfPoint(const string & label) const;

    vector<FgVertLabel>
    surfPointsAsVertLabels() const;

    // MORPHS:

    size_t
    numTargetMorphVerts() const;

    size_t
    numMorphs() const
    {return deltaMorphs.size() + targetMorphs.size(); }

    FgString
    morphName(size_t idx) const;

    FgValid<size_t>
    findDeltaMorph(const FgString & name) const;

    FgValid<size_t>
    findTargMorph(const FgString & name) const;

    FgValid<size_t>
    findMorph(const FgString & name) const;

    // Morph using member base and target vertices:
    void
    morph(
        const FgFloats &    coord,
        FgVerts &           outVerts)       // RETURNED
        const;

    // Morph using given base and target vertices:
    void
    morph(
        const FgVerts &     allVerts,       // Must have same number of verts as base plus targets
        const FgFloats &    coord,
        FgVerts &           outVerts)       // RETURNED. Same size as base verts
        const;

    // Morph using member base and target vertices:
    void
    morph(
        const FgFloats &    deltaMorphCoord,
        const FgFloats &    targMorphCoord,
        FgVerts &           outVerts)       // RETURNED
        const;

    FgIndexedMorph
    getMorphAsIndexedDelta(size_t idx) const;

    // Overwrites any existing morph of the same name:
    void
    addDeltaMorph(const FgMorph & deltaMorph);

    // Overwrites any existing morph of the same name:
    void
    addDeltaMorphFromTarget(const FgString & name,const FgVerts & targetShape);

    // Overwrites any existing morph of the same name:
    void
    addTargMorph(const FgIndexedMorph & morph);

    // Overwrites any existing morph of the same name:
    void
    addTargMorph(const FgString & name,const FgVerts & targetShape);

    // EDITING:

    void
    addSurfaces(const vector<Fg3dSurface> & s);

    void
    transform(FgMat33F xform);

    void
    transform(FgAffine3F xform);

    Fg3dMesh
    subdivideFlat() const;

    Fg3dMesh
    subdivideLoop() const;

    Fg3dSurface
    mergedSurfaces() const;

    void
    mergeAllSurfaces();

    void
    convertToTris();

    void
    checkConsistency();
};

Fg3dMesh
fg3dMesh(const FgVerts &);

template<class T>
FgMatrixC<T,3,1>
Fg3dMesh::getSurfPoint(
    const vector<FgMatrixC<T,3,1> > &		vts,
    uint                                    num)
    const
{
    for (uint ss=0; ss<surfaces.size(); ss++)
    {
        if (num < surfaces[ss].numSurfPoints())
            return surfaces[ss].getSurfPoint(vts,num);
        else
            num -= surfaces[ss].numSurfPoints();
    }
    FGASSERT_FALSE;
    return FgMatrixC<T,3,1>(0,0,0);        // Avoid warning.
}

template<class T>
vector<FgMatrixC<T,3,1> >
Fg3dMesh::getSurfPoints(
    const vector<FgMatrixC<T,3,1> > &  verts)
    const
{
    vector<FgMatrixC<T,3,1> >  pts(numSurfPoints());
    for (size_t ii=0; ii<pts.size(); ++ii)
        pts[ii] = getSurfPoint(verts,uint(ii));
    return pts;
}

std::ostream &
operator<<(std::ostream &,const Fg3dMesh &);

#endif
