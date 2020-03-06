//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Multi-dimensional index iterator with offset and stride.
//
// If necessary it might be made faster by hard-coding 2D and 3D versions separately.

#ifndef FGITER_HPP
#define FGITER_HPP

#include "FgStdStream.hpp"
#include "FgBounds.hpp"

namespace Fg {

template<typename T,uint dim>
struct  Iter
{
    static_assert(std::is_integral<T>::value,"Iter only supports integral types");
    Mat<T,dim,1>        m_bndsLoIncl;   // Inclusive lower bounds
    Mat<T,dim,1>        m_bndsHiExcl;   // Exclusive upper bounds
    // Strides (Step sizes) for each dimension. MUST NOT be larger than difference between bounds:
    Mat<T,dim,1>        m_strides;
    Mat<T,dim,1>        m_idx;
    bool                m_inBounds=true;    // Not redundant since m_idx wraps around.

    explicit
    Iter(Mat<T,dim,2> inclLowerExclUpperBounds)
    :   m_bndsLoIncl(inclLowerExclUpperBounds.colVec(0)),
        m_bndsHiExcl(inclLowerExclUpperBounds.colVec(1)),
        m_strides(Mat<T,dim,1>(1)),
        m_idx(inclLowerExclUpperBounds.colVec(0))
    {setValid(); }

    Iter(
        Mat<T,dim,1>      lowerBounds,
        Mat<T,dim,1>      exclusiveUpperBounds)
        :   m_bndsLoIncl(lowerBounds),
            m_bndsHiExcl(exclusiveUpperBounds),
            m_strides(Mat<T,dim,1>(1)),
            m_idx(lowerBounds)
    {setValid(); }

    // Can't have last argument as optional on above since old versions of gcc have a bug
    // with non-type template arguments in default arguments:
    Iter(
        Mat<T,dim,1>      lowerBounds,
        Mat<T,dim,1>      exclusiveUpperBounds,
        Mat<T,dim,1>      strides)
        :   m_bndsLoIncl(lowerBounds),
            m_bndsHiExcl(exclusiveUpperBounds),
            m_strides(strides),
            m_idx(lowerBounds)
    {setValid(); }

    // Lower bounds implicitly zero, strides implicitly 1:
    explicit
    Iter(Mat<T,dim,1> dims)
    :   m_bndsLoIncl(0),
        m_bndsHiExcl(dims),
        m_strides(1),
        m_idx(0)
    {setValid(); }

    // Lower bounds implicitly zero, upper bounds all the same, strides 1:
    explicit
    Iter(T exclusiveUpperBound)
    :   m_bndsLoIncl(0),
        m_bndsHiExcl(exclusiveUpperBound),
        m_strides(1),
        m_idx(0)
    {setValid(); }

    bool
    next()
    {
        FGASSERT(m_inBounds);
        for (uint dd=0; dd<dim; ++dd) {
            m_idx[dd] += m_strides[dd];
            if (m_idx[dd] >= m_bndsHiExcl[dd])
                m_idx[dd] = m_bndsLoIncl[dd] + m_idx[dd] % m_strides[dd];
            else
                return true;
        }
        m_inBounds = false;
        return false;
    }

    bool
    valid() const
    {return m_inBounds; }

    const Mat<T,dim,1> &
    operator()() const
    {return m_idx; }

    // Mirror the point around the centre in all axes:
    Mat<T,dim,1>
    mirror() const
    {return (m_bndsHiExcl - m_idx - Mat<T,dim,1>(1)); }

    // Mirror the point around the centre in the given axis:
    Mat<T,dim,1>
    mirror(uint axis) const
    {
        Mat<T,dim,1>      ret = m_idx;
        ret[axis] = m_bndsHiExcl[axis] - m_idx[axis] - 1;
        return ret;
    }

    Mat<T,dim,1>
    delta() const
    {return m_idx - m_bndsLoIncl; }

    Mat<T,dim,2>
    inclusiveRange() const
    {return catHoriz(m_bndsLoIncl,m_bndsHiExcl - Mat<T,dim,1>(1)); }

    Mat<T,dim,1>
    dims() const
    {
        return (m_bndsHiExcl - m_bndsLoIncl);
    }

    // Return clipped bounds of all supremum norm (L_inf) distance = 1 neighbours:
    Mat<T,dim,2>
    neighbourBounds() const
    {
        return 
            Mat<T,dim,2>(
                cBoundsIntersection(
                    Mat<int,dim,1>(m_idx) * Mat<int,1,2>(1) +
                        Mat<int,dim,2>(-1,1,-1,1,-1,1),
                    Mat<int,dim,2>(inclusiveRange())));
    }

private:
    // Return false if range is empty or negative (invalid):
    bool
    validRange()
    {
        for (uint dd=0; dd<dim; dd++)
            if (m_bndsHiExcl[dd] <= m_bndsLoIncl[dd])
                return false;
        return true;
    }

    void
    setValid()
    {
        FGASSERT(validRange());
        FGASSERT(m_strides.cmpntsProduct() > 0);
        m_inBounds = fgLt(m_idx,m_bndsHiExcl);
    }
};

template<typename T,uint dim>
std::ostream &
operator<<(std::ostream & os,const Iter<T,dim> & it)
{
    return os
        << "lo: " << it.m_bndsLoIncl
        << " hi: " << it.m_bndsHiExcl
        << " stride: " << it.m_strides
        << " idx: " << it.m_idx
        << " valid: " << it.m_inBounds;
}

typedef Iter<int,2>         Iter2I;
typedef Iter<uint,2>        Iter2UI;
typedef Iter<uint64,2>      Iter2UL;

typedef Iter<uint,3>        Iter3UI;
typedef Iter<size_t,3>      Iter3SZ;
typedef Iter<uint64,3>      Iter3UL;

// Inclusive bounds iterator:
template<typename T,uint dim>
struct  IterIub
{
    static_assert(std::is_integral<T>::value,"IterIub only supports integral types");
    Mat<T,dim,1>  bndLo;          // Inclusive
    Mat<T,dim,1>  bndHi;          // Inclusive
    Mat<T,dim,1>  idx;            // Current index. Z out of bounds at end of iteration.

    explicit
    IterIub(Mat<T,dim,2> inclusiveBounds)
    :   bndLo(inclusiveBounds.colVec(0)),
        bndHi(inclusiveBounds.colVec(1)),
        idx(inclusiveBounds.colVec(0))
    {init(); }

    bool
    next()
    {
        uint        dd = 0;
        for (; dd<dim-1; ++dd) {
            if (++idx[dd] > bndHi[dd])      // Inclusive upper bound
                idx[dd] = bndLo[dd];        // Wrap-around and loop to next dimension
            else
                return true;
        }
        return (++idx[dd] <= bndHi[dd]);    // Don't wrap-around the final dimension
    }

    bool
    valid() const
    {return (idx[dim-1] <= bndHi[dim-1]); }

    const Mat<T,dim,1> &
    operator()() const
    {return idx; }

    // Set iterator to invalid if the range is empty (or negative):
    void
    init()
    {
        for (uint dd=0; dd<dim; ++dd)
            if (bndLo[dd] > bndHi[dd])
                idx[dim-1] = bndHi[dim-1] + 1;
    }
};

typedef IterIub<uint,3>   IterIub3UI;

}

#endif
