//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgMatrixC.hpp"

#include "FgMath.hpp"
#include "FgRandom.hpp"
#include "FgQuaternion.hpp"
#include "FgCommand.hpp"
#include "FgApproxEqual.hpp"

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

Doubles             toDoubles(Floatss const & v)
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

// 2D case allows for simple explicit formula:
Vec2D               solveLinear(Mat22D const & A,Vec2D const & b)
{
    double              det = A[0]*A[3] - A[1]*A[2];
    FGASSERT(det != 0.0);
    return {
        (b[0]*A[3] - b[1]*A[1])/det,
        (b[1]*A[0] - b[0]*A[2])/det
    };
}

Vec3D               solveLinear(Mat33D const & M,Vec3D const & b)
{
    Eigen::Matrix3d         mat;
    Eigen::Vector3d         vec;
    for (uint rr=0; rr<3; ++rr)
        for (uint cc=0; cc<3; ++cc)
            mat(rr,cc) = M.rc(rr,cc);
    for (uint rr=0; rr<3; ++rr)
        vec(rr) = b[rr];
    // There are many alternatives to this in Eigen: ParialPivLU, FullPivLU, HouseholderQR etc.
    auto                    qr = mat.colPivHouseholderQr();
    FGASSERT(qr.isInvertible());
    Eigen::Vector3d         sol = qr.solve(vec);
    return {sol(0),sol(1),sol(2)};
}

Vec4D               solveLinear(Mat44D const & M,Vec4D const & b)
{
    Eigen::Matrix4d         mat;
    Eigen::Vector4d         vec;
    for (uint rr=0; rr<4; ++rr)
        for (uint cc=0; cc<4; ++cc)
            mat(rr,cc) = M.rc(rr,cc);
    for (uint rr=0; rr<4; ++rr)
        vec(rr) = b[rr];
    // There are many alternatives to this in Eigen: ParialPivLU, FullPivLU, HouseholderQR etc.
    auto                    qr = mat.colPivHouseholderQr();
    FGASSERT(qr.isInvertible());
    Eigen::Vector4d         sol = qr.solve(vec);
    return {sol(0),sol(1),sol(2),sol(3)};
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

MatS3D              MatS3D::randSpd(double lnEigStdev)
{
    // Create a random 3D SPD by generating log-normal eigvals and a random rotation for the eigvecs:
    Mat33D          D = cDiagMat(mapExp(Vec3D::randNormal(lnEigStdev))),
                    R = QuaternionD::rand().asMatrix(),
                    M = R.transpose() * D * R;
    // M will have precision-level asymmetry so construct return value from upper triangular values
    // (Eigen's QR decomp fails badly if symmetry is not precise):
    return MatS3D {{{M[0],M[4],M[8]}},{{M[1],M[2],M[5]}}};
}

ostream &           operator<<(ostream & os,MatS3D const & m)
{
    return os << m.asMatC();
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
    return MatS3D {{{m00,m11,m22}},{{m01,m02,m12}}};
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

MatS3D::MatS3D(Mat33D const & m) :
    diag {{m[0],m[4],m[8]}},
    offd {{m[1],m[2],m[5]}}
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
                        ident = Mat<double,S,S>::identity();
    FGASSERT(isApproxEqualPrec(inv*mat,ident,40));
}

void                testRotate(CLArgs const &)
{
    randSeedRepeatable();
    for (uint ii=0; ii<100; ii++)
    {
        double          angle = randUniform(-pi(),pi());
        Vec3D           axis = normalize(Vec3D::randNormal());
        Mat33D          mat = matRotateAxis(angle,axis);
        double          err = (mat * mat.transpose() - Mat33D::identity()).len(),
                        err2 = (mat * axis - axis).len();
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
        MatS3D              m0 = MatS3D::randSpd(1.0);
        {
            double              ref = cDeterminant(m0.asMatC()),
                                tst = cDeterminant(m0);
            FGASSERT(isApproxEqualRel(ref,tst,epsBits(40)));
        }
        {
            Mat33D              tst = cInverse(m0).asMatC(),
                                ref = cInverse(m0.asMatC());
            FGASSERT(isApproxEqualPrec(tst,ref,40));
        }
        {
            Mat33D              tst = m0.square().asMatC(),
                                ref = m0.asMatC() * m0.asMatC();
            FGASSERT(isApproxEqualPrec(tst,ref,30));
        }
        {
            Vec3D               v = Vec3D::randNormal(),
                                tst = m0 * v,
                                ref = m0.asMatC() * v;
            FGASSERT(isApproxEqualPrec(tst,ref,30));
        }
        MatS3D              m1 = MatS3D::randSpd(1);
        {
            Mat33D              tst = m0 * m1,
                                ref = m0.asMatC() * m1.asMatC();
            FGASSERT(isApproxEqualPrec(tst,ref,30));
        }
        {
            Mat33D              m2 = Mat33D::randNormal(1),
                                tst = m0 * m2,
                                ref = m0.asMatC() * m2;
            FGASSERT(isApproxEqualPrec(tst,ref,30));
        }
    }
}

template<uint D>
void                testSolveLinearT()
{
    for (size_t ii=0; ii<256; ++ii) {
        Mat<double,D,D>     M = Mat<double,D,D>::randNormal();
        Mat<double,D,1>     b = Mat<double,D,1>::randNormal(),
                            x = solveLinear(M,b);
        if (abs(cDeterminant(M)) < epsBits(20))         // don't test with ill conditioned
            continue;
        FGASSERT(isApproxEqualPrec(M*x,b,30));
    }
}

void                testSolveLinear(CLArgs const &)
{
    randSeedRepeatable();
    testSolveLinearT<2>();
    testSolveLinearT<3>();
    testSolveLinearT<4>();
}

}

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
