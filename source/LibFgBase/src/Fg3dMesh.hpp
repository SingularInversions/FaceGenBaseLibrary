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
// Floats are used for vertex positions due to size. Note that in a 1 metre 
// square volume, float precision gives at least 1 micron resolution. Since the thinnest human
// hairs are 17 microns (the thickest 180) float is adequate for most applications.
//

#ifndef FG3DMESH_HPP
#define FG3DMESH_HPP

#include "FgStdLibs.hpp"

#include "Fg3dPose.hpp"

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

struct  Fg3dMesh
{
    FgString                    name;           // Optional
    FgVerts                     verts;          // Base shape
    FgVect2Fs                   uvs;            // Texture coordinates in OTCS
    vector<Fg3dSurface>         surfaces;
    vector<FgMorph>             deltaMorphs;
    vector<FgIndexedMorph>      targetMorphs;
    FgMarkedVerts               markedVerts;

    Fg3dMesh() {}

    explicit
    Fg3dMesh(const FgVerts & vts) : verts(vts) {}

    explicit
    Fg3dMesh(const FgTriSurf & ts) : verts(ts.verts), surfaces(fgSvec(Fg3dSurface(ts.tris))) {}

    Fg3dMesh(const FgVerts & vts,const FgVect3UIs & ts) : verts(vts), surfaces(fgSvec(Fg3dSurface(ts))) {}

    Fg3dMesh(const FgVerts & vts,const Fg3dSurface & surf) : verts(vts), surfaces(fgSvec(surf)) {}

    Fg3dMesh(const FgVerts & vts,const Fg3dSurfaces & surfs) : verts(vts), surfaces(surfs) {}

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

    const Fg3dSurface &
    surface(const FgString & surfName) const
    {return fgFindFirst(surfaces,surfName); }

    size_t
    surfPointNum() const;              // Over all surfaces

    FgVect3F
    surfPointPos(const FgVerts & verts,size_t num) const;

    FgVect3F
    surfPointPos(size_t num) const
    {return surfPointPos(verts,num); }

    FgOpt<FgVect3F>
    surfPointPos(const string & label) const;

    FgLabelledVerts
    surfPointsAsVertLabels() const;

    FgVerts
    surfPointPositions(const FgStrs & labels) const;

    FgVect3F
    markedVertPos(const string & name_) const
    {return verts[fgFindFirst(markedVerts,name_).idx]; }

    void
    addMarkedVert(FgVect3F pos,const string & label)
    {
        markedVerts.push_back(FgMarkedVert(uint(verts.size()),label));
        verts.push_back(pos);
    }

    vector<boost::shared_ptr<FgImgRgbaUb> >
    albedoMaps() const
    {
        vector<boost::shared_ptr<FgImgRgbaUb> > ret;
        ret.reserve(surfaces.size());
        for (size_t ss=0; ss<surfaces.size(); ++ss)
            ret.push_back(surfaces[ss].material.albedoMap);
        return ret;
    }

    uint
    numValidAlbedoMaps() const
    {
        uint        ret = 0;
        for (size_t ss=0; ss<surfaces.size(); ++ss)
            if (surfaces[ss].material.albedoMap)
                ++ret;
        return ret;
    }

    vector<FgImgRgbaUb>
    albedoMapsOld() const
    {
        vector<FgImgRgbaUb>     ret;
        ret.reserve(surfaces.size());
        for (size_t ss=0; ss<surfaces.size(); ++ss) {
            if (surfaces[ss].material.albedoMap)
                ret.push_back(*surfaces[ss].material.albedoMap);
            else
                ret.push_back(FgImgRgbaUb());
        }
        return ret;
    }

    FgTriSurf
    asTriSurf() const;

    // MORPHS:

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

    FgVerts
    poseShape(const FgVerts & allVerts,const std::map<FgString,float> & poseVals) const;

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
    removeUVs();

    // Throws if the mesh is not valid:
    void
    checkValidity();
};

std::ostream &
operator<<(std::ostream &,const Fg3dMesh &);

void    fgReadp(std::istream &,Fg3dMesh &);
void    fgWritep(std::ostream &,const Fg3dMesh &);

typedef std::vector<Fg3dMesh>   Fg3dMeshes;

std::ostream &
operator<<(std::ostream &,const Fg3dMeshes &);

FgMat32F
fgBounds(const Fg3dMeshes & meshes);

size_t
fgNumTriEquivs(const Fg3dMeshes & meshes);

std::set<FgString>
fgMorphs(const vector<Fg3dMesh> & meshes);

inline
FgPoses
fgPoses(const Fg3dMesh & mesh)
{return fgCat(fgPoses(mesh.deltaMorphs),fgPoses(mesh.targetMorphs)); }

FgPoses
fgPoses(const vector<Fg3dMesh> & meshes);

inline
FgAffineCw2F
fgOtcsToIpcs(FgVect2UI imgDims)
{return FgAffineCw2F(FgMat22F(0,1,1,0),FgMat22F(0,imgDims[0],0,imgDims[1])); }

// If 'loop' not selected then just do flat subdivision:
Fg3dMesh
fgSubdivide(const Fg3dMesh &,bool loop = true);

#endif
