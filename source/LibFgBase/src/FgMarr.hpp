//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Multivariate (>2) static-size array
//

#ifndef FGMARR_HPP
#define FGMARR_HPP

#include "FgStdExtensions.hpp"
#include "FgTypes.hpp"
#include "FgMath.hpp"
#include "FgMatrixC.hpp"

namespace Fg {

template <typename T,size_t Z,size_t Y,size_t X>
struct  Marr3
{
    Arr<T,Z*Y*X>        data;

    Marr3() {}

    explicit
    Marr3(Arr<T,Z*Y*X> const & d) : data(d) {}

    Vec3UI
    dims() const
    {return Vec3UI{Z,Y,X}; }

    size_t
    numElems() const
    {return Z*Y*X; }

    T &
    at(size_t z,size_t y,size_t x)
    {
        FGASSERT((z<Z) && (y<Y) && (x<X));
        return data[z*Y*X + y*X + x];
    }
    T const &
    at(size_t z,size_t y,size_t x) const
    {
        FGASSERT((z<Z) && (y<Y) && (x<X));
        return data[z*Y*X + y*X + x];
    }
};

}

#endif
