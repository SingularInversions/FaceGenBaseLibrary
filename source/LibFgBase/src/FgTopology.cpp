//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgTopology.hpp"
#include "FgCommand.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dDisplay.hpp"
#include "FgImgDisplay.hpp"

using namespace std;

namespace Fg {


Vec2UI
directEdgeVertInds(Vec2UI vertInds,Vec3UI tri)
{
    uint            idx0 = findFirstIdx(tri,vertInds[0]),
                    idx1 = findFirstIdx(tri,vertInds[1]),
                    del = (idx1+3-idx0) % 3;
    if (del == 2)
        swap(vertInds[0],vertInds[1]);
    else if (del != 1)
        FGASSERT_FALSE;
    return vertInds;
}

Vec2UI
SurfTopo::Tri::edge(uint relIdx) const
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
SurfTopo::Edge::otherVertIdx(uint vertIdx) const
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

SurfTopo::SurfTopo(Vec3UIs const & tris)
{
    setup(0,tris);      // 0 means just use max reference
}

SurfTopo::SurfTopo(size_t numVerts,Vec3UIs const & tris)
{
    setup(uint(numVerts),tris);
}

void
SurfTopo::setup(uint numVerts,Vec3UIs const & tris)
{
    uint            maxVertReferenced = 0;
    for (Vec3UI const & tri : tris)
        for (uint idx : tri.m)
            updateMax_(maxVertReferenced,idx);
    if (numVerts == 0)
        numVerts = maxVertReferenced + 1;
    else if (numVerts < maxVertReferenced+1)
        fgThrow("SurfTopo called with vertex count smaller than max index reference");
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
        fgout << fgnl << "WARNING Ignored " << duplicates << " duplicate tris";
    if (nulls > 0)
        fgout << fgnl << "WARNING Ignored " << nulls << " null tris.";
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
SurfTopo::edgeFacingVertInds(uint edgeIdx) const
{
    Uints const &    triInds = m_edges[edgeIdx].triInds;
    FGASSERT(triInds.size() == 2);
    uint        ov0 = oppositeVert(triInds[0],edgeIdx),
                ov1 = oppositeVert(triInds[1],edgeIdx);
    return Vec2UI(ov0,ov1);
}

bool
SurfTopo::vertOnBoundary(uint vertIdx) const
{
    Uints const &    eis = m_verts[vertIdx].edgeInds;
    // If this vert is unused it is not on a boundary:
    for (size_t ii=0; ii<eis.size(); ++ii)
        if (m_edges[eis[ii]].triInds.size() == 1)
            return true;
    return false;
}

Uints
SurfTopo::vertBoundaryNeighbours(uint vertIdx) const
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
SurfTopo::vertNeighbours(uint vertIdx) const
{
    Uints            ret;
    Uints const &    edgeInds = m_verts[vertIdx].edgeInds;
    for (size_t ee=0; ee<edgeInds.size(); ++ee)
        ret.push_back(m_edges[edgeInds[ee]].otherVertIdx(vertIdx));
    return ret;
}

SurfTopo::BoundEdges
SurfTopo::boundaryContainingEdgeP(uint boundEdgeIdx) const
{
    BoundEdges      boundEdges;
    bool            moreEdges = false;
    do {
        Edge const &    boundEdge = m_edges[boundEdgeIdx];
        Tri const &     tri = m_tris[boundEdge.triInds[0]];     // Every boundary edge has exactly 1
        Vec2UI          vertInds = directEdgeVertInds(boundEdge.vertInds,tri.vertInds);
        boundEdges.push_back({boundEdgeIdx,vertInds[1]});
        Vert const &    vert = m_verts[vertInds[1]];            // Follow edge direction to vert
        moreEdges = false;
        for (uint edgeIdx : vert.edgeInds) {
            if (edgeIdx != boundEdgeIdx) {
                if (m_edges[edgeIdx].triInds.size() == 1) {     // Another boundary edge
                    if (!containsMember(boundEdges,&BoundEdge::edgeIdx,edgeIdx)) {
                        boundEdgeIdx = edgeIdx;
                        moreEdges = true;
                        continue;
                    }
                }
            }
        }
    } while (moreEdges);
    return boundEdges;
}

SurfTopo::BoundEdges
SurfTopo::boundaryContainingVert(uint vertIdx) const
{
    FGASSERT(vertIdx < m_verts.size());
    Vert const &                vert = m_verts[vertIdx];
    for (uint edgeIdx : vert.edgeInds) {
        Edge const &                edge = m_edges[edgeIdx];
        if (edge.triInds.size() == 1)                           // boundary
            return boundaryContainingEdgeP(edgeIdx);
    }
    return SurfTopo::BoundEdges{};
}

Svec<SurfTopo::BoundEdges>
SurfTopo::boundaries() const
{
    Svec<BoundEdges>    ret;
    auto                alreadyAdded = [&ret](uint edgeIdx)
    {
        for (BoundEdges const & bes : ret)
            if (containsMember(bes,&BoundEdge::edgeIdx,edgeIdx))
                return true;
        return false;
    };
    for (uint ee=0; ee<m_edges.size(); ++ee) {
        if (m_edges[ee].triInds.size() == 1)            // Boundary edge
            if (!alreadyAdded(ee))
                ret.push_back(boundaryContainingEdgeP(ee));
    }
    return ret;
}

Bools
SurfTopo::boundaryVertFlags() const
{
    Svec<BoundEdges>    bess = boundaries();
    Bools               ret (m_verts.size(),false);
    for (BoundEdges const & bes : bess) {
        for (BoundEdge const & be : bes) {
            Edge const &        edge = m_edges[be.edgeIdx];
            ret[edge.vertInds[0]] = true;
            ret[edge.vertInds[1]] = true;
        }
    }
    return ret;
}

set<uint>
SurfTopo::traceFold(
    MeshNormals const & norms,
    vector<FatBool> &    done,
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
SurfTopo::oppositeVert(uint triIdx,uint edgeIdx) const
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
SurfTopo::isManifold() const
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
SurfTopo::unusedVerts() const
{
    size_t      ret = 0;
    for (size_t ii=0; ii<m_verts.size(); ++ii)
        if (m_verts[ii].triInds.empty())
            ++ret;
    return ret;
}

Floats
SurfTopo::edgeDistanceMap(Vec3Fs const & verts,size_t vertIdx) const
{
    Floats       ret(verts.size(),lims<float>::max());
    FGASSERT(vertIdx < verts.size());
    ret[vertIdx] = 0;
    edgeDistanceMap(verts,ret);
    return ret;
}

void
SurfTopo::edgeDistanceMap(Vec3Fs const & verts,Floats & vertDists) const
{
    FGASSERT(verts.size() == m_verts.size());
    FGASSERT(vertDists.size() == verts.size());
    bool                done = false;
    while (!done) {
        done = true;
        for (size_t vv=0; vv<vertDists.size(); ++vv) {
            // Important: check each vertex each time since the topology will often result in 
            // the first such assignment not being the optimal:
            if (vertDists[vv] < lims<float>::max()) {
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

Vec3Ds
SurfTopo::boundaryVertNormals(BoundEdges const & boundary,Vec3Ds const & verts) const
{
    Vec3Ds              edgeNorms; edgeNorms.reserve(boundary.size());
    Vec3D               v0 = verts[boundary.back().vertIdx];
    for (BoundEdge const & be : boundary) {
        Vec3D               v1 = verts[be.vertIdx];
        Edge const &        edge = m_edges[be.edgeIdx];
        Tri const &         tri = m_tris[edge.triInds[0]];  // must be exactly 1 tri
        Vec3D               triNorm = cTriNorm(tri.vertInds,verts),
                            xp = crossProduct(v1-v0,triNorm);
        edgeNorms.push_back(normalize(xp));
        v0 = v1;
    }
    Vec3Ds              vertNorms; vertNorms.reserve(boundary.size());
    for (size_t e0=0; e0<boundary.size(); ++e0) {
        size_t              e1 = (e0 + 1) % boundary.size();
        Vec3D               dir = edgeNorms[e0] + edgeNorms[e1];
        vertNorms.push_back(normalize(dir));
    }
    return vertNorms;
}

set<uint>
cFillMarkedVertRegion(Mesh const & mesh,SurfTopo const & topo,uint seedIdx)
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

void
testmSurfTopo(CLArgs const & args)
{
    // Test boundary vert normals by adding marked verts along normals and viewing:
    Mesh                mesh = loadTri(dataDir()+"base/JaneLoresFace.tri");
    TriSurf             triSurf = mesh.asTriSurf();
    Vec3Ds              verts = mapCast<Vec3D>(triSurf.verts);
    double              scale = cMax(cDims(verts).m) * 0.01;        // Extend norms 1% of max dim
    SurfTopo            topo {triSurf.verts.size(),triSurf.tris};
    Svec<SurfTopo::BoundEdges> boundaries = topo.boundaries();
    for (auto const & boundary : boundaries) {
        Vec3Ds              vertNorms = topo.boundaryVertNormals(boundary,verts);
        for (size_t bb=0; bb<boundary.size(); ++bb) {
            auto const &        be = boundary[bb];
            Vec3D               vert = verts[be.vertIdx] + vertNorms[bb] * scale;
            mesh.addMarkedVert(Vec3F(vert),"");
        }
    }
    if (!isAutomated(args))
        viewMesh(mesh);
}

void
testmEdgeDist(CLArgs const & args)
{
    Mesh                mesh = loadTri(dataDir()+"base/Jane.tri");
    Surf                surf = merge(mesh.surfaces).convertToTris();
    SurfTopo            topo {mesh.verts.size(),surf.tris.vertInds};
    size_t              vertIdx = 0;    // Randomly choose the first
    Floats              edgeDists = topo.edgeDistanceMap(mesh.verts,vertIdx);
    float               distMax = 0;
    for (size_t ii=0; ii<edgeDists.size(); ++ii)
        if (edgeDists[ii] < lims<float>::max())
            updateMax_(distMax,edgeDists[ii]);
    float               distToCol = 255.99f / distMax;
    Uchars              colVal(edgeDists.size(),255);
    for (size_t ii=0; ii<colVal.size(); ++ii)
        if (edgeDists[ii] < lims<float>::max())
            colVal[ii] = uint(distToCol * edgeDists[ii]);
    mesh.surfaces[0].setAlbedoMap(ImgRgba8(128,128,Rgba8(255)));
    AffineEw2F          otcsToIpcs = cOtcsToIpcs(Vec2UI(128));
    for (size_t tt=0; tt<surf.tris.size(); ++tt) {
        Vec3UI              vertInds = surf.tris.vertInds[tt];
        Vec3UI              uvInds = surf.tris.uvInds[tt];
        for (uint ii=0; ii<3; ++ii) {
            Rgba8           col(255);
            col.red() = colVal[vertInds[ii]];
            col.green() = 255 - col.red();
            mesh.surfaces[0].material.albedoMap->paint(Vec2UI(otcsToIpcs*mesh.uvs[uvInds[ii]]),col);
        }
    }
    if (!isAutomated(args))
        viewMesh(mesh);
}

void
testmBoundVertFlags(CLArgs const & args)
{
    Mesh                mesh = loadTri(dataDir()+"base/JaneLoresFace.tri");     // 1 surface, all tris
    TriInds const &        tris = mesh.surfaces[0].tris;
    SurfTopo            topo {mesh.verts.size(),tris.vertInds};
    Bools               boundVertFlags = topo.boundaryVertFlags();
    Vec2UI              sz {64};
    ImgRgba8            map {sz,Rgba8{255}};
    AffineEw2F          otcsToIpcs = cOtcsToIpcs(sz);
    auto                paintFn = [&](uint uvIdx)
    {
        Vec2F           otcs = mesh.uvs[uvIdx];
        Vec2UI          ircs = Vec2UI(otcsToIpcs * otcs);
        map.paint(ircs,{255,0,0,255});
    };
    for (size_t tt=0; tt<tris.size(); ++tt) {
        Vec3UI          uv = tris.uvInds[tt],
                        tri = tris.vertInds[tt];
        for (uint vv=0; vv<3; ++vv)
            if (boundVertFlags[tri[vv]])
                paintFn(uv[vv]);
    }
    //viewImage(map);
    mesh.surfaces[0].setAlbedoMap(map);
    if (!isAutomated(args))
        viewMesh(mesh);
}

void
testSurfTopo(CLArgs const & args)
{
    Cmds                cmds {
        {testmSurfTopo,"bnorm","view boundary vertex normals"},
        {testmEdgeDist,"edist","view edge distances"},
        {testmBoundVertFlags,"bvf","view boundary vertex flags"},
    };
    return doMenu(args,cmds,true);
}

}

// */
