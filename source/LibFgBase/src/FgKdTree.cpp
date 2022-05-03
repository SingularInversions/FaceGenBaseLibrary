//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//

#include "stdafx.h"

#include "FgKdTree.hpp"
#include "FgMath.hpp"
#include "FgCommand.hpp"
#include "FgIter.hpp"

using namespace std;

namespace Fg {

namespace {

struct  KdLess
{
    uint            dim;

    explicit KdLess(uint d) : dim(d) {}

    bool            operator()(Vec3F lhs,Vec3F rhs) {return (lhs[dim] < rhs[dim]); }
};

uint                createNode(Svec<KdTree::Node> & tree,Vec3Fs const & v,uint dim)
{
    FGASSERT(!v.empty());
    if (v.size() == 1) {
        uint        ret = uint(tree.size());
        tree.push_back(KdTree::Node{v[0]});
        return ret;
    }
    Vec3Fs          verts(v);
    sort(verts.begin(),verts.end(),KdLess(dim));  // Smallest first
    uint            split = uint(verts.size() / 2),
                    nextDim = (dim+1)%3;
    if (v.size() == 2)
        tree.push_back(KdTree::Node{verts[split],createNode(tree,cHead(verts,split),nextDim)});
    else            // > 2 verts
        tree.push_back(
            KdTree::Node{
                verts[split],
                createNode(tree,cHead(verts,split),nextDim),
                createNode(tree,cRest(verts,split+1),nextDim)});
    return uint(tree.size()-1);   // Last node pushed was child for caller
}

KdVal               findClst(Svec<KdTree::Node> const & tree,Vec3F query,KdVal best,uint idx,uint dim)
{
    KdTree::Node const &    node = tree[idx];
    float                   distMag = cMag(query-node.vert);
    if (distMag < best.distMag) {
        best.distMag = distMag;
        best.closest = node.vert;
    }
    uint                nextDim = (dim+1)%3;
    bool                lte = (query[dim] <= node.vert[dim]);
    Valid<uint>         checkFirst = lte ? node.idxLo : node.idxHi,
                        checkSecond = lte ? node.idxHi : node.idxLo;
    if (checkFirst.valid())
        best = findClst(tree,query,best,checkFirst.val(),nextDim);
    if (best.distMag < sqr(query[dim]-node.vert[dim]))          // Closest point cannot be in other node
        return best;
    if (checkSecond.valid())                                    // Must check other node to be sure:
        best = findClst(tree,query,best,checkSecond.val(),nextDim);
    return best;
}

double              testClosest(Svec<KdTree::Node> const & tree,Vec3F query)
{
    float           ret = lims<float>::max();
    for (KdTree::Node const & node : tree) {
        float           mag = cMag(query - node.vert);
        if (mag < ret)
            ret = mag;
    }
    return ret;
}

}

KdTree::KdTree(Vec3Fs const & pnts)
{
    FGASSERT(!pnts.empty());
    Vec3Fs          noDups = getUniqueSorted(cSort(pnts));
    m_tree.reserve(noDups.size());
    createNode(m_tree,noDups,0);
}

KdVal               KdTree::findClosest(Vec3F pos) const
{
    FGASSERT(!m_tree.empty());
    KdVal           init {Vec3F{0},std::numeric_limits<float>::max()};
    return findClst(m_tree,pos,init,uint(m_tree.size()-1),0);
}

void                testKdTree(CLArgs const &)
{
    Vec3Fs          targs = randVecNormals<float,3>(512,1.0f);
    // Grid data (challenging for KD tree):
    for (Iter3UI it(4); it.valid(); it.next())
        targs.push_back(Vec3F(it()) * 0.5f - Vec3F(0.75f));
    KdTree          kd {targs};
    // Test random query points:
    for (size_t ii=0; ii<512; ++ii) {
        Vec3F       p = randVecNormal<float,3>();
        FGASSERT(kd.findClosest(p).distMag == testClosest(kd.m_tree,p));
    }
    // Test exact matches:
    for (Vec3F t : targs)
        FGASSERT(kd.findClosest(t).distMag == 0.0f);
}

}

// */
