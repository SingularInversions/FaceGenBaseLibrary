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
#include "FgTime.hpp"
#include "FgCommand.hpp"

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

FgMatrixC<FgMatrixD,2,2>
fgPartition(const FgMatrixD & m,size_t loSize)
{
    FgMatrixC<FgMatrixD,2,2>    ret;
    FGASSERT(m.nrows == m.ncols);
    size_t                      hiSize = m.ncols - loSize;
    ret.rc(0,0) = m.subMatrix(0,0,loSize,loSize);
    ret.rc(0,1) = m.subMatrix(0,loSize,loSize,hiSize);
    ret.rc(1,0) = m.subMatrix(loSize,0,hiSize,loSize);
    ret.rc(1,1) = m.subMatrix(loSize,loSize,hiSize,hiSize);
    return ret;
}

static
void
testCorrect(const FgArgs &)
{
    FgMatrixD       M = {2,2,{1,2,3,5}},
                    N = M * M,
                    R = {2,2,{7,12,18,31}};
    FGASSERT(N == R);
}

static
FgMatrixD
tt0(const FgMatrixD & lhs,const FgMatrixD & rhs)
{
    FgMatrixD           mat(lhs.nrows,rhs.ncols,0.0);
    FGASSERT(lhs.ncols == rhs.nrows);
    const size_t          CN = 8;
    for (size_t rr=0; rr<mat.nrows; rr++) {
        for (size_t cc=0; cc<mat.ncols; cc+=CN) {
            for (size_t kk=0; kk<lhs.ncols; kk++) {
                double          lv = lhs.rc(rr,kk);
                for (size_t cc2=0; cc2<CN; ++cc2)
                    mat.rc(rr,cc+cc2) += lv * rhs.rc(kk,cc+cc2);
            }
        }
    }
    return mat;
}

static
FgMatrixD
tt1(const FgMatrixD & lhs,const FgMatrixD & rhs)
{
    FgMatrixD           mat(lhs.nrows,rhs.ncols,0.0);
    FGASSERT(lhs.ncols == rhs.nrows);
    const size_t          CN = 8;
    for (size_t rr=0; rr<mat.nrows; rr+=CN) {
        for (size_t cc=0; cc<mat.ncols; cc+=CN) {
            for (size_t kk=0; kk<lhs.ncols; kk+=CN) {
                for (size_t rr2=0; rr2<CN; ++rr2) {
                    for (size_t kk2=0; kk2<CN; ++kk2) {
                        double          lv = lhs.rc(rr+rr2,kk+kk2);
                        for (size_t cc2=0; cc2<CN; ++cc2)
                            mat.rc(rr+rr2,cc+cc2) += lv * rhs.rc(kk+kk2,cc+cc2);
                    }
                }
            }
        }
    }
    return mat;
}

static
FgMatrixD
tt2(const FgMatrixD & lhs,const FgMatrixD & rhs)
{
    const size_t        CN = 64 / sizeof(double);    // Number of elements that fit in L1 Cache (est)
    FgMatrixD           mat(lhs.nrows,rhs.ncols,0.0);
    FGASSERT(lhs.ncols == rhs.nrows);
    for (size_t rr=0; rr<mat.nrows; rr++) {
        size_t            mIdx = mat.ncols * rr;
        for (size_t cc=0; cc<mat.ncols; cc+=CN) {
            if (cc+CN<=mat.ncols) {             // Full cache size case hard-coded for unrolling:
                for (size_t kk=0; kk<lhs.ncols; kk++) {
                    double          lv = lhs.rc(rr,kk);
                    size_t            rIdx = rhs.ncols * kk + cc;
                    for (size_t cc2=0; cc2<CN; ++cc2)
                        mat.m_data[mIdx+cc+cc2] += lv * rhs.m_data[rIdx+cc2];
                }
            }
            else {                              // Remaining < CN elements:
                for (size_t kk=0; kk<lhs.ncols; kk++) {
                    double          lv = lhs.rc(rr,kk);
                    size_t            rIdx = rhs.ncols * kk;
                    for (size_t cc2=cc; cc2<mat.ncols; ++cc2)
                        mat.m_data[mIdx+cc2] += lv * rhs.m_data[rIdx+cc2];
                }
            }
        }
    }
    return mat;
}

static
FgMatrixD
tt3(const FgMatrixD & lhs,const FgMatrixD & rhs)
{
    const size_t        CN = 64 / sizeof(double);    // Number of elements that fit in L1 Cache (est)
    FgMatrixD           mat(lhs.nrows,rhs.ncols,0.0);
    FGASSERT(lhs.ncols == rhs.nrows);
    for (size_t rr=0; rr<mat.nrows; rr+=CN) {
        size_t          R2 = fgMin(CN,mat.nrows-rr);
        for (size_t cc=0; cc<mat.ncols; cc+=CN) {
            size_t          C2 = fgMin(CN,mat.ncols-cc);
            if (C2 < CN) {                          // Keep paths separate so inner loop can be unrolled below
                for (size_t kk=0; kk<lhs.ncols; kk+=CN) {
                    size_t          K2 = fgMin(CN,lhs.ncols-kk);
                    for (size_t rr2=0; rr2<R2; ++rr2) {
                        size_t          mIdx = (rr+rr2)*mat.ncols + cc;
                        for (size_t kk2=0; kk2<K2; ++kk2) {
                            double          lv = lhs.rc(rr+rr2,kk+kk2);
                            size_t          rIdx = (kk+kk2)*rhs.ncols + cc;
                            for (size_t cc2=0; cc2<C2; ++cc2)
                                mat.m_data[mIdx+cc2] += lv * rhs.m_data[rIdx+cc2];
                        }
                    }
                }
            }
            else {
                for (size_t kk=0; kk<lhs.ncols; kk+=CN) {
                    size_t          K2 = fgMin(CN,lhs.ncols-kk);
                    for (size_t rr2=0; rr2<R2; ++rr2) {
                        size_t          mIdx = (rr+rr2)*mat.ncols + cc;
                        for (size_t kk2=0; kk2<K2; ++kk2) {
                            double          lv = lhs.rc(rr+rr2,kk+kk2);
                            size_t          rIdx = (kk+kk2)*rhs.ncols + cc;
                            for (size_t cc2=0; cc2<CN; ++cc2)   // CN is compile-time const so this loop is unrolled
                                mat.m_data[mIdx+cc2] += lv * rhs.m_data[rIdx+cc2];
                        }
                    }
                }
            }
        }
    }
    return mat;
}

static
void
testMul(function<FgMatrixD(const FgMatrixD &,const FgMatrixD &)> fn,const FgMatrixD & l,const FgMatrixD & r,const string & desc)
{
    fgout << fgnl << desc << " : ";
    FgTimer         timer;
    FgMatrixD       m3 = fn(l,r);
    size_t          elapsed = timer.readMs();
    fgout << elapsed << " ms";
}

static
void
testTime(const FgArgs &)
{
    uint            dim = 128;  // Must divide 8 for non-generalized tests
    FgMatrixD       m1 = FgMatrixD::randNormal(dim,dim),
                    m2 = FgMatrixD::randNormal(dim,dim);
    testMul(tt0,m1,m2,"1 sub-loop");
    testMul(tt1,m1,m2,"3 sub-loops");
    testMul(tt2,m1,m2,"1 sub-loop generalized");
    testMul(tt3,m1,m2,"3 sub-loops generalized");
}

void
fgMatrixVTest(const FgArgs & args)
{
    vector<FgCmd>   cmds;
    cmds.push_back(FgCmd(testCorrect,"correct"));
#ifndef _DEBUG          // Too slow for debug
    FGADDCMD1(fgMatrixEigenTest,"eigen");
#endif
    cmds.push_back(FgCmd(testTime,"time"));
    fgMenu(args,cmds,true);
}
