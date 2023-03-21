//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Data structures for topological analysis of a triangulated surface
//

#ifndef FG3TOPOLOGY_HPP
#define FG3TOPOLOGY_HPP

#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"
#include "FgSerial.hpp"
#include "Fg3dMesh.hpp"

namespace Fg {

// Set the ordering of 'vertInds' so that the edge is in the winding direction of 'tri':
// Both values in 'vertInds' must exist in 'tri' and 'tri' must not contain duplicates:
Vec2UI              directEdgeVertInds(Vec2UI vertInds,Vec3UI tri);

struct SurfTopo
{
    struct      Tri
    {
        Vec3UI          vertInds;
        Vec3UI          edgeInds;       // In same order as verts (0-1,1-2,2-0)

        Vec2UI          edge(uint relIdx) const;        // Return ordered vert inds of 0,1,2 edge of tri
    };
    struct      Edge                    // [Shared] undirected edge
    {
        Vec2UI          vertInds;       // Lower index first
        Uints           triInds;

        uint            otherVertIdx(uint vertIdx) const;       // Of the 2 in 'vertIndx'
    };
    struct      Vert
    {
        Uints           edgeInds;   // Can be zero if vert is unused
        Uints           triInds;    // "
    };
    Svec<Tri>           m_tris;
    Svec<Edge>          m_edges;
    Svec<Vert>          m_verts;

    explicit SurfTopo(Vec3UIs const & tris);
    SurfTopo(size_t numVerts,Vec3UIs const & tris);     // checks for out of bounds vertex indices

    Vec2UI                  edgeFacingVertInds(uint edgeIdx) const;
    bool                    vertOnBoundary(uint vertIdx) const;
    // Returns a list of vertex indices which are separated by a boundary edge. This list
    // is always non-empty, and will always be of size 2 for a manifold surface:
    Uints                   vertBoundaryNeighbours(uint vertIdx) const;
    Uints                   vertNeighbours(uint vertIdx) const;

    struct      BoundEdge
    {
        uint        edgeIdx;        // A boundary edge; part of only 1 tri which determines its winding direction.
        uint        vertIdx;        // The vertex at the end of the directed edge above.
    };
    typedef Svec<BoundEdge> BoundEdges;
    // If the given vertex is part of a boundary, returns that boundary, otherwise empty:
    BoundEdges              boundaryContainingVert(uint vertIdx) const;
    // Returns boundaries on the surface in winding order (arbitrary starting point).
    // Mesh must be manifold, and thus boundaries form closed loops:
    Svec<BoundEdges>        boundaries() const;
    // Returns an array of bools 1-1 with vertices, flagging all boundary edge verts:
    Bools                   boundaryVertFlags() const;
    // Returns the outward-facing normals for each vertex in the given boundary from the given tri norms:
    Vec3Ds                  boundaryVertNormals(BoundEdges const & boundary,Vec3Ds const & verts) const;
    // Trace a fold consisting only of edges whose facet normals differ by at least 60 degrees.
    std::set<uint>
    traceFold(
        MeshNormals const & norms,  // Must be created from a single (unified) tri-only surface
        FatBools &      done,
        uint                vertIdx) const;
    // Returns number of boundary edges, intersection edges and reversed edges respectively.
    // If all are zero the mesh is watertight. If the last 2 are zero the mesh is manifold.
    Vec3UI                  isManifold() const;
    size_t                  unusedVerts() const;
    // Returns the minimum edge distance to the given vertex for each vertex.
    // Unconnected verts will have value float_max.
    Floats                  edgeDistanceMap(Vec3Fs const & verts,size_t vertIdx) const;
    // As above where 'init' has at least 1 distance defined, the rest set to float_max:
    void                    edgeDistanceMap(Vec3Fs const & verts,Floats & init) const;

private:
    void                    setup(uint numVerts,Vec3UIs const & tris);
    BoundEdges              boundaryContainingEdgeP(uint edgeIdx) const; // edgeIdx must be a boundary edge
    Uints                   findSeam(FatBools & done) const;
    uint                    oppositeVert(uint triIdx,uint edgeIdx) const;
};

struct  IdxNorm
{
    uint            vertIdx;
    Vec3D           norm;       // Normalized direction
};
typedef Svec<IdxNorm>   IdxNorms;

// Return inds of all connected verts starting at 'seedIdx' in a region bounded by marked verts,
// including the boundary marked verts:
std::set<uint>
cFillMarkedVertRegion(Mesh const &,SurfTopo const &,uint seedIdx);

}

#endif

// */
