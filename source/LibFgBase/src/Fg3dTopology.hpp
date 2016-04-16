//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Dec 14, 2009
//
// Data structures for topological analysis of a 3D triangulated surface
//

#ifndef FG3TOPOLOGY_HPP
#define FG3TOPOLOGY_HPP

#include "FgStdLibs.hpp"

#include "FgTypes.hpp"
#include "FgMatrix.hpp"
#include "FgOpt.hpp"
#include "Fg3dNormals.hpp"

struct Fg3dTopology
{
    struct      Tri
    {
        FgVect3UI           vertInds;
        FgVect3UI           edgeInds;   // In same order as verts (0-1,1-2,3-0)

        FgVect2UI
        edge(uint relIdx) const;        // Return ordered vert inds of 0,1,2 edge of tri
    };
    struct      Edge
    {
        FgVect2UI           vertInds;   // Lower index first
        vector<uint>        triInds;

        uint
        otherVertIdx(uint vertIdx) const;
    };
    struct      Vert
    {
        vector<uint>        edgeInds;   // Can be zero if vert is unused
        vector<uint>        triInds;    // "
    };
    vector<Tri>             m_tris;
    vector<Edge>            m_edges;
    vector<Vert>            m_verts;

    Fg3dTopology(
        const FgVerts &            verts,
        const vector<FgVect3UI> &  tris);

    FgVect2UI
    edgeFacingVertInds(uint edgeIdx) const;

    bool
    vertOnBoundary(uint vertIdx) const;

    // Returns a list of vertex indices which are separated by a boundary edge. This list
    // is always non-empty, and will always be of size 2 for a manifold surface:
    vector<uint>
    vertBoundaryNeighbours(uint vertIdx) const;

    vector<uint>
    vertNeighbours(uint vertIdx) const;

    // Sets of vertex indices corresponding to each connected seam.
    // A seam vertex has > 0 (always an even number for valid topologies) single-valence edges connected.
    vector<std::set<uint> >
    seams() const;

    // Returns the connected seam containing vertIdx, unless vertIdx is not on a seam in which
    // case the empty set is returned:
    std::set<uint>
    seamContaining(uint vertIdx) const;

    // Trace a fold consisting only of edges whose facet normals differ by at least 60 degrees.
    std::set<uint>
    traceFold(
        const Fg3dNormals & norms,  // Must be created from a single (unified) tri-only surface
        vector<FgBool> &    done,
        uint                vertIdx) const;

    // Returns true if all edges are either boundaries or the connection of exactly 2
    // tris with opposite winding on that edge:
    bool
    isManifold() const;

    size_t
    unusedVerts() const;

    // Returns the minimum edge distance to the given vertex for each vertex.
    // Unconnected verts will have value float_max.
    vector<float>
    edgeDistanceMap(const FgVerts & verts,size_t vertIdx) const;

    // As above where 'init' has at least 1 distance defined, the rest set to float_max:
    void
    edgeDistanceMap(const FgVerts & verts,vector<float> & init) const;

private:

    vector<uint>
    findSeam(vector<FgBool> & done) const;

    uint
    oppositeVert(uint triIdx,uint edgeIdx) const;
};

#endif

// */
