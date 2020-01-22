//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Keep up to N best key/val pairs in sorted order from largest to smallest.

#ifndef FGBESTN_HPP
#define FGBESTN_HPP

#include "FgStdLibs.hpp"
#include "FgOpt.hpp"

namespace Fg {

template<class Key,class Val,uint MaxNum>
struct      FgBestN
{
    uint                    m_num = 0;          // How many valid values do we have ?
    std::pair<Key,Val>      m_best[MaxNum];

    bool
    update(Key key,Val val)
    {
        for (uint ii=0; ii<m_num; ++ii) {
            if (key > m_best[ii].first) {
                for (uint jj=MaxNum-1; jj>ii; --jj)
                    m_best[jj] = m_best[jj-1];
                m_best[ii].first = key;
                m_best[ii].second = val;
                if (m_num < MaxNum)
                    ++m_num;
                return true;
            }
        }
        if (m_num < MaxNum) {
            m_best[m_num].first = key;
            m_best[m_num++].second = val;
            return true;
        }
        return false;
    }

    uint
    size() const
    {return m_num; }

    bool
    empty() const
    {return (m_num == 0); }

    std::pair<Key,Val>
    operator[](uint idx) const
    {return m_best[idx]; }

    Svec<Val>
    vals() const
    {
        Svec<Val>     ret(m_num);
        for (uint ii=0; ii<m_num; ++ii)
            ret[ii] = m_best[ii].second;
        return ret;
    }
};


// Keep the value corresponding to the minimum key:
template<class Key,class Val>
class   FgMin
{
    Key     m_key = std::numeric_limits<Key>::max();
    Val     m_val {};   // default init to avoid gcc warning -Wmaybe-uninitialized

public:
    void
    update(Key key,const Val & val)
    {
        if (key < m_key) {
            m_key = key;
            m_val = val;
        }
    }

    bool
    valid() const
    {return (m_key != std::numeric_limits<Key>::max()); }

    const Val &
    val()
    {
        FGASSERT(valid());
        return m_val;
    }
};

template<class Key,class Val>
class   FgMax
{
    Key     m_key = std::numeric_limits<Key>::lowest();
    Val     m_val {};   // default init to avoid gcc warning -Wmaybe-uninitialized

public:
    void
    update(Key key,const Val & val)
    {
        if (key > m_key) {
            m_key = key;
            m_val = val;
        }
    }

    bool
    valid() const
    {return (m_key != std::numeric_limits<Key>::lowest()); }

    const Val &
    val()
    {
        FGASSERT(valid());
        return m_val;
    }
};

}

#endif
