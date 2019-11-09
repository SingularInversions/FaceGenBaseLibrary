//
// Copyright (c) 2017 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//
// Heap-based variable-dimension array (aka variable-rank "tensor").
//
// * A rank 0 tensor should act as a scalar.
// * To avoid confusion we keep this explicit by using operator[] with an empty coord vec. to access a scalar.

#ifndef FGTENSORV_HPP
#define FGTENSORV_HPP

#include "FgStdLibs.hpp"
#include "FgStdVector.hpp"

namespace Fg {

inline
size_t
fgTensorIdx(const Sizes & dims,const Sizes & coord)
{
    FGASSERT(coord.size() == dims.size());
    size_t          idx = 0,
                    fac = 1;
    for (size_t ii=0; ii<coord.size(); ++ii) {
        FGASSERT(coord[ii] < dims[ii]);
        idx += fac * coord[ii];
        fac *= dims[ii];
    }
    return idx;
}

template <class T>
struct  FgTensorV
{
    Sizes             dims;           // From minor to major (in terms of 'data' layout)
    Svec<T>           data;

    FgTensorV() {}                                  // Uninitialized is not a valid tensor value

    explicit FgTensorV(const Sizes & d) : dims(d)
    {data.resize(fgProduct(dims),0); }              // 'fgProduct' returns 1 for zero-size 'dims'

    explicit FgTensorV(T v) : data(1,v) {}          // Scalar

    FgTensorV(const Sizes & dims_,const Svec<T> & data_) : dims(dims_), data(data_)
    {FGASSERT(data.size() == fgProduct(dims)); }

    T
    scalar() const                          // simpler access to 0-dimensional case
    {FGASSERT(dims.empty() && !data.empty()); return data[0]; }

    T &
    operator[](const Sizes & coord)       // Similar to matrix classes, coordinate elements from minor to major
    {
        size_t      idx = fgTensorIdx(dims,coord);
        FGASSERT(idx < data.size());
        return data[idx];
    }

    const T &
    operator[](const Sizes & coord) const
    {
        size_t      idx = fgTensorIdx(dims,coord);
        FGASSERT(idx < data.size());
        return data[idx];
    }
};

typedef FgTensorV<double>   FgTensorD;
typedef FgTensorV<float>    FgTensorF;

// Tensor iterator:
struct  FgTitr
{
    Sizes             bounds;
    Sizes             coord;
    bool                valid = true;           // Not redundant due to wraparound
    bool                minorToMajor = true;    // 'bounds' and 'coord' are minor to major. Vice versa if false.

    explicit
    FgTitr(const Sizes & b) : bounds(b), coord(b.size(),0)
    {FGASSERT(fgProduct(b) > 0); }

    FgTitr(const Sizes & b,bool minorToMajor_)
        : bounds(b), coord(b.size(),0), minorToMajor(minorToMajor_)
        {FGASSERT(fgProduct(b) > 0); }

    bool
    next()
    {
        FGASSERT(valid);
        for (size_t dd=0; dd<coord.size(); ++dd) {
            size_t      idx = minorToMajor ? dd : coord.size()-dd-1;
            coord[idx] += 1;
            if (coord[idx] == bounds[idx])
                coord[idx] = 0;
            else
                return true;
        }
        valid = false;      // Overflows all dimensions
        return false;
    }

    const Sizes &
    operator()() const
    {return coord; }

    // Give the index into a flat array of the tensor, with the same major to minor ordering:
    size_t
    flatIdx() const
    {
        size_t          idx = 0,
                        fac = 1;
        for (size_t dd=0; dd<coord.size(); ++dd) {
            size_t      didx = minorToMajor ? dd : coord.size()-dd-1;
            idx += fac * coord[didx];
            fac *= bounds[didx];
        }
        return idx;
    }
};

}

#endif
