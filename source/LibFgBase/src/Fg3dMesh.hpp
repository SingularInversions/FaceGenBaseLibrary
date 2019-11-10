//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
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

namespace Fg {

struct  MarkedVert
{
    uint        idx;
    String      label;

    MarkedVert() {}

    explicit
    MarkedVert(uint i) : idx(i) {}

    MarkedVert(uint i,const String & l) : idx(i), label(l) {}

    bool operator==(uint rhs) const
    {return (idx == rhs); }

    bool operator==(const String & rhs) const
    {return (label == rhs); }
};

void    fgReadp(std::istream &,MarkedVert &);
void    fgWritep(std::ostream &,const MarkedVert &);

typedef Svec<MarkedVert>    MarkedVerts;

struct  Mesh
{
    Ustring                 name;           // Optional. Not loaded/saved to file. Useful when passing mesh as arg
    Vec3Fs                  verts;          // Base shape
    Vec2Fs                  uvs;            // Texture coordinates in OTCS
    Surfs                   surfaces;
    Morphs                  deltaMorphs;
    IndexedMorphs           targetMorphs;
    MarkedVerts             markedVerts;

    Mesh() {}

    explicit
    Mesh(const Vec3Fs & vts) : verts(vts) {}

    explicit
    Mesh(const TriSurf & ts) : verts(ts.verts), surfaces(fgSvec(Surf(ts.tris))) {}

    Mesh(const Vec3Fs & vts,const Vec3UIs & ts) : verts(vts), surfaces(fgSvec(Surf(ts))) {}

    Mesh(const Vec3Fs & vts,const Surf & surf) : verts(vts), surfaces(fgSvec(surf)) {}

    Mesh(const Vec3Fs & vts,const Surfs & surfs) : verts(vts), surfaces(surfs) {}

    // Total number of verts including target morph verts:
    size_t
    totNumVerts() const;

    // Return base verts plus all target morph verts:
    Vec3Fs
    allVerts() const;

    // Update base verts and all target morph verts:
    void
    updateAllVerts(const Vec3Fs &);

    uint
    numFacets() const;                  // tris plus quads over all surfaces

    uint
    numTriEquivs() const;               // tris plus 2*quads over all surfaces

    Vec3UI
    getTriEquivPosInds(uint idx) const;

    FacetInds<3>
    getTriEquivs() const;               // Over all surfaces

    size_t
    numTris() const;                    // Just the number of tris over all surfaces

    size_t
    numQuads() const;                   // Just the number of quads over all surfaces

    const Surf &
    surface(const Ustring & surfName) const
    {return fgFindFirst(surfaces,surfName); }

    size_t
    surfPointNum() const;              // Over all surfaces

    Vec3F
    surfPointPos(const Vec3Fs & verts,size_t num) const;

    Vec3F
    surfPointPos(size_t num) const
    {return surfPointPos(verts,num); }

    Opt<Vec3F>
    surfPointPos(const String & label) const;

    LabelledVerts
    surfPointsAsVertLabels() const;

    Vec3Fs
    surfPointPositions(const Strings & labels) const;

    Vec3F
    markedVertPos(const String & name_) const
    {return verts[fgFindFirst(markedVerts,name_).idx]; }

    Vec3Fs
    markedVertPositions() const;        // Return positions of all marked verts

    void
    addMarkedVert(Vec3F pos,const String & label)
    {
        markedVerts.push_back(MarkedVert(uint(verts.size()),label));
        verts.push_back(pos);
    }

    Svec<std::shared_ptr<ImgC4UC> >
    albedoMaps() const
    {
        Svec<std::shared_ptr<ImgC4UC> > ret;
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

    Svec<ImgC4UC>
    albedoMapsOld() const
    {
        Svec<ImgC4UC>     ret;
        ret.reserve(surfaces.size());
        for (size_t ss=0; ss<surfaces.size(); ++ss) {
            if (surfaces[ss].material.albedoMap)
                ret.push_back(*surfaces[ss].material.albedoMap);
            else
                ret.push_back(ImgC4UC());
        }
        return ret;
    }

    TriSurf
    asTriSurf() const;

    // MORPHS:

    size_t
    numMorphs() const
    {return deltaMorphs.size() + targetMorphs.size(); }

    Ustring
    morphName(size_t idx) const;

    Ustrings
    morphNames() const;

    Valid<size_t>
    findDeltaMorph(const Ustring & name) const;

    Valid<size_t>
    findTargMorph(const Ustring & name) const;

    // Return the combined morph index:
    Valid<size_t>
    findMorph(const Ustring & name) const;

    // Morph using member base and target vertices:
    void
    morph(
        const Floats &      coord,
        Vec3Fs &            outVerts)       // RETURNED
        const;

    // Morph using given base and target vertices:
    void
    morph(
        const Vec3Fs &      allVerts,       // Must have same number of verts as base plus targets
        const Floats &      coord,          // Combined morph coordinate over delta then targer morphs
        Vec3Fs &            outVerts)       // RETURNED. Same size as base verts
        const;

    // Morph using member base and target vertices:
    Vec3Fs
    morph(
        const Floats &      deltaMorphCoord,
        const Floats &      targMorphCoord)
        const;

    // Apply just a single morph by its universal index (ie over deltas & targets):
    Vec3Fs
    morphSingle(size_t idx,float val = 1.0f) const;

    IndexedMorph
    getMorphAsIndexedDelta(size_t idx) const;

    // Overwrites any existing morph of the same name:
    void
    addDeltaMorph(const Morph & deltaMorph);

    // Overwrites any existing morph of the same name:
    void
    addDeltaMorphFromTarget(const Ustring & name,const Vec3Fs & targetShape);

    // Overwrites any existing morph of the same name:
    void
    addTargMorph(const IndexedMorph & morph);

    // Overwrites any existing morph of the same name:
    void
    addTargMorph(const Ustring & name,const Vec3Fs & targetShape);

    Vec3Fs
    poseShape(const Vec3Fs & allVerts,const std::map<Ustring,float> & poseVals) const;

    // EDITING:

    void
    addSurfaces(const Surfs & s);

    void
    transform(Mat33F xform);

    void
    transform(Affine3F xform);

    void
    convertToTris();

    void
    removeUVs();

    // Throws if the mesh is not valid:
    void
    checkValidity();
};

typedef Svec<Mesh>   Meshes;

std::ostream &
operator<<(std::ostream &,const Mesh &);

void    fgReadp(std::istream &,Mesh &);
void    fgWritep(std::ostream &,const Mesh &);

std::ostream &
operator<<(std::ostream &,const Meshes &);

Mat32F
cBounds(const Meshes & meshes);

size_t
fgNumTriEquivs(const Meshes & meshes);

std::set<Ustring>
fgMorphs(const Meshes & meshes);

inline
PoseVals
fgPoses(const Mesh & mesh)
{return cat(fgPoses(mesh.deltaMorphs),fgPoses(mesh.targetMorphs)); }

PoseVals
fgPoses(const Svec<Mesh> & meshes);

inline
AffineEw2F
fgOtcsToIpcs(Vec2UI imgDims)
{return AffineEw2F(Mat22F(0,1,1,0),Mat22F(0,imgDims[0],0,imgDims[1])); }

// If 'loop' not selected then just do flat subdivision:
Mesh
fgSubdivide(const Mesh &,bool loop = true);

}

#endif
