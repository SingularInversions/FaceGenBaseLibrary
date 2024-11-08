//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgMatrixSolver.hpp"
#include "FgTransform.hpp"
#include "FgCommand.hpp"
#include "FgApproxEqual.hpp"

using namespace std;

namespace Fg {

Mat33D              matRotateAxis(double radians,Vec3D const & ax)
{
    double              c = cos(radians),
                        s = sin(radians),
                        v = 1.0 - c;
    return {
        ax[0]*ax[0]*v + c,          ax[0]*ax[1]*v - ax[2]*s,        ax[0]*ax[2]*v + ax[1]*s,
        ax[0]*ax[1]*v + ax[2]*s,    ax[1]*ax[1]*v + c,              ax[1]*ax[2]*v - ax[0]*s,
        ax[0]*ax[2]*v - ax[1]*s,    ax[1]*ax[2]*v + ax[0]*s,        ax[2]*ax[2]*v + c,
    };
}

Doubles             flattenD(Floatss const & v)
{
    size_t          sz = 0;
    for (Floats const & fs : v)
        sz += fs.size();
    Doubles         ret;
    ret.reserve(sz);
    for (Floats const & fs : v)
        for (float f : fs)
            ret.push_back(scast<double>(f));
    return ret;
}

Mat33D              cRandPositiveDefinite3D()
{
    Mat33D              L = QuaternionD::rand().asMatrix(),
                        R = QuaternionD::rand().asMatrix();
    Arr3D               lnEigs {randNormal(),randNormal(),randNormal(),};
    return L * scaleRows(R,mapExp(lnEigs));
}

MatS2D              MatS2D::randSpd(double lnEigStdev)
{
    Mat22D              D = cDiagMat(mapExp(Vec2D::randNormal(lnEigStdev)));
    Vec2D               r = normalize(Vec2D::randNormal());
    Mat22D              R {r[0],-r[1],r[1],r[0]},
                        M = R.transpose() * D * R;      // will be symmetric within precision
    return {M[0],M[3],M[1]};
}

Mat33D              MatS3D::operator+(Mat33D const & r) const
{
    return {
        diag[0] + r.rc(0,0),    offd[0] + r.rc(0,1),    offd[1] + r.rc(0,2),
        offd[0] + r.rc(1,0),    diag[1] + r.rc(1,1),    offd[2] + r.rc(1,2),
        offd[1] + r.rc(2,0),    offd[2] + r.rc(2,1),    diag[2] + r.rc(2,2),
    };
}

Vec3D               MatS3D::operator*(Vec3D rhs) const
{
    return {
        diag[0]*rhs[0] + offd[0]*rhs[1] + offd[1]*rhs[2],
        offd[0]*rhs[0] + diag[1]*rhs[1] + offd[2]*rhs[2],
        offd[1]*rhs[0] + offd[2]*rhs[1] + diag[2]*rhs[2],
    };
}

Mat33D              MatS3D::operator*(MatS3D const & r) const
{
    return {
        diag[0]*r.diag[0] + offd[0]*r.offd[0] + offd[1]*r.offd[1],
        diag[0]*r.offd[0] + offd[0]*r.diag[1] + offd[1]*r.offd[2],
        diag[0]*r.offd[1] + offd[0]*r.offd[2] + offd[1]*r.diag[2],

        offd[0]*r.diag[0] + diag[1]*r.offd[0] + offd[2]*r.offd[1],
        offd[0]*r.offd[0] + diag[1]*r.diag[1] + offd[2]*r.offd[2],
        offd[0]*r.offd[1] + diag[1]*r.offd[2] + offd[2]*r.diag[2],

        offd[1]*r.diag[0] + offd[2]*r.offd[0] + diag[2]*r.offd[1],
        offd[1]*r.offd[0] + offd[2]*r.diag[1] + diag[2]*r.offd[2],
        offd[1]*r.offd[1] + offd[2]*r.offd[2] + diag[2]*r.diag[2],
    };
}

Mat33D              MatS3D::operator*(Mat33D const & r) const
{
    return {
        diag[0]*r.rc(0,0) + offd[0]*r.rc(1,0) + offd[1]*r.rc(2,0),
        diag[0]*r.rc(0,1) + offd[0]*r.rc(1,1) + offd[1]*r.rc(2,1),
        diag[0]*r.rc(0,2) + offd[0]*r.rc(1,2) + offd[1]*r.rc(2,2),

        offd[0]*r.rc(0,0) + diag[1]*r.rc(1,0) + offd[2]*r.rc(2,0),
        offd[0]*r.rc(0,1) + diag[1]*r.rc(1,1) + offd[2]*r.rc(2,1),
        offd[0]*r.rc(0,2) + diag[1]*r.rc(1,2) + offd[2]*r.rc(2,2),

        offd[1]*r.rc(0,0) + offd[2]*r.rc(1,0) + diag[2]*r.rc(2,0),
        offd[1]*r.rc(0,1) + offd[2]*r.rc(1,1) + diag[2]*r.rc(2,1),
        offd[1]*r.rc(0,2) + offd[2]*r.rc(1,2) + diag[2]*r.rc(2,2),
    };
}

double              MatS3D::determinant() const
{
    return
        diag[0]*diag[1]*diag[2] +
        offd[0]*offd[1]*offd[2] * 2 -
        diag[0]*offd[2]*offd[2] -
        diag[1]*offd[1]*offd[1] -
        diag[2]*offd[0]*offd[0];
}

MatS3D              MatS3D::inverse() const
{
    double          fac = cProd(diag) + 2.0*offd[0]*offd[1]*offd[2]
        - diag[0]*sqr(offd[2])
        - diag[1]*sqr(offd[1])
        - diag[2]*sqr(offd[0]);
    FGASSERT(fac != 0.0);
    MatS3D          ret;
    ret.diag[0] = diag[1] * diag[2] - sqr(offd[2]);
    ret.diag[1] = diag[0] * diag[2] - sqr(offd[1]);
    ret.diag[2] = diag[0] * diag[1] - sqr(offd[0]);
    ret.offd[0] = offd[1] * offd[2] - diag[2] * offd[0];
    ret.offd[1] = offd[0] * offd[2] - diag[1] * offd[1];
    ret.offd[2] = offd[0] * offd[1] - diag[0] * offd[2];
    return ret * (1.0/fac);
}

MatS3D              MatS3D::square() const
{
    Arr3D           ds = mapSqr(diag),
                    os = mapSqr(offd);
    return {
        {
            ds[0] + os[0] + os[1],
            ds[1] + os[0] + os[2],
            ds[2] + os[1] + os[2],
        },
        {
            diag[0]*offd[0] + diag[1]*offd[0] + offd[1]*offd[2],
            diag[0]*offd[1] + offd[0]*offd[2] + diag[2]*offd[1],
            offd[0]*offd[1] + diag[1]*offd[2] + diag[2]*offd[2],
        },
    };
}

MatS3D              MatS3D::transform(Mat33D const & M) const
{
    MatS3D              ret;
    for (size_t rr=0; rr<3; ++rr) {
        for (size_t cc=rr; cc<3; ++cc) {
            double              acc {0};
            for (size_t ii=0; ii<3; ++ii)
                for (size_t jj=0; jj<3; ++jj)
                    acc += rc(ii,jj) * M.rc(ii,rr) * M.rc(jj,cc);
            ret.rc(rr,cc) = acc;
            ret.rc(cc,rr) = acc;
        }
    }
    return ret;
}

MatS3D              MatS3D::randSpd(double lnEigStdev)
{
    // Create a random 3D SPD by generating log-normal eigvals and a random rotation for the eigvecs:
    Mat33D          D = cDiagMat(mapExp(Vec3D::randNormal(lnEigStdev))),
                    R = QuaternionD::rand().asMatrix(),
                    M = R.transpose() * D * R;
    // M will have precision-level asymmetry so construct return value from upper triangular values
    // (Eigen's QR decomp fails badly if symmetry is not precise):
    return {{M[0],M[4],M[8]},{M[1],M[2],M[5]}};
}

ostream &           operator<<(ostream & os,MatS3D const & m) {return os << m.asMatC(); }

MatS3D              cCongruentTransform(MatS3D const & S,Mat33D const & M)
{
    Arr3D               diag {0},
                        offd {0};
    for (size_t aa=0; aa<3; ++aa)
        for (size_t bb=0; bb<3; ++bb)
            for (size_t cc=0; cc<3; ++cc)
                diag[aa] += S.rc(bb,cc) * M.rc(bb,aa) * M.rc(cc,aa);
    for (size_t bb=0; bb<3; ++bb) {
        for (size_t cc=0; cc<3; ++cc) {
            offd[0] += S.rc(bb,cc) * M.rc(bb,0) * M.rc(cc,1);
            offd[1] += S.rc(bb,cc) * M.rc(bb,0) * M.rc(cc,2);
            offd[2] += S.rc(bb,cc) * M.rc(bb,1) * M.rc(cc,2);
        }
    }
    return {diag,offd};
}
static void         testCongruentTransform()
{
    auto                fn = []()
    {
        // only test for PD here since that's all we really care about:
        MatS3D          S = MatS3D::randSpd(1);
        Mat33D          M = cRandPositiveDefinite3D(),
                        tst = cCongruentTransform(S,M).asMatC(),
                        ref = M.transpose() * S.asMatC() * M;
        FGASSERT(isApproxEqualPrec(tst,ref,30));
    };
    repeat(fn,10);
}

MatS3D              transposeSelfProduct(Mat33D const & M)
{
    return {
        {
            sqr(M[0]) + sqr(M[3]) + sqr(M[6]),
            sqr(M[1]) + sqr(M[4]) + sqr(M[7]),
            sqr(M[2]) + sqr(M[5]) + sqr(M[8]),
        },
        {
            M[0]*M[1] + M[3]*M[4] + M[6]*M[7],
            M[0]*M[2] + M[3]*M[5] + M[6]*M[8],
            M[1]*M[2] + M[4]*M[5] + M[7]*M[8],
        },
    };
}

MeanCov3            cMeanCov(Vec3Ds const & samps)
{
    Vec3D               mean = cMean(samps);
    Vec3Ds              zms = mapSub(samps,mean);
    MatS3D              acc {0};
    for (Vec3D const & zm : zms)
        acc += outerProductSelf(zm);
    return {mean,acc*(1.0/samps.size())};
}

MatUT3D             MatUT3D::operator*(MatUT3D r) const
{
    return          MatUT3D {
        m[0]*r.m[0],    m[0]*r.m[1]+m[1]*r.m[3],    m[0]*r.m[2]+m[1]*r.m[4]+m[2]*r.m[5],
                        m[3]*r.m[3],                m[3]*r.m[4]+m[4]*r.m[5],
                                                    m[5]*r.m[5]
    };
}
MatS3D              MatUT3D::luProduct() const
{
    double      m00 = sqr(m[0]),
                m01 = m[0]*m[1],
                m02 = m[0]*m[2],
                m11 = sqr(m[1]) + sqr(m[3]),
                m12 = m[1]*m[2] + m[3]*m[4],
                m22 = sqr(m[2]) + sqr(m[4]) + sqr(m[5]);
    return MatS3D {{m00,m11,m22},{m01,m02,m12}};
}

MatUT3D             MatUT3D::inverse() const
{
    MatUT3D    ret;
    ret.m[0] = 1.0/m[0];
    ret.m[1] = -m[1]/(m[0]*m[3]);
    ret.m[2] = (m[1]*m[4]/m[3] - m[2])/(m[0]*m[5]);
    ret.m[3] = 1.0/m[3];
    ret.m[4] = -m[4]/(m[3]*m[5]);
    ret.m[5] = 1.0/m[5];
    return ret;
}

std::ostream &      operator<<(std::ostream & os,MatUT3D const & ut)
{
    Vec3D       diag {ut.m[0],ut.m[3],ut.m[5]},
                upper {ut.m[1],ut.m[2],ut.m[4]};
    return os << "Diag: " << diag << " UT: " << upper;
}

MatUT3D             axesToPcut(Vec3D a0,Vec3D a1,Vec3D a2)
{
    Mat33D              toMhlbs {cat(a0.m,a1.m,a2.m)};
    return cCholesky(transposeSelfProduct(toMhlbs));
}

MatS3D::MatS3D(Mat33D const & m) :
    diag {m[0],m[4],m[8]},
    offd {m[1],m[2],m[5]}
{
    for (uint rr=0; rr<3; ++rr)
        for (uint cc=rr+1; cc<3; ++cc)
            FGASSERT(m.rc(rr,cc) == m.rc(cc,rr));
}

namespace {

template<uint S>
void                testInverseT()
{
    Mat<double,S,S>     mat;
    do
        mat = Mat<double,S,S>::randNormal();
    while
        (cDeterminant(mat) < 0.01);
    Mat<double,S,S>     inv = cInverse(mat),
                        ident = cDiagMat<double,S>(1);
    FGASSERT(isApproxEqualPrec(inv*mat,ident,40));
}

void                testRotate(CLArgs const &)
{
    randSeedRepeatable();
    for (uint ii=0; ii<100; ii++)
    {
        double          angle = randUniform(-pi,pi);
        Vec3D           axis = normalize(Vec3D::randNormal());
        Mat33D          mat = matRotateAxis(angle,axis);
        double          err = cLenD(mat * mat.transpose() - cDiagMat<double,3>(1)),
                        err2 = cLenD(mat * axis - axis);
        FGASSERT(err < (lims<double>::epsilon() * 10.0));
        FGASSERT(err2 < (lims<double>::epsilon() * 10.0));
        FGASSERT(cDeterminant(mat) > 0.0);      // Ensure SO(3) not just O(3)
    }
}

void                testInverse(CLArgs const &)
{
    randSeedRepeatable();
    for (size_t ii=0; ii<10; ++ii) {
        testInverseT<2>();
        testInverseT<3>();
    }
}

void                testMat(CLArgs const & args)
{
    Cmds            cmds {
        {testInverse,"inv"},
        {testRotate,"rot"},
    };
    doMenu(args,cmds,true);
}

void                testMatS(CLArgs const &)
{
    randSeedRepeatable();
    for (size_t ii=0; ii<10; ++ii) {
        MatS3D              S = MatS3D::randSpd(1.0);
        {
            double              ref = cDeterminant(S.asMatC()),
                                tst = cDeterminant(S);
            FGASSERT(isApproxEqualRel(ref,tst,epsBits(40)));
        }
        {
            Mat33D              tst = cInverse(S).asMatC(),
                                ref = cInverse(S.asMatC());
            FGASSERT(isApproxEqualPrec(tst,ref,40));
        }
        {
            Mat33D              tst = S.square().asMatC(),
                                ref = S.asMatC() * S.asMatC();
            FGASSERT(isApproxEqualPrec(tst,ref,30));
        }
        {
            Vec3D               v = Vec3D::randNormal(),
                                tst = S * v,
                                ref = S.asMatC() * v;
            FGASSERT(isApproxEqualPrec(tst,ref,30));
        }
        MatS3D              m1 = MatS3D::randSpd(1);
        {
            Mat33D              tst = S * m1,
                                ref = S.asMatC() * m1.asMatC();
            FGASSERT(isApproxEqualPrec(tst,ref,30));
        }
        {
            Mat33D              m2 = Mat33D::randNormal(1),
                                tst = S * m2,
                                ref = S.asMatC() * m2;
            FGASSERT(isApproxEqualPrec(tst,ref,30));
        }
        {
            Mat33D              T = Mat33D::randNormal();
            Vec3D               x = Vec3D::randNormal(),
                                Tx = T * x;
            double              ref = cDot(Tx,S*Tx);
            MatS3D              TS = S.transform(T);
            double              tst = cDot(x,TS*x);
            FGASSERT(isApproxEqualPrec(tst,ref,30));
        }
        {   // test transposeSelfProduct(MatS)
            Mat33D              M = Mat33D::randNormal(),
                                ref = M.transpose() * M;
            MatS3D              P = transposeSelfProduct(M);
            Mat33D              tst = P.asMatC();
            FGASSERT(isApproxEqualPrec(tst,ref,30));
        }
    }
    testCongruentTransform();
}

}

void                testSolveLinear(CLArgs const &);

void                testMatrixC(CLArgs const & args)
{
    Cmds            cmds {
        {testMat,"mat","general matrix"},
        {testMatS,"matS","symmetric matrix"},
        {testSolveLinear,"solve","solve linear Mx=b"},
    };
    doMenu(args,cmds,true);
}

}
