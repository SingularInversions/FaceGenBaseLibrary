//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 29, 2012
//
// Multi-dimensional index iterator with offset and stride.
//
// If necessary it might be made faster by hard-coding 2D and 3D versions separately.

#ifndef FGITER_HPP
#define FGITER_HPP

#include "FgBounds.hpp"
#include "FgDefaultVal.hpp"

template<typename T,uint dim>
struct  FgIter
{
    FgMatrixC<T,dim,1>  m_bndsLoIncl;   // Inclusive lower bounds
    FgMatrixC<T,dim,1>  m_bndsHiExcl;   // Exclusive upper bounds
    // Strides (Step sizes) for each dimension. MUST NOT be larger than difference between bounds:
    FgMatrixC<T,dim,1>  m_strides;
    FgMatrixC<T,dim,1>  m_idx;
    FgBoolT             m_inBounds;     // Not redundant since m_idx wraps around.

    explicit
    FgIter(FgMatrixC<T,dim,2> inclLowerExclUpperBounds)
    :   m_bndsLoIncl(inclLowerExclUpperBounds.colVec(0)),
        m_bndsHiExcl(inclLowerExclUpperBounds.colVec(1)),
        m_strides(FgMatrixC<T,dim,1>(1)),
        m_idx(inclLowerExclUpperBounds.colVec(0))
    {setValid(); }

    FgIter(
        FgMatrixC<T,dim,1>      lowerBounds,
        FgMatrixC<T,dim,1>      exclusiveUpperBounds)
        :   m_bndsLoIncl(lowerBounds),
            m_bndsHiExcl(exclusiveUpperBounds),
            m_strides(FgMatrixC<T,dim,1>(1)),
            m_idx(lowerBounds)
    {setValid(); }

    // Can't have last argument as optional on above since old versions of gcc have a bug
    // with non-type template arguments in default arguments:
    FgIter(
        FgMatrixC<T,dim,1>      lowerBounds,
        FgMatrixC<T,dim,1>      exclusiveUpperBounds,
        FgMatrixC<T,dim,1>      strides)
        :   m_bndsLoIncl(lowerBounds),
            m_bndsHiExcl(exclusiveUpperBounds),
            m_strides(strides),
            m_idx(lowerBounds)
    {setValid(); }

    // Lower bounds implicitly zero, strides implicitly 1:
    explicit
    FgIter(FgMatrixC<T,dim,1> dims)
    :   m_bndsLoIncl(0),
        m_bndsHiExcl(dims),
        m_strides(1),
        m_idx(0)
    {setValid(); }

    // Lower bounds implicitly zero, upper bounds all the same, strides 1:
    explicit
    FgIter(T exclusiveUpperBound)
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

    const FgMatrixC<T,dim,1> &
    operator()() const
    {return m_idx; }

    // Mirror the point around the centre in all axes:
    FgMatrixC<T,dim,1>
    mirror() const
    {return (m_bndsHiExcl - m_idx - FgMatrixC<T,dim,1>(1)); }

    // Mirror the point around the centre in the given axis:
    FgMatrixC<T,dim,1>
    mirror(uint axis) const
    {
        FgMatrixC<T,dim,1>      ret = m_idx;
        ret[axis] = m_bndsHiExcl[axis] - m_idx[axis] - 1;
        return ret;
    }

    FgMatrixC<T,dim,1>
    delta() const
    {return m_idx - m_bndsLoIncl; }

    FgMatrixC<T,dim,2>
    inclusiveRange() const
    {return fgConcatHoriz(m_bndsLoIncl,m_bndsHiExcl - FgMatrixC<T,dim,1>(1)); }

    FgMatrixC<T,dim,1>
    dims() const
    {
        return (m_bndsHiExcl - m_bndsLoIncl);
    }

    // Return clipped bounds of all supremum norm (L_inf) distance = 1 neighbours:
    FgMatrixC<T,dim,2>
    neighbourBounds() const
    {
        return 
            FgMatrixC<T,dim,2>(
                fgBoundsIntersection(
                    FgMatrixC<int,dim,1>(m_idx) * FgMatrixC<int,1,2>(1) +
                        FgMatrixC<int,dim,2>(-1,1,-1,1,-1,1),
                    FgMatrixC<int,dim,2>(inclusiveRange())));
    }

private:
    // Return false if range is empty or negative (invalid):
    bool
    validRange()
    {
        for (uint dd=0; dd<dim; dd++)
            if (m_bndsHiExcl[dd] < m_bndsLoIncl[dd])
                return false;
        return true;
    }

    void
    setValid()
    {
        FGASSERT(validRange());
        FGASSERT(m_strides.volume() > 0);
        m_inBounds = fgLt(m_idx,m_bndsHiExcl);
    }
    FgTypeAttributeFixedS<T>    fixed_point_types_only; // Do not instantiate with floating
};

template<typename T,uint dim>
ostream &
operator<<(ostream & os,const FgIter<T,dim> & it)
{
    return os
        << "lo: " << it.m_bndsLoIncl
        << " hi: " << it.m_bndsHiExcl
        << " stride: " << it.m_strides
        << " idx: " << it.m_idx
        << " valid: " << it.m_inBounds;
}

typedef FgIter<int,2>   FgIter2I;
typedef FgIter<int,3>   FgIter3I;
typedef FgIter<uint,2>  FgIter2UI;
typedef FgIter<uint,3>  FgIter3UI;

#endif
