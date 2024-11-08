//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgTopology.hpp"
#include "FgCommand.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dDisplay.hpp"
#include "FgImgDisplay.hpp"
#include "FgFileSystem.hpp"

using namespace std;

namespace Fg {

using BoundEdges = SurfTopo::BoundEdges;
using BoundEdgess = SurfTopo::BoundEdgess;

Vec2UI              directEdgeVertInds(Vec2UI vertInds,Arr3UI tri)
{
    size_t          idx0 = findFirstIdx(tri,vertInds[0]),
                    idx1 = findFirstIdx(tri,vertInds[1]),
                    del = (idx1+3-idx0) % 3;
    if (del == 2)
        swap(vertInds[0],vertInds[1]);
    else if (del != 1)
        FGASSERT_FALSE;
    return vertInds;
}

Vec2UI              SurfTopo::Tri::edge(uint relIdx) const
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

uint                SurfTopo::Edge::otherVertIdx(uint vertIdx) const
{
    if (vertIdx == vertInds[0])
        return vertInds[1];
    else if (vertIdx == vertInds[1])
        return vertInds[0];
    else
        FGASSERT_FALSE;
    return 0;       // make compiler happy
}

struct      EdgeVerts
{
    uint            loIdx;
    uint            hiIdx;

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
    bool            operator<(const EdgeVerts & rhs) const
    {
        if (loIdx < rhs.loIdx)
            return true;
        else if (loIdx == rhs.loIdx)
            return (hiIdx < rhs.hiIdx);
        else
            return false;
    }

    bool            contains(uint idx) const {return ((idx == loIdx) || (idx == hiIdx)); }
};

struct      TriVerts
{
    Arr3UI          inds;

    TriVerts(Arr3UI i) : inds{sortAll(i)} {}

    bool            operator<(const TriVerts & rhs) const
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

SurfTopo::SurfTopo(Arr3UIs const & tris)
{
    setup(0,tris); // 0 means just use max reference
}

SurfTopo::SurfTopo(size_t numVerts,Arr3UIs const & tris) {setup(uint(numVerts),tris); }

void                SurfTopo::setup(uint numVerts,Arr3UIs const & tris)
{
    uint            maxVertReferenced = 0;
    for (Arr3UI const & tri : tris)
        for (uint idx : tri)
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
        Arr3UI           vis = tris[ii];
        if ((vis[0] == vis[1]) || (vis[1] == vis[2]) || (vis[2] == vis[0]))
            ++nulls;
        else {
            TriVerts            tv(vis);
            if (vset.find(tv) == vset.end()) {
                vset.insert(tv);
                Tri             tri {vis,Arr3UI{lims<uint>::max()}};
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
        Arr3UI       vertInds = m_tris[ii].vertInds;
        for (uint jj=0; jj<3; ++jj) {
            m_verts[vertInds[jj]].triInds.push_back(uint(ii));
            EdgeVerts        edge(vertInds[jj],vertInds[(jj+1)%3]);
            edgesToTris[edge].push_back(uint(ii));
        }
    }
    for (map<EdgeVerts,Uints >::const_iterator it=edgesToTris.begin(); it!=edgesToTris.end(); ++it) {
        EdgeVerts       edgeVerts = it->first;
        Edge            edge {
            Vec2UI(edgeVerts.loIdx,edgeVerts.hiIdx),
            it->second
        };
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
            Arr3UI           tri = m_tris[triIdx].vertInds;
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

Vec2UI              SurfTopo::edgeFacingVertInds(uint edgeIdx) const
{
    Uints const &    triInds = m_edges[edgeIdx].triInds;
    FGASSERT(triInds.size() == 2);
    uint        ov0 = oppositeVert(triInds[0],edgeIdx),
                ov1 = oppositeVert(triInds[1],edgeIdx);
    return Vec2UI(ov0,ov1);
}

bool                SurfTopo::vertOnBoundary(uint vertIdx) const
{
    Uints const &    eis = m_verts[vertIdx].edgeInds;
    // If this vert is unused it is not on a boundary:
    for (size_t ii=0; ii<eis.size(); ++ii)
        if (m_edges[eis[ii]].triInds.size() == 1)
            return true;
    return false;
}

Uints               SurfTopo::vertBoundaryNeighbours(uint vertIdx) const
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

Uints               SurfTopo::vertNeighbours(uint vertIdx) const
{
    Uints            ret;
    Uints const &    edgeInds = m_verts[vertIdx].edgeInds;
    for (size_t ee=0; ee<edgeInds.size(); ++ee)
        ret.push_back(m_edges[edgeInds[ee]].otherVertIdx(vertIdx));
    return ret;
}

BoundEdges          SurfTopo::boundaryContainingEdgeP(uint boundEdgeIdx) const
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

BoundEdges          SurfTopo::boundaryContainingVert(uint vertIdx) const
{
    FGASSERT(vertIdx < m_verts.size());
    Vert const &                vert = m_verts[vertIdx];
    for (uint edgeIdx : vert.edgeInds) {
        Edge const &                edge = m_edges[edgeIdx];
        if (edge.triInds.size() == 1)                           // boundary
            return boundaryContainingEdgeP(edgeIdx);
    }
    return BoundEdges{};
}

BoundEdgess         SurfTopo::boundaries() const
{
    BoundEdgess         ret;
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

Bools               SurfTopo::boundaryVertFlags() const
{
    BoundEdgess         bess = boundaries();
    Bools               ret (m_verts.size(),false);
    for (BoundEdges const & bes : bess) {
        for (BoundEdge const & be : bes) {
            Edge const &        edge = m_edges[be.edgeIdx];
            // for a manifold surface we only need to set one of these, but just in case:
            ret[edge.vertInds[0]] = true;
            ret[edge.vertInds[1]] = true;
        }
    }
    return ret;
}

set<uint>           SurfTopo::traceFold(
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

uint                SurfTopo::oppositeVert(uint triIdx,uint edgeIdx) const
{
    Arr3UI       tri = m_tris[triIdx].vertInds;
    Vec2UI       vertInds = m_edges[edgeIdx].vertInds;
    for (uint ii=0; ii<3; ++ii)
        if ((tri[ii] != vertInds[0]) && (tri[ii] != vertInds[1]))
            return tri[ii];
    FGASSERT_FALSE;
    return 0;
}

Arr3UI              SurfTopo::isManifold() const
{
    Arr3UI   ret(0);
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
            size_t      edgeIdx0 = findFirstIdx(tri0.edgeInds,uint(ee)),
                        edgeIdx1 = findFirstIdx(tri1.edgeInds,uint(ee));
            if (tri0.edge(scast<uint>(edgeIdx0)) == tri1.edge(scast<uint>(edgeIdx1)))
                ++ret[2];
            // Worked on all 3DP meshes but doesn't work for some screwy input (eg. frameforge base):
            // FGASSERT(tri0.edge(edgeIdx0)[0] == tri1.edge(edgeIdx1)[1]);
            // FGASSERT(tri0.edge(edgeIdx0)[1] == tri1.edge(edgeIdx1)[0]);
        }
    }
    return ret;
}

size_t              SurfTopo::unusedVerts() const
{
    size_t      ret = 0;
    for (size_t ii=0; ii<m_verts.size(); ++ii)
        if (m_verts[ii].triInds.empty())
            ++ret;
    return ret;
}

Floats              SurfTopo::edgeDistanceMap(Vec3Fs const & verts,size_t vertIdx) const
{
    Floats       ret(verts.size(),lims<float>::max());
    FGASSERT(vertIdx < verts.size());
    ret[vertIdx] = 0;
    edgeDistanceMap(verts,ret);
    return ret;
}

void                SurfTopo::edgeDistanceMap(Vec3Fs const & verts,Floats & vertDists) const
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
                    float               neighDist = vertDists[vv] + cLenD(verts[neighVertIdx]-verts[vv]);
                    if (neighDist < vertDists[neighVertIdx]) {
                        vertDists[neighVertIdx] = neighDist;
                        done = false;
                    }
                }
            }
        }
    }
}

Vec3Ds              SurfTopo::boundaryVertNormals(BoundEdges const & boundary,Vec3Ds const & verts) const
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

set<uint>           cFillMarkedVertRegion(Mesh const & mesh,SurfTopo const & topo,uint seedIdx)
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

// TODO: lots of possible optimization:
TriEdgess           cSharedEdges(Arr3UIs const & tris)
{
    TriEdgess           ret (tris.size());
    uint                T = scast<uint>(tris.size());
    auto                checkMatch = [&,T](uint tt,uint ee,Arr2UI edge)
    {
        for (uint uu=tt+1; uu<T; ++uu) {
            Arr3UI              tri = tris[uu];
            for (uint ff=0; ff<3; ++ff) {
                if (edge == swapElems(getEdge(tri,ff))) {   // shared edge is opposite winding in manifold mesh
                    ret[tt][ee] = {uu,ff};
                    ret[uu][ff] = {tt,ee};
                    return;         // only 1 match for manifold mesh
                }
            }
        }
    };
    for (uint tt=0; tt<T; ++tt) {
        Arr3UI              tri = tris[tt];
        for (uint ee=0; ee<3; ++ee) {
            if (!ret[tt][ee].valid()) {     // if this edge has not yet found a match:
                Arr2UI              edge = getEdge(tri,ee);
                checkMatch(tt,ee,edge);
            }
        }
    }
    return ret;
}

Arr3Bs              cBoundFlags(TriEdgess const & tess)
{
    auto                fn = [](TriEdges const & tes) -> Arr3B
    {
        return mapCall(tes,[](TriEdge te){return !te.valid(); });
    };
    return mapCall(tess,fn);
}
void                updateSharedEdges(Bools const & removedTris,TriEdgess & tess)
{
    size_t              T = tess.size();
    FGASSERT(removedTris.size() == T);
    for (size_t tt=0; tt<T; ++tt) {
        if (removedTris[tt]) {
            for (TriEdge & te : tess[tt]) {
                if (te.valid() && !removedTris[te.triIdx]) {    // shared edge between a removed and non-removed tri
                    tess[te.triIdx][te.edgeNum].invalidate();
                    te.invalidate();
                }
            }
        }
    }
}
static void         testSharedEdges(CLArgs const & args)
{
    Mesh                mesh = loadTri(dataDir()+"base/JaneLoresFace.tri");     // 1 surface, all tris
    Surf &              surf = mesh.surfaces[0];
    TriEdgess           tess = cSharedEdges(surf.tris.vertInds);
    surf.edgeFlags = cBoundFlags(tess);
    if (!isAutomated(args))
        viewMesh(mesh);
    size_t              T = surf.tris.vertInds.size();
    Bools               removeFlags (T,false);
    for (size_t ii=0; ii<T/4; ++ii)
        removeFlags[randUniformUint(T)] = true;
    updateSharedEdges(removeFlags,tess);
    surf.edgeFlags = cBoundFlags(tess);
    if (!isAutomated(args))
        viewMesh(mesh);
}

void                testTopoBnorm(CLArgs const & args)
{
    // Test boundary vert normals by adding marked verts along normals and viewing:
    Mesh                mesh = loadTri(dataDir()+"base/JaneLoresFace.tri");
    TriSurf             triSurf = mesh.asTriSurf();
    Vec3Ds              verts = mapCast<Vec3D>(triSurf.verts);
    double              scale = cMaxElem(cDims(verts)) * 0.01;        // Extend norms 1% of max dim
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

void                testTopoEdist(CLArgs const & args)
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
    Uchars              colVals (edgeDists.size(),255);
    for (size_t ii=0; ii<colVals.size(); ++ii)
        if (edgeDists[ii] < lims<float>::max())
            colVals[ii] = uint(distToCol * edgeDists[ii]);
    mesh.surfaces[0].setAlbedoMap(ImgRgba8(128,128,Rgba8(255)));
    AxAffine2F          otcsToPacs = cOtcsToPacs<float>(Vec2UI(128));
    for (size_t tt=0; tt<surf.tris.size(); ++tt) {
        Arr3UI              vertInds = surf.tris.vertInds[tt];
        Arr3UI              uvInds = surf.tris.uvInds[tt];
        for (uint ii=0; ii<3; ++ii) {
            Rgba8           col(255);
            col.red() = colVals[vertInds[ii]];
            col.green() = 255 - col.red();
            mesh.surfaces[0].material.albedoMap->paint(Vec2UI(otcsToPacs*mesh.uvs[uvInds[ii]]),col);
        }
    }
    if (!isAutomated(args))
        viewMesh(mesh);
}

void                testTopoBvf(CLArgs const & args)
{
    Mesh                mesh = loadTri(dataDir()+"base/JaneLoresFace.tri");     // 1 surface, all tris
    size_t              V = mesh.verts.size();
    SurfTopo            topo {V,mesh.surfaces[0].tris.vertInds};
    Bools               boundVertFlags = topo.boundaryVertFlags();
    FGASSERT(boundVertFlags.size() == V);
    for (size_t vv=0; vv<V; ++vv)
        if (boundVertFlags[vv])
            mesh.markedVerts.emplace_back(scast<uint>(vv));
    if (!isAutomated(args))
        viewMesh(mesh);
}

void                testTopo(CLArgs const & args)
{
    Cmds                cmds {
        {testTopoBnorm,"bnorm","view boundary vertex normals"},
        {testSharedEdges,"edges","cSharedEges(), cBoundFlags(), updateSharedEdges()"},
        {testTopoEdist,"edist","view edge distances"},
        {testTopoBvf,"bvf","view boundary vertex flags"},
    };
    return doMenu(args,cmds,true);
}

}

// */
