//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Constant dimension affine transform of the form: f(x) = Mx + b
//
// USE:
//
// operator*(Affine,Mat) is interpreted as *application* of the operator rather
// than *composition* of operators even when the rhs is a matrix.
//
// All definitinos of operator* other than that above are interpreted as composition of
// operators.
//
// Composition of operators with Affine is NOT commutative.
// 

#ifndef FGAFFINEC_HPP
#define FGAFFINEC_HPP

#include "FgStdLibs.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"

namespace Fg {

template <class T,uint D>
struct  Affine
{
    Mat<T,D,D>          linear;           // Applied first
    Mat<T,D,1>          translation;      // Applied second

    FG_SERIALIZE2(linear,translation);

    Affine() {linear.setIdentity(); }

    // Construct from translation: f(x) = x + b
    explicit Affine(Mat<T,D,1> const & trans) : translation(trans) {linear.setIdentity(); }

    // Construct from linear transform: f(x) = Mx
    explicit Affine(const Mat<T,D,D> & lin) : linear(lin) {}

    // Construct from native form: f(x) = Mx + b
    Affine(Mat<T,D,D> const & lin,Mat<T,D,1> const & trans) :
        linear(lin),
        translation(trans)
        {}

    // Construct from opposite order form: f(x) = M(x+b):
    Affine(Mat<T,D,1> const & trans,const Mat<T,D,D> & lin) :
        linear(lin),
        translation(lin * trans)
        {}

    // Conversion constructor:
    template<typename U>
    Affine(Affine<U,D> const & v) : linear(v.linear), translation(v.translation) {}

    // Don't let conversion constructor override default copy constructor:
    Affine(Affine const & v) = default;

    // If 'vec' is a matrix, its columns are transformed as vectors into a new matrix:
    template<uint ncols>
    Mat<T,D,ncols>
    operator*(const Mat<T,D,ncols> & vec) const
    {
        Mat<T,D,ncols> ret = linear * vec;
        for (uint col=0; col<ncols; col++)
            for (uint row=0; row<D; row++)
                ret.cr(col,row) += translation[row];
        return ret;
    }

    // Operator composition: L*R -> L(Rx+r) + l = LRx + Lr + l = (LR)x + (Lr+l)
    Affine
    operator*(const Affine & rhs) const
    {
        Affine       ret;
        ret.linear = linear * rhs.linear;
        ret.translation = linear * rhs.translation + translation;
        return ret;
    }

    // new = scalar * old:
    void
    postScale(T val)
    {linear *= val; translation *= val; }

    // Ax + a = y -> x = A^-1(y - a) = (A^-1)y - (A^-1a)
    Affine
    inverse() const
    {
        Affine       ret;
        ret.linear = cInverse(linear);
        ret.translation = - ret.linear * translation;
        return ret;
    }
};

typedef Affine<float,2>        Affine2F;
typedef Affine<double,2>       Affine2D;
typedef Affine<float,3>        Affine3F;
typedef Affine<double,3>       Affine3D;

// Operator composition: N(Mx+b) = (NM)x + Nb
template<class T,uint dim>
Affine<T,dim>
operator*(
    const Mat<T,dim,dim> &    lhs,
    const Affine<T,dim> &        rhs)
{
    return
        Affine<T,dim>(lhs*rhs.linear,lhs*rhs.translation);
}

template<class T,uint dim>
std::ostream &
operator<<(std::ostream & os,const Affine<T,dim> & v)
{
    return
        os  << fgnl << "Linear: " << v.linear 
            << fgnl << " Translation: " << v.translation;
}

template<typename T,uint dim>
Mat<T,dim+1,dim+1>
asHomogMat(Affine<T,dim> a)
{
    Mat<T,dim+1,dim+1>      ret = asHomogMat(a.linear);
    for (uint ii=0; ii<dim; ++ii)
        ret.rc(ii,dim) = a.translation[ii];
    return ret;
}

}

#endif
