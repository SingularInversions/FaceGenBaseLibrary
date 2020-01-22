//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//

#include "stdafx.h"

#include "FgKdTree.hpp"
#include "FgMath.hpp"

using namespace std;

namespace Fg {

FgKdTree::FgKdTree(const std::vector<Vec3F> & verts)
{
    FGASSERT(verts.size() > 0);
    m_tree.reserve(verts.size());
    createNode(verts,0);
}

FgMagPnt
FgKdTree::closest(Vec3F pos) const
{
    FgMagPnt    none;
    none.mag = maxFloat();
    return findBest(pos,none,uint(m_tree.size()-1),0);
}

struct  Less
{
    uint    dim;

    Less(uint d) : dim(d) {}

    bool
    operator()(Vec3F lhs,Vec3F rhs)
    {return (lhs[dim] < rhs[dim]); }
};

uint
FgKdTree::createNode(const vector<Vec3F> & v,uint dim)
{
    FGASSERT(v.size() > 0);
    if (v.size() == 1) {
        m_tree.push_back(Node(v[0]));
        return uint(m_tree.size()-1);
    }
    vector<Vec3F>    verts(v);
    sort(verts.begin(),verts.end(),Less(dim));  // Smallest first
    uint        split = uint(verts.size() / 2),
                newDim = (dim+1)%3;
    if (verts.size() > 2)
        m_tree.push_back(
            Node(
                verts[split],
                createNode(
                    vector<Vec3F>(verts.begin(),verts.begin()+split),
                    newDim),
                createNode(
                    vector<Vec3F>(verts.begin()+split+1,verts.end()),
                    newDim)));
    else
        m_tree.push_back(
            Node(
                verts[split],
                createNode(
                    vector<Vec3F>(verts.begin(),verts.begin()+split),
                    newDim)));
    return uint(m_tree.size()-1);
}

FgMagPnt
FgKdTree::findBest(const Vec3F & pos,FgMagPnt best,uint rootIdx,uint dim) const
{
    const uint      maxDepth = 18;
    FGASSERT(m_tree.size() < (1 << (maxDepth-2)));      // Safe bound check
    Valid<uint>   idxArr[maxDepth];                   // keep on stack for speed
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
            best.pnt = node.vert;
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

}

// */
