//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// * Multi-dimensional index iterator with offset and stride.
// * Multi-dimensional inclusive upper bounds iterator.
//

#ifndef FGITER_HPP
#define FGITER_HPP

#include "FgFile.hpp"
#include "FgBounds.hpp"

namespace Fg {

// TODO: start migrating 2D iterations to this version
struct      Iter2
{
    Vec2Z               m_begin,            // inclusive lower bound
                        m_eub,              // exclusive upper bound. Must be > m_begin.
                        m_idx {0};          // current index value
    size_t              m_ystride {1};      // stride for outer dimension (X stride assumed 1)

    explicit Iter2(size_t eub) : m_begin{0}, m_eub{eub} {FGASSERT(valid()); }   // begin zero, eub all same value
    explicit Iter2(Vec2Z eub) : m_begin{0}, m_eub{eub} {FGASSERT(valid()); }    // begin zero
    Iter2(Vec2Z begin,Vec2Z eub) : m_begin{begin}, m_eub{eub} {FGASSERT(valid()); }
    Iter2(Vec2Z begin,Vec2Z eub,size_t ystride) : m_begin{begin}, m_eub{eub}, m_ystride{ystride}
        {FGASSERT(valid()); }

    bool                valid() const {return ((m_begin[0]<m_eub[0]) && (m_begin[0]<m_eub[1])); }
    // must be in valid() state before this is called. Returns whether it's in valid state afterward:
    bool                next()
    {
        ++m_idx[0];
        if (m_idx[0] >= m_eub[0]) {
            m_idx[0] = 0;
            m_idx[1] += m_ystride;
            if (m_idx[1] >= m_eub[1])
                return false;
            else
                return true;
        }
        else
            return true;
    }
    Vec2Z               operator()() const {return m_idx; }
};

template<typename T,uint dim>
std::ostream &      operator<<(std::ostream & os,Iter2 const & it)
{
    return os << "begin: " << it.m_begin << " eub: " << it.m_eub
        << " idx: " << it.m_idx
        << " stride: " << it.m_ystride;
}

// TODO: replace this with 2D and 3D hard-coded versions:
template<typename T,uint dim>
struct      Iter
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

    bool                next()
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

    bool                valid() const {return m_inBounds; }
    Mat<T,dim,1> const & operator()() const {return m_idx; }
    // Mirror the point around the centre in all axes:
    Mat<T,dim,1>        mirror() const {return (m_bndsHiExcl - m_idx - Mat<T,dim,1>(1)); }
    // Mirror the point around the centre in the given axis:
    Mat<T,dim,1>
    mirror(uint axis) const
    {
        Mat<T,dim,1>      ret = m_idx;
        ret[axis] = m_bndsHiExcl[axis] - m_idx[axis] - 1;
        return ret;
    }
    Mat<T,dim,1>        delta() const {return m_idx - m_bndsLoIncl; }
    Mat<T,dim,2>        inclusiveRange() const {return catHoriz(m_bndsLoIncl,m_bndsHiExcl - Mat<T,dim,1>(1)); }
    Mat<T,dim,1>        dims() const {return (m_bndsHiExcl - m_bndsLoIncl); }
    // Return clipped bounds of all supremum norm (L_inf) distance = 1 neighbours:
    Mat<T,dim,2>        neighbourBounds() const
    {
        return 
            Mat<T,dim,2>(
                intersectBounds(
                    Mat<int,dim,1>(m_idx) * Mat<int,1,2>(1) +
                        Mat<int,dim,2>(-1,1,-1,1,-1,1),
                    Mat<int,dim,2>(inclusiveRange())));
    }

private:
    // Return false if range is empty or negative (invalid):
    bool                validRange()
    {
        for (uint dd=0; dd<dim; dd++)
            if (m_bndsHiExcl[dd] <= m_bndsLoIncl[dd])
                return false;
        return true;
    }
    void                setValid()
    {
        FGASSERT(validRange());
        FGASSERT(m_strides.cmpntsProduct() > 0);
        m_inBounds = true;
        for (uint ii=0; ii<dim; ++ii)
            if (!(m_idx[ii] < m_bndsHiExcl[ii]))
                m_inBounds = false;
    }
};

template<typename T,uint dim>
std::ostream &      operator<<(std::ostream & os,const Iter<T,dim> & it)
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

typedef Iter<uint,3>        Iter3UI;

// Inclusive bounds iterator:
template<typename T,uint dim>
struct      IterIub
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

    bool                next()
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
    bool                valid() const {return (idx[dim-1] <= bndHi[dim-1]); }
    Mat<T,dim,1> const & operator()() const {return idx; }
    // Set iterator to invalid if the range is empty (or negative):
    void                init()
    {
        for (uint dd=0; dd<dim; ++dd)
            if (bndLo[dd] > bndHi[dd])
                idx[dim-1] = bndHi[dim-1] + 1;
    }
};

typedef IterIub<uint,3>   IterIub3UI;

}

#endif
