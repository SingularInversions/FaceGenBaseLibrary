//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//

#include "stdafx.h"

#include "FgViz.hpp"
#include "FgAffineCwC.hpp"
#include "FgImgDisplay.hpp"
#include "Fg3dMeshOps.hpp"
#include "Fg3dDisplay.hpp"

using namespace std;

namespace Fg {

void
fgVizFuncAsImage(
    std::function<double(Vec2D)>   func,
    Mat22D                         domainBounds,
    uint                                sz)
{
    ImgD        viz(sz,sz);
    AffineEw2D  imgToFuncDomain(Mat22D(0,sz-1,0,sz-1),domainBounds);
    for (Iter2UI it(sz); it.valid(); it.next())
        viz[it()] = func(imgToFuncDomain * Vec2D(it()));
    fgImgDisplayColorize(viz);
}

Mesh
fgFuncToMesh(
    std::function<double(Vec2D)>   func,
    Mat22D                         domainBounds,
    uint                                sz)
{
    ImgD        viz(sz,sz);
    AffineEw2D  imgToFuncDomain(Mat22D(0,sz-1,0,sz-1),domainBounds);
    for (Iter2UI it(sz); it.valid(); it.next())
        viz[it()] = func(imgToFuncDomain * Vec2D(it()));
    return fgMeshFromImage(viz);
}

}
