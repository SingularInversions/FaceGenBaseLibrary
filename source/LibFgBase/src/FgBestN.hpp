//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 9, 2010
//
// Simplify common case where up to N best key/val pairs need to be retained.

#ifndef FGBESTN_HPP
#define FGBESTN_HPP

#include "FgStdLibs.hpp"
#include "FgOpt.hpp"

template<class Key,class Val>
struct FgKeyVal
{
    Key     key;
    Val     val;
};

template<class Key,class Val,uint nvals>
class   FgBestN
{
    uint                m_num;          // How many valid values do we have ?
    FgKeyVal<Key,Val>   m_best[nvals];

public:
    FgBestN()
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
};

#endif
