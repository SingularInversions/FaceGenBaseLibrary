//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgMatrixV.hpp"
#include "FgMath.hpp"
#include "FgSerial.hpp"
#include "FgApproxEqual.hpp"
#include "FgTime.hpp"
#include "FgCommand.hpp"
#include "FgBounds.hpp"

#ifdef _MSC_VER
    #pragma warning(push,0)     // Eigen triggers lots of warnings
#endif

#define EIGEN_MPL2_ONLY         // Only use permissive licensed source files from Eigen
#include "Eigen/Dense"
#include "Eigen/Core"

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

using namespace std;

using namespace Eigen;

namespace Fg {

template<>
MatF                operator*(const MatF & lhs,const MatF & rhs)
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
MatD                operator*(MatD const & lhs,MatD const & rhs)
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

MatD                cRelDiff(MatD const & a,MatD const & b,double minAbs)
{
    MatD   ret;
    FGASSERT(a.dims() == b.dims());
    ret.resize(a.dims());
    ret.m_data = cRelDiff(a.m_data,b.m_data,minAbs);
    return ret;
}

// Uses simple Gram-Schmidt; probably not very accurate for large 'dim':
MatD                cRandMatOrthogonal(size_t dim)
{
    FGASSERT(dim > 1);
    Doubless            vecs;
    for (uint row=0; row<dim; ++row) {
        Doubles             vec = cRandNormals(dim);
        for (Doubles const & axis : vecs)
            vec -=  axis * cDot(vec,axis);
        vecs.push_back(normalize(vec));
    }
    return MatD{vecs};
}

MatD                cRandMatMhlbs(size_t D,double lnScaleStdev)
{
    auto                fn = [=](size_t){return exp(cRandNormal()*lnScaleStdev); };
    Doubles             scales = genSvec(D,fn);
    MatD                R = cRandMatOrthogonal(D);
    return cMatDiag(scales) * R;
}

MatSD               cRandMatRsm(size_t D,double eigvalStdev)
{
    Doubles             eigs = cRandNormals(D,0,eigvalStdev);
    MatD                R = cRandMatOrthogonal(D);
    return selfDiagTransposeProduct(R,eigs);
}

MatSD               cRandMatSpd(size_t D,double lnScaleStdev)
{
    auto                fn = [=](size_t){return exp(cRandNormal()*lnScaleStdev); };
    Doubles             scales = genSvec(D,fn);
    MatD                R = cRandMatOrthogonal(D);
    return selfDiagTransposeProduct(R,scales);
}

Doubles             SparseRsm::apply(double const * v) const
{
    auto                fn = [&](size_t ii){return diags[ii] * v[ii]; };
    Doubles             ret = genSvec(diags.size(),fn);
    for (IdxVal iv : offds) {
        ret[iv.rc[0]] += v[iv.rc[1]] * iv.val;
        ret[iv.rc[1]] += v[iv.rc[0]] * iv.val;
    }
    return ret;
}

Doubles             SparseRsm::operator*(Doubles const & v) const
{
    FGASSERT(v.size() == diags.size());
    Doubles             ret = mapMul(diags,v);
    for (IdxVal iv : offds) {
        ret[iv.rc[0]] += v[iv.rc[1]] * iv.val;
        ret[iv.rc[1]] += v[iv.rc[0]] * iv.val;
    }
    return ret;
}

MatSD               SparseRsm::asMatS() const
{
    MatSD               ret = cMatSDiag(diags);
    for (IdxVal iv : offds)
        ret.rc(iv.rc[0],iv.rc[1]) = iv.val;
    return ret;
}

ostream &           operator<<(ostream & os,SparseRsm const & sr)
{
    std::ios::fmtflags  oldFlag = os.setf(std::ios::fixed | std::ios::showpos | std::ios::right);
    std::streamsize     oldPrec = os.precision(6);
    os << fgpush;
    // just print the LT with the remaining left blank for clarity:
    for (uint rr=0; rr<sr.dim(); rr++) {
        os << fgnl;
        os << "[ ";
        for (uint cc=0; cc<rr; cc++) {
            size_t          idx = findFirstIdx(sr.offds,Arr2UI{cc,rr});
            if (idx < sr.offds.size())
                os << sr.offds[idx].val << " ";
            else
                os << "    0     ";
        }
        os << sr.diags[rr] << " ";
        for (uint cc=rr+1; cc<sr.dim(); ++cc)
            os << "          ";
        os << "]";
    }
    os << fgpop;
    os.flags(oldFlag);
    os.precision(oldPrec);
    return os;
}

double              cQuadForm(SparseRsm const & prec,double const * v)
{
    Doubles             pv = prec.apply(v);
    double              ret {0};
    for (size_t ii=0; ii<prec.dim(); ++ii)
        ret += pv[ii] * v[ii];
    return ret;
}

double              cQuadForms(SparseRsm const & prec,MatD const & vs)
{
    FGASSERT(prec.dim() == vs.numCols());
    double              ret {0};
    for (size_t rr=0; rr<vs.numRows(); ++rr)
        ret += cQuadForm(prec,vs.rowPtr(rr));
    return ret;
}

SparseRsm           cRandSparseRsm(size_t D)
{
    SparseRsm           sr {
        genSvec(D,[](size_t){return exp(cRandNormal()); }),
        {},
    };
    // add 50% sparsity:
    for (uint rr=0; rr<D; ++rr) {
        for (uint cc=rr+1; cc<D; ++cc) {
            if (cRandUniform() > 0.5) {
                double          val = cRandUniform(-0.9,0.9) * sqrt(sr.diags[rr]*sr.diags[cc]);
                sr.offds.emplace_back(Arr2UI{rr,cc},val);   // these are in sorted order
            }
        }
    }
    return sr;
}

void                testMatSparse(CLArgs const &)
{
    randSeedRepeatable();
    size_t constexpr    D = 5;
    for (size_t ii=0; ii<3; ++ii) {
        SparseRsm           sr = cRandSparseRsm(D);
        MatSD               matS = sr.asMatS();
        Doubles             v = cRandNormals(D);
        FGASSERT(isApproxEqual(cQuadForm(sr,v.data()),cQuadForm(matS,v.data()),epsBits(20)));
    }
}

MatUT2D             cCholesky(MatS2D s)
{
    FGASSERT(s.m00 > 0);
    double              r00 = sqrt(s.m00),
                        r01 = s.m01 / r00,
                        tmp = s.m11 - sqr(r01);
    FGASSERT(tmp > 0);
    return {r00,sqrt(tmp),r01};
}

MatUT3D             cCholesky(MatS3D const & S)
{
    MatUT3D             U;
    FGASSERT(S.diag[0] > 0);
    U.m[0] = sqrt(S.diag[0]);
    U.m[1] = S.offd[0] / U.m[0];
    U.m[2] = S.offd[1] / U.m[0];
    double              tmp0 = S.diag[1] - sqr(S.offd[0])/S.diag[0];
    FGASSERT(tmp0 > 0);
    U.m[3] = sqrt(tmp0);
    double              tmp1 = S.offd[2] - S.offd[1]*S.offd[0]/S.diag[0];
    U.m[4] = tmp1 / U.m[3];
    double              tmp2 = S.diag[2] - sqr(S.offd[1])/S.diag[0] - sqr(tmp1)/tmp0;
    FGASSERT(tmp2 > 0);
    U.m[5] = sqrt(tmp2);
    return U;
}
static void         testCholesky(CLArgs const &)
{
    randSeedRepeatable();
    double constexpr    tol = epsBits(30);
    size_t              N = 1000;
    for (size_t ii=0; ii<N; ++ii) {
        MatS2D              spds = MatS2D::randSpd(3.0);
        MatUT2D             ch = cCholesky(spds);
        MatS2D              lu = ch.luProduct();
        FGASSERT(isApproxEqual(spds.m00,lu.m00,tol));
        FGASSERT(isApproxEqual(spds.m01,lu.m01,tol));
        FGASSERT(isApproxEqual(spds.m11,lu.m11,tol));
    }
    for (size_t ii=0; ii<N; ++ii) {
        MatS3D              spds = MatS3D::randSpd(3.0);
        MatUT3D             ch = cCholesky(spds);
        MatS3D              lu = ch.luProduct();
        FGASSERT(isApproxEqual(spds.diag,lu.diag,tol));
        FGASSERT(isApproxEqual(spds.offd,lu.offd,tol));
    }
}

Vec2D               solveLinear(MatS2D fr,Vec2D b)
{
    double              det = fr.m00 * fr.m11 - sqr(fr.m01),    // will be non-zero for full rank matrix
                        n0 = fr.m11 * b[0] - fr.m01 * b[1],
                        n1 = fr.m00 * b[1] - fr.m01 * b[0];
    return {n0/det,n1/det};
}
void                testSolveS2(CLArgs const &)
{
    for (size_t ii=0; ii<100; ++ii) {
        // TODO: currently just tests SPD but 'solve' should work for all full rank:
        double              lnEigRat = cRandUniform(0,-log(epsBits(20))),    // ln eigvalue ratio within limits
                            ev0 = exp(cRandNormal()),
                            ev1 = ev0 * exp(-lnEigRat),
                            theta = cRandUniform(-pi,pi),
                            c = cos(theta),
                            s = sin(theta);
        Mat22D              D {ev0, 0, 0, ev1},
                            R {c, s, -s, c},
                            M = R * D * R.transpose();
        MatS2D              S {M.rc(0,0), M.rc(1,1), M.rc(0,1)};
        Vec2D               x = Vec2D::randNormal(),
                            b = M * x,
                            t = solveLinear(S,b);
        FGASSERT(isApproxEqualPrec(t,x,20));
    }
}

Vec3D               solveLinear(MatS3D A,Vec3D b)
{
    MatUT3D             cd = cCholesky(A);
    MatUT3D             I = cd.inverse();
    Vec3D               c = I.tranposeMul(b);
    return I * c;
}

ostream &           operator<<(ostream & os,RsmEigs const & rsm)
{
    return os << rsm.vals << rsm.vecs;
}

Doubles             solveLinear(MatSD const & M,Doubles b)
{
    FGASSERT(M.dim == b.size());
    RsmEigs             eigs = cRsmEigs(M);
    Arr2D               evBounds = cBounds(mapAbs(eigs.vals));
    double              condRatio = evBounds[0] / evBounds[1];
    if (condRatio < epsBits(20))
        fgThrow("solveLinear poorly conditioned",condRatio);
    // RLR^x=b -> LR^x = R^b, solve for R^x then x = R(R^x)
    MatD                rt = transpose(eigs.vecs);
    Doubles             rtb = rt * b,
                        rtx = mapDiv(rtb,eigs.vals);
    return eigs.vecs * rtx;
}
void                testSolveLinearMatSD(CLArgs const &)
{
    auto                fn = [](size_t D)
    {
        MatSD               M = cRandMatRsm(D,3);
        Doubles             x = cRandNormals(D,3),
                            b = M*x,
                            r = solveLinear(M,b);
        FGASSERT(isApproxEqual(x,r,epsBits(20)));
    };
    for (size_t dd=2; dd<=32; ++dd)
        fn(dd);
}

double              cLnDeterminant(MatSD const & rsm)
{
    double              ret {0};
    for (double ev : cEigvalsRsm(rsm)) {
        FGASSERT(ev>0);
        ret += std::log(ev);
    }
    return ret;
}

namespace {

void                testMatVec(CLArgs const &)
{
    for (size_t ii=0; ii<8; ++ii) {
        MatD                M = MatD::randNormal(4,4);
        Doubles             v = cRandNormals(4),
                            t0 = M * v,
                            t1 = v * transpose(M);
        FGASSERT(isApproxEqual(t0,t1,epsBits(30)));
    }
}

void                testMatMul(CLArgs const & args)
{
    {   // built-in version:
        MatD            M = {2,2,{1,2,3,5}},
                        N = M * M,
                        R = {2,2,{7,12,18,31}};
        FGASSERT(N == R);
    }
    {   // eigen version:
        if (isAutomated(args))
            return;
        Syntax            syn(args,"<size>");
        size_t              sz = fromStr<size_t>(syn.next()).value();
        MatrixXd            l(sz,sz),
                            r(sz,sz);
        for (size_t rr=0; rr<sz; ++rr) {
            for (size_t cc=0; cc<sz; ++cc) {
                l(rr,cc) = cRandUniform();
                r(rr,cc) = cRandUniform();
            }
        }
        {
            PushTimer     ts("Eigen mat mul " + toStr(sz));
            MatrixXd        m = l * r;
        }
    }
}

MatD                tt0(MatD const & lhs,MatD const & rhs)
{
    MatD                mat(lhs.nrows,rhs.ncols,0.0);
    FGASSERT(lhs.ncols == rhs.nrows);
    size_t const        CN = 8;
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

MatD                tt1(MatD const & lhs,MatD const & rhs)
{
    MatD           mat(lhs.nrows,rhs.ncols,0.0);
    FGASSERT(lhs.ncols == rhs.nrows);
    size_t const          CN = 8;
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

MatD                tt2(MatD const & lhs,MatD const & rhs)
{
    size_t const        CN = 64 / sizeof(double);    // Number of elements that fit in L1 Cache (est)
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

MatD                tt3(MatD const & lhs,MatD const & rhs)
{
    size_t const        CN = 64 / sizeof(double);    // Number of elements that fit in L1 Cache (est)
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

void                testMatMulSpeed(CLArgs const & args)
{
    if (isAutomated(args))
        return;
    Syntax              syn {args,"<size>"};
    size_t              sz = fromStr<size_t>(syn.next()).value();
    MatD                m0 = MatD::randNormal(sz,sz),
                        m1 = MatD::randNormal(sz,sz);
    Timer               timer;
    MatD                m2 = m0 * m1;
    size_t              time = timer.elapsedMilliseconds();
    fgout << sz << ": " << time << "ms";
}

void                testMatMulStruct(CLArgs const & args)
{
    if (isAutomated(args))
        return;
    uint                dim = 1024;     // Must divide 8 for non-generalized tests
    MatD                m1 = MatD::randNormal(dim,dim),
                        m2 = MatD::randNormal(dim,dim);
    auto                showMul = [&](Sfun<MatD(MatD const &,MatD const &)> fn,String const & desc)
    {
        fgout << fgnl << desc << " : ";
        Timer               timer;
        MatD                m3 = fn(m1,m2);
        size_t              elapsed = timer.elapsedMilliseconds();
        fgout << elapsed << " ms";
    };
    showMul(tt0,"1 sub-loop");
    showMul(tt1,"3 sub-loops");
    showMul(tt2,"1 sub-loop generalized");
    showMul(tt3,"3 sub-loops generalized");
}

void                testMatCol(CLArgs const &)
{
    MatI                M {2,2, {1,2, 3,4,}},
                        N {2,0, {}};
    if (appendCol(M,{5,6}) != MatI{2,3,{1,2,5, 3,4,6,}}) FGASSERT_FALSE;   // macros can't contain {}
    if (appendCol(N,{5,6}) != MatI{2,1,{5,6}}) FGASSERT_FALSE;
    if (eraseCol(M,0) != MatI{2,1, {2,4}}) FGASSERT_FALSE;
    if (eraseCol(M,1) != MatI{2,1, {1,3}}) FGASSERT_FALSE;
}

// simple, slow generator of the prime series up to the given number of primes:
template<class T>
Svec<T>             genPrimes(size_t numPrimes)
{
    Svec<T>             ret; ret.reserve(numPrimes);
    ret.push_back(2);
    auto                isPrime = [&](T n)
    {
        for (T prime : ret)
            if ((n%prime) == 0)
                return false;
        return true;
    };
    for (T ii=3; ii<lims<T>::max(); ii+=2) {
        if (isPrime(ii)) {
            ret.push_back(ii);
            if (ret.size() == numPrimes)
                break;
        }
    }
    return ret;
}

void                testMatS(CLArgs const &)
{
    size_t constexpr    D = 5;
    MatSI               S {D, genPrimes<int>(cTriangular(D))};
    MatI                M {D,D, genPrimes<int>(D*D)};
    Ints                v = genPrimes<int>(D);
    {
        PushIndent          pind {"rc(), asMatV(), operator*"};
        MatI                SM = S.asMatV();
        for (size_t rr=0; rr<D; ++rr)
            for (size_t cc=0; cc<D; ++cc)
                FGASSERT(S.rc(rr,cc) == SM.rc(rr,cc));
        FGASSERT(transpose(SM) == SM);
        FGASSERT(S*v == SM*v);
    }
    {
        PushIndent          pind {"cMatSDiag"};
        Ints                vals = genPrimes<int>(D);
        MatI                tst = cMatSDiag(vals).asMatV(),
                            ref = cMatDiag(vals);
        FGASSERT(tst==ref);
    }
    {
        PushIndent          pind {"selfTransposeProduct"};
        MatI                tst = selfTransposeProduct(M).asMatV(),
                            ref = M * transpose(M);
        FGASSERT(tst == ref);
    }
    {
        PushIndent          pind {"selfDiagTransposeProduct"};
        MatI                tst = selfDiagTransposeProduct(M,v).asMatV(),
                            ref = M * cMatDiag(v) * transpose(M);
        FGASSERT(tst == ref);
    }
    {
        PushIndent          pind {"selfSymmTransposeProduct"};
        MatI                tst = selfSymmTransposeProduct(M,S).asMatV(),
                            ref = M * S.asMatV() * transpose(M);
        FGASSERT(tst == ref);
    }
    {
        PushIndent          pind {"partition / catRect"};
        SymmPart<int>       tmp = cPartition(S,2);
        MatSI               tst = catRect(tmp.p00,tmp.p11,tmp.p01);
        FGASSERT(tst == S);
    }
    {
        PushIndent          pind {"cPdDeterminant"};
        MatS                pdrsm = cRandMatSpd(D,1);
        RsmEigs             eigs = cRsmEigs(pdrsm);
        double              detTst = cPdDeterminant(pdrsm),
                            detRef = cProduct(eigs.vals);
        fgout << fgnl << pdrsm << fgnl << " det tst: " << detTst << " ref: " << detRef;
        FGASSERT(isApproxEqual(detTst,detRef,detRef*epsBits(30)));
    }
}

void                testRsmEigs(CLArgs const &)
{
    randSeedRepeatable();
    auto                fn = [](size_t dim)
    {
        Doubles             eigvals = sortAll(cRandNormals(dim));
        MatD                rot = cRandMatOrthogonal(dim);
        MatSD               rsm = selfDiagTransposeProduct(rot,eigvals);
        RsmEigs             eigs = cRsmEigs(rsm);
        MatSD               tst = eigs.asMatS(),
                            inv = cInverse(tst);
        FGASSERT(isApproxEqual(eigs.vals,eigvals,epsBits(20)));
        FGASSERT(isApproxEqual(rsm,tst,epsBits(20)));
        FGASSERT(isApproxEqual(inv.asMatV()*tst.asMatV(),MatD::identity(dim),epsBits(20)));
    };
    for (size_t dd=2; dd<=32; ++dd)
        fn(dd);
}

void                testAsymEigs(CLArgs const &)
{
    Mat33D              mat = matRotateAxis(1.0,normalize(Vec3D{1,1,1}));
    EigsC<3>            eigs = cEigs(mat);
    // We have to test against the reconstructed matrix as the order of eigvals/vecs will
    // differ on different platforms (eg. gcc):
    Mat33D              tst = cReal(eigs.vecs * cMatDiag(eigs.vals) * cHermitian(eigs.vecs));
    FGASSERT(isApproxEqual(mat.m,tst.m,epsBits(20)));
    fgout << eigs << fgnl << "Residual: " << tst-mat;
}

// Random RSM timing tests (21.04, i9-9900K 3.6GHz)
// Dim/1000     eigvecs    eigval-only     mul
//      1       1.3s            14%
//      2       9.3s            13%         0.8s
//      3
//      4      80.3s            15%         6.7s
//      5       2.6m            15%        13.1s
//      6       4.5m                       22.5s
//      7       7.1m                       36s
//
void                testEigsRsmTime(CLArgs const & args)
{
    if (isAutomated(args))
        return;
    Syntax              syn(args,"<size>");
    // Random symmetric matrix, uniform distribution:
    size_t              dim = syn.nextAs<size_t>();
    randSeedRepeatable();
    MatSD               mat = cRandMatSpd(dim,1);
    Timer               timer;
    RsmEigs             eigs = cRsmEigs(mat);
    double              time0 = timer.elapsedSeconds();
    fgout << fgnl << "With eigenvectors: " << toPrettyTime(time0);
    MatD                eigValMat(dim,dim);
    eigValMat.setZero();
    for (uint ii=0; ii<dim; ii++)
        eigValMat.rc(ii,ii) = eigs.vals[ii];
    MatD                recon = eigs.vecs * eigValMat * transpose(eigs.vecs);
    double              residual = 0.0;
    for (uint ii=0; ii<dim; ii++)
        for (uint jj=ii; jj<dim; jj++)
            residual += sqr(recon.rc(ii,jj) - mat.rc(ii,jj));
    residual = sqrt(residual/double(dim*dim));      // root of mean value
    // RMS residual appears to go with the square root of the matrix dimension:
    double              tol = lims<double>::epsilon() * sqrt(dim) * 2.0;
    FGASSERT(residual < tol);
    Doubles             valsOnly;
    timer.start();
    valsOnly = cEigvalsRsm(mat);
    double              time1 = timer.elapsedSeconds();
    fgout
        << fgnl << "No eigenvectors: " << toPrettyTime(time1) << " " << toStrPercent(time1/time0)
        << fgnl << "RMS Residual: " << residual << " RR/tol " << residual/tol;
    FGASSERT(valsOnly == eigs.vals);
}

void                testSymmEigen(CLArgs const & args)
{
    Cmds                cmds {
        {testEigsRsmTime,"time","Eigenvector timing test"},
        {testRsmEigs,"eigs","RSM eigenspectrum"},
    };
    doMenu(args,cmds,true);
}

}

void                testMatrixV(CLArgs const & args)
{
    Cmds                cmds {
        {testMatCol,"col","matrix column-wide editing functions"},
        {testMatMul,"mm","matrix-matrix multiplication correctness"},
        {testMatVec,"mv","matrix-vector multilication correctness"},
        {testMatMulSpeed,"mt","matrix multiplication timing"},
        {testMatMulStruct,"ml","matrix multiplcation loop structure"},
        {testMatSparse,"sparse","sparse matrix operations"},
        {testMatS,"symm","symmetric matrix (MatS)"},
    };
    doMenu(args,cmds,true);
}

void                testMatrixSolver(CLArgs const & args)
{
    Cmds                cmds {
        {testCholesky,"chol","Cholesky 3x3 decomposition"},
        {testAsymEigs,"asym","Arbitrary real matrix eigensystem"},
        {testSymmEigen,"symm","Real symmetric matrix eigensystem"},
        {testSolveS2,"solveS2","Mx=b solver for M 2x2 symmetric"},
        {testSolveLinearMatSD,"slin","solveLinear for MatS"},
    };
    doMenu(args,cmds,true);
}

}
