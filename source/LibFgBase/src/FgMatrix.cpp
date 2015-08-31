//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 10, 2015
//

#include "stdafx.h"

#include "FgMatrix.hpp"

using namespace std;

FgMatrixC<FgMatrixD,2,2>
fgPartition(const FgMatrixD & m,uint loSize)
{
    FgMatrixC<FgMatrixD,2,2>    ret;
    FGASSERT(m.nrows == m.ncols);
    uint                        hiSize = m.ncols - loSize;
    ret.elem(0,0) = m.subMatrix(0,0,loSize,loSize);
    ret.elem(0,1) = m.subMatrix(0,loSize,loSize,hiSize);
    ret.elem(1,0) = m.subMatrix(loSize,0,hiSize,loSize);
    ret.elem(1,1) = m.subMatrix(loSize,loSize,hiSize,hiSize);
    return ret;
}

FgMatrixD
fgJoin(
    const FgMatrixD &   ll,
    const FgMatrixD &   lh,
    const FgMatrixD &   hl,
    const FgMatrixD &   hh)
{
    FGASSERT((ll.nrows == lh.nrows) && (ll.ncols == hl.ncols));
    FGASSERT((hh.nrows == hl.nrows) && (hh.ncols == lh.ncols));
    return fgConcatVert(fgConcatHoriz(ll,lh),fgConcatHoriz(hl,hh));
}

// */
