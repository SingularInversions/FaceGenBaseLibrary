//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 24, 2009
//

#include "stdafx.h"

#include "FgMatrixV.hpp"
#include "FgMath.hpp"
#include "FgAlgs.hpp"
#include "FgOut.hpp"
#include "FgApproxEqual.hpp"
#include "FgSyntax.hpp"

using namespace std;

double
fgMatSumElems(const FgMatrixD & mat)
{
    double      acc = 0.0;
    for (uint ii=0; ii<mat.numElems(); ii++)
        acc += mat[ii];
    return acc;
}

struct    StackElem
{
    StackElem() : acc(0.0), overflow(false) {}
    double        acc;
    bool        overflow;
};

FgMatrixD
fgRelDiff(const FgMatrixD & a,const FgMatrixD & b,double minAbs)
{
    FgMatrixD   ret;
    FGASSERT(a.dims() == b.dims());
    ret.resize(a.dims());
    ret.m_data = fgRelDiff(a.m_data,b.m_data,minAbs);
    return ret;
}
