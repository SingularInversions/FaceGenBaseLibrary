//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgMatrixV.hpp"
#include "FgMath.hpp"
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

namespace Fg {

template<>
MatF
operator*(const MatF & lhs,const MatF & rhs)
{
    if (lhs.ncols < 100)
        return matMul(lhs,rhs);
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
    MatF           ret(lhs.nrows,rhs.ncols);
    for (size_t rr=0; rr<ret.nrows; ++rr)
        for (size_t cc=0; cc<ret.ncols; ++cc)
            ret.rc(rr,cc) = m(rr,cc);
    return ret;
}

template<>
MatD
operator*(const MatD & lhs,const MatD & rhs)
{
    if (lhs.ncols < 100)
        return matMul(lhs,rhs);
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
    MatD           ret(lhs.nrows,rhs.ncols);
    for (size_t rr=0; rr<ret.nrows; ++rr)
        for (size_t cc=0; cc<ret.ncols; ++cc)
            ret.rc(rr,cc) = m(rr,cc);
    return ret;
}

double
fgMatSumElems(const MatD & mat)
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

MatD
fgRelDiff(const MatD & a,const MatD & b,double minAbs)
{
    MatD   ret;
    FGASSERT(a.dims() == b.dims());
    ret.resize(a.dims());
    ret.m_data = fgRelDiff(a.m_data,b.m_data,minAbs);
    return ret;
}

Mat<MatD,2,2>
fgPartition(const MatD & m,size_t loSize)
{
    Mat<MatD,2,2>    ret;
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
testCorrect(const CLArgs &)
{
    MatD       M = {2,2,{1,2,3,5}},
                    N = M * M,
                    R = {2,2,{7,12,18,31}};
    FGASSERT(N == R);
}

MatD
tt0(const MatD & lhs,const MatD & rhs)
{
    MatD           mat(lhs.nrows,rhs.ncols,0.0);
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

MatD
tt1(const MatD & lhs,const MatD & rhs)
{
    MatD           mat(lhs.nrows,rhs.ncols,0.0);
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

MatD
tt2(const MatD & lhs,const MatD & rhs)
{
    const size_t        CN = 64 / sizeof(double);    // Number of elements that fit in L1 Cache (est)
    MatD           mat(lhs.nrows,rhs.ncols,0.0);
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

MatD
tt3(const MatD & lhs,const MatD & rhs)
{
    const size_t        CN = 64 / sizeof(double);    // Number of elements that fit in L1 Cache (est)
    MatD           mat(lhs.nrows,rhs.ncols,0.0);
    FGASSERT(lhs.ncols == rhs.nrows);
    for (size_t rr=0; rr<mat.nrows; rr+=CN) {
        size_t          R2 = cMin(CN,mat.nrows-rr);
        for (size_t cc=0; cc<mat.ncols; cc+=CN) {
            size_t          C2 = cMin(CN,mat.ncols-cc);
            if (C2 < CN) {                          // Keep paths separate so inner loop can be unrolled below
                for (size_t kk=0; kk<lhs.ncols; kk+=CN) {
                    size_t          K2 = cMin(CN,lhs.ncols-kk);
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
                    size_t          K2 = cMin(CN,lhs.ncols-kk);
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
showMul(function<MatD(const MatD &,const MatD &)> fn,const MatD & l,const MatD & r,const string & desc)
{
    fgout << fgnl << desc << " : ";
    FgTimer         timer;
    MatD       m3 = fn(l,r);
    size_t          elapsed = timer.readMs();
    fgout << elapsed << " ms";
}

void
testMul(const CLArgs & args)
{
    if (fgAutomatedTest(args))
        return;
    Syntax        syn(args,"<size>");
    size_t          sz = fgFromStr<size_t>(syn.next()).val();
    MatD       m0 = MatD::randNormal(sz,sz),
                    m1 = MatD::randNormal(sz,sz);
    FgTimer         timer;
    MatD       m2 = m0 * m1;
    size_t          time = timer.readMs();
    fgout << sz << ": " << time << "ms";
}

void
loopStructTime(const CLArgs & args)
{
    if (fgAutomatedTest(args))
        return;
    uint            dim = 1024;     // Must divide 8 for non-generalized tests
    MatD       m1 = MatD::randNormal(dim,dim),
                    m2 = MatD::randNormal(dim,dim);
    showMul(tt0,m1,m2,"1 sub-loop");
    showMul(tt1,m1,m2,"3 sub-loops");
    showMul(tt2,m1,m2,"1 sub-loop generalized");
    showMul(tt3,m1,m2,"3 sub-loops generalized");
}

void
eigenTest(const CLArgs & args)
{
    if (fgAutomatedTest(args))
        return;
    Syntax            syn(args,"<size>");
    size_t              sz = fgFromStr<size_t>(syn.next()).val();
    MatrixXd            l(sz,sz),
                        r(sz,sz);
    for (size_t rr=0; rr<sz; ++rr) {
        for (size_t cc=0; cc<sz; ++cc) {
            l(rr,cc) = randUniform();
            r(rr,cc) = randUniform();
        }
    }
    {
        FgTimeScope     ts("Eigen mat mul " + toString(sz));
        MatrixXd        m = l * r;
    }
}

}

void
fgMatrixVTest(const CLArgs & args)
{
    vector<Cmd>   cmds;
    cmds.push_back(Cmd(testCorrect,"correct"));
    cmds.push_back(Cmd(eigenTest,"tem","Time eigen mat mul"));
    cmds.push_back(Cmd(testMul,"tlm","Time loop mat mul"));
    cmds.push_back(Cmd(loopStructTime,"lst","Loop structure timing experiment"));
    fgMenu(args,cmds,true);
}

}
