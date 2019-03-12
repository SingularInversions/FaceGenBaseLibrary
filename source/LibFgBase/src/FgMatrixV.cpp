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

#ifdef _MSC_VER
    #pragma warning(push,0)     // Eigen triggers lots of warnings
#endif

#define EIGEN_MPL2_ONLY     // Only use permissive licensed source files from Eigen
#include "Eigen/Dense"
#include "Eigen/Core"

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

using namespace std;

using namespace Eigen;

template<>
FgMatrixF
operator*(const FgMatrixF & lhs,const FgMatrixF & rhs)
{
    if (lhs.ncols < 100)
        return fgMatMul(lhs,rhs);
    FGASSERT(lhs.ncols == rhs.nrows);
    MatrixXf            l(lhs.nrows,lhs.ncols),
                        r(rhs.nrows,rhs.ncols);
    for (size_t rr=0; rr<lhs.nrows; ++rr)
        for (size_t cc=0; cc<lhs.ncols; ++cc)
            l(rr,cc) = lhs.rc(rr,cc);
    for (size_t rr=0; rr<rhs.nrows; ++rr)
        for (size_t cc=0; cc<rhs.ncols; ++cc)
            r(rr,cc) = rhs.rc(rr,cc);
    MatrixXf            m = l * r;
    FgMatrixF           ret(lhs.nrows,rhs.ncols);
    for (size_t rr=0; rr<ret.nrows; ++rr)
        for (size_t cc=0; cc<ret.ncols; ++cc)
            ret.rc(rr,cc) = m(rr,cc);
    return ret;
}

template<>
FgMatrixD
operator*(const FgMatrixD & lhs,const FgMatrixD & rhs)
{
    if (lhs.ncols < 100)
        return fgMatMul(lhs,rhs);
    FGASSERT(lhs.ncols == rhs.nrows);
    MatrixXd            l(lhs.nrows,lhs.ncols),
                        r(rhs.nrows,rhs.ncols);
    for (size_t rr=0; rr<lhs.nrows; ++rr)
        for (size_t cc=0; cc<lhs.ncols; ++cc)
            l(rr,cc) = lhs.rc(rr,cc);
    for (size_t rr=0; rr<rhs.nrows; ++rr)
        for (size_t cc=0; cc<rhs.ncols; ++cc)
            r(rr,cc) = rhs.rc(rr,cc);
    MatrixXd            m = l * r;
    FgMatrixD           ret(lhs.nrows,rhs.ncols);
    for (size_t rr=0; rr<ret.nrows; ++rr)
        for (size_t cc=0; cc<ret.ncols; ++cc)
            ret.rc(rr,cc) = m(rr,cc);
    return ret;
}

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

namespace {

void
testCorrect(const FgArgs &)
{
    FgMatrixD       M = {2,2,{1,2,3,5}},
                    N = M * M,
                    R = {2,2,{7,12,18,31}};
    FGASSERT(N == R);
}

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

void
showMul(function<FgMatrixD(const FgMatrixD &,const FgMatrixD &)> fn,const FgMatrixD & l,const FgMatrixD & r,const string & desc)
{
    fgout << fgnl << desc << " : ";
    FgTimer         timer;
    FgMatrixD       m3 = fn(l,r);
    size_t          elapsed = timer.readMs();
    fgout << elapsed << " ms";
}

void
testMul(const FgArgs & args)
{
    if (fgAutomatedTest(args))
        return;
    FgSyntax        syn(args,"<size>");
    size_t          sz = fgFromStr<size_t>(syn.next()).val();
    FgMatrixD       m0 = FgMatrixD::randNormal(sz,sz),
                    m1 = FgMatrixD::randNormal(sz,sz);
    FgTimer         timer;
    FgMatrixD       m2 = m0 * m1;
    size_t          time = timer.readMs();
    fgout << sz << ": " << time << "ms";
}

void
loopStructTime(const FgArgs & args)
{
    if (fgAutomatedTest(args))
        return;
    uint            dim = 1024;     // Must divide 8 for non-generalized tests
    FgMatrixD       m1 = FgMatrixD::randNormal(dim,dim),
                    m2 = FgMatrixD::randNormal(dim,dim);
    showMul(tt0,m1,m2,"1 sub-loop");
    showMul(tt1,m1,m2,"3 sub-loops");
    showMul(tt2,m1,m2,"1 sub-loop generalized");
    showMul(tt3,m1,m2,"3 sub-loops generalized");
}

void
eigenTest(const FgArgs & args)
{
    if (fgAutomatedTest(args))
        return;
    FgSyntax            syn(args,"<size>");
    size_t              sz = fgFromStr<size_t>(syn.next()).val();
    MatrixXd            l(sz,sz),
                        r(sz,sz);
    for (size_t rr=0; rr<sz; ++rr) {
        for (size_t cc=0; cc<sz; ++cc) {
            l(rr,cc) = fgRand();
            r(rr,cc) = fgRand();
        }
    }
    {
        FgTimeScope     ts("Eigen mat mul " + fgToStr(sz));
        MatrixXd        m = l * r;
    }
}

}

void
fgMatrixVTest(const FgArgs & args)
{
    vector<FgCmd>   cmds;
    cmds.push_back(FgCmd(testCorrect,"correct"));
    cmds.push_back(FgCmd(eigenTest,"tem","Time eigen mat mul"));
    cmds.push_back(FgCmd(testMul,"tlm","Time loop mat mul"));
    cmds.push_back(FgCmd(loopStructTime,"lst","Loop structure timing experiment"));
    fgMenu(args,cmds,true);
}
