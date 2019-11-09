//
// Copyright (c) 2019 Singular Inversions Inc.
//

//

#ifndef FGKDTREE_HPP
#define FGKDTREE_HPP

#include "FgMatrixC.hpp"
#include "FgOpt.hpp"
#include "FgMath.hpp"

namespace Fg {

struct  FgMagPnt
{
    float       mag;        // Squared distance to closest point
    Vec3F    pnt;        // Closest point
};

class   FgKdTree
{
public:
    FgKdTree(const Svec<Vec3F> & pnts);   // Must be non-empty

    FgMagPnt
    closest(Vec3F pos) const;

private:
    struct  Node
    {
        Node(Vec3F v) : vert(v) {}
        Node(Vec3F v,uint l) : vert(v), idxLo(l) {}
        Node(Vec3F v,uint l,uint h) : vert(v), idxLo(l), idxHi(h) {}

        Vec3F        vert;
        Valid<uint>   idxLo;
        Valid<uint>   idxHi;
    };
    Svec<Node>   m_tree;

    uint
    createNode(const Svec<Vec3F> & verts,uint dim);

    FgMagPnt
    findBest(const Vec3F & pos,FgMagPnt currBest,uint rootIdx,uint dim) const;
};

}

#endif
