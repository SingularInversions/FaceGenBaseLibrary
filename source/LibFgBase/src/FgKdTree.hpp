//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGKDTREE_HPP
#define FGKDTREE_HPP

#include "FgMatrixC.hpp"
#include "FgOpt.hpp"
#include "FgMath.hpp"
#include "FgGeometry.hpp"

namespace Fg {

struct  KdVal
{
    Vec3F               closest;
    float               distMag;    // squared magnitude of distance from query point to 'closest' above
};

struct  KdTree
{
    struct  Node
    {
        Vec3F           vert;
        Valid<uint>     idxLo;
        Valid<uint>     idxHi;

        explicit Node(Vec3F v) : vert(v) {}
        Node(Vec3F v,uint l) : vert(v), idxLo(l) {}
        Node(Vec3F v,uint l,uint h) : vert(v), idxLo(l), idxHi(h) {}
    };
    Svec<Node>          m_tree;                 // Last node is root

    explicit KdTree(Vec3Fs const & pnts);       // Can't be empty. Duplicates are ignored.

    // If multiple points are exactly equidistant 1 is arbirarily chosen:
    KdVal               findClosest(Vec3F query) const;
    KdVal               findClosest(Vec3D query) const {return findClosest(Vec3F(query)); }
};

}

#endif
