//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Multidimensional (>2) static-size array
//

#ifndef FGMARR_HPP
#define FGMARR_HPP

#include "FgMath.hpp"
#include "FgMatrixC.hpp"

namespace Fg {

template <typename T,size_t Z,size_t Y,size_t X>
struct      Marr3
{
    Arr<T,Z*Y*X>            data;

    Marr3() {}
    explicit Marr3(Arr<T,Z*Y*X> const & d) : data(d) {}

    Vec3UI                  dims() const {return {Z,Y,X}; }
    size_t                  numElems() const {return Z*Y*X; }
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
};

template <typename T,size_t A,size_t Z,size_t Y,size_t X>
struct      Marr4
{
    Arr<T,A*Z*Y*X>          data;

    Marr4() {}
    explicit Marr4(Arr<T,A*Z*Y*X> const & d) : data(d) {}

    Vec4UI                  dims() const {return {A,Z,Y,X}; }
    size_t                  numElems() const {return A*Z*Y*X; }
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
struct      Marr5
{
    Arr<T,B*A*Z*Y*X>        data;

    Marr5() {}
    explicit Marr5(Arr<T,B*A*Z*Y*X> const & d) : data(d) {}

    Vec5UI                  dims() const {return {B,A,Z,Y,X}; }
    size_t                  numElems() const {return B*A*Z*Y*X; }
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

}

#endif
