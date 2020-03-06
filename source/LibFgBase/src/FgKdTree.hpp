//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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
    Svec<Node>      m_tree;

    explicit KdTree(Vec3Fs const & pnts);    // Can't be empty

    VecMagF
    closest(Vec3F pos) const;

    VecMagD
    closest(Vec3D pos) const
    {
        VecMagF     vm = closest(Vec3F(pos));
        return VecMagD(Vec3D(vm.vec));
    }

private:
    uint
    createNode(Vec3Fs const & verts,uint dim);

    VecMagF
    findBest(Vec3F pos,VecMagF currBest,uint rootIdx,uint dim) const;
};

}

#endif
