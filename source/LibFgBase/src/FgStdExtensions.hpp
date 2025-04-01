//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGSTDEXTENSIONS_HPP
#define FGSTDEXTENSIONS_HPP

#include "FgDiagnostics.hpp"

namespace Fg {

// Multidimensional static size (stack-based) arrays.
// Use a single Arr<> rather than nested Arr<>'s to ensure no padding and keep the access
// interface to a single function (easier to read).
template <typename T,size_t Z,size_t Y,size_t X>
struct      D3Arr
{
    Arr<T,Z*Y*X>            data;

    D3Arr() {}
    explicit D3Arr(Arr<T,Z*Y*X> const & d) : data(d) {}

    Arr3UI                  dims() const {return {Z,Y,X}; }
    size_t constexpr        numElems() const {return Z*Y*X; }
    T const &               at(size_t z,size_t y,size_t x) const
    {
        FGASSERT((z<Z) && (y<Y) && (x<X));
        return data[z*Y*X + y*X + x];
    }
    T &                     at(size_t z,size_t y,size_t x)
    {
        FGASSERT((z<Z) && (y<Y) && (x<X));
        return data[z*Y*X + y*X + x];
    }
    T const &               operator[](Arr3UI c) const {return data[c[0]+c[1]*X+c[2]*X*Y]; }
    T &                     operator[](Arr3UI c) {return data[c[0]+c[1]*X+c[2]*X*Y]; }
};

template <typename T,size_t A,size_t Z,size_t Y,size_t X>
struct      D4Arr
{
    Arr<T,A*Z*Y*X>          data;

    D4Arr() {}
    explicit D4Arr(Arr<T,A*Z*Y*X> const & d) : data(d) {}

    Arr4UI                  dims() const {return {A,Z,Y,X}; }
    size_t constexpr        numElems() const {return A*Z*Y*X; }
    T const &               at(size_t a,size_t z,size_t y,size_t x) const
    {
        FGASSERT((a<A) && (z<Z) && (y<Y) && (x<X));
        return data[a*Z*Y*X + z*Y*X + y*X + x];
    }
    T &                     at(size_t a,size_t z,size_t y,size_t x)
    {
        FGASSERT((a<A) && (z<Z) && (y<Y) && (x<X));
        return data[a*Z*Y*X + z*Y*X + y*X + x];
    }
};

template <typename T,size_t B,size_t A,size_t Z,size_t Y,size_t X>
struct      D5Arr
{
    Arr<T,B*A*Z*Y*X>        data;

    D5Arr() {}
    explicit D5Arr(Arr<T,B*A*Z*Y*X> const & d) : data(d) {}

    Arr5UI                  dims() const {return {B,A,Z,Y,X}; }
    size_t constexpr        numElems() const {return B*A*Z*Y*X; }
    T const &               at(size_t b,size_t a,size_t z,size_t y,size_t x) const
    {
        FGASSERT((b<B) && (a<A) && (z<Z) && (y<Y) && (x<X));
        return data[b*A*Z*Y*X + a*Z*Y*X + z*Y*X + y*X + x];
    }
    T &                     at(size_t b,size_t a,size_t z,size_t y,size_t x)
    {
        FGASSERT((b<B) && (a<A) && (z<Z) && (y<Y) && (x<X));
        return data[b*A*Z*Y*X + a*Z*Y*X + z*Y*X + y*X + x];
    }
};

// this is not an iterator, it only holds information about an array with stride, not a place in the array:
template<class T>
struct      PArr
{
    T const *       ptr;
    size_t          S,      // size (num elements)
                    R;      // stride (step between elements)

    constexpr PArr() : ptr{nullptr}, S{0}, R{1} {}
    PArr(T const * p,size_t s) : ptr{p}, S{s}, R{1} {}
    PArr(T const * p,size_t s,size_t r) : ptr{p}, S{s}, R{r} {}

    bool                valid() const {return (ptr!=nullptr); }
};

// STD CONTAINERS TO OUTPUT STREAMS:

// we want to see 'uchar'/'schar' values as numbers, not as characters (the default):
inline std::ostream & operator<<(std::ostream & os,uchar uc) {return os << scast<uint>(uc); }
inline std::ostream & operator<<(std::ostream & os,schar uc) {return os << scast<int>(uc); }

template<class T,size_t S>
std::ostream &      operator<<(std::ostream & os,Arr<T,S> const & arr)
{
    os << "[";
    for (T const & e : arr)
        os << e << " ";
    return os << "]";
}
template<class T>
std::ostream &      operator<<(std::ostream & ss,Svec<T> const & vv)
{
    std::ios::fmtflags       oldFlag = ss.setf(
        std::ios::fixed |
        std::ios::showpos |
        std::ios::right);
    std::streamsize          oldPrec = ss.precision(6);
    ss << "[" << Fg::fgpush;
    if (vv.size() > 0)
        ss << vv[0];
    for (size_t ii=1; ii<vv.size(); ii++)
        ss << "," << vv[ii];
    ss << Fg::fgpop << "]";
    ss.flags(oldFlag);
    ss.precision(oldPrec);
    return ss;
}
template<class T>
std::ostream &      operator<<(std::ostream & ss,std::shared_ptr<T> const & p)
{
    if (p)
        return ss << *p;
    else
        return ss << "NULL";
}
template<class T,class U>
std::ostream &      operator<<(std::ostream & ss,Pair<T,U> const & pp)
{
    return ss << "(" << pp.first << "," << pp.second << ")";
}
template<typename T>
std::ostream &      operator<<(std::ostream & os,Opt<T> const & v)
{
    if (v.has_value())
        os << v.value();
    else
        os << "no_value";
    return os;
}

typedef Svec<std::thread>   Threads;

// Simple blocking thread dispatcher - limits running threads to hardware capacity.
struct      ThreadDispatcher
{
    explicit ThreadDispatcher(bool enable=true);    // true: use all available threads. false: no threading
    explicit ThreadDispatcher(size_t maxThreads);   // 0,1: no threading.
    ~ThreadDispatcher();                            // threads terminate if destructed before join()

    void            dispatch(std::function<void()> const & fn);
    void            finish();

private:
    size_t const            numThreads;     // 0 if disabled
    Threads                 threads;
    // thread provides no non-blocking way if testing if it's done so use flags.
    // vector requires copyable which atomic is not so use unique pointers to flags.
    Svec<Uptr<std::atomic<bool>>> dones;      // 1-1 with above
    // if any of these are non-empty, the respective thread threw an exception:
    Strings                 errors;

    void                    worker(Sfun<void()> fn,size_t tt);
    void                    reserve();
};

// Like C++17 std::data() but better named:
template <class _Elem>
static constexpr const _Elem* dataPtr(std::initializer_list<_Elem> _Ilist) noexcept
{return _Ilist.begin(); }

// copy a sequence of objects from one pointer to another and return the updated destination pointer:
template<class T>
T *                 copy_(T const * from,T * to,size_t num)
{
    std::copy(from,from+num,to);
    return to + num;
}

template<class T,size_t S>
Arr<T,S>            asArr(Svec<T> const & v)
{
    FGASSERT(v.size() == S);
    Arr<T,S>            ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = v[ii];
    return ret;
}
template<class T,size_t S>
Svec<T>             asSvec(Arr<T,S> const & arr)
{
    Svec<T>             ret; ret.reserve(S);
    std::copy(arr.begin(),arr.end(),std::back_inserter(ret));
    return ret;
}

// repeat a function call R times, passing an index from 0 to R-1:
template<class C>
void                repeat(size_t R,C const & fn)
{
    for (size_t ii=0; ii<R; ++ii)
        fn(ii);
}
// range-based for loops can't handle parallel arrays so use this with lambdas instead:
template<class A,class B,size_t S,class F>
void                iterate(Arr<A,S> const & a,Arr<B,S> const & b,F const & fn)
{
    for (size_t ii=0; ii<S; ++ii)
        fn(a[ii],b[ii]);
}
template<class A,class B,class C,size_t S,class F>
void                iterate(Arr<A,S> const & a,Arr<B,S> const & b,Arr<C,S> const & c,F const & fn)
{
    for (size_t ii=0; ii<S; ++ii)
        fn(a[ii],b[ii],c[ii]);
}
template<class A,class B,class F>
void                iterate(Svec<A> const & a,Svec<B> const & b,F const & fn)
{
    FGASSERT(a.size() == b.size());
    for (size_t ii=0; ii<a.size(); ++ii)
        fn(a[ii],b[ii]);
}
template<class A,class B,class C,class F>
void                iterate(Svec<A> const & a,Svec<B> const & b,Svec<C> const & c,F const & fn)
{
    size_t              S = a.size();
    FGASSERT((b.size() == S) && (c.size() == S));
    for (size_t ii=0; ii<S; ++ii)
        fn(a[ii],b[ii],c[ii]);
}
template<class A,class B,class C,class D,class F>
void                iterate(Svec<A> const & a,Svec<B> const & b,Svec<C> const & c,Svec<D> const & d,F const & fn)
{
    size_t              S = a.size();
    FGASSERT((b.size() == S) && (c.size() == S) && (d.size() == S));
    for (size_t ii=0; ii<S; ++ii)
        fn(a[ii],b[ii],c[ii],d[ii]);
}
template<class A,class B,class F>
void                iterate_(Svec<A> & a,Svec<B> & b,F const & fn)
{
    FGASSERT(a.size() == b.size());
    for (size_t ii=0; ii<a.size(); ++ii)
        fn(a[ii],b[ii]);
}
template<class A,class B,class C,class F>
void                iterate_(Svec<A> & a,Svec<B> & b,Svec<C> & c,F const & fn)
{
    size_t              S = a.size();
    FGASSERT((b.size() == S) && (c.size() == S));
    for (size_t ii=0; ii<S; ++ii)
        fn(a[ii],b[ii],c[ii]);
}

// Generate elements using a callable type that accepts a 'size_t' argument and returns a T:
template<class T,size_t S,class C>
Arr<T,S>            genArr(C const & fn)
{
    Arr<T,S>            ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = fn(ii);
    return ret;
}
template<class C>
auto                genSvec(size_t S,C const & fn)        // 'fn' must accept a 'size_t' argument
{
    typedef decltype(fn(0))   R;
    Svec<R>             ret; ret.reserve(S);
    for (size_t ii=0; ii<S; ++ii)
        ret.push_back(fn(ii));
    return ret;
}
template<class C>
auto                genSvecMT(size_t S,C const & fn)
{
    typedef decltype(fn(0))   R;
    Svec<R>             ret(S);
    ThreadDispatcher    td;
    for (size_t ii=0; ii<S; ++ii) {
        auto            tf = [&,ii](){ret[ii] = fn(ii); };
        td.dispatch(tf);
    }
    td.finish();
    return ret;
}
// generate sequential integers starting at zero. Requires explicit template:
template<class T>
Svec<T>             genIntegers(size_t num) {return genSvec(num,[](size_t i){return scast<T>(i); }); }

// Map callable over Arr elements.
// The callable is passed by value for the same reasons as in the standard library,
// and large objects can be explicitly referenced using std::ref:
template<class T,size_t S,class F>
auto                mapCall(Arr<T,S> const & a,F f)
{
    typedef decltype(f(a[0]))   R;
    Arr<R,S>                    ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = f(a[ii]);
    return ret;
}
template<class T,class U,size_t S,class F>
auto                mapCall(Arr<T,S> const & l,Arr<U,S> const & r,F f)
{
    typedef decltype(f(l[0],r[0]))  R;
    Arr<R,S>                        ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = f(l[ii],r[ii]);
    return ret;
}
template<class T,class U,class V,size_t S,class F>
auto                mapCall(Arr<T,S> const & l,Arr<U,S> const & c,Arr<V,S> const & r,F f)
{
    typedef decltype(f(l[0],c[0],r[0])) R;
    Arr<R,S>                        ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = f(l[ii],c[ii],r[ii]);
    return ret;
}

// Unary map callable over std::vector elements (functional version of std::transform):
template<class T,class C>
auto                mapCall(Svec<T> const & v,C f)
{
    typedef decltype(f(v[0]))   R;
    Svec<R>                     ret; ret.reserve(v.size());
    // std::transform is smart enough to avoid output (index < size) checks at each iteration:
    std::transform(v.cbegin(),v.cend(),std::back_inserter(ret),f);
    return ret;
}
// Map callable binary operation to same type
template<class T,class U,class C>
auto                mapCall(Svec<T> const & l,Svec<U> const & r,C f)
{
    typedef decltype(f(l[0],r[0]))  R;
    FGASSERT(l.size() == r.size());
    Svec<R>                         ret; ret.reserve(l.size());
    for (size_t ii=0; ii<l.size(); ++ii)
        ret.push_back(f(l[ii],r[ii]));
    return ret;
}
// Map callable ternary operation to same type
template<class T,class U,class V,class C>
auto                mapCall(Svec<T> const & t,Svec<U> const & u,Svec<V> const & v,C c)
{
    typedef decltype(c(t[0],u[0],v[0])) R;
    size_t              S = t.size();
    FGASSERT(u.size() == S);
    FGASSERT(v.size() == S);
    Svec<R>             ret; ret.reserve(S);
    for (size_t ii=0; ii<S; ++ii)
        ret.push_back(c(t[ii],u[ii],v[ii]));
    return ret;
}
// Multithreaded versions require T to have default intializer:
template<class T,class C>
auto                mapCallMT(Svec<T> const & v,C f)
{
    typedef decltype(f(v[0]))   R;
    Svec<R>                     ret (v.size());
    ThreadDispatcher            td;
    for (size_t ii=0; ii<v.size(); ++ii)
        td.dispatch([&,ii](){ret[ii]=f(v[ii]);});
    return ret;
}
template<class T,class U,class C>
auto                mapCallMT(Svec<T> const & l,Svec<U> const & r,C f)
{
    typedef decltype(f(l[0],r[0]))  R;
    FGASSERT(l.size() == r.size());
    Svec<R>                         ret (l.size());
    ThreadDispatcher                td;
    for (size_t ii=0; ii<l.size(); ++ii)
        td.dispatch([&,ii](){ret[ii]=f(l[ii],r[ii]);});
    return ret;
}

template<class T,class U,class V,class W,class C>
Svec<T>             mapCallT_(Svec<U> const & in0,Svec<V> & in1,Svec<W> & in2,C fn)
{
    size_t              S = in0.size();
    FGASSERT(in1.size() == S);
    FGASSERT(in2.size() == S);
    Svec<T>             ret; ret.reserve(S);
    for (size_t ii=0; ii<S; ++ii)
        ret.push_back(fn(in0[ii],in1[ii],in2[ii]));
    return ret;
}

// Alias for min/max to allow overloading and avoid collisions (eg. when windows.h is included with its min/max macros):
template<class T,
    FG_ENABLE_IF(T,is_arithmetic)>     // avoid surprising results if T has an operator< overload (esp. with MatC)
inline T            cMin(T x1,T x2) {return std::min(x1,x2); }
template<class T>
inline T            cMin(T x1,T x2,T x3) {return cMin(cMin(x1,x2),x3); }
template<class T,FG_ENABLE_IF(T,is_arithmetic)>
inline T            cMax(T x1,T x2) {return std::max(x1,x2); }
template<class T> 
inline T            cMax(T x1,T x2,T x3) {return cMax(cMax(x1,x2),x3); }

// be explicit about min/max over elements within a type rather than between function arguments:
template<class T,size_t S>
inline T            cMinElem(Arr<T,S> const & a) {return *std::min_element(a.begin(),a.end()); }
template<class T,size_t S>
inline T            cMaxElem(Arr<T,S> const & a) {return *std::max_element(a.begin(),a.end()); }
template<typename T>
T                   cMinElem(Svec<T> const & v)
{
    auto                it = std::min_element(v.cbegin(),v.cend());
    if (it == v.cend())
        return lims<T>::max();
    return *it;
}
template<class T>
T                   cMaxElem(Svec<T> const & v)
{
    auto                it = std::max_element(v.cbegin(),v.cend());
    if (it == v.cend())
        return lims<T>::min();
    return *it;
}

template<class T,size_t S>
Arr<T,S>            permuteElemsRotate(Arr<T,S> const & a,size_t shift)
{
    FGASSERT(shift < S);
    Arr<T,S>            ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[(ii+shift)%S] = a[ii];
    return ret;
}

// range-based binary loop. Not quite as efficient as an index loop due to iterator requirements.
// returns a tuple for structured binding, but it has to be of pointers since structured bindings are copied.
template<class T,class U>
struct      Range2
{
    size_t              sz;
    T const *           lhs;
    U const *           rhs;
    template<size_t S>
    Range2(Arr<T,S> const & l,Arr<U,S> const & r) : sz{S}, lhs{&l[0]}, rhs{&r[0]} {}
    Range2(Svec<T> const & l,Svec<U> const & r) : sz{l.size()}
    {
        FGASSERT(l.size() == r.size());
        if (l.empty()) {
            lhs = nullptr;
            rhs = nullptr;
        }
        else {
            lhs = &l[0];
            rhs = &r[0];
        }
    }

    struct      It
    {
        T const *           lhs;
        U const *           rhs;
        bool                operator!=(It it) const {return (it.lhs!=lhs); }
        auto                operator*() const {return std::make_tuple(lhs,rhs); }
        void                operator++() {++lhs; ++rhs; }
    };

    It                  begin() const {return {lhs,rhs}; }
    It                  end() const {return {lhs+sz,rhs+sz}; }
};
template<class T,class U> inline typename Range2<T,U>::It begin(Range2<T,U> const & r) {return r.begin(); }
template<class T,class U> inline typename Range2<T,U>::It end(Range2<T,U> const & r) {return r.end(); }

// combine rounding with conversion to integer types. No bounds checking:
template<class T,class F,FG_ENABLE_IF(T,is_signed),FG_ENABLE_IF(T,is_integral),FG_ENABLE_IF(F,is_floating_point)>
inline T            roundT(F v) {return scast<T>(std::round(v)); }
template<class T,class F,FG_ENABLE_IF(T,is_unsigned),FG_ENABLE_IF(T,is_integral),FG_ENABLE_IF(F,is_floating_point)>
inline T            roundT(F v) {return scast<T>(v+F(0.5)); }
template<class T,class F,FG_ENABLE_IF(T,is_floating_point)>
inline T            roundT(F v) {return scast<T>(v); }
template<class T,class F>
void                mapRound_(F from,T & to) {to = roundT<T,F>(from); }

// std::lerp is C++20.
// linear interpolation with a base case for any class that supports operator*(scalar) and operator+(T,T):
template<class T,FG_ENABLE_IF(T,is_floating_point)>
inline T            lerp(T a,T b,T c) {return a*(1-c) + b*c; }          // c==0 ? a, c==1 ? b
template<class T,class U,FG_ENABLE_IF(T,is_class),FG_ENABLE_IF(U,is_floating_point)>
inline T            lerp(T const & a,T const & b,U c) {return a*(1-c) + b*c; }
 
// 'interpolate' is linear for scalars and aggregates but not eg. Quaternions so we do not define a
// templated base case using operator*(scalar) & operator+(T,T), but instead overload 'interpolate'
// for each specific class we need:
template<class T,FG_ENABLE_IF(T,is_floating_point)>
inline T            interpolate(T a,T b,T c) {return lerp(a,b,c); }
// aggregates of floating point are linearly interpolated:
template<class T,size_t S,FG_ENABLE_IF(T,is_floating_point)>
Arr<T,S>            interpolate(Arr<T,S> const & as,Arr<T,S> const & bs,T c)
{
    T                   omc = 1 - c;
    return mapCall(as,bs,[c,omc](T a,T b){return a*omc + b*c; });
}
template<class T,FG_ENABLE_IF(T,is_floating_point)>
Svec<T>             interpolate(Svec<T> const & as,Svec<T> const & bs,T c)
{
    T                   omc = 1 - c;
    return mapCall(as,bs,[c,omc](T a,T b){return a*omc + b*c; });
}
// dispatch type signature with endpoints combined in an array:
template<class T,class U,FG_ENABLE_IF(U,is_floating_point)>
inline T            interpolate(Arr<T,2> const & vals,U coeff) {return interpolate(vals[0],vals[1],coeff); }

template <typename T>
inline constexpr T  sqr(T a) {return (a*a); }
template <typename T>
inline constexpr T  cube(T a) {return (a*a*a); }

// Recursive squared magnitude operator returns same type as underlying scalar ("arithmetic"):
template<class T,FG_ENABLE_IF(T,is_arithmetic)>
inline T            cMag(T v) {return v*v; }
template<class T,size_t S>
auto                cMag(Arr<T,S> const & arr)
{
    typedef decltype(cMag(T{})) R;
    R                   acc {0};
    for (T const & e : arr)
        acc += cMag(e);
    return acc;
}
template<class T>
auto                cMag(PArr<T> const & parr)
{
    typedef decltype(cMag(T{})) R;
    R                   acc {0};
    for (size_t ii=0; ii<parr.S*parr.R; ii+=parr.R)
        acc += cMag(parr.ptr[ii]);
    return acc;
}
template<class T>
auto                cMag(Svec<T> const & vec)
{
    typedef decltype(cMag(T{})) R;
    R                   acc {0};
    for (T const & e : vec)
        acc += cMag(e);
    return acc;
}
template<class T>
inline auto         cLen(T const & a){return std::sqrt(cMag(a)); }

// Recursive squared magnitude operator always accumulates and returns type double:
inline double       cMagD(double v) {return sqr(v); }       // catches smaller types and automatically upgrades them
inline double       cMagD(std::complex<double> v) {return std::norm(v); }
// must forward declare as there are instances of bot cMagD(Arr<Svec<...>>) and cMagD(Svec<Arr<...>>)
template<class T> double  cMagD(Svec<T> const & v);
template<class T,size_t S>
double              cMagD(Arr<T,S> const & arr)
{
    double              acc {0};
    for (T const & e : arr)
        acc += cMagD(e);
    return acc;
}
template<class T>
double              cMagD(Svec<T> const & vec)
{
    double              acc {0};
    for (T const & e : vec)
        acc += cMagD(e);
    return acc;
}
template<class T>
double              cLenD(T const & v) {return std::sqrt(cMagD(v)); }

template<class T,size_t U,size_t V>
Arr<T,U+V>          cat(Arr<T,U> const & a0,Arr<T,V> const & a1)
{
    Arr<T,U+V>              ret;
    for (size_t ii=0; ii<U; ++ii)
        ret[ii] = a0[ii];
    for (size_t ii=0; ii<V; ++ii)
        ret[U+ii] = a1[ii];
    return ret;
}
template<class T,size_t U,size_t V,size_t W>
Arr<T,U+V+W>        cat(Arr<T,U> const & a0,Arr<T,V> const & a1,Arr<T,W> const & a2)
{
    Arr<T,U+V+W>            ret;
    for (size_t ii=0; ii<U; ++ii)
        ret[ii] = a0[ii];
    for (size_t ii=0; ii<V; ++ii)
        ret[U+ii] = a1[ii];
    for (size_t ii=0; ii<W; ++ii)
        ret[U+V+ii] = a2[ii];
    return ret;
}

// shift elements by one with wraparound:
template<class T,size_t S>
void                shiftLeft_(Arr<T,S> & a)
{
    T                   tmp = a[0];
    for (size_t ii=0; ii<S-1; ++ii)
        a[ii] = a[ii+1];
    a[S-1] = tmp;
}

template<class T>
Arr<T,2>            swapElems(Arr<T,2> a) {return {a[1],a[0]}; }

template<class T,size_t S>
inline size_t       countEqual(Arr<T,S> const & arr,T const & val) {return count(arr.cbegin(),arr.cend(),val); }
template<class T>
inline size_t       countEqual(Svec<T> const & v,T const & val) {return count(v.cbegin(),v.cend(),val); }

// forward declarations to handle arr<svec<T>>:
template<class T> Svec<T> operator+(Svec<T> const &  l,Svec<T> const &  r);
template<class T> Svec<T> operator-(Svec<T> const &  l,Svec<T> const &  r);

// unary negation:
template<class T,size_t S>
Arr<T,S>        operator-(Arr<T,S> const & a) {return mapCall(a,[](T const & e){return -e; }); }
// addition and subtraction operators between 2 arrays are by convention element-wise
// note that we can't use std::plus / std::minus since these do not support overloads of operator+/-:
template<class T,size_t S>
Arr<T,S>        operator+(Arr<T,S> const & l,Arr<T,S> const & r) {return mapCall(l,r,[](T const & l,T const & r){return l+r; }); }
template<class T,size_t S>
Arr<T,S>        operator-(Arr<T,S> const & l,Arr<T,S> const & r) {return mapCall(l,r,[](T const & l,T const & r){return l-r; }); }

// addition and subtraction between an array and an element is explicit, not operator-overloaded
template<class T,size_t S>
Arr<T,S>            mapAdd(Arr<T,S> const & l,T r) {return mapCall(l,[r](T v){return v+r; }); }
template<class T,size_t S>
Arr<T,S>            mapSub(Arr<T,S> const & l,T r) {return mapCall(l,[r](T v){return v-r; }); }

// implicit map over array for multiplication / division by a scalar (TODO: generalize for non-scalar array type)
template<class T,size_t S>
Arr<T,S>            operator*(Arr<T,S> const & l,T r) {return mapCall(l,[r](T v){return v*r; }); }
template<class T,size_t S>
Arr<T,S>            operator/(Arr<T,S> const & l,T r) {return mapCall(l,[r](T v){return v/r; }); }

// map type-preserving binary in-place modifying (too simple to make generic):
template<class T,size_t S>
void            operator+=(Arr<T,S> & l,Arr<T,S> const & r) {for (size_t ii=0; ii<S; ++ii) l[ii] += r[ii]; }
template<class T,size_t S>
void            operator-=(Arr<T,S> & l,Arr<T,S> const & r) {for (size_t ii=0; ii<S; ++ii) l[ii] -= r[ii]; }
template<class T,size_t S>
void            operator*=(Arr<T,S> & l,T r) {for (size_t ii=0; ii<S; ++ii) l[ii] *= r; }
template<class T,size_t S>
void            operator/=(Arr<T,S> & l,T r) {for (size_t ii=0; ii<S; ++ii) l[ii] /= r; }

template<class T,size_t S>
Arr<T,S>            mapAbs(Arr<T,S> a) {return mapCall(a,[](T e){return std::abs(e); }); }
template<class T>
Svec<T>             mapExp(Svec<T> const & v) {return mapCall(v,[](T e){return std::exp(e); }); }
template<class T,size_t S>
Arr<T,S>            mapExp(Arr<T,S> a) {return mapCall(a,[](T e){return std::exp(e); }); }
template<class T>
Svec<T>             mapLog(Svec<T> const & v) {return mapCall(v,[](T e){return std::log(e); }); }
template<class T,size_t S>
Arr<T,S>            mapLog(Arr<T,S> a) {return mapCall(a,[](T e){return std::log(e); }); }

template<class T,size_t S>
Arr<T,S>            mapMin(Arr<T,S> const & lhs,T rhs)
{
    return mapCall(lhs,[rhs](T v){return cMin(v,rhs); });
}
template<class T,size_t S>
Arr<T,S>            mapMin(Arr<T,S> const & lhs,Arr<T,S> const & rhs)
{
    return mapCall(lhs,rhs,[](T l,T r){return cMin(l,r); });
}

template<size_t S>
bool                anyTrue(Arr<bool,S> a)
{
    for (bool e : a)
        if (e)
            return true;
    return false;
}
template<size_t S>
bool                allTrue(Arr<bool,S> a)
{
    for (bool e : a)
        if (!e)
            return false;
    return true;
}
template<size_t S>
bool                allFalse(Arr<bool,S> a)
{
    for (bool e : a)
        if (e)
            return false;
    return true;
}

template<class T,size_t S>
bool                allGteZero(Arr<T,S> const & a)      // are all elements >= zero aka non-negative ?
{
    for (T e : a)
        if (e < 0)
            return false;
    return true;
}
template<class T,size_t S>
bool                allGtZero(Arr<T,S> const & a)      // are all elements > zero ?
{
    for (T e : a)
        if (e <= 0)
            return false;
    return true;
}
template<class T>
bool                allGtZero(Svec<T> const & v)
{
    for (T e : v)
        if (e <= 0)
            return false;
    return true;
}
template<size_t S>
Arr<bool,S>         mapOr(Arr<bool,S> a0,Arr<bool,S> a1) {return mapCall(a0,a1,[](bool l,bool r){return l || r; }); }

template<class T,class F,size_t S>
Arr<T,S>           mapCast(Arr<F,S> const & arr) {return mapCall(arr,[](F v){return scast<T>(v); }); }

template<typename T,typename U>
Svec<T>            mapCast(Svec<U> const & v) {return mapCall(v,[](U e){return scast<T>(e); }); }

template<class T,class F,size_t S>
void                mapCast_(Arr<F,S> const & from,Arr<T,S> & to)
{
    for (size_t ii=0; ii<S; ++ii)
        to[ii] = scast<T>(from[ii]);
}

template<class T,class U,size_t S>
Arr<T,S>            mapCtor(Arr<U,S> const & v) {return mapCall(v,[](U const & e){return T{e};}); }
template<class T,class U>
Svec<T>             mapCtor(Svec<U> const & v) {return mapCall(v,[](U const & e){return T{e};}); }
template<class T,class U,class V>
Svec<T>             mapCtor(Svec<U> const & u,Svec<V> const & v)
{
    return mapCall(u,v,[](U const & ue,V const & ve){return T{ue,ve};});
}

template<class T,size_t S>
Arr<T,S>            mapSqr(Arr<T,S> const & arr) {return mapCall(arr,[](T v){return v*v;}); }
template<class T>
Svec<T>             mapSqr(Svec<T> const & v) {return mapCall(v,[](T e){return e*e; }); }
template<class T,size_t S,FG_ENABLE_IF(T,is_floating_point)>
Arr<T,S>            mapSqrt(Arr<T,S> const & a) {return mapCall(a,[](T e){FGASSERT(e>=0); return std::sqrt(e);}); }
template<class T,FG_ENABLE_IF(T,is_floating_point)>
Svec<T>             mapSqrt(Svec<T> const & v) {return mapCall(v,[](T e){FGASSERT(e>=0); return std::sqrt(e); }); }
template<class T,FG_ENABLE_IF(T,is_floating_point)>
Svec<T>             mapInv(Svec<T> const & v) {return mapCall(v,[](T e){FGASSERT(e!=0); return 1/e; }); }
template<class T,FG_ENABLE_IF(T,is_floating_point)>
Svec<T>             mapInvSqrt(Svec<T> const & v) {return mapCall(v,[](T e){FGASSERT(e>0); return std::sqrt(1/e); }); }

template<class T,size_t S>
Arr<T,S>            mapFloor(Arr<T,S> const & a) {return mapCall(a,[](T v){return std::floor(v); }); }
template<class To,class From,size_t S>
void                mapRound_(Arr<From,S> const & from,Arr<To,S> & to)
{
    for (size_t ii=0; ii<S; ++ii)
        mapRound_(from[ii],to[ii]);
}
template<class T,class F,size_t S>
Arr<T,S>            mapRound(Arr<F,S> const & in) {return mapCall(in,[](F v){return roundT<T,F>(v); }); }

template<class T,class I,size_t S>
Arr<T,S>            mapIndex(Arr<I,S> inds,Svec<T> const & vals)
{
    auto                fn = [&vals](I idx)
    {
        FGASSERT(idx < vals.size());
        return vals[idx];
    };
    return mapCall(inds,fn);
}
template<class T,class I>
Svec<T>             mapIndex(Svec<I> const & inds,Svec<T> const & vals)
{
    auto                fn = [&vals](I idx)
    {
        FGASSERT(idx < vals.size());
        return vals[idx];
    };
    return mapCall(inds,fn);
}
template<class T,class I,size_t S>
Svec<Arr<T,S>>      mapMapIndex(Svec<Arr<I,S>> const & tris,Svec<T> const & vertVals)
{
    auto                fn = [&](Arr<I,S> tri){return mapIndex(tri,vertVals); };
    return mapCall(tris,fn);
}

template<class T,class U,class C>
T               fold(Svec<U> const & l,T val,C const & fn_)
{
    for (U const & e : l)
        fn_(e,val);
    return val;
}
template<class T,class U,class V,class C>
T               fold(Svec<U> const & l,Svec<V> const & r,T val,C const & fn_)
{
    FGASSERT(l.size() == r.size());
    for (size_t ii=0; ii<l.size(); ++ii)
        fn_(l[ii],r[ii],val);
    return val;
}

template<class T,size_t S>                      // S must be > 0
T               cSum(Arr<T,S> const & a)
{
    T               acc = a[0];                 // avoids requiring T{0}
    for (size_t ii=1; ii<S; ++ii)               // std::accumulate w/ std::plus can't find Fg::operator+(Arr...)
        acc += a[ii];
    return acc;
}
template<class T,size_t S>                                  // S > 0
T               cProd(Arr<T,S> const & a)
{
    return std::accumulate(a.begin()+1,a.end(),a[0],std::multiplies<T>{});  // avoids requiring T{1}
}

template<class T,size_t S>
size_t              cMinIdx(const Arr<T,S> & v)
{
    size_t      ret = 0;
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] < v[ret])
            ret = ii;
    return ret;
}
template<class T,size_t S>
size_t              cMaxIdx(Arr<T,S> const & v)
{
    size_t      ret = 0;
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] > v[ret])
            ret = ii;
    return ret;
}
template<size_t N,class T,size_t M>
Arr<T,N>            cHead(Arr<T,M> const & v)
{
    static_assert(N<M,"Not a proper subset");
    Arr<T,N>        ret;
    for (size_t ii=0; ii<N; ++ii)
        ret[ii] = v[ii];
    return ret;
}
template<class T,size_t S,class U>
bool                contains(Arr<T,S> const & arr,U const & val)
{
    for (T const & e : arr)
        if (e == val)
            return true;
    return false;
}
template<class T,size_t S,class U,size_t R>
bool                containsAny(Arr<T,S> const & arr,Arr<U,R> const & vals)
{
    for (T const & val : vals)
        if (contains(arr,val))
            return true;
    return false;
}
// if not found, returns the size of the array:
template<class T,size_t S,class U>
size_t              findFirstIdx(Arr<T,S> const & arr,U const & val)
{
    for (size_t ii=0; ii<S; ++ii)
        if (arr[ii] == val)
            return ii;
    return S;
}
template<class T,size_t S>
Arr<T,S>            reverseOrder(Arr<T,S> const & a)
{
    Arr<T,S>            ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[S-ii-1] = a[ii];
    return ret;
}

// It's probably more correct to do the element-wise add/sub operations with 'map*' functions
// but this is convenient and widely used:
template<class T>
Svec<T>             operator-(Svec<T> const &  l,Svec<T> const &  r)
{
    return mapCall(l,r,[](T const & l,T const & r){return l-r; });
}
template<class T>
Svec<T>             operator+(Svec<T> const &  l,Svec<T> const &  r)
{
    return mapCall(l,r,[](T const & l,T const & r){return l+r; });
}
template<class T>
void                operator-=(Svec<T> & l,Svec<T> const & r)
{
    FGASSERT(l.size() == r.size());
    for (size_t ii=0; ii<l.size(); ++ii)
        l[ii] -= r[ii];
}
template<class T>
void                operator+=(Svec<T> & l,Svec<T> const & r)
{
    FGASSERT(l.size() == r.size());
    for (size_t ii=0; ii<l.size(); ++ii)
        l[ii] += r[ii];
}
// can also be achieved by 'mapMul' but this is compatible when used implicitly via multAcc on
// an outer collection:
template<class T,class U>
auto                operator*(Svec<T> const & lhs,U rhs) {return mapCall(lhs,[rhs](T const & l){return l*rhs; }); }
template<class T,class U>
auto                operator*(T lhs,Svec<U> const & rhs) {return mapCall(rhs,[lhs](T const & r){return lhs*r; }); }
template<class T,class U>
Svec<T>             operator/(Svec<T> const & l,U r) {return mapCall(l,[r](T e){return e/r; }); }

template<class T,class U>
void                operator*=(Svec<T> & lhs,U rhs)
{
    for (T & l : lhs)
        l *= rhs;
}
template<class T,class U>
void                operator/=(Svec<T> & lhs,U rhs)
{
    for (T & l : lhs)
        l /= rhs;
}

// Acts just like bool for use with vector but retains a memory address for use with references
// and pointers (unlike std::vector<bool> specialization which is a bit field):
struct FatBool
{
    bool            m;
    FatBool() {}
    FatBool(bool v) : m{v} {}
    operator bool () const {return m; }
};
typedef Svec<FatBool>   FatBools;

template<class T>
void                mapAssign_(Svec<T> & vec,T val) {std::fill(vec.begin(),vec.end(),val); }

template<class T,class U>
void                mapAssign_(Svec<T> const & src,Svec<U> & dst)
{
    FGASSERT(src.size() == dst.size());
    for (size_t ii=0; ii<src.size(); ++ii)
        dst[ii] = src[ii];
}

template<class T,class U>
Svec<T>             mapConstruct(Svec<U> const & us)
{
    Svec<T>         ret;
    ret.reserve(us.size());
    for (U const & u : us)
        ret.emplace_back(u);
    return ret;
}
template<class From,class To>
Svec<To>            mapConvert(const Svec<From> & in)
{
    Svec<To>   ret;
    ret.reserve(in.size());
    for (typename Svec<From>::const_iterator it=in.begin(); it != in.end(); ++it)
        ret.push_back(To(*it));
    return ret;
}

// Returns a vector one smaller in size than 'vec', where each element is the difference between
// the corresponding element in 'vec' and its successor. Cannot be empty:
template<class T>
Svec<T>             differentiate(Svec<T> const & vec)
{
    size_t              S = vec.size();
    FGASSERT(S>0);
    Svec<T>             ret; ret.reserve(S-1);
    for (size_t ss=0; ss<S-1; ++ss)
        ret.push_back(vec[ss+1]-vec[ss]);
    return ret;
}
// Discrete integration returns a vector one larger in size than 'vec', where each element is the integral
// of all elements of 'vec' up to but not including the corresponding index (thus the first value is always zero)
// 'constVal' is added to each element of 'vec' for the integration:
template<class T>
Svec<T>             integrate(Svec<T> const & vec,T constVal=0)
{
    Svec<T>             ret; ret.reserve(vec.size()+1);
    T                   acc {0};
    ret.push_back(acc);
    for (T v : vec) {
        acc += v + constVal;
        ret.push_back(acc);
    }
    return ret;
}

// Structural:

template<class T>
Svec<T>             cSubvec(Svec<T> const & vec,size_t start,size_t size)
{
    FGASSERT(start <= vec.size());      // allow for boundary case empty result but not beyond
    size_t              sz = std::min(size,vec.size()-start);
    return Svec<T>(vec.begin()+start,vec.begin()+start+sz);
}
// Truncate after the first elements of 'vec' up to 'size', if present:
template<class T>
void                cHead_(Svec<T> & vec,size_t size)
{
    size_t              sz = std::min(vec.size(),size);
    vec.resize(sz);
}
// Return the first elements of 'vec' up to 'size', if present:
template<class T>
Svec<T>             cHead(Svec<T> const & vec,size_t size)
{
    size_t              sz = std::min(vec.size(),size);
    return Svec<T>(vec.begin(),vec.begin()+sz);
}
template<class T>
Svec<T>             cRest(Svec<T> const & vec,size_t start=1)
{
    FGASSERT(start <= vec.size());      // Can be size zero
    return Svec<T>(vec.begin()+start,vec.end());
}
template<class T>
Svec<T>             cTail(Svec<T> const & vec,size_t size)
{
    FGASSERT(size <= vec.size());
    return Svec<T>(vec.end()-size,vec.end());
}
template<class T,size_t S>
Arr<T,S+1>          append(Arr<T,S> const & arr,T e)
{
    Arr<T,S+1>          ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = arr[ii];
    ret[S] = e;
    return ret;
}
// append is the functional equivalent of push_back:
template<class T>
Svec<T>             append(Svec<T> const & vec,T const & val)
{
    Svec<T>             ret; ret.reserve(vec.size()+1);
    ret.insert(ret.begin(),vec.cbegin(),vec.cend());
    ret.push_back(val);
    return ret;
}
// Functional equivalent of insert at front:
template<class T>
Svec<T>             prepend(T const & val,Svec<T> const & vec)
{
    Svec<T>             ret; ret.reserve(vec.size()+1);
    ret.push_back(val);
    ret.insert(ret.end(),vec.begin(),vec.end());
    return ret;
}
// concatenate from pointer and size:
template<class T>
void                cat_(Svec<T> & vec,T const * vals,size_t num)
{
    vec.reserve(vec.size()+num);
    for (size_t ii=0; ii<num; ++ii)
        vec.push_back(vals[ii]);
}
// concatenation of 2 or 3 std::vectors (concatenation of a vector of vectors is called 'flatten'):
template<class T>
inline void         cat_(Svec<T> & base,Svec<T> const & app) {base.insert(base.end(),app.begin(),app.end()); }
// see 'flatten' instead of cat(Svec<Svec<T>>)
template<class T>
Svec<T>             cat(Svec<T> const & v0,Svec<T> const & v1)
{
    Svec<T>             ret; ret.reserve(v0.size()+v1.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    return ret;
}
template<class T>
Svec<T>             cat(Svec<T> const & v0,Svec<T> const & v1,Svec<T> const & v2)
{
    Svec<T>             ret; ret.reserve(v0.size()+v1.size()+v2.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    ret.insert(ret.end(),v2.begin(),v2.end());
    return ret;
}
template<class T>
Svec<T>             cat(Svec<T> const & v0,Svec<T> const & v1,Svec<T> const & v2,Svec<T> const & v3)
{
    Svec<T>             ret; ret.reserve(v0.size()+v1.size()+v2.size()+v3.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    ret.insert(ret.end(),v2.begin(),v2.end());
    ret.insert(ret.end(),v3.begin(),v3.end());
    return ret;
}
template<class T>
Svec<T>             catDeref(Svec<Svec<T> const *> const & tsPtrs)
{
    Svec<T>             ret;
    size_t              sz {0};
    for (Svec<T> const * tsPtr : tsPtrs)
        sz += tsPtr->size();
    ret.reserve(sz);
    for (Svec<T> const * tsPtr : tsPtrs)
        cat_(ret,*tsPtr);
    return ret;
}
// improved version of std::vector::erase:
template<class T>
void                removeElem_(size_t idx,Svec<T> & v)
{
    FGASSERT(idx < v.size());
    v.erase(v.begin()+idx);
}
// Functional version of vector::erase for single element:
template<class T,size_t S>
Arr<T,S-1>          removeElem(Arr<T,S> const & a,size_t idx)
{
    FGASSERT(idx < S);
    Arr<T,S-1>          ret;
    for (size_t ii=0; ii<idx; ++ii)
        ret[ii] = a[ii];
    for (size_t ii=idx+1; ii<S; ++ii)
        ret[ii-1] = a[ii];
    return ret;
}
template<class T>
Svec<T>             removeElem(Svec<T> const & v,size_t idx)
{
    FGASSERT(idx < v.size());
    Svec<T>             ret; ret.reserve(v.size()-1);
    auto                src = v.cbegin();
    ret.insert(ret.end(),src,src+idx);
    if (idx+1 < v.size())
        ret.insert(ret.end(),src+idx+1,v.cend());
    return ret;
}
// Not recursive; only flattens outermost aggregate:
template<class T>
Svec<T>             flatten(Svec<Svec<T>> const & v)
{
    Svec<T>             ret;
    size_t              sz = 0;
    for (size_t ii=0; ii<v.size(); ++ii)
        sz += v[ii].size();
    ret.reserve(sz);
    for (size_t ii=0; ii<v.size(); ++ii)
        ret.insert(ret.end(),v[ii].begin(),v[ii].end());
    return ret;       
}
template<class T,size_t R,size_t C>
Arr<T,R*C>          flatten(Arr<Arr<T,C>,R> const & a)
{
    Arr<T,R*C>          ret;
    for (size_t rr=0; rr<R; ++rr)
        for (size_t cc=0; cc<C; ++cc)
            ret[rr*C+cc] = a[rr][cc];
    return ret;
}
// Like std::find except it returns index rather than iterator of first occurance.
// If not found, returns v.size() or throws:
template<class T,class U>
size_t              findFirstIdx(
    Svec<T> const &     vec,
    U const &           val,     // Allow for T::operator==(U)
    bool                throwOnFail=false)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (vec[ii] == val)
            return ii;
    if (throwOnFail)
        FGASSERT_FALSE;
    return vec.size();
}
template<class T,class C>
size_t              findFirstIdxIf(Svec<T> const & vec,C fn)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (fn(vec[ii]))
            return ii;
    return vec.size();
}
template<class T,class C>
size_t              findLastIdxIf(Svec<T> const & vec,C fn)
{
    for (size_t ii=vec.size(); ii>0; --ii)
        if (fn(vec[ii-1]))
            return ii-1;
    return vec.size();
}
// Functional specialization of std::find, throws if not found:
template<class T,class U>
T const &           findFirst(Svec<T> const & vec,U const & rhs)        // Allow for T::operator==(U)
{
    auto            it = std::find(vec.cbegin(),vec.cend(),rhs);
    FGASSERT(it != vec.end());
    return *it;
}
// non-const version:
template<class T,class U>
T &                 findFirst(Svec<T> & vec,U const & rhs)              // Allow for T::operator==(U)
{
    auto            it = std::find(vec.begin(),vec.end(),rhs);
    FGASSERT(it != vec.end());
    return *it;
}
// finds only the first instance of each desired item, throws if any of them cannot be found:
template<class T,class U>
Svec<T>             findEach(Svec<T> const & lookIn,Svec<U> const & lookFor)
{
    Svec<T>             ret; ret.reserve(lookFor.size());
    for (U const & lf : lookFor)
        ret.push_back(findFirst(lookIn,lf));
    return ret;
}
// Functional specialization of std::find_if. Throws if no match found.
// Use 'U' instead of Sfun<bool(T)> to match lambdas and args by ref (like std::find_if):
template<class T,class U>
T const &           findFirstIf(Svec<T> const & vec,U const & fn)
{
    auto                it = std::find_if(vec.begin(),vec.end(),fn);
    FGASSERT(it != vec.end());
    return *it;
}
template<class T,class U>
size_t              findLastIdx(Svec<T> const & vec,const U & val)
{
    for (size_t ii=vec.size(); ii!=0; --ii)
        if (vec[ii-1] == val)
            return ii-1;
    return vec.size();
}
// Returns index of first instance whose given member matches 'val', or vec.size() if none found:
template<class T,class U>
size_t              findFirstIdxByMember(Svec<T> const & vec,U T::*mbr,U const & val)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (vec[ii].*mbr == val)
            return ii;
    return vec.size();
}
// Returns first instance whose given member matches 'val'. Throws if none.
template<class T,class U>
T const &           findFirstByMember(Svec<T> const & v,U T::*mbr,U const & val)
{
    size_t          ii = 0;
    for (; ii<v.size(); ++ii)
        if (v[ii].*mbr == val)
            break;
    if (ii >= v.size())
        fgThrow("findFirstByMember (const) not found");
    return v[ii];
}
// non-const version:
template<class T,class U>
T &                 findFirstByMember(Svec<T> & v,U T::*mbr,U const & val)
{
    size_t          ii = 0;
    for (; ii<v.size(); ++ii)
        if (v[ii].*mbr == val)
            break;
    if (ii >= v.size())
        fgThrow("findFirstByMember not found");
    return v[ii];
}
// functional version of std::copy_if over entire vector (like a filter):
template<class T,class P>
Svec<T>             select(Svec<T> const & vec,P const & pred)         // pred() must take one T argument
{
    Svec<T>             ret;
    for (T const & v : vec)
        if (pred(v))
            ret.push_back(v);
    return ret;
}
// as above but returns the indices rather than the elements:
template<class T,class P>
Sizes               selectInds(Svec<T> const & vec,P const & pred)    // pred() must take one T argument
{
    Sizes               ret;
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (pred(vec[ii]))
            ret.push_back(ii);
    return ret;
}
template<class T,class U>
Svec<T>             selectEqual(Svec<T> const & vec,U const & val)
{
    return select(vec,[&](T const & v){return (v == val); });
}
template<class T,class U>
Sizes               selectEqualInds(Svec<T> const & vec,U const & val)
{
    return selectInds(vec,[&](T const & v){return (v == val); });
}
// Returns all instances whose given member matches 'val':
template<class T,class U>
Svec<T>             selectEqualMember(Svec<T> const & vec,U T::*mbr,U const & val)
{
    return select(vec,[mbr,&val](T const & v){return (v.*mbr == val); });
}
template<class T>
Svec<T>             selectIf(Svec<T> const & in,Bools const & accept)
{
    FGASSERT(in.size() == accept.size());
    size_t              S {0};
    for (bool f : accept)
        if (f)
            ++S;
    Svec<T>             ret; ret.reserve(S);
    for (size_t ii=0; ii<in.size(); ++ii)
        if (accept[ii])
            ret.push_back(in[ii]);
    return ret;
}
template<class T,class U>
bool                contains(Svec<T> const & vec,const U & val)     // Allows for T::operator==(U)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (vec[ii] == val)
            return true;
    return false;
}
template<class T,class U>
bool                containsAny(Svec<T> const & ctr,const Svec<U> & vals)       // Simple and slow: O(ctr * vals)
{
    for (size_t ii=0; ii<vals.size(); ++ii)
        if (contains(ctr,vals[ii]))
            return true;
    return false;
}
// note that if 'vals' contains duplicates then it can be larger than 'ctr' and still return true:
template<class T,size_t R,size_t S>
bool                containsAll(Arr<T,R> const & ctr,Arr<T,S> const & vals)     // Simple and slow: O(ctr * vals)
{
    for (size_t ii=0; ii<vals.size(); ++ii)
        if (!contains(ctr,vals[ii]))
            return false;
    return true;
}
template<class T,class U>
bool                containsAll(Svec<T> const & ctr,const Svec<U> & vals)       // Simple and slow: O(ctr * vals)
{
    for (size_t ii=0; ii<vals.size(); ++ii)
        if (!contains(ctr,vals[ii]))
            return false;
    return true;
}
template<class T,class U>
bool                containsMember(Svec<T> const & vec,U T::*mbr,U const & val)
{
    for (T const & v : vec)
        if (v.*mbr == val)
            return true;
    return false;
}
template<class T>
void                replaceAll_(Svec<T> & v,T a,T b)       // Replace each 'a' with 'b'
{
    for (size_t ii=0; ii<v.size(); ++ii)
        if (v[ii] == a)
            v[ii] = b;
}
template<class T>
Svec<T>             replaceAll(Svec<T> const & vec,T const & a,T const & b) // Replace each 'a' with 'b'
{
    Svec<T>       ret;
    ret.reserve(vec.size());
    for (T const & v : vec) {
        if (v == a)
            ret.push_back(b);
        else
            ret.push_back(v);
    }
    return ret;
}
// Returns at least size 1, with 1 additional for each split element:
template<class T>
Svec<Svec<T>>       splitAtChar(Svec<T> const & str,T ch)
{
    Svec<Svec<T>>    ret;
    Svec<T>                  ss;
    for(size_t ii=0; ii<str.size(); ++ii) {
        if (str[ii] == ch) {
            ret.push_back(ss);
            ss.clear();
        }
        else
            ss.push_back(str[ii]);
    }
    ret.push_back(ss);
    return ret;
}
// Overwrite the subset of 'target' starting at 'startPos' with the values from 'data':
template<class T>
void                inject_(Svec<T> const & data,size_t startPos,Svec<T> & target)
{
    FGASSERT(data.size() + startPos <= target.size());
    copy(data.begin(),data.end(),target.begin()+startPos);
}
// Returns the result of overwriting the subset of 'target' starting at 'startPos' with the values from 'data':
template<class T>
Svec<T>             inject(Svec<T> const & data,size_t startPos,Svec<T> const & target)
{
    FGASSERT(data.size() + startPos <= target.size());
    Svec<T>                 ret = target;
    copy(data.begin(),data.end(),ret.begin()+startPos);
    return ret;
}
template<class T>
bool                beginsWith(Svec<T> const & base,Svec<T> const & pattern)
{
    if (pattern.size() > base.size())
        return false;
    for (size_t ii=0; ii<pattern.size(); ++ii)
        if (pattern[ii] != base[ii])
            return false;
    return true;
}
template<class T>
bool                endsWith(Svec<T> const & base,Svec<T> const & pattern)
{
    if (pattern.size() > base.size())
        return false;
    size_t      offset = base.size() - pattern.size();
    for (size_t ii=0; ii<pattern.size(); ++ii)
        if (pattern[ii] != base[ii+offset])
            return false;
    return true;
}

// Numerical:

// Sum into an existing accumulator:
template<class T>
void                cSum_(Svec<T> const & in,T & out)
{
    for (T const & i : in)
        out += i;
}
// NOTE: The value is accumulated in the templated type. Make a special purpose function
// if the accumulator type must be larger than the templated type:
template<class T>
T                   cSum(Svec<T> const & v)
{
    // written to handle the case where T itself is a container (must support operator+=(C,C) )
    // Thus don't use std::accumulate, handle empty case carefully:
    if (v.empty())
        return T(0);
    T           ret = v[0];
    for (size_t ii=1; ii<v.size(); ++ii)
        ret += v[ii];
    return ret;
}
template<class T,size_t S>
T                   cProduct(Arr<T,S> const & v)
{
    T                   acc = v[0];
    for (size_t ii=1; ii<v.size(); ++ii)
        acc *= v[ii];
    return acc;
}
template<class T>
T                   cProduct(Svec<T> const & v)
{
    T                   acc {1};
    for (T e : v)
        acc *= e;
    return acc;
}
// use this one when you want to specify a different accumulator type:
template<class T,class U,size_t S>
T                   cProductT(Arr<U,S> const & v)
{
    T                   acc = scast<T>(v[0]);
    for (size_t ii=1; ii<v.size(); ++ii)
        acc *= scast<T>(v[ii]);
    return acc;
}

template<class T,size_t S>
T                   cMean(Arr<T,S> const & a)
{
    typedef typename Traits<T>::Scalar   U;
    return cSum(a) / scast<U>(a.size());            // size() always > 0
}
// The value is accumulated in the templated type:
template<class T>
T                   cMean(Svec<T> const & v)
{
    FGASSERT(!v.empty());                           // 'mean' has no meaning in this case
    typedef typename Traits<T>::Scalar    S;
    return cSum(v) / scast<S>(v.size());
}

template<class T>
T                   normalize(T const & vals)
{
    typedef typename Traits<T>::Scalar  S;
    static_assert(std::is_floating_point_v<S>,"normalize requires floating point type");
    S                   len = cLen(vals);
    FGASSERT(len > 0);
    return vals * (scast<S>(1)/len);
}

// be sure that 'v' below remains valid for the lifetime of the returned pointers:
template<class T>
Svec<T const *>     mapAddr(Svec<T> const & v)
{
    // it is critical that the lambda function argument is by reference or the returned pointer will be invalid:
    return mapCall(v,[](T const & e){return &e; });
}

// element-wise mutiplication between 2 arrays is explicit, not operator-overloaded:
template<class T,class U,size_t S>
auto                mapMul(Arr<T,S> const & lh,Arr<U,S> const & rh) {return mapCall(lh,rh,[](T l,U r){return l*r;}); }
template<class T,class U>
auto                mapMul(Svec<T> const & l,Svec<U> const & r) {return mapCall(l,r,[](T l,U r){return l*r;}); }
// single value to array must have a different name to disambiguate all possible cases from above:
template<class T,class U,size_t S>
auto                mapMulR(T lh,Arr<U,S> const & rh) {return mapCall(rh,[lh](U r){return lh*r;}); }
template<class T,class U>
auto                mapMulR(T lh,Svec<U> const & rh) {return mapCall(rh,[lh](U r){return lh*r;}); }
// elements to single value (the implicit operator* only accepts the same type for array elements and RHS):
template<class T,class U,size_t S>
auto                mapMulL(Arr<T,S> const & lh,U rh) {return mapCall(lh,[rh](T l){return l*rh;}); }
template<class T,class U>
auto                mapMulL(Svec<T> const & lh,U rh) {return mapCall(lh,[rh](T l){return l*rh;}); }

// Non-functional version:
template<class T,class U,class Op>
void                mapMul_(Op const & op,Svec<T> const & in,Svec<U> & out)
{
    out.reserve(in.size());
    for (size_t ii=0; ii<out.size(); ++ii)
        out[ii] = op * in[ii];
    for (size_t ii=out.size(); ii<in.size(); ++ii)
        out.push_back(op * in[ii]);
}
// Non-functional in-place version:
template<class T,class Op>
void                mapMul_(Op const & op,Svec<T> & data)
{
    for (size_t ii=0; ii<data.size(); ++ii)
        data[ii] = op * data[ii];
}

// element-wise division between 2 arrays, or a constant and an array, is explicit, not operator-overloaded:
template<class T,size_t S>
Arr<T,S>            mapDiv(T l,Arr<T,S> r) {return mapCall(r,[l](T e){return l/e; }); }
template<class T,size_t S>
Arr<T,S>            mapDiv(Arr<T,S> const & l,Arr<T,S> const & r) {return mapCall(l,r,std::divides<T>{}); }
template<class T>
Svec<T>             mapDiv(Svec<T> const & l,Svec<T> const & r) {return mapCall(l,r,std::divides<T>{}); }
template<class T>
Svec<T>             mapDiv(T val,Svec<T> const & vec) {return mapCall(vec,[val](T e){return val/e; }); }
template<class T>
Svec<T>             mapAbs(Svec<T> const & vec) {return mapCall(vec,[](T e){return std::abs(e); }); }
template<class T>
Svec<T>             mapAdd(Svec<T> const & vec,T val) {return mapCall(vec,[val](T e){return e+val; }); }
template<class T>
Svec<T>             mapSub(T val,Svec<T> const & vec) {return mapCall(vec,[val](T e){return val-e; }); }
template<class T>
Svec<T>             mapSub(Svec<T> const & vec,T val) {return mapCall(vec,[val](T e){return e-val; }); }

// Add a weighted vector of values to an existing vector of values: acc += val * vec
// aka multiply-accumulate loop:
template<class T,class U,class V>
void                mapMulAcc_(Svec<T> const & vec,U val,Svec<V> & acc)
{
    FGASSERT(vec.size() == acc.size());
    for (size_t ii=0; ii<acc.size(); ++ii)
        acc[ii] += vec[ii] * val;
}
template<class T,class U,class V>
Svec<V>             mapMulAcc(Svec<T> const & base,U val,Svec<V> const & vec)
{
    FGASSERT(base.size() == vec.size());
    Svec<T>                 ret = base;
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii] += vec[ii] * val;
    return ret;
}

template<size_t S>
bool                reduceAnd(Arr<bool,S> a)    // aka conjunction
{
    for (bool v : a)
        if (!v) return false;
    return true;
}
template<size_t S>
bool                reduceOr(Arr<bool,S> a)    // aka disjunction
{
    for (bool v : a)
        if (v) return true;
    return false;
}

// map then sum in a single pass:
template<class T,class U,size_t S,class C>
auto                mapSum(Arr<T,S> const & l,Arr<U,S> const & r,C const & fn)
{
    typedef decltype(fn(l[0],r[0])) R;
    R                   acc {0};
    for (size_t ii=0; ii<S; ++ii)
        acc += fn(l[ii],r[ii]);
    return acc;
}
template<class T,class U,class V,size_t S,class C>
auto                mapSum(Arr<T,S> const & a,Arr<U,S> const & b,Arr<V,S> const & c,C const & fn)
{
    typedef decltype(fn(a[0],b[0],c[0])) R;
    R                   acc {0};
    for (size_t ii=0; ii<S; ++ii)
        acc += fn(a[ii],b[ii],c[ii]);
    return acc;
}
template<class T,class C>
auto                mapSum(Svec<T> const & v,C const & fn)
{
    typedef decltype(fn(v[0]))  R;
    R                   acc {0};
    for (T const & e : v)
        acc += fn(e);
    return acc;
}
template<class T,class U,class C>
auto                mapSum(Svec<T> const & l,Svec<U> const & r,C const & fn)
{
    size_t              S = l.size();
    FGASSERT(S>0);
    FGASSERT(r.size() == S);
    auto                acc = fn(l[0],r[0]);
    for (size_t ii=1; ii<S; ++ii)
        acc += fn(l[ii],r[ii]);
    return acc;
}
template<class T,class U,class V,class C>
auto                mapSum(Svec<T> const & l,Svec<U> const & m,Svec<V> const & r,C const & fn)
{
    size_t              S = l.size();
    FGASSERT(S>0);
    FGASSERT(m.size() == S);
    FGASSERT(r.size() == S);
    auto                acc = fn(l[0],m[0],r[0]);
    for (size_t ii=1; ii<S; ++ii)
        acc += fn(l[ii],m[ii],r[ii]);
    return acc;
}

// multiply-accumulate reduction loop (aka dot product / inner product) on outer indices of both args:
template<class T,class U,size_t D>
auto                multAcc(Arr<T,D> const & lhs,Arr<U,D> const & rhs)
{
    auto                ret = lhs[0] * rhs[0];
    for (size_t ii=1; ii<D; ++ii)
        ret += lhs[ii] * rhs[ii];
    return ret;
}
template<class T,class U>
auto                multAcc(PArr<T> l,PArr<U> r)
{
    size_t              S = l.S;
    FGASSERT(S>0);
    FGASSERT(r.S == S);
    auto                acc = l.ptr[0]*r.ptr[0];
    for (size_t ii=1; ii<S; ++ii)
        acc += l.ptr[ii*l.R] * r.ptr[ii*r.R];
    return acc;
}
template<class T,class U>
auto                multAccPtr(T const * a,U const * b,size_t D)
{
    FGASSERT(D>0);
    auto                acc = a[0]*b[0];
    for (size_t ii=1; ii<D; ++ii)
        acc += a[ii] * b[ii];
    return acc;
}
template<class T,class U,class V>
auto                multAccPtr(T const * a,U const * b,V const * c,size_t D)
{
    FGASSERT(D>0);
    auto                acc = a[0]*b[0]*c[0];
    for (size_t ii=1; ii<D; ++ii)
        acc += a[ii]*b[ii]*c[ii];
    return acc;
}
template<class T,class U>
auto                multAcc(Svec<T> const & lhs,Svec<U> const & rhs)
{
    FGASSERT(!lhs.empty());
    FGASSERT(lhs.size() == rhs.size());
    return multAccPtr(&lhs[0],&rhs[0],lhs.size());
}

// matMul is a contraction of the next-to-outer index of the left arg with the outer index
// of the right arg. IE assume matrices are stored row-major.
template<class T,class U,size_t R,size_t S>
auto                matMul(Arr<Arr<T,R>,S> const & lhs,Arr<U,S> const & rhs)
{
    typedef decltype(multAcc(lhs[0],rhs))   V;
    Arr<V,R>            ret;
    for (size_t rr=0; rr<R; ++rr)
        ret[rr] = multAcc(lhs[rr],rhs);
    return ret;
}

// explicit overload to treat vec<vec<>> * vec<> as matrix (by row then by col) * colVec.
// return type give by matrix. matrix sub-vecs must all be of same size.
template<class T,class U>
Svec<T>             matMul(Svec<Svec<T>> const & mat,Svec<U> const & vec)
{
    Svec<T>             ret; ret.reserve(mat.size());
    for (Svec<T> const & row : mat)
        ret.push_back(multAcc(row,vec));
    return ret;
}

template<class T>
size_t              cMinIdx(Svec<T> const & v)
{
    FGASSERT(!v.empty());
    size_t      ret = 0;
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] < v[ret])
            ret = ii;
    return ret;
}
template<class T>
size_t              cMaxIdx(Svec<T> const & v)
{
    FGASSERT(!v.empty());
    size_t      ret = 0;
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] > v[ret])
            ret = ii;
    return ret;
}
// To get a min version, just use greater than for 'lessThan':
template<class T>
size_t              cMaxIdx(Svec<T> const & v,const std::function<bool(T const & lhs,T const & rhs)> & lessThan)
{
    FGASSERT(!v.empty());
    size_t      ret = 0;
    for (size_t ii=1; ii<v.size(); ++ii)
        if (lessThan(v[ret],v[ii]))
            ret = ii;
    return ret;    
}
// Common case where we want to sort the whole vector (2 signatures like std::sort):
template<class T>
inline void         sort_(Svec<T> & v) {std::sort(v.begin(),v.end()); }
template<class T,class C>
inline void         sort_(Svec<T> & v,C cmp) {std::sort(v.begin(),v.end(),cmp); }
// Functional version of sorting whole vector:
template<class T>
Svec<T>             sortAll(Svec<T> v) {sort_(v); return v; }
template<class T,class C>
Svec<T>             sortAll(Svec<T> v,C cmp) {sort_(v,cmp); return v; }
// and for Arr:
template<class T,size_t S>
Arr<T,S>            sortAll(Arr<T,S> a) {std::sort(a.begin(),a.end()); return a; }
// Return array of indices into the given array such that the input elements are sorted,
// ie. A mapping from the SORTED order to the ORIGINAL order.
// NB. Making comparison operator a templated argument would allow use of lambdas but that requires
// two functions (like STL) since the compiler can no longer infer the type of C for the default argument:
template<class T,class C=std::less<T>>
Sizes               sortInds(Svec<T> const & v)
{
    Sizes               ret = genIntegers<size_t>(v.size());
    std::sort(ret.begin(),ret.end(),[&v](size_t l,size_t r){return C{}(v[l],v[r]); });
    return ret;
}
template<class T,class C>
void                insertSorted_(Svec<T> & vec,T const & val,C const & cmp)    // 'vec' must be in sorted order
{
    auto                it = vec.begin();
    while (it != vec.end()) {
        if (cmp(val,*it)) {
            vec.insert(it,val);
            return;
        }
        ++it;
    }
    vec.push_back(val);
}
// default comparison version. Like STL, cannot be combined with above:
template<class T>
inline void         insertSorted_(Svec<T> & vec,T const & val) {insertSorted_(vec,val,std::less<T>{}); }
template<class T>
bool                containsDuplicates(Svec<T> const & mustBeSorted)
{
    for (size_t ii=1; ii<mustBeSorted.size(); ++ii)
        if (mustBeSorted[ii] == mustBeSorted[ii-1])
            return true;
    return false;
}
// Removes duplicates from a sorted vector (functional version of std::unique):
template<class T>
Svec<T>             cUnique(Svec<T> const & sorted)
{
    Svec<T>       ret;
    if (!sorted.empty()) {
        ret.push_back(sorted[0]);
        for (size_t ii=1; ii<sorted.size(); ++ii)
            if (!(sorted[ii] == ret.back()))     // In case type only defines operator==
                ret.push_back(sorted[ii]);
    }
    return ret;
}
// Removes duplicates from an unsorted vector and returns uniques in order of appearance.
// Not designed for large return lists as uniqueness test is linear search:
template<class T>
Svec<T>             cUniqueUnsorted(Svec<T> const & vs)
{
    Svec<T>       ret;
    for (T const & v : vs)
        if (!contains(ret,v))
            ret.push_back(v);
    return ret;
}

template<class T,class U,size_t S>
Arr<T,S>            mapMember(Arr<U,S> const & arr,T U::*m) {return mapCall(arr,[m](U const & e){return e.*m; }); }
template<class T,class U>
Svec<U>             mapMember(Svec<T> const & vs,U T::*m) {return mapCall(vs,[m](T const & e){return e.*m; }); }
// pointers to pointers member slice:
template<class T,class U>
Ptrs<U>             mapPtrsMember(Ptrs<T> const & ps,U T::*m) {return mapCall(ps,[m](T const * p){return &(p->*m); }); }

// Transpose a vector of vectors just like Python 'zip' on lists.
// All sub-vectors must have the same size().
// This function is an involution when both sizes are non-zero.
template<class T>
Svec<Svec<T>>       transpose(Svec<Svec<T>> const & v)
{
    Svec<Svec<T>>       ret;
    if (!v.empty()) {
        size_t          sz = v[0].size();
        for (size_t jj=1; jj<v.size(); ++jj)
            FGASSERT(v[jj].size() == sz);
        ret.resize(sz);
        for (size_t ii=0; ii<sz; ++ii) {
            ret[ii].reserve(v.size());
            for (size_t jj=0; jj<v.size(); ++jj)
                ret[ii].push_back(v[jj][ii]);
        }
    }
    return ret;
}
template<class T,size_t R,size_t C>
Arr<Arr<T,R>,C>     transpose(Arr<Arr<T,C>,R> const & a)
{
    Arr<Arr<T,R>,C>     ret;
    for (size_t cc=0; cc<R; ++cc)
        for (size_t rr=0; rr<C; ++rr)
            ret[rr][cc] = a[cc][rr];
    return ret;
}
// similar to Python zip, when one of the sizes is compile-time fixed:
template<class T>
Svec<Arr<T,2>>      zip(Svec<T> const & l,Svec<T> const & r)
{
    return mapCall(l,r,[](T le,T re){return Arr<T,2>{le,re}; });
}

// Inject all elements of domain 'src' into codomain 'dst' in order at true values of 'where'.
// REQUIRED: where.size() == dst.size()
// REQUIRED: number of 'true' values in where == src.size()
template<class T>
Svec<T>             inject(Svec<T> const & src,Svec<T> const & dst,Bools const & where)
{
    FGASSERT(dst.size() == where.size());
    Svec<T>             ret;
    ret.reserve(dst.size());
    size_t              cnt {0};
    for (size_t ii=0; ii<dst.size(); ++ii) {
        if (where[ii]) {
            FGASSERT(cnt < src.size());
            ret.push_back(src[cnt++]);
        }
        else
            ret.push_back(dst[ii]);
    }
    FGASSERT(cnt == src.size());
    return ret;
}

template<class T,size_t S>
Arr<size_t,S>       cSizes(Arr<T,S> const & vss)
{
    return mapCall(vss,[](T const & v){return v.size(); });
}
template<class T>
Sizes               cSizes(Svec<T> const & vss)
{
    return mapCall(vss,[](T const & vs){return vs.size(); });
}
// overload for vector of pointers to vector:
template<class T>
Sizes               cSizes(Svec<Svec<T> const *> const & vPtrs)
{
    return mapCall(vPtrs,[](Svec<T> const * vPtr){return vPtr->size(); });
}
template<class T>
size_t              sumSizes(Svec<T> const & vss)
{
    return fold<size_t>(vss,0,[](T const & vs,size_t & acc){acc += vs.size(); });
}
template<class T,class U>
size_t              sumSizesMember(Svec<T> const & v,U T::*m)
{
    return fold<size_t>(v,0,[m](T const & e,size_t & acc){acc += (e.*m).size(); });
}

// Set intersection with vector containers; ignores duplicates returns intersection of uniques:
template<class T>
Svec<T>             setwiseIntersect(Svec<T> const & v0,Svec<T> const & v1)
{
    Svec<T>       ret;
    for (T const & v : v0) {
        if (contains(v1,v) && !contains(ret,v))
            ret.push_back(v);
    }
    return ret;
}
template<class T>
Svec<T>             setwiseIntersect(Svec<Svec<T>> const & vs)
{
    Svec<T>       ret;
    if (!vs.empty()) {
        ret = vs[0];
        for (size_t ii=1; ii<vs.size(); ++ii)
            ret = setwiseIntersect(ret,vs[ii]);
    }
    return ret;
}
// Multiset intersection with vector containers:
template<class T>
Svec<T>             multisetIntersect(Svec<T> const & v0,Svec<T> v1)
{
    Svec<T>         ret;
    for (T const & v : v0) {
        auto            it = find(v1.cbegin(),v1.cend(),v);
        if (it != v1.end()) {
            ret.push_back(v);
            v1.erase(it);
        }
    }
    return ret;
}
// Set union on vector containers, retaining lhs order then rhs order:
template<class T>
void                setwiseAdd_(Svec<T> & lhs,Svec<T> const & rhs)
{
    for (T const & r : rhs)
        if (!contains(lhs,r))
            lhs.push_back(r);
}
// Set union on vector containers, retaining lhs order then rhs order:
template<class T>
Svec<T>             setwiseAdd(Svec<T> lhs,Svec<T> const & rhs)
{
    setwiseAdd_(lhs,rhs);
    return lhs;
}
// Set subtraction on vector containers (lhs retains ordering):
template<class T>
Svec<T>             setwiseSubtract(Svec<T> const & lhs,Svec<T> const & rhs)
{
    Svec<T>       ret;
    for (T const & l : lhs) {
        if (!contains(rhs,l))
            ret.push_back(l);
    }
    return ret;
}
template<class T>
Svec<T>             cReverse(Svec<T> v)
{
    std::reverse(v.begin(),v.end());
    return v;
}
template<class T,class U>
Svec<T>             lookupRs(Svec<std::pair<T,U>> const & table,U const & val)
{
    Svec<T>                 ret;
    for (auto const & row : table)
        if (row.second == val)
            ret.push_back(row.first);
    return ret;
}
template<class T,class U>
Sizes               lookupIndsL(Svec<std::pair<T,U>> const & table,T const & val)
{
    Sizes               ret;
    for (size_t ii=0; ii<table.size(); ++ii)
        if (table[ii].first == val)
            ret.push_back(ii);
    return ret;
}
template<class T,class U>
Sizes               lookupIndsR(Svec<std::pair<T,U>> const & table,U const & val)
{
    Sizes               ret;
    for (size_t ii=0; ii<table.size(); ++ii)
        if (table[ii].second == val)
            ret.push_back(ii);
    return ret;
}

// Throws if not found:
template<class T,class U>
U const &           lookupFirstL(Svec<std::pair<T,U>> const & table,T const & val)
{
    auto        it=table.begin();
    for (; it!=table.end(); ++it)
        if (it->first == val)
            return it->second;
    FGASSERT_FALSE;
    return it->second;          // avoid warning
}
template<class T,class U>
T const &           lookupFirstR(Svec<std::pair<T,U>> const & table,U const & val)
{
    auto        it=table.begin();
    for (; it!=table.end(); ++it)
        if (it->second == val)
            return it->first;
    FGASSERT_FALSE;
    return it->first;           // avoid warning
}

// BASIC_STRING:

template<typename T>
bool                contains(std::basic_string<T> const & str,T ch)
{
    return (str.find(ch) != std::basic_string<T>::npos);
}

template<typename T>
bool                containsSubstr(std::basic_string<T> const & str,std::basic_string<T> const & pattern)
{
    return (str.find(pattern) != std::basic_string<T>::npos);
}

template<typename T>
bool                containsSubstr(std::basic_string<T> const & str,T const * pattern_c_str)
{
    return containsSubstr(str,std::basic_string<T>(pattern_c_str));
}

template<typename T>
std::basic_string<T> cHead(std::basic_string<T> const & str,size_t size)
{
    FGASSERT(size <= str.size());
    return std::basic_string<T>(str.begin(),str.begin()+size);
}
template<typename T>
std::basic_string<T> cRest(std::basic_string<T> const & str,size_t pos)
{
    FGASSERT(pos <= str.size());
    return std::basic_string<T>(str.begin()+pos,str.end());
}

template<typename T>
std::basic_string<T> cTail(std::basic_string<T> const & str,size_t size)
{
    FGASSERT(size <= str.size());
    return std::basic_string<T>(str.end()-size,str.end());
}

template<class T>
std::basic_string<T> cSubstr(std::basic_string<T> const & str,size_t start,size_t size)
{
    FGASSERT(start+size <= str.size());
    return  std::basic_string<T>(str.begin()+start,str.begin()+start+size);
}

// Returns at least size 1, with 1 additional for each split element:
template<class T>
Svec<std::basic_string<T> > splitAtChar(std::basic_string<T> const & str,T ch)
{
    Svec<std::basic_string<T> >  ret;
    std::basic_string<T>                ss;
    for(T c : str) {
        if (c == ch) {
            ret.push_back(ss);
            ss.clear();
        }
        else
            ss.push_back(c);
    }
    ret.push_back(ss);
    return ret;
}

template<class T>
bool                beginsWith(std::basic_string<T> const & base,std::basic_string<T> const & pattern)
{
    return (base.rfind(pattern,0) != std::basic_string<T>::npos);
}

template<class T>
bool                beginsWith(std::basic_string<T> const & base,T const * pattern_c_str)
{
    return (base.rfind(pattern_c_str,0) != std::basic_string<T>::npos);
}

template<class T>
bool                endsWith(std::basic_string<T> const & str,std::basic_string<T> const & pattern)
{
    if (pattern.size() > str.size())
        return false;
    return (str.find(pattern,str.size()-pattern.size()) != std::basic_string<T>::npos);
}

template<class T>
bool                endsWith(std::basic_string<T> const & str,T const * pattern_c_str)
{
    return endsWith(str,std::basic_string<T>(pattern_c_str));
}

// STRING

// std::to_string can cause ambiguous call errors and doesn't let you adjust precision:
template<class T>
String              toStr(T const & val)
{
    std::ostringstream   msg;
    msg << val;
    return msg.str();
}

template<>
inline String       toStr(String const & str) {return str; }

// ostringstream defaults to stupidly little precision. Default to full:
template<>
inline String       toStr(float const & val)
{
    std::ostringstream   msg;
    msg << std::setprecision(7) << val;
    return msg.str();
}
template<>
inline String       toStr(double const & val)
{
    std::ostringstream   msg;
    msg << std::setprecision(17) << val;
    return msg.str();
}
template<class T,size_t S>
Strings             toStrs(Arr<T,S> const & v) {return mapCall(v,[](T const & e){return toStr(e); }); }

template<class T>
Strings             toStrs(Svec<T> const & v) {return mapCall(v,[](T const & e){return toStr(e); }); }

template<class T,class U>
Strings             toStrs(T const & t,U const & u) {return {toStr(t),toStr(u)}; }
template<class T,class U,class V>
Strings             toStrs(T const & t,U const & u,V const & v) {return {toStr(t),toStr(u),toStr(v)}; }
template<class T,class U,class V,class W>
Strings             toStrs(T const & t,U const & u,V const & v,W const & w) {return {toStr(t),toStr(u),toStr(v),toStr(w)}; }

template<class T>
std::ostream &      operator<<(std::ostream & os,const std::set<T> & v)
{
    os << "{";
    for (typename std::set<T>::const_iterator it=v.begin(); it != v.end(); ++it)
        os << *it << ",";
    return os << "}";
}

// Useful in functional contexts where 's' is already an expression:
template<class T,class U>
inline std::vector<T> setToSvec(const std::set<T,U> & s) {return std::vector<T>(s.begin(),s.end()); }
template<class T>
inline std::set<T>  svecToSet(const std::vector<T> & v) {return std::set<T>(v.begin(),v.end()); }
template<class T,class Lt>
inline bool         contains(const std::set<T,Lt> & s,T const & v) {return (s.find(v) != s.end()); }
// Returns true if the intersect of s0 and s1 is non-empty. Loop is through s1 so prefer s0 for the larger set.
template<class T>
bool                containsAny(const std::set<T> & s0,const std::set<T> & s1)
{
    for (auto it=s1.begin(); it != s1.end(); ++it)
        if (contains(s0,*it))
            return true;
    return false;
}
// Returns true if s0 contains all elements of s1. Loop is through s1 so prefer s0 for the larger set.
template<class T>
bool                containsAll(const std::set<T> & s0,const std::set<T> & s1)
{
    for (auto it=s1.begin(); it != s1.end(); ++it)
        if (!contains(s0,*it))
            return false;
    return true;
}
template<class T>
void                cUnion_(std::set<T> & s0,const std::set<T> & s1) {s0.insert(s1.begin(),s1.end()); }
template<class T>
void                cUnion_(std::set<T> & s0,const std::vector<T> & s1) {s0.insert(s1.begin(),s1.end()); }
template<class T>
std::set<T>         cUnion(const std::set<T> & s0,const std::set<T> & s1)
{
    // WTF is with set_union ...
    std::set<T>         ret = s0;
    ret.insert(s1.begin(),s1.end());
    return ret;
}
// std::set_intersection is stupidly complex.
// Loop is through s1 so prefer s0 for the larger set.
template<class T>
std::set<T>         cIntersect(const std::set<T> & s0,const std::set<T> & s1)
{
    std::set<T>         ret;
    for (T const & s : s1)
        if (contains(s0,s))
            ret.insert(s);
    return ret;
}

template<class T>
std::set<T>         operator+(std::set<T> lhs,const std::set<T> & rhs)
{
    lhs.insert(rhs.begin(),rhs.end());
    return lhs;
}

// set_difference is ridiculously verbose:
template<class T>
std::set<T>         operator-(const std::set<T> & lhs,const std::set<T> & rhs)
{
    std::set<T>         ret;
    for (T const & l : lhs)
        if (!contains(rhs,l))
            ret.insert(l);
    return ret;
}
// Arithmetic notation is a nice short-hand for union:
template<class T>
void                operator+=(std::set<T> & l,const std::set<T> & r) {l.insert(r.begin(),r.end()); }
template<class T>
void                operator+=(std::set<T> & l,const std::vector<T> & r) {l.insert(r.begin(),r.end()); }
// In case you prefer arithmetic notation for all:
template<class T>
void                operator+=(std::set<T> & l,T const & r) {l.insert(r); }
template<class T>
void                operator-=(std::set<T> & lhs,const std::set<T> & rhs)
{
    for (T const & r : rhs) {
        auto it = lhs.find(r);
        if (it != lhs.end())
            lhs.erase(it);
    }
}
template<class T>
void                operator-=(std::set<T> & lhs,T const & rhs)
{
    auto it = lhs.find(rhs);
    if (it != lhs.end())
        lhs.erase(it);
}
template<class T>
void                operator-=(std::set<T> & lhs,const std::vector<T> & rhs)
{
    for (T const & r : rhs) {
        auto it = lhs.find(r);
        if (it != lhs.end())
            lhs.erase(it);
    }
}

template<class T>
bool                contains(const std::unordered_set<T> & s,T const & v) {return (s.find(v) != s.end()); }

template<class Key,class Val>
std::map<Key,Val>   operator+(std::map<Key,Val> const & lhs,std::map<Key,Val> const & rhs)
{
    std::map<Key,Val>    ret = lhs;
    ret.insert(rhs.begin(),rhs.end());
    return ret;
}

template<class Key,class Val>
void                operator+=(std::map<Key,Val> & lhs,std::map<Key,Val> const & rhs)
{lhs.insert(rhs.begin(),rhs.end()); }

template<class Key,class Val,class Lt>
bool                contains(const std::map<Key,Val,Lt> & map,const Key & key)
{return (map.find(key) != map.end()); }

// Accumulate weights of type W for different values of type T in map<T,W>.
// Useful for frequency maps or weight maps:
template<class T,class W>
void                mapAccWeight_(std::map<T,W> & map,T const & val,W weight=W(1))
{
    auto    it = map.find(val);
    if (it == map.end())
        map[val] = weight;
    else
        it->second += weight;
}

// As above but accumulate all weights in 'rhs' to 'lhs':
template<class T,class W>
void                mapAccWeight_(std::map<T,W> & lhs,const std::map<T,W> & rhs)
{
    for (auto const & it : rhs)
        mapAccWeight_(lhs,it.first,it.second);
}

template<typename T>
struct      Valid       // like std::optional but overloads a value:
{
    T               m_val;      // = numeric_limits<T>::max() if not valid

    Valid() : m_val(lims<T>::max()) {}
    explicit Valid(T const & v) : m_val(v) {}

    void            operator=(T const & v) {m_val = v; }
    bool            valid() const {return (m_val != std::numeric_limits<T>::max()); }
    void            invalidate() {m_val = std::numeric_limits<T>::max(); }
    // Implicit conversion caused inexplicable errors with gcc and explicit conversion
    // is required in many cases anyway:
    T               val() const {FGASSERT(valid()); return m_val; }
    // The constPtr() and ptr() functions below couldn't be overloads of operator&() since
    // this doesn't play nice with standard library containers:
    T const *       constPtr() const {FGASSERT(valid()); return &m_val; }
    // You're on your own if you use this one, NO CHECKING, since it may be used to set the val,
    // so do a manual check using valid() if you need one along with non-const pointer access:
    T *             ptr() {return &m_val; }
};

template<typename T>
std::ostream &      operator<<(std::ostream & os,const Valid<T> & v)
{
    if (v.valid())
        return (os << v.val());
    else
        return (os << "invalid");
}

}

#endif
