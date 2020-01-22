//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Constant size (stack based) matrix / vector
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

#include "FgStdExtensions.hpp"
#include "FgTypes.hpp"
#include "FgMath.hpp"
#include "FgDiagnostics.hpp"
#include "FgSerialize.hpp"
#include "FgOut.hpp"

namespace Fg {

// Declare only due to mutual dependence; Mat and MatV can be constructed
// from each other:
template<class T>
struct  MatV;

template <typename T,uint nrows,uint ncols>
struct  Mat
{
    Arr<T,nrows*ncols>    m;
    FG_SERIALIZE1(m)

    // Auto-initialization of builtins to zero is deprecated, avoid depending on it by using
    // constant-value initialization constructor below:
    Mat()
    {
        for (uint ii=0; ii<nrows*ncols; ++ii)
            fgInitializeBuiltinsToZero(m[ii]);
    }

    explicit
    Mat(Arr<T,nrows*ncols> const & data) : m(data) {}

    // Constant-value initialization constructor:
    // (unfortunately a pointer-based constructor cannot be added without being ambiguous
    // to the below which is widely used. Use 'cMat' below instead)
    explicit
    Mat(T x)
    {setConstant(x); }

    // Value-set constructors; # args must agree with # elements:
    Mat(T x,T y) {
        static_assert(nrows*ncols == 2,"Number of arguments does not match elements");
        m[0] = x; m[1] = y; }                               //-V557 (for PVS-Studio)
    Mat(T x,T y,T z) {
        static_assert(nrows*ncols == 3,"Number of arguments does not match elements");
        m[0] = x; m[1] = y; m[2] = z; }                     //-V557 (for PVS-Studio)
    Mat(T a,T b,T c,T d) {
        static_assert(nrows*ncols == 4,"Number of arguments does not match elements");
        m[0]=a; m[1]=b; m[2]=c; m[3]=d; }                   //-V557 (for PVS-Studio)
    Mat(T a,T b,T c,T d,T e) {
        static_assert(nrows*ncols == 5,"Number of arguments does not match elements");
        m[0]=a; m[1]=b; m[2]=c; m[3]=d; m[4]=e; }           //-V557 (for PVS-Studio)
    Mat(T a,T b,T c,T d,T e,T f) {
        static_assert(nrows*ncols == 6,"Number of arguments does not match elements");
        m[0]=a; m[1]=b; m[2]=c; m[3]=d; m[4]=e; m[5]=f; }   //-V557 (for PVS-Studio)
    Mat(T a,T b,T c,T d,T e,T f,T g,T h,T i) {
        static_assert(nrows*ncols == 9,"Number of arguments does not match elements");
        m[0]=a; m[1]=b; m[2]=c; m[3]=d; m[4]=e; m[5]=f; m[6]=g; m[7]=h; m[8]=i; } //-V557 (for PVS-Studio)

    // CC explicit definition required to differentiate from conversion constructors below:
    Mat(const Mat & mat) = default;

    // Type conversion constructor. Use 'round<int>' if float->fixed rounding desired:
    template<class U>
    explicit
    Mat(const Mat<U,nrows,ncols> & mat) {
        for (uint ii=0; ii<nrows*ncols; ++ii)
            m[ii] = static_cast<T>(mat[ii]);
    }

    explicit
    Mat(const MatV<T>& mm) {
        FGASSERT((nrows == mm.numRows()) && (ncols == mm.numCols()));
        for (uint ii=0; ii<nrows*ncols; ++ii)
            m[ii] = mm[ii];
    }

    uint
    numRows() const
    {return nrows; }

    uint
    numCols() const
    {return ncols; }

    uint
    numElems() const
    {return nrows*ncols; }

    size_t
    size() const    // For template interface compatibility with std::vector
    {return nrows*ncols; }

    // Element access by (row,column):
    T &
    rc(size_t row,size_t col)
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m[row*ncols+col];
    }
    const T &
    rc(size_t row,size_t col) const
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m[row*ncols+col];
    }
    // Element access by (column,row):
    T &
    cr(size_t col,size_t row)
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m[row*ncols+col];
    }
    const T &
    cr(size_t col,size_t row) const
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m[row*ncols+col];
    }
    T &
    operator[](size_t xx)
    {
        FGASSERT_FAST(xx < nrows*ncols);
        return m[xx];
    }
    const T &
    operator[](size_t xx) const
    {
        FGASSERT_FAST(xx < nrows*ncols);
        return m[xx];
    }
    T &
    operator[](Mat<uint,2,1> crd)
    {
        FGASSERT_FAST((crd.m[0]<ncols) && (crd.m[1]<nrows));
        return m[crd.m[1]*ncols+crd.m[0]];
    }
    const T &
    operator[](Mat<uint,2,1> crd) const
    {
        FGASSERT_FAST((crd.m[0]<ncols) && (crd.m[1]<nrows));
        return m[crd.m[1]*ncols+crd.m[0]];
    }
    const T *
    data() const
    {return m.data(); }

    Mat
    operator+(const Mat & rhs) const
    {
        Mat           ret;
        for (uint ii=0; ii<nrows*ncols; ++ii)
            ret.m[ii] = m[ii] + rhs.m[ii];
        return ret;
    }

    Mat
    operator-(const Mat & rhs) const
    {
        Mat           ret;
        for (uint ii=0; ii<nrows*ncols; ++ii)
            ret.m[ii] = m[ii] - rhs.m[ii];
        return ret;
    }

    Mat
    operator-() const
    {
        Mat           ret;
        for (uint ii=0; ii<nrows*ncols; ++ii)
            ret.m[ii] = -m[ii];
        return ret;
    }

    Mat
    operator*(T val) const
    {
        Mat           ret;
        for (uint ii=0; ii<nrows*ncols; ++ii)
            ret.m[ii] = m[ii] * val;
        return ret;
    }

    Mat
    operator/(T val) const
    {
        Mat           ret;
        for (uint ii=0; ii<nrows*ncols; ++ii)
            ret.m[ii] = m[ii] / val;
        return ret;
    }

    void
    operator*=(T val)
    {for (uint ii=0; ii<nrows*ncols; ++ii) m[ii] *= val; }

    void
    operator/=(T val)
    {for (uint ii=0; ii<nrows*ncols; ++ii) m[ii] /= val; }

    void
    operator+=(const Mat & rhs)
    {for (uint ii=0; ii<nrows*ncols; ++ii) m[ii] += rhs.m[ii]; }

    void
    operator-=(const Mat & rhs)
    {for (uint ii=0; ii<nrows*ncols; ++ii) m[ii] -= rhs.m[ii]; }

    bool
    operator==(const Mat & rhs) const
    {
        for (uint ii=0; ii<nrows*ncols; ++ii)
            if (!(m[ii] == rhs.m[ii]))
                return false;
        return true;
    }

    bool
    operator!=(const Mat & rhs) const
    {
        for (uint ii=0; ii<nrows*ncols; ++ii)
            if (m[ii] != rhs.m[ii])
                return true;
        return false;
    }

    // Ternary compare to match STL containers. Ordering by axis:
    int
    compare(const Mat & rhs) const
    {
        for (uint ii=0; ii<nrows*ncols; ++ii) {
            if (m[ii] < rhs.m[ii])
                return -1;
            else if (rhs.m[ii] < m[ii])
                return 1;
        }
        return 0;
    }

    void
    setConstant(T v)
    {
        for (uint ii=0; ii<nrows*ncols; ii++)
            m[ii] = v;
    }

    void
    setZero()
    {setConstant(T(0)); }

    void
    setIdentity()
    {
        setConstant(T(0));
        uint nn = std::min(nrows,ncols);
        for (uint ii=0; ii<nn; ++ii)
            m[ii*ncols+ii] = T(1);
    }

    template<uint srows,uint scols>
    Mat<T,srows,scols>
    subMatrix(uint firstRow,uint firstCol) const
    {
        FGASSERT_FAST((firstRow+srows <= nrows) && (firstCol+scols <= ncols));
        Mat<T,srows,scols>    ret;
        uint                        cnt = 0;
        for (uint rr=firstRow; rr<firstRow+srows; ++rr)
            for (uint cc=firstCol; cc<firstCol+scols; ++cc)
                ret[cnt++] = rc(rr,cc);
        return ret;
    }

    template <uint srows,uint scols>
    void
    setSubMat(const Mat<T,srows,scols> & sub,uint row,uint col)
    {
        FGASSERT((srows+row <= nrows) && (scols+col <= ncols));
        for (uint rr=0; rr<srows; rr++)
            for (uint cc=0; cc<scols; cc++)
                rc(rr+row,cc+col) = sub.rc(rr,cc);
    }

    double
    mag() const         // Squared magnitude
    {
        double      ret = 0.0;
        for (uint ii=0; ii<nrows*ncols; ++ii)
            ret += cMag(m[ii]);    // T can be non-scalar (eg. complex)
        return ret;
    }

    T
    len() const
    {return sqrt(mag()); }

    Mat<T,ncols,nrows>
    transpose() const
    {
        Mat<T,ncols,nrows> tMat;
        for (uint ii=0; ii<nrows; ii++)
            for (uint jj=0; jj<ncols; jj++)
                tMat.rc(jj,ii) = rc(ii,jj);
        return tMat;
    }

    Mat<T,nrows,1>
    colVec(uint col) const
    {
        Mat<T,nrows,1>    ret;
        FGASSERT_FAST(col < nrows);
        for (uint rr=0; rr<nrows; rr++)
            ret[rr] = rc(rr,col);
        return ret;
    }

    Mat<T,1,ncols>
    rowVec(uint row) const
    {
        Mat<T,1,ncols>    ret;
        FGASSERT_FAST(row < nrows);
        for (uint cc=0; cc<ncols; ++cc)
            ret[cc] = rc(row,cc);
        return ret;
    }

    static
    Mat
    identity()
    {
        static_assert(nrows == ncols,"Identity matrix must be square");
        Mat               ret(T(0));
        for (uint ii=0; ii<nrows; ++ii)
            ret.rc(ii,ii) = T(1);
        return ret;
    }

    // Initialize from array data. This is done via a proxy type (accessed via a convenient
    // static member) since compilers interpret '0' as either 'int' or pointer, potentially
    // resulting in either:
    // 1. Ambiguity with single-value constructor (compile-time error)
    // 2. Accidental interpretation of '0' value as a pointer (run-time error)
    struct  FromPtr
    {
        FromPtr(const T * p) : _p(p) {}
        const T * _p;
    };
    explicit
    Mat(FromPtr p)
    {
        for (uint ii=0; ii<nrows*ncols; ++ii)
            m[ii] = *(p._p++);
    }
    static
    Mat
    fromPtr(const T * p)
    {return Mat(FromPtr(p)); }

    T
    cmpntsSum() const
    {
        T   acc = m[0];
        for (uint ii=1; ii<ncols*nrows; ++ii)
            acc += m[ii];
        return acc;
    }

    T
    cmpntsProduct() const
    {
        T   acc = m[0];
        for (uint ii=1; ii<ncols*nrows; ++ii)
            acc *= m[ii];
        return acc;
    }

    bool
    operator<(const Mat & rhs) const      // Useful for putting in a std::map
    {
        for (uint ii=0; ii<nrows*ncols; ++ii) {
            if (m[ii] < rhs[ii])
                return true;
            if (rhs[ii] < m[ii])
                return false;
        }
        return false;
    }

    // Preserves row major ordering:
    Svec<T>
    asStdVector() const
    {
        Svec<T>   ret;
        ret.reserve(size());
        for (size_t ii=0; ii<nrows*ncols; ++ii)
            ret.push_back(m[ii]);
        return ret;
    }

    // Static creation functions:

    static Mat randUniform(T lo,T hi);
    static Mat randNormal(T stdev=T(1));
};

template<class T,uint nrows,uint ncols>
struct  Traits<Mat<T,nrows,ncols> >
{
    typedef typename Traits<T>::Scalar                                Scalar;
    typedef Mat<typename Traits<T>::Accumulator,nrows,ncols>    Accumulator;
    typedef Mat<typename Traits<T>::Floating,nrows,ncols>       Floating;
};

typedef Mat<float,2,2>          Mat22F;
typedef Mat<double,2,2>         Mat22D;
typedef Mat<int,2,2>            Mat22I;
typedef Mat<uint,2,2>           Mat22UI;
typedef Mat<float,3,3>          Mat33F;
typedef Mat<double,3,3>         Mat33D;
typedef Mat<float,4,4>          Mat44F;
typedef Mat<double,4,4>         Mat44D;

typedef Mat<float,2,3>          Mat23F;
typedef Mat<double,2,3>         Mat23D;
typedef Mat<float,3,2>          Mat32F;
typedef Mat<double,3,2>         Mat32D;
typedef Mat<int,3,2>            Mat32I;
typedef Mat<uint,3,2>           Mat32UI;
typedef Mat<double,3,4>         Mat34D;

typedef Mat<float,2,1>          Vec2F;
typedef Mat<double,2,1>         Vec2D;
typedef Mat<short,2,1>          Vec2S;
typedef Mat<int,2,1>            Vec2I;
typedef Mat<uint,2,1>           Vec2UI;
typedef Mat<bool,2,1>           Vec2B;
typedef Mat<float,3,1>          Vec3F;
typedef Mat<double,3,1>         Vec3D;
typedef Mat<short,3,1>          Vec3S;
typedef Mat<int,3,1>            Vec3I;
typedef Mat<uint,3,1>           Vec3UI;
typedef Mat<int16,3,1>          Vec3I16;
typedef Mat<schar,3,1>          Vec3SC;
typedef Mat<uchar,3,1>          Vec3UC;
typedef Mat<float,4,1>          Vec4F;
typedef Mat<double,4,1>         Vec4D;
typedef Mat<int,4,1>            Vec4I;
typedef Mat<uint,4,1>           Vec4UI;
typedef Mat<uchar,4,1>          Vec4UC;
typedef Mat<double,5,1>         Vec5D;

typedef Mat<float,1,2>          VecF2;
typedef Mat<float,1,3>          VecF3;
typedef Mat<double,1,2>         VecD2;

typedef Svec<Vec2UI>            Vec2UIs;
typedef Svec<Vec2F>             Vec2Fs;
typedef Svec<Vec2D>             Vec2Ds;
typedef Svec<VecD2>             VecD2s;
typedef Svec<Vec3F>             Vec3Fs;
typedef Svec<Vec3UI>            Vec3UIs;
typedef Svec<Vec3D>             Vec3Ds;
typedef Svec<Vec4UI>            Vec4UIs;

typedef Svec<Vec3Fs>            Vec3Fss;
typedef Svec<Vec3Ds>            Vec3Dss;
typedef Svec<Vec4UIs>           Vec4UIss;

template<typename To,typename From,uint nrows,uint ncols>
inline
Mat<To,nrows,ncols>
scast(Mat<From,nrows,ncols> const & mat)
{return Mat<To,nrows,ncols>(scast<To,From,nrows*ncols>(mat.m)); }

template<typename To,typename From,uint nrows,uint ncols>
inline
Svec<Mat<To,nrows,ncols> >
scast(Svec<Mat<From,nrows,ncols> > const & vm)
{
    Svec<Mat<To,nrows,ncols> >      ret;
    ret.reserve(vm.size());
    for (auto it=vm.cbegin(); it!=vm.cend(); ++it)
        ret.push_back(scast<To>(*it));
    return ret;
}

template <class T,uint nrows,uint ncols>
std::ostream &
operator<<(std::ostream& ss,const Mat<T,nrows,ncols> & mm)
{
    FGASSERT(mm.numRows()*mm.numCols()>0);
    bool        isVec((mm.numRows() == 1) || mm.numCols() == 1);
    std::ios::fmtflags
        oldFlag = ss.setf(
            std::ios::fixed |
            std::ios::showpos |
            std::ios::right);
    std::streamsize oldPrec = ss.precision(6);
    if (isVec) {
        ss << "[" << mm[0];
        for (uint ii=1; ii<mm.numElems(); ii++)
            ss << "," << mm[ii];
        ss << "]";
        if (mm.numRows() > 1) ss << "^T";       // Indicate transpose of column vector
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

template<class T,uint nrows,uint ncols>
void
fgReadp(std::istream & is,Mat<T,nrows,ncols> & m)
{
    for (uint ii=0; ii<nrows*ncols; ++ii)
        fgReadp(is,m[ii]);
}

template<class T,uint nrows,uint ncols>
void
fgWritep(std::ostream & os,const Mat<T,nrows,ncols> & m)
{
    for (uint ii=0; ii<nrows*ncols; ++ii)
        fgWritep(os,m[ii]);
}

// function 'constructors':

template<typename T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
cMat(T * const ptr)
{
    Mat<T,nrows,ncols>    ret;
    for (size_t ii=0; ii<nrows*ncols; ++ii)
        ret.m[ii] = *ptr++;
    return ret;
}

template<typename T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
cMat(const Svec<T> & v)
{
    Mat<T,nrows,ncols>    ret;
    FGASSERT(v.size() == nrows*ncols);
    for (size_t ii=0; ii<v.size(); ++ii)
        ret.m[ii] = v[ii];
    return ret;
}

template<class T,uint nrows,uint ncols>
Mat<int,nrows,ncols>
fgToInt(const Mat<T,nrows,ncols> & m)
{return Mat<int,nrows,ncols>(m); }

}

#endif
