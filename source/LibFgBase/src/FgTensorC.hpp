//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Stack-based templated-dimension array (aka templated-rank "tensor").
//

#ifndef FGTENSOR_HPP
#define FGTENSOR_HPP

#include "FgStdLibs.hpp"
#include "FgStdVector.hpp"
#include "FgTypes.hpp"
#include "FgDiagnostics.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"
#include "FgIter.hpp"

namespace Fg {

template <class T,uint rank>
struct  FgTensorC
{
    Mat<uint,rank,1>  m_dims;     // From minor to major
    Svec<T>               m_data;     // Always of size fgProduct(m_dims)

    FG_SERIALIZE2(m_dims,m_data);

    typedef Mat<uint,rank,1>   Crd;

    FgTensorC() {}

    FgTensorC(size_t dim0,size_t dim1,size_t dim2,const Svec<T> & data)
    : m_dims(uint(dim0),uint(dim1),uint(dim2)), m_data(data)
    {
        static_assert(rank == 3,"Number of arguments does not dimensions");
        FGASSERT(dim0*dim1*dim2 == m_data.size());
    }

    FgTensorC(size_t dim0,size_t dim1,size_t dim2,size_t dim3,const Svec<T> & data)
    : m_dims(uint(dim0),uint(dim1),uint(dim2),uint(dim3)), m_data(data)
    {
        static_assert(rank == 4,"Number of arguments does not match elements");
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

    FgTensorC
    reorder(const Crd & perm) const             // Must represent a valid permutation
    {
        FGASSERT(fgIsValidPermutation(perm));
        FgTensorC        ret;
        ret.m_dims = fgPermute(m_dims,perm);
        ret.m_data.resize(m_data.size());
        // Loop in source tensor order so we can use permuation as forward transform:
        for (Iter<uint,rank> it(m_dims); it.valid(); it.next())
            ret[fgPermute(it(),perm)] = (*this)[it()];
        return ret;
    }

    FgTensorC
    transpose(uint d0,uint d1) const
    {
        FGASSERT((d0<rank) && (d1<rank));
        FgTensorC        ret;
        ret.m_dims = m_dims;
        ret.m_data.resize(m_data.size());
        std::swap(ret.m_dims[d0],ret.m_dims[d1]);
        for (Iter<uint,rank> it(ret.m_dims); it.valid(); it.next()) {
            Crd         itTr = it();
            std::swap(itTr[d0],itTr[d1]);
            ret[it()] = (*this)[itTr];
        }
        return ret;
    }

    bool
    operator==(const FgTensorC & rhs) const
    {return ((m_dims == rhs.m_dims) && (m_data == rhs.m_data)); }
};

typedef FgTensorC<int,3>         FgTensor3I;
typedef FgTensorC<int,4>         FgTensor4I;
typedef FgTensorC<float,3>       FgTensor3F;
typedef FgTensorC<float,4>       FgTensor4F;
typedef FgTensorC<double,3>      FgTensor3D;
typedef FgTensorC<double,4>      FgTensor4D;

}

#endif
