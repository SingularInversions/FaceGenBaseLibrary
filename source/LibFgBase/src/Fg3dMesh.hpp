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
#include "FgStdStream.hpp"
#include "FgOpt.hpp"

// Delta morphs and target morphs will have the same effect when applied to the base
// face on which they were defined but will have very different effects when applied
// to a face created by an statistical shape model:

struct  FgMorph
{
    FgString            name;
    FgVerts             verts;      // 1-1 correspondence with base verts

    FgMorph() {}
    FgMorph(const FgString & n,const FgVerts & v)
        : name(n), verts(v) {}

    void
    applyAsDelta(FgVerts & accVerts,float val) const
    {
        FGASSERT(verts.size() == accVerts.size());
        for (size_t ii=0; ii<verts.size(); ++ii)
            accVerts[ii] += verts[ii] * val;
    }
};

void    fgReadp(std::istream &,FgMorph &);
void    fgWritep(std::ostream &,const FgMorph &);

struct  FgIndexedMorph
{
    FgString            name;
    FgUints             baseInds;   // Indices of base vertices to be morphed.
    // Can represent target position or delta depending on type of morph.
    // Must be same size() as baseInds.:
    FgVerts             verts;

    void
    applyAsTarget(const FgVerts & baseVerts,FgVerts & accVerts,float val) const
    {
        for (size_t ii=0; ii<baseInds.size(); ++ii) {
            uint        idx = baseInds[ii];
            FgVect3F    del = verts[ii] - baseVerts[idx];
            accVerts[idx] += del * val;
        }
    }

    // Name of morph does not affect equality:
    bool
    operator==(const FgIndexedMorph & rhs) const
    {return ((baseInds == rhs.baseInds) && (verts == rhs.verts)); }
};

void    fgReadp(std::istream &,FgIndexedMorph &);
void    fgWritep(std::ostream &,const FgIndexedMorph &);

void
fgAccDeltaMorphs(
    const vector<FgMorph> &     deltaMorphs,
    const FgFlts &              coord,
    FgVerts &                   accVerts);  // MODIFIED: morphing delta accumualted here

// This version of target morph application is more suited to SSM dataflow, where the
// target positions have been transformed as part of the 'allVerts' array:
void
fgAccTargetMorphs(
    const FgVerts &             allVerts,   // Base verts plus all target morph verts
    const vector<FgIndexedMorph> & targMorphs,  // Only 'baseInds' is used.
    const FgFlts &              coord,      // morph coefficient for each target morph
    FgVerts &                   accVerts);  // MODIFIED: target morphing delta accumulated here

struct  FgMarkedVert
{
    uint        idx;
    string      label;

    FgMarkedVert() {}

    explicit
    FgMarkedVert(uint i) : idx(i) {}

    FgMarkedVert(uint i,const string & l) : idx(i), label(l) {}

    bool operator==(uint rhs) const
    {return (idx == rhs); }

    bool operator==(const string & rhs) const
    {return (label == rhs); }
};

void    fgReadp(std::istream &,FgMarkedVert &);
void    fgWritep(std::ostream &,const FgMarkedVert &);

typedef vector<FgMarkedVert>    FgMarkedVerts;

struct  FgMaterial 
{
    bool    shiny;
    FgMaterial() : shiny(false) {}
};

struct  Fg3dMesh
{
    FgString                    name;           // Optional
    FgVerts                     verts;          // Base shape
    FgUvs                       uvs;            // Texture coordinates in OTCS
    vector<Fg3dSurface>         surfaces;
    vector<FgMorph>             deltaMorphs;
    vector<FgIndexedMorph>      targetMorphs;
    FgMarkedVerts               markedVerts;
    FgMaterial                  material;

    Fg3dMesh()
    {}

    Fg3dMesh(
        const FgVerts &                 verts,
        const Fg3dSurface &             surf);

    Fg3dMesh(
        const FgVerts &                 verts,
        const vector<Fg3dSurface> &     surfaces);

    Fg3dMesh(
        const FgVerts &                 verts,
        const vector<FgVect2F> &        uvs,
        const vector<Fg3dSurface> &     surfaces,
        const vector<FgMorph> &         morphs=vector<FgMorph>());

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

    size_t
    numTris() const;                    // Just the number of tris over all surfaces

    size_t
    numQuads() const;                   // Just the number of quads over all surfaces

    size_t
    surfPointNum() const;              // Over all surfaces

    FgVect3F
    surfPointPos(const FgVerts & verts,size_t num) const;

    FgVect3F
    surfPointPos(size_t num) const
    {return surfPointPos(verts,num); }

    FgOpt<FgVect3F>
    surfPointPos(const string & label) const;

    vector<FgVertLabel>
    surfPointsAsVertLabels() const;

    FgVect3F
    markedVertPos(const string & name) const
    {return verts[fgFindFirst(markedVerts,name).idx]; }

    vector<boost::shared_ptr<FgImgRgbaUb> >
    albedoMaps() const
    {
        vector<boost::shared_ptr<FgImgRgbaUb> > ret;
        ret.reserve(surfaces.size());
        for (size_t ss=0; ss<surfaces.size(); ++ss)
            ret.push_back(surfaces[ss].albedoMap);
        return ret;
    }

    uint
    numValidAlbedoMaps() const
    {
        uint        ret = 0;
        for (size_t ss=0; ss<surfaces.size(); ++ss)
            if (surfaces[ss].albedoMap)
                ++ret;
        return ret;
    }

    vector<FgImgRgbaUb>
    albedoMapsOld() const
    {
        vector<FgImgRgbaUb>     ret;
        ret.reserve(surfaces.size());
        for (size_t ss=0; ss<surfaces.size(); ++ss) {
            if (surfaces[ss].albedoMap)
                ret.push_back(*surfaces[ss].albedoMap);
            else
                ret.push_back(FgImgRgbaUb());
        }
        return ret;
    }

    // MORPHS:

    size_t
    numTargetMorphVerts() const;

    size_t
    numMorphs() const
    {return deltaMorphs.size() + targetMorphs.size(); }

    FgString
    morphName(size_t idx) const;

    FgStrings
    morphNames() const;

    FgValid<size_t>
    findDeltaMorph(const FgString & name) const;

    FgValid<size_t>
    findTargMorph(const FgString & name) const;

    FgValid<size_t>
    findMorph(const FgString & name) const;

    // Morph using member base and target vertices:
    void
    morph(
        const FgFlts &      coord,
        FgVerts &           outVerts)       // RETURNED
        const;

    // Morph using given base and target vertices:
    void
    morph(
        const FgVerts &     allVerts,       // Must have same number of verts as base plus targets
        const FgFlts &      coord,
        FgVerts &           outVerts)       // RETURNED. Same size as base verts
        const;

    // Morph using member base and target vertices:
    FgVerts
    morph(
        const FgFlts &      deltaMorphCoord,
        const FgFlts &      targMorphCoord)
        const;

    // Apply just a single morph by its universal index (ie over deltas & targets):
    FgVerts
    morphSingle(size_t idx,float val = 1.0f) const;

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

    void
    mergeAllSurfaces();

    void
    convertToTris();

    void
    checkConsistency();
};

void    fgReadp(std::istream &,Fg3dMesh &);
void    fgWritep(std::ostream &,const Fg3dMesh &);

typedef std::vector<Fg3dMesh>   Fg3dMeshes;

Fg3dMesh
fg3dMesh(const FgVerts &);

std::set<FgString>
fgMorphs(const vector<Fg3dMesh> & meshes);

std::ostream &
operator<<(std::ostream &,const Fg3dMesh &);

std::ostream &
operator<<(std::ostream &,const Fg3dMeshes &);

inline
FgAffineCw2F
fgOtcsToIpcs(FgVect2UI imgDims)
{return FgAffineCw2F(FgMat22F(0,1,1,0),FgMat22F(0,imgDims[0],0,imgDims[1])); }

// If 'loop' not selected then just do flat subdivision:
Fg3dMesh
fgSubdivide(const Fg3dMesh &,bool loop = true);

#endif
