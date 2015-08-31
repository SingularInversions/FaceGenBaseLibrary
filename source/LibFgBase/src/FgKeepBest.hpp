//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 9, 2010
//
// Simplify common idiom where best (or N best) key/val pair needs to be retained in a loop.

#ifndef FGKEEPBEST_HPP
#define FGKEEPBEST_HPP

#include "FgStdLibs.hpp"
#include "FgValidVal.hpp"

template<class Key,class Val>
class   FgKeepBest
{
    bool    m_valid;
    Key     m_key;
    Val     m_val;

public:
    FgKeepBest()
    : m_valid(false),
      m_key(Key()),         // Avoid initialization warnings
      m_val(Val())
    {}

    void
    update(const Key & key,const Val & val)
    {
        if (!m_valid) {
            m_key = key;
            m_val = val;
            m_valid = true;
        }
        else if (key > m_key) {
            m_key = key;
            m_val = val;
        }
    }

    bool
    valid() const
    {return m_valid; }

    const Key &
    key() const
    {FGASSERT(m_valid); return m_key; }

    const Val &
    value() const
    {FGASSERT(m_valid); return m_val; }
};

template<class Key,class Val>
struct FgKeyVal
{
    Key     key;
    Val     val;
};

template<class Key,class Val,uint nvals>
class FgKeepBestN
{
public:
    FgKeepBestN()
    : m_num(0)
    {}

    bool
    update(Key key,Val val)
    {
        for (uint ii=0; ii<m_num; ++ii) {
            if (key < m_best[ii].key) {
                for (uint jj=nvals-1; jj>ii; --jj)
                    m_best[jj] = m_best[jj-1];
                m_best[ii].key = key;
                m_best[ii].val = val;
                if (m_num < nvals)
                    ++m_num;
                return true;
            }
        }
        if (m_num < nvals) {
            m_best[m_num].key = key;
            m_best[m_num++].val = val;
            return true;
        }
        return false;
    }

    uint
    num() const
    {return m_num; }

    FgKeyVal<Key,Val>
    operator[](uint idx) const
    {return m_best[idx]; }

    vector<Val>
    vals() const
    {
        vector<Val>     ret(m_num);
        for (uint ii=0; ii<m_num; ++ii)
            ret[ii] = m_best[ii].val;
        return ret;
    }

private:
    uint                m_num;          // How many valid values do we have ?
    FgKeyVal<Key,Val>   m_best[nvals];
};

#endif
