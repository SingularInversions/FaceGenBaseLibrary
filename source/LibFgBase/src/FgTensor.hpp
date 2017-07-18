//
// Copyright (c) 2017 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 14, 2017
//
// Heap-based variable-rank tensor
//

#ifndef FGTENSOR_HPP
#define FGTENSOR_HPP

#include "FgStdLibs.hpp"
#include "FgStdVector.hpp"
#include "FgTypes.hpp"
#include "FgDiagnostics.hpp"
#include "FgMatrix.hpp"
#include "FgIter.hpp"

template <class T,uint rank>
struct  FgTensor
{
    FgMatrixC<uint,rank,1>  m_dims;     // From minor to major
    vector<T>               m_data;     // Always of size fgProduct(m_dims)

    FG_SERIALIZE2(m_dims,m_data);

    typedef FgMatrixC<uint,rank,1>   Crd;

    FgTensor() {}

    FgTensor(size_t dim0,size_t dim1,size_t dim2,const vector<T> & data)
    : m_dims(uint(dim0),uint(dim1),uint(dim2)), m_data(data)
    {
        FG_STATIC_ASSERT(rank == 3);
        FGASSERT(dim0*dim1*dim2 == m_data.size());
    }

    FgTensor(size_t dim0,size_t dim1,size_t dim2,size_t dim3,const vector<T> & data)
    : m_dims(uint(dim0),uint(dim1),uint(dim2),uint(dim3)), m_data(data)
    {
        FG_STATIC_ASSERT(rank == 4);
        FGASSERT(dim0*dim1*dim2*dim3 == m_data.size());
    }

    size_t
    crdToIdx(const Crd & crd) const
    {
        size_t      idx = 0,
                    fac = 1;
        for (uint dd=0; dd<rank; ++dd) {
            FGASSERT(crd[dd] < m_dims[dd]);
            idx += fac * crd[dd];
            fac *= m_dims[dd];
        }
        return idx;
    }

    const T &
    operator[](const Crd & crd) const          // Index from minor to major
    {return m_data[crdToIdx(crd)]; }

    T &
    operator[](const Crd & crd)                // Index from minor to major
    {return m_data[crdToIdx(crd)]; }

    FgTensor
    reorder(const Crd & perm) const             // Must represent a valid permutation
    {
        FGASSERT(fgIsValidPermutation(perm));
        FgTensor        ret;
        ret.m_dims = fgPermute(m_dims,perm);
        ret.m_data.resize(m_data.size());
        // Loop in source tensor order so we can use permuation as forward transform:
        for (FgIter<uint,rank> it(m_dims); it.valid(); it.next())
            ret[fgPermute(it(),perm)] = (*this)[it()];
        return ret;
    }

    FgTensor
    transpose(uint d0,uint d1) const
    {
        FGASSERT((d0<rank) && (d1<rank));
        FgTensor        ret;
        ret.m_dims = m_dims;
        ret.m_data.resize(m_data.size());
        std::swap(ret.m_dims[d0],ret.m_dims[d1]);
        for (FgIter<uint,rank> it(ret.m_dims); it.valid(); it.next()) {
            Crd         itTr = it();
            std::swap(itTr[d0],itTr[d1]);
            ret[it()] = (*this)[itTr];
        }
        return ret;
    }

    bool
    operator==(const FgTensor & rhs) const
    {return ((m_dims == rhs.m_dims) && (m_data == rhs.m_data)); }
};

typedef FgTensor<int,3>         FgTensor3I;
typedef FgTensor<int,4>         FgTensor4I;
typedef FgTensor<float,3>       FgTensor3F;
typedef FgTensor<float,4>       FgTensor4F;
typedef FgTensor<double,3>      FgTensor3D;
typedef FgTensor<double,4>      FgTensor4D;

#endif
