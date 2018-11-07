//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 9, 2013
//

#include "stdafx.h"

#include "FgViz.hpp"
#include "FgAffineCwC.hpp"
#include "FgImgDisplay.hpp"
#include "Fg3dMeshOps.hpp"
#include "Fg3dDisplay.hpp"

using namespace std;

void
fgVizFuncAsImage(
    std::function<double(FgVect2D)>   func,
    FgMat22D                         domainBounds,
    uint                                sz)
{
    FgImgD        viz(sz,sz);
    FgAffineCw2D  imgToFuncDomain(FgMat22D(0,sz-1,0,sz-1),domainBounds);
    for (FgIter2UI it(sz); it.valid(); it.next())
        viz[it()] = func(imgToFuncDomain * FgVect2D(it()));
    fgImgDisplayColorize(viz);
}

Fg3dMesh
fgFuncToMesh(
    std::function<double(FgVect2D)>   func,
    FgMat22D                         domainBounds,
    uint                                sz)
{
    FgImgD        viz(sz,sz);
    FgAffineCw2D  imgToFuncDomain(FgMat22D(0,sz-1,0,sz-1),domainBounds);
    for (FgIter2UI it(sz); it.valid(); it.next())
        viz[it()] = func(imgToFuncDomain * FgVect2D(it()));
    return fgMeshFromImage(viz);
}
