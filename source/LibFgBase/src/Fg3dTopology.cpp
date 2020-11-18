//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgStdSet.hpp"
#include "Fg3dTopology.hpp"
#include "FgOpt.hpp"
#include "FgStdVector.hpp"

using namespace std;

namespace Fg {

Vec2UI
MeshTopology::Tri::edge(uint relIdx) const
{
    if (relIdx == 0)
        return Vec2UI(vertInds[0],vertInds[1]);
    else if (relIdx == 1)
        return Vec2UI(vertInds[1],vertInds[2]);
    else if (relIdx == 2)
        return Vec2UI(vertInds[2],vertInds[0]);
    else
        FGASSERT_FALSE;
    return Vec2UI();
}

uint
MeshTopology::Edge::otherVertIdx(uint vertIdx) const
{
    if (vertIdx == vertInds[0])
        return vertInds[1];
    else if (vertIdx == vertInds[1])
        return vertInds[0];
    else
        FGASSERT_FALSE;
    return 0;       // make compiler happy
}

struct  EdgeVerts
{
    uint        loIdx;
    uint        hiIdx;

    EdgeVerts(uint i0,uint i1)
    {
        if (i0 < i1) {
            loIdx = i0;
            hiIdx = i1;
        }
        else {
            loIdx = i1;
            hiIdx = i0;
        }
        FGASSERT(i0 != i1);
    }

    // Comparison operator to use as a key for std::map:
    bool operator<(const EdgeVerts & rhs) const
    {
        if (loIdx < rhs.loIdx)
            return true;
        else if (loIdx == rhs.loIdx)
            return (hiIdx < rhs.hiIdx);
        else
            return false;
    }

    bool
    contains(uint idx) const
    {return ((idx == loIdx) || (idx == hiIdx)); }
};

struct  TriVerts
{
    Vec3UI   inds;

    TriVerts(Vec3UI i)
    {
        if (i[1] < i[0])
            std::swap(i[0],i[1]);
        if (i[2] < i[1])
            std::swap(i[1],i[2]);
        if (i[1] < i[0])
            std::swap(i[0],i[1]);
        inds = i;
    }

    bool operator<(const TriVerts & rhs) const
    {
        for (uint ii=0; ii<3; ++ii) {
            if (inds[ii] < rhs.inds[ii])
                return true;
            else if (inds[ii] == rhs.inds[ii])
                continue;
            else
                return false;
        }
        return false;
    }
};

MeshTopology::MeshTopology(size_t numVerts,Vec3UIs const & tris)
{
    // Detect null or duplicate tris:
    uint                    duplicates = 0,
                            nulls = 0;
    set<TriVerts>           vset;
    for (size_t ii=0; ii<tris.size(); ++ii) {
        Vec3UI           vis = tris[ii];
        if ((vis[0] == vis[1]) || (vis[1] == vis[2]) || (vis[2] == vis[0]))
            ++nulls;
        else {
            TriVerts            tv(vis);
            if (vset.find(tv) == vset.end()) {
                vset.insert(tv);
                Tri             tri;
                tri.vertInds = vis;
                m_tris.push_back(tri);
            }
            else
                ++duplicates;
        }
    }
    if (duplicates > 0)
        fgout << fgnl << "WARNING: Duplicate tris: " << duplicates;
    if (nulls > 0)
        fgout << fgnl << "WARNING: Null tris: " << nulls;
    m_verts.resize(numVerts);
    std::map<EdgeVerts,Uints >    edgesToTris;
    for (size_t ii=0; ii<m_tris.size(); ++ii) {
        Vec3UI       vertInds = m_tris[ii].vertInds;
        m_tris[ii].edgeInds = Vec3UI(std::numeric_limits<uint>::max());
        for (uint jj=0; jj<3; ++jj) {
            m_verts[vertInds[jj]].triInds.push_back(uint(ii));
            EdgeVerts        edge(vertInds[jj],vertInds[(jj+1)%3]);
            edgesToTris[edge].push_back(uint(ii));
        }
    }
    for (map<EdgeVerts,Uints >::const_iterator it=edgesToTris.begin(); it!=edgesToTris.end(); ++it) {
        EdgeVerts       edgeVerts = it->first;
        Edge            edge;
        edge.vertInds = Vec2UI(edgeVerts.loIdx,edgeVerts.hiIdx);
        edge.triInds = it->second;
        m_edges.push_back(edge);
    }
    for (size_t ii=0; ii<m_edges.size(); ++ii) {
        Vec2UI               vts = m_edges[ii].vertInds;
        m_verts[vts[0]].edgeInds.push_back(uint(ii));
        m_verts[vts[1]].edgeInds.push_back(uint(ii));
        EdgeVerts               edge(vts[0],vts[1]);
        Uints const &    triInds = edgesToTris.find(edge)->second;
        for (size_t jj=0; jj<triInds.size(); ++jj) {
            uint                triIdx = triInds[jj];
            Vec3UI           tri = m_tris[triIdx].vertInds;
            for (uint ee=0; ee<3; ++ee)
                if ((edge.contains(tri[ee]) && edge.contains(tri[(ee+1)%3])))
                    m_tris[triIdx].edgeInds[ee] = uint(ii);
        }
    }
    // validate:
    for (size_t ii=0; ii<m_verts.size(); ++ii) {
        Uints const &    edgeInds = m_verts[ii].edgeInds;
        if (edgeInds.size() > 1)
            for (size_t jj=0; jj<edgeInds.size(); ++jj)
                m_edges[edgeInds[jj]].otherVertIdx(uint(ii));   // throws if index ii not found in edge verts
    }
    for (size_t ii=0; ii<m_tris.size(); ++ii)
        for (uint jj=0; jj<3; ++jj)
            FGASSERT(m_tris[ii].edgeInds[jj] != std::numeric_limits<uint>::max());
    for (size_t ii=0; ii<m_edges.size(); ++ii)
        FGASSERT(m_edges[ii].triInds.size() > 0);
}

Vec2UI
MeshTopology::edgeFacingVertInds(uint edgeIdx) const
{
    Uints const &    triInds = m_edges[edgeIdx].triInds;
    FGASSERT(triInds.size() == 2);
    uint        ov0 = oppositeVert(triInds[0],edgeIdx),
                ov1 = oppositeVert(triInds[1],edgeIdx);
    return Vec2UI(ov0,ov1);
}

bool
MeshTopology::vertOnBoundary(uint vertIdx) const
{
    Uints const &    eis = m_verts[vertIdx].edgeInds;
    // If this vert is unused it is not on a boundary:
    for (size_t ii=0; ii<eis.size(); ++ii)
        if (m_edges[eis[ii]].triInds.size() == 1)
            return true;
    return false;
}

Uints
MeshTopology::vertBoundaryNeighbours(uint vertIdx) const
{
    Uints            neighs;
    Uints const &    edgeInds = m_verts[vertIdx].edgeInds;
    for (size_t ee=0; ee<edgeInds.size(); ++ee) {
        Edge                edge = m_edges[edgeInds[ee]];
        if (edge.triInds.size() == 1)
            neighs.push_back(edge.otherVertIdx(vertIdx));
    }
    return neighs;
}

Uints
MeshTopology::vertNeighbours(uint vertIdx) const
{
    Uints            ret;
    Uints const &    edgeInds = m_verts[vertIdx].edgeInds;
    for (size_t ee=0; ee<edgeInds.size(); ++ee)
        ret.push_back(m_edges[edgeInds[ee]].otherVertIdx(vertIdx));
    return ret;
}

// Had to re-write to avoid using inefficient recursive seam tracing as this actually managed to
// give a malloc error (on scan data) running 64-bit with 16GB RAM:
vector<set<uint> >
MeshTopology::seams() const
{
    vector<set<uint> >  ret;
    Uints               vertLabels(m_verts.size(),0);   // 0 is the label for non-edge vertices
    // Initialization sweep through edges:
    uint                currLabel = 1;
    for (size_t ee=0; ee<m_edges.size(); ++ee) {
        const Edge &    edge = m_edges[ee];
        if (edge.triInds.size() == 1) {                 // Boundary edge
            uint        v0 = edge.vertInds[0],
                        v1 = edge.vertInds[1];
            if (vertLabels[v0] == 0) {
                if (vertLabels[v1] == 0) {
                    vertLabels[v0] = currLabel;
                    vertLabels[v1] = currLabel;
                    ++currLabel;
                }
                else
                    vertLabels[v0] = vertLabels[v1];
            }
            else if (vertLabels[v1] == 0)
                vertLabels[v1] = vertLabels[v0];
        }
    }
    // Iterative merge sweeps through vertices:
    bool            done = false;
    while (!done) {
        done = true;
        for (size_t ii=0; ii<vertLabels.size(); ++ii) {
            if (vertLabels[ii] != 0) {
                Uints const &       edgeInds = m_verts[ii].edgeInds;
                for (size_t ee=0; ee<edgeInds.size(); ++ee) {
                    if (m_edges[edgeInds[ee]].triInds.size() == 1) {    // Boundary edge
                        uint        v = m_edges[edgeInds[ee]].otherVertIdx(uint(ii));
                        FGASSERT(vertLabels[v] != 0);
                        if (vertLabels[ii] != vertLabels[v]) {
                            fgReplace_(vertLabels,vertLabels[v],vertLabels[ii]);
                            done = false;
                        }
                    }
                }
            }
        }
    }
    // Collate labelled sets:
    map<uint,uint>      labToIdx;
    for (size_t ii=0; ii<vertLabels.size(); ++ii) {
        if (vertLabels[ii] != 0) {
            if (labToIdx.find(vertLabels[ii]) == labToIdx.end()) {
                labToIdx[vertLabels[ii]] = uint(ret.size());
                ret.push_back(set<uint>());
                ret.back().insert(uint(ii));
            }
            else {
                ret[labToIdx[vertLabels[ii]]].insert(uint(ii));
            }
        }
    }
    return ret;
}

set<uint>
MeshTopology::seamContaining(uint vertIdx) const
{
    set<uint>           ret;
    vector<set<uint> >  sms = seams();
    for (size_t ii=0; ii<sms.size(); ++ii)
        if (sms[ii].find(vertIdx) != sms[ii].end())
            ret = sms[ii];
    return ret;
}

set<uint>
MeshTopology::traceFold(
    MeshNormals const & norms,
    vector<FgBool> &    done,
    uint                vertIdx)
    const
{
    set<uint>           ret;
    if (done[vertIdx])
        return ret;
    done[vertIdx] = true;
    Uints const &    edgeInds = m_verts[vertIdx].edgeInds;
    for (size_t ii=0; ii<edgeInds.size(); ++ii) {
        const Edge &           edge = m_edges[edgeInds[ii]];
        if (edge.triInds.size() == 2) {         // Can not be part of a fold otherwise
            const FacetNormals &    facetNorms = norms.facet[0];
            float       dot = cDot(facetNorms.tri[edge.triInds[0]],facetNorms.tri[edge.triInds[1]]);
            if (dot < 0.5f) {                   // > 60 degrees
                ret.insert(vertIdx);
                cUnion_(ret,traceFold(norms,done,edge.otherVertIdx(vertIdx)));
            }
        }
    }
    return ret;
}

uint
MeshTopology::oppositeVert(uint triIdx,uint edgeIdx) const
{
    Vec3UI       tri = m_tris[triIdx].vertInds;
    Vec2UI       vertInds = m_edges[edgeIdx].vertInds;
    for (uint ii=0; ii<3; ++ii)
        if ((tri[ii] != vertInds[0]) && (tri[ii] != vertInds[1]))
            return tri[ii];
    FGASSERT_FALSE;
    return 0;
}

Vec3UI
MeshTopology::isManifold() const
{
    Vec3UI   ret(0);
    for (size_t ee=0; ee<m_edges.size(); ++ee) {
        const Edge &    edge = m_edges[ee];
        if (edge.triInds.size() == 1)
            ++ret[0];
        else if (edge.triInds.size() > 2)
            ++ret[1];
        else {
            // Check that winding directions of the two facets are opposite on this edge:
            Tri         tri0 = m_tris[edge.triInds[0]],
                        tri1 = m_tris[edge.triInds[1]];
            uint        edgeIdx0 = findFirstIdx(tri0.edgeInds,uint(ee)),
                        edgeIdx1 = findFirstIdx(tri1.edgeInds,uint(ee));
            if (tri0.edge(edgeIdx0) == tri1.edge(edgeIdx1))
                ++ret[2];
            // Worked on all 3DP meshes but doesn't work for some screwy input (eg. frameforge base):
            // FGASSERT(tri0.edge(edgeIdx0)[0] == tri1.edge(edgeIdx1)[1]);
            // FGASSERT(tri0.edge(edgeIdx0)[1] == tri1.edge(edgeIdx1)[0]);
        }
    }
    return ret;
}

size_t
MeshTopology::unusedVerts() const
{
    size_t      ret = 0;
    for (size_t ii=0; ii<m_verts.size(); ++ii)
        if (m_verts[ii].triInds.empty())
            ++ret;
    return ret;
}

Floats
MeshTopology::edgeDistanceMap(Vec3Fs const & verts,size_t vertIdx) const
{
    Floats       ret(verts.size(),maxFloat());
    FGASSERT(vertIdx < verts.size());
    ret[vertIdx] = 0;
    edgeDistanceMap(verts,ret);
    return ret;
}

void
MeshTopology::edgeDistanceMap(Vec3Fs const & verts,Floats & vertDists) const
{
    FGASSERT(verts.size() == m_verts.size());
    FGASSERT(vertDists.size() == verts.size());
    bool                done = false;
    while (!done) {
        done = true;
        for (size_t vv=0; vv<vertDists.size(); ++vv) {
            // Important: check each vertex each time since the topology will often result in 
            // the first such assignment not being the optimal:
            if (vertDists[vv] < maxFloat()) {
                Uints const &    edges = m_verts[vv].edgeInds;
                for (size_t ee=0; ee<edges.size(); ++ee) {
                    uint                neighVertIdx = m_edges[edges[ee]].otherVertIdx(uint(vv));
                    float               neighDist = vertDists[vv] + (verts[neighVertIdx]-verts[vv]).len();
                    if (neighDist < vertDists[neighVertIdx]) {
                        vertDists[neighVertIdx] = neighDist;
                        done = false;
                    }
                }
            }
        }
    }
}

set<uint>
cFillMarkedVertRegion(Mesh const & mesh,MeshTopology const & topo,uint seedIdx)
{
    FGASSERT(seedIdx < topo.m_verts.size());
    set<uint>           ret;
    for (MarkedVert const & mv : mesh.markedVerts)
        ret.insert(uint(mv.idx));
    set<uint>           todo;
    todo.insert(seedIdx);
    while (!todo.empty()) {
        set<uint>           next;
        for (uint idx : todo) {
            if (!contains(ret,idx)) {
                for (uint n : topo.vertNeighbours(idx))
                    next.insert(n);
                ret.insert(idx);
            }
        }
        todo = next;
    }
    return ret;
}

}

// */
