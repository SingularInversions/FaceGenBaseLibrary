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

// The simple accumulator method has identical precision as more complex methods such as 
// 'fgMatSumPrecise' below on test sets with 1M identical irrationals under VS2008.
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

// Theoretically increases precision for large summations, without the compiler flag difficulties
// of Kahan accumulation, but tests have shown no advantage. No idea why.
double      fgMatSumPrecise(const FgMatrixD & mat)
{
    const uint    stackSize = 32;
    FGASSERT(fgLog2Ceil(mat.numElems()) < stackSize);
    StackElem    acc[stackSize];
    for (uint ii=0; ii<mat.numElems(); ++ii)
    {
        acc[0].acc += mat[ii];
        uint    jj=0;
        while (acc[jj].overflow)
        {
            acc[jj+1].acc += acc[jj].acc;
            acc[jj].acc = 0.0;
            acc[jj].overflow = false;
            ++jj;
        }
    }
    double        tmp = 0.0;
    for (uint jj=0; jj<stackSize; jj++)
        tmp += acc[jj].acc;
    return tmp;
}

FgMatrixD
fgRelDiff(const FgMatrixD & a,const FgMatrixD & b)
{
    FgMatrixD   ret;
    FGASSERT(a.dims() == b.dims());
    ret.resize(a.dims());
    ret.m_data = fgRelDiff(a.m_data,b.m_data);
    return ret;
}

static
void
testFgMatSumPrecise()
{
    const uint      numElems = 10000000;
    const double    elem = sqrt(2.0);
    double          sum0 = elem * double(numElems);
    FgMatrixD       mat(numElems,1,elem);
    double          sum1 = 0.0;
    for (uint ii=0; ii<numElems; ii++)
        sum1 += mat[ii];
    double          sum2 = fgMatSumPrecise(mat);
    double          diff1 = sum0 - sum1,
                    diff2 = sum0 - sum2;
    FGASSERT(fgApproxEqual(diff1+1.0,diff2+1.0));
    fgout << "Diff approx: " << diff1 << " diff precise: " << diff2;
}

void
fgMatrixVTest(const FgArgs &)
{
    testFgMatSumPrecise();
}
