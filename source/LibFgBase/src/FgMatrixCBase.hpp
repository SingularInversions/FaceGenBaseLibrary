//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
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
// Templatable by any copyable type, although some member functions require 
// mathematical operators.
//
//      DESIGN:
//
// Due to compiler optimizations such as loop unrolling the code should run pretty much
// as fast as if we'd written the various cases out separately.
//
// Accumulator types were deemed unnecessary for small, constant size matrices.
//

#ifndef FGMATRIXCBASE_HPP
#define FGMATRIXCBASE_HPP

#include "FgMath.hpp"
#include "FgFile.hpp"

namespace Fg {

// Declare only due to mutual dependence; Mat and MatV can be constructed
// from each other:
template<class T>
struct      MatV;

template <typename T,uint R,uint C>
struct      Mat
{
    Arr<T,R*C>          m;
    FG_SER1(m)

    // Auto-initialization of builtins to zero is deprecated, avoid depending on it by using
    // constant-value initialization constructor below:
    Mat() : m{} {}
    explicit Mat(Arr<T,R*C> const & data) : m(data) {}

    // Constant-value initialization constructor:
    // (unfortunately a pointer-based constructor cannot be added without being ambiguous
    // to the below which is widely used. Use 'cMat' below instead)
    explicit Mat(T v)
    {
        for (size_t ii=0; ii<R*C; ii++)
            m[ii] = v;
    }
    // Value-set constructors; # args must agree with # elements:
    Mat(T x,T y) : m {{x,y}}
    {static_assert(R*C == 2,"Number of arguments does not match elements"); }   //-V557 (for PVS-Studio)
    Mat(T x,T y,T z) : m {{x,y,z}}
    {static_assert(R*C == 3,"Number of arguments does not match elements"); }   //-V557 (for PVS-Studio)
    Mat(T a,T b,T c,T d) : m {{a,b,c,d}}
    {static_assert(R*C == 4,"Number of arguments does not match elements"); }   //-V557 (for PVS-Studio)
    Mat(T a,T b,T c,T d,T e) : m {{a,b,c,d,e}}
    {static_assert(R*C == 5,"Number of arguments does not match elements"); }   //-V557 (for PVS-Studio)
    Mat(T a,T b,T c,T d,T e,T f) : m {{a,b,c,d,e,f}}
    {static_assert(R*C == 6,"Number of arguments does not match elements"); }   //-V557 (for PVS-Studio)
    Mat(T a,T b,T c,T d,T e,T f,T g,T h,T i) : m {{a,b,c,d,e,f,g,h,i}}
    {static_assert(R*C == 9,"Number of arguments does not match elements"); }   //-V557 (for PVS-Studio)

    // default copy constructor (and thus assignment operator) must be explicitly defined to
    // differentiate from (explicit) conversion constructors below:
    Mat(Mat const &) = default;
    Mat &       operator=(Mat const &) = default;

    // Type conversion constructor very handy for float <-> double:
    template<class U>
    explicit Mat(Mat<U,R,C> const & mat) {
        for (uint ii=0; ii<R*C; ++ii)
            m[ii] = static_cast<T>(mat[ii]);
    }
    explicit Mat(MatV<T> const& mm) {
        FGASSERT((R == mm.numRows()) && (C == mm.numCols()));
        for (uint ii=0; ii<R*C; ++ii)
            m[ii] = mm[ii];
    }
    // Initialize from array data. This is done via a proxy type (accessed via a convenient
    // static member) since compilers interpret '0' as either 'int' or pointer, potentially
    // resulting in either:
    // 1. Ambiguity with single-value constructor (compile-time error)
    // 2. Accidental interpretation of '0' value as a pointer (run-time error)
    struct      FromPtr
    {
        FromPtr(T const * p) : _p(p) {}
        T const * _p;
    };
    explicit Mat(FromPtr p)
    {
        for (uint ii=0; ii<R*C; ++ii)
            m[ii] = *(p._p++);
    }
    static Mat      fromPtr(T const * p) {return Mat(FromPtr(p)); }

    uint            numRows() const {return R; }
    uint            numCols() const {return C; }
    uint            numElems() const {return R*C; }
    size_t          size() const {return R*C; }
    // Element access by (row,column):
    T &             rc(size_t row,size_t col)
    {
        FGASSERT_FAST((row < R) && (col < C));
        return m[row*C+col];
    }
    T const &       rc(size_t row,size_t col) const
    {
        FGASSERT_FAST((row < R) && (col < C));
        return m[row*C+col];
    }
    // Element access by (column,row):
    T &             cr(size_t col,size_t row)
    {
        FGASSERT_FAST((row < R) && (col < C));
        return m[row*C+col];
    }
    T const &       cr(size_t col,size_t row) const
    {
        FGASSERT_FAST((row < R) && (col < C));
        return m[row*C+col];
    }
    T &             operator[](size_t xx)
    {
        FGASSERT_FAST(xx < R*C);
        return m[xx];
    }
    T const &       operator[](size_t xx) const
    {
        FGASSERT_FAST(xx < R*C);
        return m[xx];
    }
    T &             operator[](Mat<uint,2,1> crd)
    {
        FGASSERT_FAST((crd.m[0]<C) && (crd.m[1]<R));
        return m[crd.m[1]*C+crd.m[0]];
    }
    T const &       operator[](Mat<uint,2,1> crd) const
    {
        FGASSERT_FAST((crd.m[0]<C) && (crd.m[1]<R));
        return m[crd.m[1]*C+crd.m[0]];
    }
    Mat             operator-() const
    {
        Mat           ret;
        for (uint ii=0; ii<R*C; ++ii)
            ret.m[ii] = -m[ii];
        return ret;
    }
    Mat             operator+(const Mat & rhs) const {return Mat{m + rhs.m}; }
    Mat             operator-(const Mat & rhs) const {return Mat{m - rhs.m}; }
    Mat             operator*(T val) const {return Mat{m * val}; }
    Mat             operator/(T val) const {return Mat{m / val}; }
    void            operator*=(T val) {for (uint ii=0; ii<R*C; ++ii) m[ii] *= val; }
    void            operator/=(T val) {for (uint ii=0; ii<R*C; ++ii) m[ii] /= val; }
    void            operator+=(const Mat & rhs) {for (uint ii=0; ii<R*C; ++ii) m[ii] += rhs.m[ii]; }
    void            operator-=(const Mat & rhs) {for (uint ii=0; ii<R*C; ++ii) m[ii] -= rhs.m[ii]; }
    bool            operator==(const Mat & rhs) const
    {
        for (uint ii=0; ii<R*C; ++ii)
            if (!(m[ii] == rhs.m[ii]))
                return false;
        return true;
    }
    bool            operator!=(const Mat & rhs) const
    {
        for (uint ii=0; ii<R*C; ++ii)
            if (m[ii] != rhs.m[ii])
                return true;
        return false;
    }
    template<uint srows,uint scols>
    Mat<T,srows,scols> subMatrix(uint firstRow,uint firstCol) const
    {
        FGASSERT_FAST((firstRow+srows <= R) && (firstCol+scols <= C));
        Mat<T,srows,scols>    ret;
        uint                        cnt = 0;
        for (uint rr=firstRow; rr<firstRow+srows; ++rr)
            for (uint cc=firstCol; cc<firstCol+scols; ++cc)
                ret[cnt++] = rc(rr,cc);
        return ret;
    }
    template <uint srows,uint scols>
    void            setSubMat(const Mat<T,srows,scols> & sub,uint row,uint col)
    {
        FGASSERT((srows+row <= R) && (scols+col <= C));
        for (uint rr=0; rr<srows; rr++)
            for (uint cc=0; cc<scols; cc++)
                rc(rr+row,cc+col) = sub.rc(rr,cc);
    }
    double          mag() const         // Squared magnitude
    {
        double      ret = 0.0;
        for (uint ii=0; ii<R*C; ++ii)
            ret += cMag(m[ii]);    // T can be non-scalar (eg. complex)
        return ret;
    }
    T               len() const {return sqrt(mag()); }
    Mat<T,C,R>      transpose() const
    {
        Mat<T,C,R> tMat;
        for (uint ii=0; ii<R; ii++)
            for (uint jj=0; jj<C; jj++)
                tMat.rc(jj,ii) = rc(ii,jj);
        return tMat;
    }
    Mat<T,R,1>      colVec(uint col) const
    {
        Mat<T,R,1>    ret;
        FGASSERT_FAST(col < R);
        for (uint rr=0; rr<R; rr++)
            ret[rr] = rc(rr,col);
        return ret;
    }
    Mat<T,1,C>      rowVec(uint row) const
    {
        Mat<T,1,C>    ret;
        FGASSERT_FAST(row < R);
        for (uint cc=0; cc<C; ++cc)
            ret[cc] = rc(row,cc);
        return ret;
    }
    T               cmpntsProduct() const
    {
        T   acc = m[0];
        for (uint ii=1; ii<C*R; ++ii)
            acc *= m[ii];
        return acc;
    }
    bool            operator<(const Mat & rhs) const      // Useful for putting in a std::map or sorting for unique check
    {
        for (uint ii=0; ii<R*C; ++ii) {
            if (m[ii] < rhs[ii])
                return true;
            if (rhs[ii] < m[ii])
                return false;
        }
        return false;
    }
    // Preserves row major ordering:
    Svec<T>         asStdVector() const
    {
        Svec<T>   ret;
        ret.reserve(size());
        for (size_t ii=0; ii<R*C; ++ii)
            ret.push_back(m[ii]);
        return ret;
    }

    // Static creation functions:

    static Mat      identity()
    {
        static_assert(R == C,"Identity matrix must be square");
        Mat               ret(T(0));
        for (uint ii=0; ii<R; ++ii)
            ret.rc(ii,ii) = T(1);
        return ret;
    }
    static Mat      diagonal(T v)
    {
        static_assert(R == C,"Diagonal matrix must be square");
        Mat               ret(T(0));
        for (uint ii=0; ii<R; ++ii)
            ret.rc(ii,ii) = v;
        return ret;
    }
    static Mat      randUniform(T lo,T hi);
    static Mat      randNormal(T stdev=T(1));
};

template<class T,uint R,uint C>
struct      Traits<Mat<T,R,C>>
{
    typedef typename Traits<T>::Scalar                                Scalar;
    typedef Mat<typename Traits<T>::Accumulator,R,C>    Accumulator;
    typedef Mat<typename Traits<T>::Floating,R,C>       Floating;
};

typedef Mat<float,2,1>          Vec2F;
typedef Mat<double,2,1>         Vec2D;
typedef Mat<short,2,1>          Vec2S;
typedef Mat<int,2,1>            Vec2I;
typedef Mat<uint,2,1>           Vec2UI;
typedef Mat<size_t,2,1>         Vec2Z;
typedef Mat<bool,2,1>           Vec2B;

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

template<typename To,typename From,uint R,uint C>
inline Mat<To,R,C>  mapCast(Mat<From,R,C> const & mat)
{return Mat<To,R,C>(mapCast<To,From,R*C>(mat.m)); }

template <class T,uint R,uint C>
std::ostream &      operator<<(std::ostream& ss,Mat<T,R,C> const & mmIn)
{
    typedef typename Traits<T>::Printable   P;
    Mat<P,R,C>              mm {mmIn};
    std::ios::fmtflags
        oldFlag = ss.setf(
            std::ios::fixed |
            std::ios::showpos |
            std::ios::right);
    std::streamsize oldPrec = ss.precision(6);
    if (mm.numRows() == 1 || mm.numCols() == 1) {   // Vector prints in single line
        ss << "[" << mm[0];
        for (uint ii=1; ii<mm.numElems(); ii++)
            ss << "," << mm[ii];
        ss << "]";
    }
    else {
        ss << fgpush;
        for (uint row=0; row<mm.numRows(); row++) {
            ss << fgnl;
            ss << "[ ";
            for (uint col=0; col<mm.numCols(); col++)
                ss << mm.rc(row,col) << "  ";
            ss << "]";
        }
        ss << fgpop;
    }
    ss.flags(oldFlag);
    ss.precision(oldPrec);
    return ss;
}

template<class T,uint R,uint C>
void                readBin_(std::istream & is,Mat<T,R,C> & mat)
{
    for (uint ii=0; ii<R*C; ++ii)
        readBin_(is,mat[ii]);
}
template<class T,uint R,uint C>
void                writeBin_(std::ostream & os,Mat<T,R,C> const & mat)
{
    for (uint ii=0; ii<R*C; ++ii)
        writeBin_(os,mat[ii]);
}

// function 'constructors':

template<typename T,uint R,uint C>
Mat<T,R,C>          cMat(T * const ptr)
{
    Mat<T,R,C>    ret;
    for (size_t ii=0; ii<R*C; ++ii)
        ret.m[ii] = *ptr++;
    return ret;
}

template<typename T,uint R,uint C>
Mat<T,R,C>          cMat(Svec<T> const & v)
{
    Mat<T,R,C>    ret;
    FGASSERT(v.size() == R*C);
    for (size_t ii=0; ii<v.size(); ++ii)
        ret.m[ii] = v[ii];
    return ret;
}

template<class T,uint R,uint C>
bool                isFinite(Mat<T,R,C> const & mat)
{
    for (uint ii=0; ii<R*C; ++ii)
        if (!std::isfinite(mat[ii]))
            return false;
    return true;
}

}

#endif
