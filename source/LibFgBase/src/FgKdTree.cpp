//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//

#include "stdafx.h"

#include "FgKdTree.hpp"
#include "FgMath.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

KdTree::KdTree(Vec3Fs const & pnts)
{
    FGASSERT(!pnts.empty());
    m_tree.reserve(pnts.size());
    createNode(pnts,0);
}

struct  Less
{
    uint    dim;

    explicit Less(uint d) : dim(d) {}

    bool
    operator()(Vec3F lhs,Vec3F rhs)
    {return (lhs[dim] < rhs[dim]); }
};

uint
KdTree::createNode(Vec3Fs const & v,uint dim)
{
    FGASSERT(v.size() > 0);
    if (v.size() == 1) {
        m_tree.push_back(Node(v[0]));
        return uint(m_tree.size()-1);
    }
    Vec3Fs    verts(v);
    sort(verts.begin(),verts.end(),Less(dim));  // Smallest first
    uint        split = uint(verts.size() / 2),
                newDim = (dim+1)%3;
    if (verts.size() > 2)
        m_tree.push_back(
            Node(
                verts[split],
                createNode(
                    Vec3Fs(verts.begin(),verts.begin()+split),
                    newDim),
                createNode(
                    Vec3Fs(verts.begin()+split+1,verts.end()),
                    newDim)));
    else
        m_tree.push_back(
            Node(
                verts[split],
                createNode(
                    Vec3Fs(verts.begin(),verts.begin()+split),
                    newDim)));
    return uint(m_tree.size()-1);
}

VecMagF
KdTree::findBest(Vec3F pos,VecMagF best,uint rootIdx,uint dim) const
{
    const uint      maxDepth = 18;
    FGASSERT(m_tree.size() < (1 << (maxDepth-2)));      // Safe bound check
    Valid<uint>   idxArr[maxDepth];                     // keep on stack for speed
    idxArr[0] = rootIdx;
    int             idxCnt = 0;
    do {
        uint    idx = idxArr[idxCnt].val();
        idxArr[++idxCnt] =
            (pos[dim] < m_tree[idx].vert[dim]) ?
                m_tree[idx].idxLo :
                m_tree[idx].idxHi;
        dim = (dim+1)%3;
    }
    while (idxArr[idxCnt].valid());
    --idxCnt;
    dim = (dim+2)%3;
    do {
        const Node &    node = m_tree[idxArr[idxCnt].val()];
        float          mag = (node.vert-pos).mag();
        if (mag < best.mag) {
            best.mag = mag;
            best.vec = node.vert;
        }
        float           planeDelta = pos[dim] - node.vert[dim];
        Valid<uint>   mirrorIdx =
            (planeDelta > 0.0) ? node.idxLo : node.idxHi;
        if (mirrorIdx.valid() && (sqr(planeDelta) < best.mag))
            best = findBest(pos,best,mirrorIdx.val(),(dim+1)%3);
        --idxCnt;
        dim = (dim+2)%3;
    }
    while (idxCnt >= 0);
    return best;
}

VecMagF
KdTree::closest(Vec3F pos) const
{
    VecMagF    none;
    none.mag = maxFloat();
    return findBest(pos,none,uint(m_tree.size()-1),0);
}

void
testKdTree(CLArgs const &)
{
    Vec3Fs      targs = randVecNormals<float,3>(512,1.0);
    KdTree      kd {targs};
    for (size_t ii=0; ii<512; ++ii) {
        Vec3F       p = randVecNormal<float,3>();
        VecMagF     closestKd = kd.closest(p);
        VecMagF     closestVec;
        for (Vec3F t : targs) {
            VecMagF     c {t-p};
            if (c.mag < closestVec.mag)
                closestVec = c;
        }
        FGASSERT(closestKd.mag == closestVec.mag);
    }
}

}

// */
