//
// Copyright (c) 2017 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Sept 20, 2017
//
// Heap-based variable-dimension array (aka variable-rank "tensor").
//
// * A rank 0 tensor should act as a scalar.
// * To avoid confusion we keep this explicit by using operator[] with an empty coord vec. to access a scalar.

#ifndef FGTENSORV_HPP
#define FGTENSORV_HPP

#include "FgStdLibs.hpp"
#include "FgStdVector.hpp"
#include "FgDefaultVal.hpp"

template <class T>
struct  FgTensorV
{
    vector<T>           data;
    FgSizes             dims;           // From minor to major (in terms of 'data' layout)

    FgTensorV() {}                                  // Uninitialized is not a valid tensor value
    explicit FgTensorV(const FgSizes & d) : dims(d)
    {data.resize(fgProduct(dims),0); }              // 'fgProduct' returns 1 for zero-size 'dims'
    explicit FgTensorV(T v) : data(1,v) {}          // Scalar
    explicit FgTensorV(const vector<T> & data_,const FgSizes & dims_) : data(data_), dims(dims_)
    {FGASSERT(data.size() == fgProduct(dims)); }

    T
    scalar() const                          // simpler access to 0-dimensional case
    {FGASSERT(dims.empty() && !data.empty()); return data[0]; }

    T &
    operator[](const FgSizes & coord)       // Similar to matrix classes, coordinate elements from minor to major
    {
        FGASSERT(!data.empty());
        FGASSERT(coord.size() == dims.size());
        size_t          idx = 0,
                        fac = 1;
        for (size_t ii=0; ii<coord.size(); ++ii) {
            FGASSERT(coord[ii] < dims[ii]);
            idx += fac * coord[ii];
            fac *= dims[ii];
        }
        return data[idx];
    }

    const T &
    operator[](const FgSizes & coord) const
    {
        FGASSERT(!data.empty());
        FGASSERT(coord.size() == dims.size());
        size_t          idx = 0,
                        fac = 1;
        for (size_t ii=0; ii<coord.size(); ++ii) {
            FGASSERT(coord[ii] < dims[ii]);
            idx += fac * coord[ii];
            fac *= dims[ii];
        }
        return data[idx];
    }
};

typedef FgTensorV<double>   FgTensorD;
typedef FgTensorV<float>    FgTensorF;

// Tensor iterator:
struct  FgTitr
{
    FgSizes             bounds;
    FgSizes             coord;
    bool                valid = true;           // Not redundant due to wraparound
    const bool          minorToMajor = true;    // 'bounds' and 'coord' are minor to major. Vice versa if false.

    explicit FgTitr(const FgSizes & b) : bounds(b), coord(b.size(),0) {FGASSERT(fgProduct(b) > 0); }
    FgTitr(const FgSizes & b,bool mtm) : bounds(b), coord(b.size(),0), minorToMajor(mtm) {FGASSERT(fgProduct(b) > 0); }

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

    const FgSizes &
    operator()() const
    {return coord; }

    size_t
    rawIdx() const
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

#endif
