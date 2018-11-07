//
// Copyright (c) 2015 Singular Inversions Inc.
//
// Authors:     Andrew Beatty
// Created:     May 23, 2011
//

#ifndef FGKDTREE_HPP
#define FGKDTREE_HPP

#include "FgMatrixC.hpp"
#include "FgOpt.hpp"
#include "FgMath.hpp"

struct  FgMagPnt
{
    float       mag;        // Squared distance to closest point
    FgVect3F    pnt;        // Closest point
};

class   FgKdTree
{
public:
    FgKdTree(const std::vector<FgVect3F> & pnts);   // Must be non-empty

    FgMagPnt
    closest(FgVect3F pos) const;

private:
    struct  Node
    {
        Node(FgVect3F v) : vert(v) {}
        Node(FgVect3F v,uint l) : vert(v), idxLo(l) {}
        Node(FgVect3F v,uint l,uint h) : vert(v), idxLo(l), idxHi(h) {}

        FgVect3F        vert;
        FgValid<uint>   idxLo;
        FgValid<uint>   idxHi;
    };
    std::vector<Node>   m_tree;

    uint
    createNode(const std::vector<FgVect3F> & verts,uint dim);

    FgMagPnt
    findBest(const FgVect3F & pos,FgMagPnt currBest,uint rootIdx,uint dim) const;
};

#endif
