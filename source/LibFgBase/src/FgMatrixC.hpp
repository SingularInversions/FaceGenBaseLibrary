//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Constant size stack based matrix / vector
//
//      USE:
//
// A vector is just a matrix with one dimension set to size 1 (thus row and column 
// vectors are different)
//
// rc(row,col) allows access to matrix elements.
//
// operator[] allows access to vector elements (or to unrolled matrix elements),
// row-major storage.
//
//      DESIGN:
//
// Due to compiler optimizations such as loop unrolling the code should run pretty much
// as fast as if we'd written the various cases out separately.
//
// Accumulator types were deemed unnecessary for small, constant size matrices.
//

#ifndef FGMATRIXC_HPP
#define FGMATRIXC_HPP

#include "FgMath.hpp"

namespace Fg {

template <typename T,size_t R,size_t C>
struct      Mat
{
    // note that Arr<Arr<T,C>,R> cannot be used since the compiler may add padding to the inner struct:
    Arr<T,R*C>          m;      // elements store by column then by row (row major)
    FG_SER(m)

    // No initialization is done for types without a default ctor. Default zero-fill is not a solution, be explicit:
    constexpr Mat() {}
    constexpr explicit Mat(Arr<T,R*C> const & data) : m{data} {}
    explicit constexpr Mat(Arr<Arr<T,C>,R> const & aa)    // input may have padding
    {
        for (size_t rr=0; rr<R; ++rr)
            for (size_t cc=0; cc<C; ++cc)
                m[rr*C+cc] = aa[rr][cc];
    }
    explicit constexpr Mat(T v) : m{v} {}     // fill value ctor
    // element value constructors; Arr enforces that number of args matches number of elems:
    constexpr Mat(T x,T y) : m{x,y} {}
    constexpr Mat(T x,T y,T z) : m{x,y,z} {}
    constexpr Mat(T a,T b,T c,T d) : m{a,b,c,d} {}
    constexpr Mat(T a,T b,T c,T d,T e) : m{a,b,c,d,e} {}
    constexpr Mat(T a,T b,T c,T d,T e,T f) : m{a,b,c,d,e,f} {}
    constexpr Mat(T a,T b,T c,T d,T e,T f,T g,T h,T i) : m{a,b,c,d,e,f,g,h,i} {}
    constexpr Mat(T a,T b,T c,T d,T e,T f,T g,T h,T i,T j,T k,T l) : m{a,b,c,d,e,f,g,h,i,j,k,l} {}
    constexpr Mat(T a,T b,T c,T d,T e,T f,T g,T h,T i,T j,T k,T l,T m,T n,T o,T p) : m{a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p} {}

    // default copy constructor (and thus assignment operator) must be explicitly defined to
    // differentiate from (explicit) conversion constructors below:
    Mat(Mat const &) = default;
    Mat &               operator=(Mat const &) = default;

    // Type conversion constructor very handy:
    template<class U>
    explicit Mat(Mat<U,R,C> const & mat) : m{mapCast<T>(mat.m)} {}

    size_t constexpr    numRows() const {return R; }
    size_t constexpr    numCols() const {return C; }
    size_t constexpr    numElems() const {return R*C; }
    size_t constexpr    size() const {return R*C; }
    // Element access by (row,column):
    T &                 rc(size_t row,size_t col)
    {
        FGASSERT_FAST((row < R) && (col < C));
        return m[row*C+col];
    }
    T const &           rc(size_t row,size_t col) const
    {
        FGASSERT_FAST((row < R) && (col < C));
        return m[row*C+col];
    }
    T &                 operator[](size_t xx) {return m[xx]; }
    T const &           operator[](size_t xx) const {return m[xx]; }
    T &                 operator[](Arr<uint,2> colRow) {return rc(colRow[1],colRow[0]); }
    T const &           operator[](Arr<uint,2> colRow) const {return rc(colRow[1],colRow[0]); }
    Mat                 operator-() const {return Mat{-m}; }
    Mat                 operator+(Mat const & r) const {return Mat{m + r.m}; }
    Mat                 operator-(Mat const & r) const {return Mat{m - r.m}; }
    Mat                 operator*(T val) const {return Mat{m * val}; }
    Mat                 operator/(T val) const {return Mat{m / val}; }
    void                operator*=(T val) {m *= val; }
    void                operator/=(T val) {m /= val; }
    void                operator+=(Mat const & r) {m += r.m; }
    void                operator-=(Mat const & r) {m -= r.m; }
    bool                operator==(Mat const & r) const {return (m == r.m); }
    bool                operator!=(Mat const & r) const {return (m != r.m); }
    template<size_t SR,size_t SC>
    Mat<T,SR,SC>        subMatrix(size_t row,size_t col) const
    {
        FGASSERT((row+SR <= R) && (col+SC <= C));
        Mat<T,SR,SC>        ret;
        size_t                cnt = 0;
        for (size_t rr=row; rr<row+SR; ++rr)
            for (size_t cc=col; cc<col+SC; ++cc)
                ret[cnt++] = rc(rr,cc);
        return ret;
    }
    template <size_t SR,size_t SC>
    void                setSubMat(Mat<T,SR,SC> & sub,size_t row,size_t col)
    {
        FGASSERT((SR+row <= R) && (SC+col <= C));
        for (size_t rr=0; rr<SR; rr++)
            for (size_t cc=0; cc<SC; cc++)
                rc(rr+row,cc+col) = sub.rc(rr,cc);
    }
    // recursive squared magnitude, accumulates in double type
    double              magD() const {return cMagD(m); }
    Mat<T,C,R>          transpose() const
    {
        Mat<T,C,R>          ret;
        for (size_t rr=0; rr<R; ++rr)
            for (size_t cc=0; cc<C; ++cc)
                ret.rc(cc,rr) = rc(rr,cc);
        return ret;
    }
    Mat<T,R,1>          colVec(size_t col) const
    {
        Mat<T,R,1>          ret;
        FGASSERT_FAST(col < C);
        for (size_t rr=0; rr<R; ++rr)
            ret[rr] = rc(rr,col);
        return ret;
    }
    Mat<T,1,C>          rowVec(size_t row) const
    {
        Mat<T,1,C>          ret;
        FGASSERT_FAST(row < R);
        for (size_t cc=0; cc<C; ++cc)
            ret[cc] = rc(row,cc);
        return ret;
    }
    T                   elemsProduct() const {return cProduct(m); }
    bool                operator<(const Mat & rhs) const      // Useful for putting in a std::map or sorting for unique check
    {
        for (size_t ii=0; ii<R*C; ++ii) {
            if (m[ii] < rhs[ii])
                return true;
            if (rhs[ii] < m[ii])
                return false;
        }
        return false;
    }

    // Static creation functions are handy because you can use the type abbreviations below:
    static Mat          cRandUniform(T lo,T hi) {return Mat{cRandArrUniform<T,R*C>(lo,hi)}; }
    static Mat          randNormal(T stdev=T(1)) {return Mat{cRandArrNormal<T,R*C>(0,stdev)}; }
};

template<class T,size_t R,size_t C>
struct      Traits<Mat<T,R,C>>
{
    typedef typename Traits<T>::Scalar                  Scalar;
    typedef Mat<typename Traits<T>::Floating,R,C>       Floating;
};

typedef Mat<float,2,1>          Vec2F;
typedef Mat<double,2,1>         Vec2D;
typedef Mat<short,2,1>          Vec2S;
typedef Mat<int,2,1>            Vec2I;
typedef Mat<uint,2,1>           Vec2UI;
typedef Mat<size_t,2,1>         Vec2Z;
typedef Mat<uint64,2,1>         Vec2UL;

typedef Mat<float,3,1>          Vec3F;
typedef Mat<double,3,1>         Vec3D;
typedef Mat<schar,3,1>          Vec3SC;
typedef Mat<uchar,3,1>          Vec3UC;
typedef Mat<int16,3,1>          Vec3S;
typedef Mat<int,3,1>            Vec3I;
typedef Mat<uint,3,1>           Vec3UI;
typedef Mat<size_t,3,1>         Vec3Z;
typedef Mat<int64,3,1>          Vec3L;
typedef Mat<uint64,3,1>         Vec3UL;

typedef Mat<float,4,1>          Vec4F;
typedef Mat<double,4,1>         Vec4D;
typedef Mat<int,4,1>            Vec4I;
typedef Mat<uint,4,1>           Vec4UI;
typedef Mat<uchar,4,1>          Vec4UC;

typedef Mat<double,5,1>         Vec5D;
typedef Mat<uint,5,1>           Vec5UI;

typedef Mat<float,1,2>          VecF2;
typedef Mat<float,1,3>          VecF3;
typedef Mat<double,1,2>         VecD2;
typedef Mat<double,1,3>         VecD3;

typedef Svec<Vec2I>             Vec2Is;
typedef Svec<Vec2UI>            Vec2UIs;
typedef Svec<Vec2F>             Vec2Fs;
typedef Svec<Vec2D>             Vec2Ds;
typedef Svec<VecD2>             VecD2s;
typedef Svec<Vec3F>             Vec3Fs;
typedef Svec<Vec3UI>            Vec3UIs;
typedef Svec<Vec3D>             Vec3Ds;
typedef Svec<Vec4UI>            Vec4UIs;
typedef Svec<Vec4F>             Vec4Fs;
typedef Svec<Vec4F>             Vec4Ds;

typedef Svec<Vec2Ds>            Vec2Dss;
typedef Svec<Vec2Fs>            Vec2Fss;
typedef Svec<Vec3Fs>            Vec3Fss;
typedef Svec<Vec3Ds>            Vec3Dss;
typedef Svec<Vec3UIs>           Vec3UIss;
typedef Svec<Vec4UIs>           Vec4UIss;

typedef Mat<float,2,2>          Mat22F;
typedef Mat<double,2,2>         Mat22D;
typedef Mat<int,2,2>            Mat22I;
typedef Mat<uint,2,2>           Mat22UI;
typedef Svec<Mat22F>            Mat22Fs;
typedef Svec<Mat22D>            Mat22Ds;

typedef Mat<float,3,3>          Mat33F;
typedef Mat<double,3,3>         Mat33D;
typedef Svec<Mat33F>            Mat33Fs;
typedef Svec<Mat33D>            Mat33Ds;

typedef Mat<float,4,2>          Mat42F;
typedef Mat<float,4,4>          Mat44F;
typedef Mat<double,4,4>         Mat44D;


typedef Mat<float,2,3>          Mat23F;
typedef Mat<double,2,3>         Mat23D;
typedef Mat<float,3,2>          Mat32F;
typedef Mat<double,3,2>         Mat32D;
typedef Mat<int,3,2>            Mat32I;
typedef Mat<size_t,3,2>         Mat32SZ;
typedef Mat<int64,3,2>          Mat32L;
typedef Mat<uint,3,2>           Mat32UI;
typedef Mat<double,3,4>         Mat34D;

template<class T,class U,size_t R,size_t C>
inline Mat<T,R,C>   mapCast(Mat<U,R,C> const & mat) {return Mat<T,R,C>(mapCast<T,U,R*C>(mat.m)); }

template <class T,size_t R,size_t C>
std::ostream &      operator<<(std::ostream & ss,Mat<T,R,C> const & mm)
{
    std::ios::fmtflags      oldFlag = ss.setf(std::ios::fixed | std::ios::showpos | std::ios::right);
    std::streamsize         oldPrec = ss.precision(6);
    if (mm.numRows() == 1 || mm.numCols() == 1) {   // Vector prints in single line
        ss << "[" << mm[0];
        for (size_t ii=1; ii<mm.numElems(); ii++)
            ss << "," << mm[ii];
        ss << "]";
    }
    else {
        ss << fgpush;
        for (size_t row=0; row<mm.numRows(); row++) {
            ss << fgnl;
            ss << "[ ";
            for (size_t col=0; col<mm.numCols(); col++)
                ss << mm.rc(row,col) << "  ";
            ss << "]";
        }
        ss << fgpop;
    }
    ss.flags(oldFlag);
    ss.precision(oldPrec);
    return ss;
}

// construct by function

template<class T,size_t D>
Mat<T,D,D>          cMatDiag(T val)         // diagonal matrix of one value. Template arguments required
{
    Mat<T,D,D>          ret {0};
    for (size_t ii=0; ii<D; ++ii)
        ret.rc(ii,ii) = val;
    return ret;
}
template<class T,size_t D>
Mat<T,D,D>          cMatDiag(Arr<T,D> diagVals)
{
    Mat<T,D,D>          ret {0};
    for (size_t ii=0; ii<D; ++ii)
        ret.rc(ii,ii) = diagVals[ii];
    return ret;
}
template<class T,size_t D>
Mat<T,D,D>          cMatDiag(Mat<T,D,1> diagVals) {return cMatDiag(diagVals.m); }

template<class T>
Mat<T,2,2>          cMatDiag(T v0,T v1) {return {v0,0, 0,v1}; }

template<class T>
Mat<T,3,3>          cMatDiag(T v0,T v1,T v2) {return {v0,0,0, 0,v1,0, 0,0,v2}; }

template<class T,size_t R,size_t C>
bool                isFinite(Mat<T,R,C> const & mat)
{
    for (size_t ii=0; ii<R*C; ++ii)
        if (!std::isfinite(mat[ii]))
            return false;
    return true;
}

template<class T,size_t R,size_t C>
inline auto         cMag(Mat<T,R,C> const & m) {return cMag(m.m); }
template<class T,size_t R,size_t C>
T                   cSsd(Mat<T,R,C> const & l,Mat<T,R,C> const & r) {return cSsd(l.m,r.m); }
template<class T,size_t R,size_t C>
Mat<T,R,C>          operator*(T val,Mat<T,R,C> const & mat) {return (mat * val); }
template <class T,size_t R,size_t C,size_t C2>
Mat<T,R,C2>         operator*(Mat<T,R,C> const & v1,Mat<T,C,C2> const & v2)
{
    Mat<T,R,C2>         newMat;
    for (size_t ii=0; ii<R; ii++) {
        for (size_t jj=0; jj<C2; jj++) {
            T               acc {0};
            for (size_t kk=0; kk<C; kk++)
                acc += v1[ii*C+kk] * v2[kk*C2+jj];
            newMat[ii*C2+jj] = acc;
        }
    }
    return newMat;
}
template <class T>
Mat<T,2,2>          matRotate(T radians)
{
    T               c = std::cos(radians),
                    s = std::sin(radians);
    return Mat<T,2,2>{
        c, -s,
        s,  c,
    };
}
template <class T>
Mat<T,3,3>          matRotateX(T radians)        // RHR rotation around X axis
{
    T               c = std::cos(radians),
                    s = std::sin(radians);
    return Mat<T,3,3>{
        1,  0,  0,
        0,  c, -s,
        0,  s,  c,
    };
}
template <class T>
Mat<T,3,3>          matRotateY(T radians)        // RHR rotation around Y axis
{
    T               c = std::cos(radians),
                    s = std::sin(radians);
    return Mat<T,3,3>{
        c,  0,  s,
        0,  1,  0,
        -s, 0,  c,
    };
}
template <class T>
Mat<T,3,3>          matRotateZ(T radians)        // RHR rotation around Z axis
{
    T               c = std::cos(radians),
                    s = std::sin(radians);
    return Mat<T,3,3>{
        c, -s,  0,
        s,  c,  0,
        0,  0,  1,
    };
}
// Create matrix for RHR around an arbitrary axis:
Mat33D              matRotateAxis(double radians,Vec3D const & normalizedZxis);

template<class T>
Mat<T,3,1>          cMirrorX(Mat<T,3,1> const & m) {return {-m[0],m[1],m[2]}; }

template<class T>
T                   cDeterminant(const Mat<T,2,2> & mat) {return (mat[0]*mat[3] - mat[1]*mat[2]); }
template<class T>
T                   cDeterminant(Mat<T,3,3> const & M)
{
    return
        M[0]*M[4]*M[8] +
        M[1]*M[5]*M[6] +
        M[2]*M[3]*M[7] -
        M[2]*M[4]*M[6] -
        M[1]*M[3]*M[8] -
        M[0]*M[5]*M[7];
}
template<class T>
T                   cDeterminant(Mat<T,4,4> const & M)
{
    // This may not be the fastest method:
    double          d0 = cDeterminant(Mat<T,3,3>{
        M.rc(1,1),  M.rc(1,2),  M.rc(1,3),
        M.rc(2,1),  M.rc(2,2),  M.rc(2,3),
        M.rc(3,1),  M.rc(3,2),  M.rc(3,3),
    }),
                    d1 = cDeterminant(Mat<T,3,3>{
        M.rc(1,0),  M.rc(1,2),  M.rc(1,3),
        M.rc(2,0),  M.rc(2,2),  M.rc(2,3),
        M.rc(3,0),  M.rc(3,2),  M.rc(3,3),
    }),
                    d2 = cDeterminant(Mat<T,3,3>{
        M.rc(1,0),  M.rc(1,1),  M.rc(1,3),
        M.rc(2,0),  M.rc(2,1),  M.rc(2,3),
        M.rc(3,0),  M.rc(3,1),  M.rc(3,3),
    }),
                    d3 = cDeterminant(Mat<T,3,3>{
        M.rc(1,0),  M.rc(1,1),  M.rc(1,2),
        M.rc(2,0),  M.rc(2,1),  M.rc(2,2),
        M.rc(3,0),  M.rc(3,1),  M.rc(3,2),
    });
    return M[0]*d0 - M[1]*d1 + M[2]*d2 - M[3]*d3;
}
// append an element onto a column vec:
template<class T,size_t D>
inline Mat<T,D+1,1> append(Mat<T,D,1> const & vec,T val) {return Mat<T,D+1,1>{append(vec.m,val)}; }
// append an element onto a row vec:
template<class T,size_t D>
inline Mat<T,1,D+1> append(Mat<T,1,D> const & vec,T val) {return Mat<T,1,D+1>{append(vec.m,val)}; }

// Concatenate 2 column vectors:
template<class T,size_t D0,size_t D1>
Mat<T,D0+D1,1>      cat(Mat<T,D0,1> const & l,Mat<T,D1,1> const & r)
{
    return Mat<T,D0+D1,1>{cat(l.m,r.m)};
}

// Flatten a Svec of matrices into a Svec of scalars:
template<class T,size_t R,size_t C>
Svec<T>             flatten(Svec<Mat<T,R,C>> const & ms)
{
    Svec<T>       ret;
    ret.reserve(ms.size()*R*C);
    for (size_t ii=0; ii<ms.size(); ++ii)
        for (size_t jj=0; jj<R*C; ++jj)
            ret.push_back(ms[ii].m[jj]);
    return ret;
}

Doubles             flattenD(Floatss const & v);
template<class T,size_t R,size_t C>
Doubles             flattenD(Mat<T,R,C> const & mat)
{
    Doubles         ret;
    ret.reserve(mat.numElems());
    for (T e : mat.m)
        ret.push_back(scast<double>(e));
    return ret;
}
template<class T,size_t R,size_t C>
Doubles             flattenD(Svec<Mat<T,R,C>> const & ms)
{
    Doubles         ret;
    ret.reserve(ms.size()*R*C);
    for (Mat<T,R,C> const & mat : ms)
        for (T e : mat.m)
            ret.push_back(scast<double>(e));
    return ret;
}
template<class T,size_t R,size_t C>
Doubles             flattenD(const Svec<Svec<Mat<T,R,C>>> & mss)
{
    size_t          sz = 0;
    for (Svec<Mat<T,R,C>> const & ms : mss)
        sz += ms.size() * R * C;
    Doubles         ret;
    ret.reserve(sz);
    for (Svec<Mat<T,R,C>> const & ms : mss)
        for (Mat<T,R,C> const & mat : ms)
            for (T e : mat.m)
                ret.push_back(scast<double>(e));
    return ret;
}
// project a homogenous position vector back to euclidean representation:
template<class T,size_t dim>
Mat<T,dim-1,1>      projectHomog(Mat<T,dim,1> v)
{
    Mat<T,dim-1,1>    ret;
    T                 w = v[dim-1];
    FGASSERT(w != T(0));                // cannot be a direction vector
    for (size_t ii=0; ii<dim-1; ++ii)
        ret[ii] = v[ii] / w;
    return ret;
}
// Return homogeneous matrix representation of an affine transform:
template<class T, size_t dims>
Mat<T,dims+1,dims+1> asHomogMat(Mat<T,dims,dims> const & linear,Mat<T,dims,1> const & translation)
{
    Mat<T,dims+1,dims+1>    ret;
    for (size_t rr=0; rr<dims; rr++)
        for (size_t cc=0; cc<dims; cc++)
            ret.rc(rr,cc) = linear.rc(rr,cc);
    for (size_t rr=0; rr<dims; rr++)
        ret.rc(rr,dims) = translation[rr];
    ret.rc(dims,dims) = 1;
    return ret;
}
// Return homogeneous matrix representation of a linear transform:
template<class T, size_t dims>
Mat<T,dims+1,dims+1> asHomogMat(Mat<T,dims,dims> const & linear)
{
    Mat<T,dims+1,dims+1>    ret {0};        // need zero-init for bottom row
    for (size_t rr=0; rr<dims; ++rr)
        for (size_t cc=0; cc<dims; ++cc)
            ret.rc(rr,cc) = linear.rc(rr,cc);
    ret.rc(dims,dims) = T(1);
    return ret;
}
// Return homogeneous matrix representation of a translation:
template<class T, size_t dims>
Mat<T,dims+1,dims+1> asHomogMat(Mat<T,dims,1> const & translation)
{
    auto            ret = cMatDiag<T,dims+1>(1);
    for (size_t rr=0; rr<dims; rr++)
        ret.rc(rr,dims) = translation[rr];
    return ret;
}
template<class T,size_t R,size_t C,size_t S>
Mat<T,R,C>          scaleRows(Mat<T,R,C> const & mat,Arr<T,S> const & scales)
{
    static_assert(C==S);            // best we can do until Matrix is changed to use size_t
    Mat<T,R,C>          ret;
    for (size_t rr=0; rr<R; ++rr)
        for (size_t cc=0; cc<C; ++cc)
            ret.rc(rr,cc) = mat.rc(rr,cc) * scales[rr];
    return ret;
}
// returns the inverse of an invertible matrix, throws if not invertible:
template <class T>
Mat<T,2,2>          cInverse(Mat<T,2,2> const & mat)
{
    static_assert(std::is_floating_point<T>::value,"Mat inverse requires floating point type");
    Mat<T,2,2>     ret;
    T   fac = (mat.rc(0,0) * mat.rc(1,1) - mat.rc(0,1) * mat.rc(1,0));
    FGASSERT(fac != T(0));
    fac = T(1) / fac;
    ret.rc(0,0) = mat.rc(1,1) * fac;
    ret.rc(0,1) = - mat.rc(0,1) * fac;
    ret.rc(1,0) = - mat.rc(1,0) * fac;
    ret.rc(1,1) = mat.rc(0,0) * fac;
    return ret;
}
template <class T,FG_ENABLE_IF(T,is_floating_point)>
Mat<T,3,3>          cInverse(Mat<T,3,3> const & i)
{
    T                   det = cDeterminant(i);
    FGASSERT(det != 0);
    Mat<T,3,3>          r {
        i.rc(1,1) * i.rc(2,2) - i.rc(1,2) * i.rc(2,1),
        i.rc(0,2) * i.rc(2,1) - i.rc(0,1) * i.rc(2,2),
        i.rc(0,1) * i.rc(1,2) - i.rc(0,2) * i.rc(1,1),
        i.rc(1,2) * i.rc(2,0) - i.rc(1,0) * i.rc(2,2),
        i.rc(0,0) * i.rc(2,2) - i.rc(0,2) * i.rc(2,0),
        i.rc(0,2) * i.rc(1,0) - i.rc(0,0) * i.rc(1,2),
        i.rc(1,0) * i.rc(2,1) - i.rc(1,1) * i.rc(2,0),
        i.rc(0,1) * i.rc(2,0) - i.rc(0,0) * i.rc(2,1),
        i.rc(0,0) * i.rc(1,1) - i.rc(1,0) * i.rc(0,1)
    };
    return r * (1/det);
}
template <class T,size_t R,size_t C>
T                   cDot(Mat<T,R,C> const & lhs,Mat<T,R,C> const & rhs) {return cDot(lhs.m,rhs.m); }
// the 'perp dot product' (aka 2D cross product) is the dot product of a 2D vector's perpendicular
// with another 2D vector, and is equivalent to:
// * the signed area of the parallegram they form
// * the determinant of the matrix formed by the two vectors as columns
// * the cross product Z component of the corresponding 3D vectors with Z=0:
template <class T>
T                   cPerpDot(Mat<T,2,1> const & lhs,Mat<T,2,1> const & rhs) {return lhs[0]*rhs[1] -  lhs[1]*rhs[0]; }
template <class T,size_t R,size_t C>
double              cCos(Mat<T,R,C> const & lhs,Mat<T,R,C> const & rhs)
{
    double      mag = cMagD(lhs) * cMagD(rhs);
    FGASSERT(mag > 0.0);
    return cDot(lhs,rhs)/sqrt(mag);
}
template<typename T>
Mat<T,3,1>          crossProduct(Mat<T,3,1> const & v1,Mat<T,3,1> const & v2)
{
    return {
        v1[1] * v2[2] - v1[2] * v2[1],
        v1[2] * v2[0] - v1[0] * v2[2],
        v1[0] * v2[1] - v1[1] * v2[0]
    };
}
// Equivalent to V * Y.transpose() (but more efficient and succinct):
template<typename T,size_t R,size_t C>
Mat<T,R,C>          outerProduct(Mat<T,R,1> const & lhs,Mat<T,C,1> const & rhs)
{
    Mat<T,R,C>      ret;
    for (size_t rr=0; rr<R; ++rr)
        for (size_t cc=0; cc<C; ++cc)
            ret.rc(rr,cc) = lhs[rr] * rhs[cc];
    return ret;
}

// Maps over Svec<Mat<>>:

template<class T,size_t R,size_t C>
Doubles             mapMag(Svec<Mat<T,R,C>> const & v) {return mapCall(v,[](Mat<T,R,C> const & e){return cMagD(e); }); }
template<class T,size_t R,size_t C>
Doubles             mapLen(Svec<Mat<T,R,C>> const & v) {return mapCall(v,[](Mat<T,R,C> const & e){return cLenD(e); }); }

// UNARY & BINARY MAP OPERATIONS:

template<size_t R,size_t C,class T>
Mat<T,R,C>          cMat(Arr<T,R*C> const & d) {return Mat<T,R,C>{d}; }                 // helper
template<class T,size_t R,size_t C,class F>
auto                mapCall(Mat<T,R,C> const & m,F f) {return cMat<R,C>(mapCall(m.m,f)); }
template<class T,class U,size_t R,size_t C,class F>
auto                mapCall(Mat<T,R,C> const & l,Mat<U,R,C> const & r,F f) {return cMat<R,C>(mapCall(l.m,r.m,f)); }

// Add / subtract same value from each element:
template<typename T,size_t R,size_t C>
Mat<T,R,C>          mapAdd(Mat<T,R,C> const & lhs,T const & rhs) {return Mat<T,R,C>{mapAdd(lhs.m,rhs)}; }
template<typename T,size_t R,size_t C>
Mat<T,R,C>          mapSub(Mat<T,R,C> const & lhs,T const & rhs) {return Mat<T,R,C>{mapSub(lhs.m,rhs)}; }

// Element-wise multiplication (aka Hadamard product):
template<typename T,size_t R,size_t C>
Mat<T,R,C>          mapMul(Mat<T,R,C> const & l,Mat<T,R,C> const & r) {return Mat<T,R,C>{mapMul(l.m,r.m)}; }

// Element-wise division & scalar-vector division:
template<typename T,size_t R,size_t C>
Mat<T,R,C>          mapDiv(Mat<T,R,C> const & l,Mat<T,R,C> const & r) {return Mat<T,R,C>{mapDiv(l.m,r.m)}; }
template<typename T,size_t R,size_t C>
Mat<T,R,C>          mapDiv(T l,Mat<T,R,C> r) {return Mat<T,R,C>{mapDiv(l,r.m)}; }

template<class T,size_t R,size_t C>
Mat<T,R,C>          mapSqr(Mat<T,R,C> m) {return Mat<T,R,C>{mapSqr(m.m)}; }

template<typename T,size_t R,size_t C>
Mat<T,R,C>          mapFloor(Mat<T,R,C> const & mat) {return Mat<T,R,C>{mapFloor(mat.m)}; }
template<typename Flt,typename Int,size_t R,size_t C>
void                mapRound_(Mat<Flt,R,C> const & lhs,Mat<Int,R,C> & rhs)
{
    for (size_t ii=0; ii<rhs.numElems(); ++ii)
        mapRound_(lhs[ii],rhs[ii]);
}
template<class T,class U,size_t R,size_t C>
Mat<T,R,C>          mapRound(Mat<U,R,C> const & m) {return Mat<T,R,C>{mapRound<T,U>(m.m)}; }

template<size_t R,size_t C>
Mat<uint,R,C>       mapPow2Ceil(Mat<uint,R,C> const & m) {return mapCall(m,pow2Ceil); }

template<typename T,size_t R,size_t C>
Mat<T,R,C>          mapMax(Mat<T,R,C> const & lhs,T rhs)
{
    return mapCall(lhs,[rhs](T l){return cMax(l,rhs); });
}
template<typename T,size_t R,size_t C>
Mat<T,R,C>          mapMax(Mat<T,R,C> const & ml,Mat<T,R,C> const & mr)
{
    return mapCall(ml,mr,[](T l,T r){return cMax(l,r); });
}
template<typename T,size_t R,size_t C>
Mat<T,R,C>          mapMin(Mat<T,R,C> const & lhs,T rhs)
{
    return mapCall(lhs,[rhs](T l){return cMin(l,rhs); });
}
template<typename T,size_t R,size_t C>
Mat<T,R,C>          mapMin(Mat<T,R,C> const & ml,Mat<T,R,C> const & mr)
{
    return mapCall(ml,mr,[](T l,T r){return cMin(l,r); });
}

template<class T,size_t R,size_t C>
Mat<T,R,C>          mapAbs(Mat<T,R,C> m) {return Mat<T,R,C>{mapAbs(m.m)}; }
template<class T,size_t R,size_t C>
Mat<T,R,C>          mapExp(Mat<T,R,C> m) {return Mat<T,R,C>{mapExp(m.m)}; }
template<class T,size_t R,size_t C>
Mat<T,R,C>          mapLog(Mat<T,R,C> m) {return Mat<T,R,C>{mapLog(m.m)}; }

template<class T,size_t R>
Mat<T,R,1>          sortAll(Mat<T,R,1> v) {return Mat<T,R,1>{sortAll(v.m)}; }

// Faster equivalent to lhs^T * rhs:
template<typename T,size_t n0,size_t n1,size_t n2>
Mat<T,n0,n1>        transposeMul(Mat<T,n2,n0> const & lhs,Mat<T,n2,n1> const & rhs)
{
    Mat<T,n0,n1>      ret(T(0));
    for (size_t i0=0; i0<n0; ++i0)
        for (size_t i1=0; i1<n1; ++i1)
            for (size_t i2=0; i2<n2; ++i2)
                ret.rc(i0,i1) += lhs.rc(i2,i0) * rhs.rc(i2,i1);
    return ret;
}
template<typename T,size_t dim,FG_ENABLE_IF(T,is_floating_point)>
Svec<Mat<T,dim,1>>  randVecNormals(size_t sz,double stdev)
{
    auto                fn = [stdev](size_t){return Mat<T,dim,1>::randNormal(stdev); };
    return genSvec(sz,fn);
}

// random positive definite linear transform with std normally distributed eigevalue logarithms:
Mat33D              cRandPositiveDefinite3D();

// Create a wider matrix by concatenating rows from 2 matrices:
template<class T,size_t R,size_t C>
Mat<T,R,C*2>        catH(Arr<Mat<T,R,C>,2> const & arrs)
{
    Mat<T,R,C*2>        ret;
    for (size_t rr=0; rr<R; ++rr) {
        size_t              col=0;
        for (size_t cc=0; cc<C; ++cc)
            ret.rc(rr,col++) = arrs[0].rc(rr,cc);
        for (size_t cc=0; cc<C; ++cc)
            ret.rc(rr,col++) = arrs[1].rc(rr,cc);
    }
    return ret;
}
template<class T,size_t R,size_t C1,size_t C2>
Mat<T,R,C1+C2>      catH(Mat<T,R,C1> const & l,Mat<T,R,C2> const & r)
{
    Mat<T,R,C1+C2>      ret;
    for (size_t rr=0; rr<R; ++rr) {
        size_t              col=0;
        for (size_t cc=0; cc<C1; ++cc)
            ret.rc(rr,col++) = l.rc(rr,cc);
        for (size_t cc=0; cc<C2; ++cc)
            ret.rc(rr,col++) = r.rc(rr,cc);
    }
    return ret;
}
// Create a taller matrix by concatenating cols from 2 matrices:
template<class T,size_t R1,size_t R2,size_t C>
Mat<T,R1+R2,C>      catV(Mat<T,R1,C> const & u,Mat<T,R2,C> const & l)
{
    Mat<T,R1+R2,C>      ret;
    for (size_t rr=0; rr<R1; ++rr)
        for (size_t cc=0; cc<C; ++cc)
            ret.rc(rr,cc) = u.rc(rr,cc);
    for (size_t rr=0; rr<R2; ++rr)
        for (size_t cc=0; cc<C; ++cc)
            ret.rc(R1+rr,cc) = l.rc(rr,cc);
    return ret;
}

// create square matrix to rotation permute elements of 3D vectors:
template<class T>
Mat<T,3,3>          cRotationPermuter(size_t axisToBecomeX)
{
    FGASSERT(axisToBecomeX < 3);
    Mat<T,3,3>          ret {0};
    for (size_t ii=0; ii<3; ++ii)
        ret.rc(ii,(ii+axisToBecomeX)%3) = T(1);
    return ret;
}

template<class T,size_t R,size_t C>
bool                isPow2(Mat<T,R,C> const & mat)
{
    for (size_t ii=0; ii<R*C; ++ii)
        if (!isPow2(mat[ii]))
            return false;
    return true;
}
template<class T,size_t R,size_t C>
double              cDot(Svec<Mat<T,R,C>> const & v0,Svec<Mat<T,R,C>> const & v1)
{
    FGASSERT(v0.size() == v1.size());
    double  acc(0);
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += cDot(v0[ii],v1[ii]);
    return acc;
}
// Weighted dot product:
template<class T,size_t R,size_t C>
double              reduceDotWgt(
    Svec<Mat<T,R,C>> const &    v0,
    Svec<Mat<T,R,C>> const &    v1,
    Svec<T> const &             w)    // Weight to apply to each dot product
{
    FGASSERT(v0.size() == v1.size());
    FGASSERT(v0.size() == w.size());
    double  acc(0);
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += cDot(v0[ii],v1[ii]) * w[ii];
    return acc;
}
template<typename T,size_t dim>
T                   cTrace(const Mat<T,dim,dim> & mat)
{
    T                   ret(0);
    size_t constexpr      inc = dim+1;    // Increment by a row's worth plus 1
    for (size_t ii=0; ii<mat.numElems(); ii+=inc)
        ret += mat[ii];
    return ret;
}
// defaults to linear interpolation for Mat<float>
template<class T,size_t R,size_t C,FG_ENABLE_IF(T,is_floating_point)>
Mat<T,R,C>          interpolate(Mat<T,R,C> a,Mat<T,R,C> b,T c) {return Mat<T,R,C>{interpolate(a.m,b.m,c)}; }

template<class T,size_t R,size_t C>
inline T            cSumElems(Mat<T,R,C> const & mat) {return cSum(mat.m); }

template<typename T,size_t R,size_t C>
Mat<T,R,C>          normalize(Mat<T,R,C> mat)
{
    double              len = cLenD(mat);
    FGASSERT(len > 0);
    return mat / scast<T>(len);
}
// Find first index of an element in a Svec. Return 'size' if not found:
template<typename T,size_t R>
size_t                findFirstIdx(Mat<T,R,1> mat,T v)
{
    for (size_t ii=0; ii<R; ++ii)
        if (mat[ii] == v)
            return ii;
    return R;
}

template<typename T,size_t R,size_t C>
bool                noZeroElems(Mat<T,R,C> mat)
{
    T                   acc = T(1);
    for (T v : mat.m)
        acc *= v;
    return (acc != T(0));
}

template<class T,size_t R,size_t C>
inline double       cMagD(Mat<T,R,C> mat) {return cMagD(mat.m); }

template<class T,size_t R,size_t C>
double              cRms(Mat<T,R,C> mat) {return cRms(mat.m); }

template<size_t R,size_t C>
Mat<double,R,C>     cReal(const Mat<std::complex<double>,R,C> & mat)   // Return real compoments
{
    Mat<double,R,C>   ret;
    for (size_t ii=0; ii<R*C; ++ii)
        ret[ii] = mat[ii].real();
    return ret;
}

// Transpose a matrix stored as an array of arrays. All sub-arrays must have same size:
template<class T,size_t R,size_t C>
Mat<Svec<T>,R,C>    transpose(Svec<Mat<T,R,C>> const & v)
{
    Mat<Svec<T>,R,C>   ret;
    for (size_t ii=0; ii<R*C; ++ii)
        ret[ii].resize(v.size());
    for (size_t jj=0; jj<v.size(); ++jj) {
        for (size_t ii=0; ii<R*C; ++ii)
            ret[ii][jj] = v[jj][ii];
    }
    return ret;
}

// project a vector onto an unnormalized arbitrary axis
// (ie. find the vector component parallel to that axis):
template<class T,size_t dim>
Mat<T,dim,1>        projectVec(Mat<T,dim,1> vec,Mat<T,dim,1> axis)
{
    return axis * (cDot(vec,axis) / cMagD(axis));
}

// remap vector indices from the given lookup list:
template<size_t N>
Arr<uint,N>       remapInds(Arr<uint,N> inds,Uints const & inToOut)
{
    Arr<uint,N>           r;
    for (uint ii=0; ii<N; ++ii)
        r[ii] = inToOut[inds[ii]];
    return r;
}
template<size_t N>
Svec<Arr<uint,N>> remapInds(Svec<Arr<uint,N>> const & inds,Uints const & inToOut)
{
    Svec<Arr<uint,N>>     ret; ret.reserve(inds.size());
    for (Arr<uint,N> const & idx : inds)
        ret.push_back(remapInds(idx,inToOut));
    return ret;
}

// Change a square matrix representation under a permutation of 2 of the axes:
template<class T,size_t dim>
Mat<T,dim,dim>      cRotationPermuter(
    Mat<T,dim,dim> const &      M,
    Mat<uint,dim,1>             p)  // Permuation map from input index to output index. NOT checked for validity.
{
    Mat<T,dim,dim>      ret;
    for (uint rr=0; rr<dim; ++rr)
        for (uint cc=0; cc<dim; ++cc)
            ret.rc(p[rr],p[cc]) = M.rc(rr,cc);
    return ret;
}

template<class T,size_t R,size_t C>
Mat<T,C,R>          cHermitian(Mat<T,R,C> const & mat)
{
    Mat<T,C,R>        ret;
    for (size_t rr=0; rr<R; ++rr)
        for (size_t cc=0; cc<C; ++cc)
            ret.rc(cc,rr) = std::conj(mat.rc(rr,cc));
    return ret;
}

struct      MatS2D              // Symmetric 2x2 double matrix
{
    double          m00,m11,m01;
    FG_SER(m00,m11,m01)

    inline Vec2D    operator*(Vec2D const & v) const {return {m00*v[0]+m01*v[1], m01*v[0]+m11*v[1] }; }
    inline double   determinant() const {return m00 * m11 - m01 * m01; }

    // isotropic random symmetric positive definite matrix with ln eigenvalues ~ N(0,lnEigStdev):
    static MatS2D   randSpd(double lnEigStdev);
};

inline MatS2D       outerProductSelf(Vec2D v) {return {sqr(v[0]), sqr(v[1]), v[0]*v[1]}; }

struct      MatUT2D             // upper triangular 2x2 double matrix
{
    double          m00,m11,m01;
    FG_SER(m00,m11,m01)

    inline Vec2D    operator*(Vec2D v) const {return {m00*v[0]+m01*v[1],m11*v[1]}; }    // M * vec
    inline MatUT2D  operator*(double s) const {return {m00*s,m11*s,m01*s}; }            // scale elements of M
    inline MatS2D   luProduct() const {return {sqr(m00),sqr(m01)+sqr(m11),m00*m01}; }   // M^T * M
    inline double   determinant() const {return m00*m11; }
};

// Symmetric 3x3 double matrix:
struct      MatS3D
{
    Arr<double,3>       diag;
    Arr<double,3>       offd;       // In order 01, 02, 12
    FG_SER(diag,offd);

    MatS3D() {}
    explicit MatS3D(double fillVal) : diag{fillVal}, offd{fillVal} {}
    explicit MatS3D(Mat33D const &);   // Checks for exact symmetry
    MatS3D(Arr<double,3> const & d,Arr<double,3> const & o) : diag(d), offd(o)  {}

    double              rc(size_t row,size_t col) const
    {
        FGASSERT((row<3)&&(col<3));
        if (row==col)
            return diag[row];
        else
            return offd[row+col-1];
    }
    double &            rc(size_t row,size_t col)
    {
        FGASSERT((row<3)&&(col<3));
        if (row==col)
            return diag[row];
        else
            return offd[row+col-1];
    }
    Mat33D              asMatC() const
    {
        return Mat33D {diag[0],offd[0],offd[1], offd[0],diag[1],offd[2], offd[1],offd[2],diag[2]};
    }
    void                operator+=(MatS3D const & rhs) {diag += rhs.diag; offd += rhs.offd; }
    MatS3D              operator+(MatS3D const & rhs) const {return {diag+rhs.diag,offd+rhs.offd}; }
    Mat33D              operator+(Mat33D const & rhs) const;
    MatS3D              operator-(MatS3D const & rhs) const {return {diag-rhs.diag,offd-rhs.offd}; }
    MatS3D              operator*(double rhs) const {return {diag*rhs,offd*rhs}; }
    Vec3D               operator*(Vec3D rhs) const;
    // product of symmetric matrices is not in general symmetric:
    // (however powers of a symmetric matrix are symmetric)
    Mat33D              operator*(MatS3D const & rhs) const;
    Mat33D              operator*(Mat33D const & rhs) const;
    double              sumElems() const {return cSum(diag) + 2.0 * cSum(offd); }
    // Sum of squares of elements (== square of frobenius norm):
    double              magD() const {return cMagD(diag) + 2.0 * cMagD(offd); }
    double              dot(MatS3D const & rhs) const {return cDot(diag,rhs.diag) + 2.0 * cDot(offd,rhs.offd); }
    double              determinant() const;
    MatS3D              inverse() const;
    MatS3D              square() const;     // the square of a symmetric matrix is also symmetric
    // returns M^T * S * M, so that the action on vectors x^T * S * x acts as if the vectors have been transformed to x' = M * x.
    MatS3D              transform(Mat33D const & M) const;

    static MatS3D       diagonal(double v) {return {Arr3D{v},Arr3D{0}}; }
    static MatS3D       identity() {return MatS3D::diagonal(1); }
    // isotropic random symmetric positive definite matrix with ln eigenvalues ~ N(0,lnEigStdev):
    static MatS3D       randSpd(double lnEigStdev);
};
typedef Svec<MatS3D>    MatS3Ds;

std::ostream &      operator<<(std::ostream & os,MatS3D const & mat);
inline double       cMagD(MatS3D const & mat) {return mat.magD(); }
inline double       cDot(MatS3D const & l,MatS3D const & r) {return l.dot(r); }
inline double       cDeterminant(MatS3D const & mat) {return mat.determinant(); }
inline MatS3D       cInverse(MatS3D const & mat) {return mat.inverse(); }
// apply only a quadratic form (no factor of 1/2, no constant):
inline double       cQuadForm(MatS3D const & Q,Vec3D const & x_m) {return cDot(x_m,Q*x_m); }
// return the congruent transform of a 3D symmetric matrix S, M^T * S * M, which is also symmetric:
MatS3D              cCongruentTransform(MatS3D const & S,Mat33D const & M);
inline MatS3D       mapSqr(MatS3D const & mat) {return MatS3D {mapSqr(mat.diag),mapSqr(mat.offd)}; }
inline MatS3D       mapMul(MatS3D const & mat,MatS3D const & n)
{
    return {mapMul(mat.diag,n.diag),mapMul(mat.offd,n.offd)};
}
inline MatS3D       outerProductSelf(Arr3D v)
{
    return          {{mapSqr(v)},{v[0]*v[1],v[0]*v[2],v[1]*v[2]}};
}
MatS3D              transposeSelfProduct(Mat33D const & M);       // returns M^T * M, which is always SPD

struct      MeanCov3
{
    Vec3D           mean;       // mean from a set of samples
    MatS3D          cov;        // covariance of samples after zero-mean correction
};
MeanCov3            cMeanCov(Vec3Ds const & samps);

// Upper triangular 3x3 matrix
struct      MatUT3D
{
    Arr<double,6>       m;              // non-zero elements in row-major order
    FG_SER(m)

    MatUT3D() {}
    explicit MatUT3D(double v) : m{v} {}                                // fill ctor
    explicit MatUT3D(Arr<double,6> const & data) : m{data} {}
    MatUT3D(double s0,double s1,double s2) : m{s0,0,0,s1,0,s2} {}         // diagonal matrix
    MatUT3D(double m0,double m1,double m2,double m3,double m4,double m5) : m{m0,m1,m2,m3,m4,m5} {}
    explicit MatUT3D(Arr3D const & s) : m{s[0],0,0,s[1],0,s[2]} {}        // diagonal matrix
    MatUT3D(Arr3D const & diag,Arr3D const & offd) : m{diag[0],offd[0],offd[1],diag[1],offd[2],diag[2]} {}

    static MatUT3D  identity()  {return MatUT3D{1,1,1}; }
    static MatUT3D  diagonal(double v) {return MatUT3D{v,v,v}; }

    MatUT3D         operator-(MatUT3D const & rhs) const {return MatUT3D(m-rhs.m); }
    MatUT3D         operator*(double s) const {return MatUT3D {m*s}; }
    MatUT3D         operator/(double s) const {return  MatUT3D {m/s}; }
    Vec3D           operator*(Vec3D v) const
    {
        return Vec3D {m[0]*v[0]+m[1]*v[1]+m[2]*v[2], m[3]*v[1]+m[4]*v[2], m[5]*v[2]};
    }
    Vec3D           tranposeMul(Vec3D r)    // U^T * v
    {
        return Vec3D {
            m[0]*r[0],
            m[1]*r[0] + m[3]*r[1],
            m[2]*r[0] + m[4]*r[1] + m[5]*r[2] };
    }
    MatUT3D         operator*(MatUT3D r) const;
    double          determinant() const {return m[0]*m[3]*m[5]; }
    Mat33D          asMatrix() const {return Mat33D {m[0],m[1],m[2],0,m[3],m[4],0,0,m[5]}; }
    // Sum of squares of elements (== square of frobenius norm):
    double          magD() const {return cMagD(m); }
    MatS3D          luProduct() const;
    MatUT3D         inverse() const;
};
std::ostream &      operator<<(std::ostream &,MatUT3D const &);
inline double       cDeterminant(MatUT3D const & mat) {return mat.determinant(); }
inline double       cMagD(MatUT3D const & mat) {return mat.magD(); }

template<class T,size_t D>
struct      NameVec
{
    String          name;
    Mat<T,D,1>      vec;
    FG_SER(name,vec)

    NameVec() {}
    explicit NameVec(Mat<T,D,1> const & v) : vec{v} {}
    NameVec(String const & n,Mat<T,D,1> const & v) : name{n}, vec{v} {}

    bool            operator==(NameVec const & r) const {return ((name==r.name)&&(vec==r.vec)); }
    bool            operator==(String const & n) const {return (name==n); }     // easy lookup
};
typedef NameVec<float,2>    NameVec2F;
typedef Svec<NameVec2F>     NameVec2Fs;
typedef Svec<NameVec2Fs>    NameVec2Fss;
typedef NameVec<float,3>    NameVec3F;
typedef Svec<NameVec3F>     NameVec3Fs;

template<class T,size_t D>
std::ostream &      operator<<(std::ostream & os,NameVec<T,D> const & nv) {return os << nv.name << ": " << nv.vec; }

template<class T,size_t D,class U>
NameVec<T,D>        operator*(U const & xf,NameVec<T,D> const & nv) {return {nv.name,xf*nv.vec}; }

// select nameVecs in order of names and return the vecs. Throws if any of names is not found in nameVecs:
template<class T,size_t D>
Svec<Mat<T,D,1>>    selectVecsByName(Svec<NameVec<T,D>> const & nameVecs,Strings const & names)
{
    auto                fn = [&](String const & name)
    {
        size_t              idx = findFirstIdx(nameVecs,name);
        if (idx == nameVecs.size())
            fgThrow("Landmark not found",name);
        return nameVecs[idx].vec;
    };
    return mapCall(names,fn);
}

}

#endif
