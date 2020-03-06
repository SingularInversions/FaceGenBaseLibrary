//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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

    MarkedVert(uint i,String const & l) : idx(i), label(l) {}

    bool operator==(uint rhs) const
    {return (idx == rhs); }

    bool operator==(String const & rhs) const
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
    Mesh(Vec3Fs const & vts) : verts(vts) {}

    explicit
    Mesh(TriSurf const & ts) : verts{ts.verts}, surfaces{{Surf{ts.tris}}} {}

    explicit
    Mesh(QuadSurf const & qs) : verts{qs.verts}, surfaces{{Surf{qs.quads}}} {}

    Mesh(Vec3Fs const & vts,Vec3UIs const & ts) : verts(vts), surfaces(svec(Surf(ts))) {}

    Mesh(Vec3Fs const & vts,Vec4UIs const & quads) : verts(vts), surfaces{{Surf{quads}}} {}

    Mesh(Vec3Fs const & vts,Surf const & surf) : verts(vts), surfaces(svec(surf)) {}

    Mesh(Vec3Fs const & vts,const Surfs & surfs) : verts(vts), surfaces(surfs) {}

    // Total number of verts including target morph verts:
    size_t
    totNumVerts() const;

    // Return base verts plus all target morph verts:
    Vec3Fs
    allVerts() const;

    // Update base verts and all target morph verts:
    void
    updateAllVerts(Vec3Fs const &);

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

    Surf const &
    surface(Ustring const & surfName) const
    {return findFirst(surfaces,surfName); }

    size_t
    surfPointNum() const;              // Over all surfaces

    Vec3F
    surfPointPos(Vec3Fs const & verts,size_t num) const;

    Vec3F
    surfPointPos(size_t num) const
    {return surfPointPos(verts,num); }

    Opt<Vec3F>
    surfPointPos(String const & label) const;

    LabelledVerts
    surfPointsAsVertLabels() const;

    Vec3Fs
    surfPointPositions(Strings const & labels) const;

    Vec3Fs
    surfPointPositions() const;

    Vec3F
    markedVertPos(String const & name_) const
    {return verts[findFirst(markedVerts,name_).idx]; }

    Vec3Fs
    markedVertPositions() const;        // Return positions of all marked verts

    void
    addMarkedVert(Vec3F pos,String const & label)
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
    findDeltaMorph(Ustring const & name) const;

    Valid<size_t>
    findTargMorph(Ustring const & name) const;

    // Return the combined morph index:
    Valid<size_t>
    findMorph(Ustring const & name) const;

    // Morph using member base and target vertices:
    void
    morph(
        const Floats &      coord,
        Vec3Fs &            outVerts)       // RETURNED
        const;

    // Morph using given base and target vertices:
    void
    morph(
        Vec3Fs const &      allVerts,       // Must have same number of verts as base plus targets
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
    addDeltaMorph(Morph const & deltaMorph);

    // Overwrites any existing morph of the same name:
    void
    addDeltaMorphFromTarget(Ustring const & name,Vec3Fs const & targetShape);

    // Overwrites any existing morph of the same name:
    void
    addTargMorph(const IndexedMorph & morph);

    // Overwrites any existing morph of the same name:
    void
    addTargMorph(Ustring const & name,Vec3Fs const & targetShape);

    Vec3Fs
    poseShape(Vec3Fs const & allVerts,const std::map<Ustring,float> & poseVals) const;

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
operator<<(std::ostream &,Mesh const &);

void    fgReadp(std::istream &,Mesh &);
void    fgWritep(std::ostream &,Mesh const &);

std::ostream &
operator<<(std::ostream &,Meshes const &);

Mat32F
cBounds(Meshes const & meshes);

size_t
fgNumTriEquivs(Meshes const & meshes);

std::set<Ustring>
fgMorphs(Meshes const & meshes);

inline
PoseVals
cPoseVals(Mesh const & mesh)
{return cat(cPoseVals(mesh.deltaMorphs),cPoseVals(mesh.targetMorphs)); }

PoseVals
cPoseVals(const Svec<Mesh> & meshes);

// If 'loop' not selected then just do flat subdivision:
Mesh
subdivide(Mesh const &,bool loop = true);

}

#endif
