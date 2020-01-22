//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FG3DMESHOPS_HPP
#define FG3DMESHOPS_HPP

#include "FgStdLibs.hpp"
#include "FgBoostLibs.hpp"
#include "Fg3dMesh.hpp"

namespace Fg {

struct IdxDelta
{
    uint        idx;            // index into a vertex list
    Vec3F       delta;          // vertex delta from original vertex

    IdxDelta() {}
    IdxDelta(uint i,Vec3F d) : idx(i), delta(d) {}
};
typedef Svec<IdxDelta>      IdxDeltas;

// Creates 2.5D surface from depth image:
// * Vertex created for each pixel (RCS X to X, Y to Y and pixel value to Z)
// * Vertices are re-scaled to fit bounding box [0,1]^3
// * Quads created by defined pixel grid.
// * UVs created to fit bounds of image (so for perfect correspondence the depth image pixel
//   centres should fill the domain bounds wheras the texture image bounds should map the domain
//   bounds).
Mesh
fgMeshFromImage(const ImgD & img);

// Creates a sphere centred at the origin by subdividing a tetrahedron and renormalizing the 
// vertex distances from the origin 'subdivision' times:
Mesh
fgCreateSphere(float radius,uint subdivisions);

Mesh
fgRemoveDuplicateFacets(Mesh const &);

// Removes vertices & uvs that are not referenced by a surface or marked vertex.
// Retains only those morphs which affect the remaining vertices:
Mesh
meshRemoveUnusedVerts(Mesh const &);

Mesh
fg3dCube(bool open=false);

Mesh
fgPyramid(bool open=false);

Mesh
fgTetrahedron(bool open=false);

Mesh
fgOctahedron();

Mesh
fgNTent(uint nn);

//Mesh
//fgFddCage(float size,float thick);

Mesh
mergeSameNameSurfaces(Mesh const &);

Mesh
unifyIdenticalVerts(Mesh const &);

Mesh
unifyIdenticalUvs(Mesh const &);

Mesh
splitSurfsByUvs(Mesh const &);

// Merge surfaces in meshes with identically sized vertex lists,
// keeping the vertex list of the first mesh:
Mesh
mergeMeshSurfaces(Mesh const & m0,Mesh const & m1);

// Material of second mesh is discarded, all else (including names) are merged:
Mesh
mergeMeshes(
    Mesh const &    m0,
    Mesh const &    m1);

// As above:
Mesh
mergeMeshes(const Svec<Mesh> & meshes);

// Doesn't preserve uvs, surface points or marked verts:
Mesh
fg3dMaskFromUvs(Mesh const & mesh,const Img<FgBool> & mask);

// Binary image of which texels (centre point) are in the mesh UV layout (0 - no map, 255 - map):
ImgUC
getUvCover(Mesh const & mesh,Vec2UI dims);

// Wireframe image of UV layout of each facet:
ImgC4UC
cUvWireframeImage(Mesh const &,RgbaUC wireColor,const ImgC4UC & background=ImgC4UC());

// Emboss the given pattern onto a mesh with UVs, with max magnitude given by image value 255,
// corresponding to a displacement (in the direction of surface normal) by 'ratio' times the
// max bounding box dimensions of all vertices whose UV coordinate in 'pattern' sample to non-zero:
Vec3Fs
embossMesh(Mesh const & mesh,const ImgUC & pattern,double ratio=0.05);

struct  MorphVal
{
    Ustring         name;
    float           val;            // 0 - no change, 1 - full application

    MorphVal() {}
    MorphVal(Ustring const & name_,float val_) : name(name_), val(val_) {}
};
typedef Svec<MorphVal>  MorphVals;

// Only applies those morphs which mesh supports, ignores the rest:
Vec3Fs
poseMesh(Mesh const & mesh,MorphVals const &  morphVals);

// Cannot be functional since marked verts are stored as indices into the vertex array, which must
// be updated in sync:
void
surfPointsToMarkedVerts_(Mesh const & in,Mesh & out);

struct  MeshMirror
{
    Mesh        mesh;
    // For each vertex, contains the index of it's mirror vertex (which is itself for vertices on saggital plane):
    Uints       mirrorInds;
};
// Mirrors geometry around X=0 plane. All input verts must have X>=0. Tris only.
// Surface points preserved but not mirrored. Morphs removed.
MeshMirror
meshMirrorX(Mesh const &);

// Copy the surface assignment (tris only) between aligned meshes of different topology:
Mesh
fgCopySurfaceStructure(Mesh const & from,Mesh const & to);

// Merge all surface facets converted to tris:
Svec<Vec3UI>
fgMeshSurfacesAsTris(Mesh const &);

TriSurf
fgTriSurface(Mesh const & src,size_t surfIdx);

Mesh
sortTransparentFaces(Mesh const & src,ImgC4UC const & albedo,Mesh const & opaque);

}

#endif
