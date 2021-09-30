//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgMatrixC.hpp"
#include "FgSyntax.hpp"
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

Mat33D
matRotateAxis(double radians,Vec3D const & ax)
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

Doubles
toDoubles(Floatss const & v)
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

// Gaussian elimination can be very simply explicit in this case:
Opt<Vec2F>
solveLinear(Mat22F A,Vec2F b)
{
    Opt<Vec2F>    ret;
    float                   a0 = A[0]*A[3],
                            a1 = A[1]*A[2],
                            a0s = a0*a0,
                            a1s = a1*a1;
    if (a0s > a1s) {
        float           r1 = A[1] / A[3],
                        a11 = A[0] - r1 * A[2],
                        x = (b[0] - r1 * b[1]) / a11,
                        y = (b[1] - A[2] * x) / A[3];
        ret = Vec2F(x,y);
    }
    else if (a0s < a1s) {
        float           r1 = A[0] / A[2],
                        a12 = A[1] - r1 * A[3],
                        y = (b[0] - r1 * b[1]) / a12,
                        x = (b[1] - A[3] * y) / A[2];
        ret = Vec2F(x,y);
    }
    return ret;
}

Opt<Vec3D>
solveLinear(Mat33D A,Vec3D b)
{
    Eigen::Matrix3d         mat;
    Eigen::Vector3d         vec;
    for (uint rr=0; rr<3; ++rr)
        for (uint cc=0; cc<3; ++cc)
            mat(rr,cc) = A.rc(rr,cc);
    for (uint rr=0; rr<3; ++rr)
        vec(rr) = b[rr];
    Opt<Vec3D>         ret;
    // There are many alternatives to this in Eigen: ParialPivLU, FullPivLU, HouseholderQR etc.
    auto                    qr = mat.colPivHouseholderQr();
    if (qr.isInvertible()) {
        Eigen::Vector3d     sol = qr.solve(vec);
        ret = Vec3D(sol(0),sol(1),sol(2));
    }
    return ret;
}

Opt<Vec4D>
solveLinear(Mat44D A,Vec4D b)
{
    Eigen::Matrix4d         mat;
    Eigen::Vector4d         vec;
    for (uint rr=0; rr<4; ++rr)
        for (uint cc=0; cc<4; ++cc)
            mat(rr,cc) = A.rc(rr,cc);
    for (uint rr=0; rr<4; ++rr)
        vec(rr) = b[rr];
    Opt<Vec4D>         ret;
    // There are many alternatives to this in Eigen: ParialPivLU, FullPivLU, HouseholderQR etc.
    auto                    qr = mat.colPivHouseholderQr();
    if (qr.isInvertible()) {
        Eigen::Vector4d     sol = qr.solve(vec);
        ret = Vec4D(sol(0),sol(1),sol(2),sol(3));
    }
    return ret;
}

Mat32D
fgTanSphere(Vec3D v)
{
    // Find permutation that sorts 'v' smallest to largest:
    Vec3UI           p(0,1,2);
    Vec3D            m = mapSqr(v);
    if (m[0] > m[1])
        std::swap(p[0],p[1]);
    if (m[p[1]] > m[p[2]])
        std::swap(p[1],p[2]);
    if (m[p[0]] > m[p[1]])
        std::swap(p[0],p[1]);
    // Gram-Schmidt starting with least co-linear axes:
    Vec3D        r0(0),
                    vn = normalize(v);
    r0[p[0]] = 1.0;
    r0 -= vn * cDot(vn,r0);
    r0 /= r0.len();
    Vec3D        r1 = crossProduct(vn,r0);
    return catHoriz(r0,r1);
}

MatS3D
MatS3D::inverse() const
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

ostream &
operator<<(ostream & os,MatS3D const & m)
{
    return os << m.asMatC();
}

MatS3D
randMatSpd3D(double lnEigStdev)
{
    // Create a random 3D SPD by generating log-normal eigvals and a random rotation for the eigvecs:
    Mat33D          D = cDiagMat(mapCall(Vec3D::randNormal(lnEigStdev),exp)),
                    R = QuaternionD::rand().asMatrix(),
                    M = R.transpose() * D * R;
    // M will have precision-level asymmetry so manually construct return value from upper triangular values
    // (Eigen's QR decomp fails badly if symmetry is not precise):
    return MatS3D {{{M[0],M[4],M[8]}},{{M[1],M[2],M[5]}}};
}

MatUT3D
MatUT3D::operator*(MatUT3D r) const
{
    return          MatUT3D {
        m[0]*r.m[0],    m[0]*r.m[1]+m[1]*r.m[3],    m[0]*r.m[2]+m[1]*r.m[4]+m[2]*r.m[5],
                        m[3]*r.m[3],                m[3]*r.m[4]+m[4]*r.m[5],
                                                    m[5]*r.m[5]
    };
}
MatS3D
MatUT3D::luProduct() const
{
    double      m00 = sqr(m[0]),
                m01 = m[0]*m[1],
                m02 = m[0]*m[2],
                m11 = sqr(m[1]) + sqr(m[3]),
                m12 = m[1]*m[2] + m[3]*m[4],
                m22 = sqr(m[2]) + sqr(m[4]) + sqr(m[5]);
    return MatS3D {{{m00,m11,m22}},{{m01,m02,m12}}};
}

MatUT3D
MatUT3D::inverse() const
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

MatUT3D
MatUT3D::randNormal(double lnScaleStdev,double shearStdev)
{
    return MatUT3D {mapExp(randNormalArr<3>(0.0,lnScaleStdev)),randNormalArr<3>(0.0,shearStdev)};
}

std::ostream &
operator<<(std::ostream & os,MatUT3D const & ut)
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
void
testInverseT()
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

void
testRotate(CLArgs const &)
{
    randSeedRepeatable();
    for (uint ii=0; ii<100; ii++)
    {
        double          angle = randUniform(-pi(),pi());
        Vec3D           axis = normalize(Vec3D::randNormal());
        Mat33D          mat = matRotateAxis(angle,axis);
        double          err = (mat * mat.transpose() - Mat33D::identity()).len(),
                        err2 = (mat * axis - axis).len();
        FGASSERT(err < (epsilonD() * 10.0));
        FGASSERT(err2 < (epsilonD() * 10.0));
        FGASSERT(cDeterminant(mat) > 0.0);      // Ensure SO(3) not just O(3)
    }
}

void
testInverse(CLArgs const &)
{
    randSeedRepeatable();
    for (size_t ii=0; ii<10; ++ii) {
        testInverseT<2>();
        testInverseT<3>();
    }
}

void
testMat(CLArgs const & args)
{
    Cmds            cmds {
        {testInverse,"inv"},
        {testRotate,"rot"},
    };
    doMenu(args,cmds,true);
}

void
testMatS(CLArgs const &)
{
    randSeedRepeatable();
    for (size_t ii=0; ii<10; ++ii) {
        MatS3D              m = randMatSpd3D(1.0);
        double              d0 = cDeterminant(m.asMatC()),
                            d1 = cDeterminant(m);
        FGASSERT(isApproxEqualRel(d0,d1,epsBits(40)));
    }
    for (size_t ii=0; ii<10; ++ii) {
        MatS3D              m = randMatSpd3D(1.0);
        Mat33D              val = cInverse(m).asMatC(),
                            ref = cInverse(m.asMatC());
        FGASSERT(isApproxEqualPrec(val,ref,40));
    }
}

void
testSolveLinear(CLArgs const &)
{
    randSeedRepeatable();
    for (size_t ii=0; ii<10; ++ii) {
        Mat44D              M = Mat44D::randNormal();
        Vec4D               b = Vec4D::randNormal();
        Opt<Vec4D>          x = solveLinear(M,b);
        // Chance of values so ill-conditioned we can't get 20 bits accuracy is negligable:
        FGASSERT(isApproxEqualPrec(M*x.val(),b,20));
    }
}

}

void
testMatrixC(CLArgs const & args)
{
    Cmds            cmds {
        {testMat,"mat","general matrix"},
        {testMatS,"matS","symmetric matrix"},
        {testSolveLinear,"solve","solve linear Mx=b"},
    };
    doMenu(args,cmds,true);
}

}
