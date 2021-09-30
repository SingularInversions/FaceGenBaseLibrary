//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGBESTN_HPP
#define FGBESTN_HPP

#include "FgStdLibs.hpp"
#include "FgOpt.hpp"

namespace Fg {

template<class Key,class Val,uint MaxNum>
struct      BestN
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

template<class Key,class Val,class Cmp=std::less<Key> >
struct      BestV
{
    size_t                      maxNum;
    Svec<std::pair<Key,Val> >   best;

    BestV(size_t maxNum_) : maxNum(maxNum_) {}

    bool
    update(Key key,Val const & val)
    {
        for (uint ii=0; ii<best.size(); ++ii) {
            if (Cmp{}(key,best[ii].first)) {
                best.insert(best.begin()+ii,std::make_pair(key,val));
                if (best.size() > maxNum)
                    best.resize(maxNum);
                return true;
            }
        }
        if (best.size() < maxNum) {
            best.push_back(std::make_pair(key,val));
            return true;
        }
        return false;
    }

    Svec<Val>
    vals() const
    {return sliceMember(best,&std::pair<Key,Val>::second); }
};

// Keep the value corresponding to the minimum key:
template<class Key,class Val>
class   Min
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

    Key
    key() const
    {return m_key; }

    const Val &
    val()
    {
        FGASSERT(valid());
        return m_val;
    }
};

template<class Key,class Val>
class   Max
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
