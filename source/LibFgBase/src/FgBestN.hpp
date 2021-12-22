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

template<class Key,class Val,size_t N>
struct      BestN
{
    size_t              m_size = 0;          // How many valid values do we have ?
    std::array<std::pair<Key,Val>,N> m_best;

    bool                update(Key key,Val const & val)
    {
        for (size_t ii=0; ii<m_size; ++ii) {
            if (key > m_best[ii].first) {
                for (uint jj=N-1; jj>ii; --jj)
                    m_best[jj] = m_best[jj-1];
                m_best[ii].first = key;
                m_best[ii].second = val;
                if (m_size < N)
                    ++m_size;
                return true;
            }
        }
        if (m_size < N) {
            m_best[m_size].first = key;
            m_best[m_size++].second = val;
            return true;
        }
        return false;
    }
    std::pair<Key,Val> const & operator[](size_t idx) const {return m_best[idx]; }
    bool                empty() const {return (m_size == 0); }
    size_t              size() const {return m_size; }
    auto                begin() const {return m_best.begin(); }
    auto                end() const {return m_best.end() - (N-m_size); }
};

template<class Key,class Val,class Cmp=std::less<Key> >
struct      BestV
{
    size_t                      maxNum;
    Svec<std::pair<Key,Val> >   best;

    BestV(size_t maxNum_) : maxNum(maxNum_) {}

    bool                update(Key key,Val const & val)
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
    Svec<Val>           vals() const {return sliceMember(best,&std::pair<Key,Val>::second); }
};

// Keep the value corresponding to the minimum key:
template<class Key,class Val>
class   Min
{
    Key             m_key = std::numeric_limits<Key>::max();
    Val             m_val {};   // default init to avoid gcc warning -Wmaybe-uninitialized

public:
    void            update(Key key,const Val & val)
    {
        if (key < m_key) {
            m_key = key;
            m_val = val;
        }
    }
    bool            valid() const {return (m_key != std::numeric_limits<Key>::max()); }
    Key             key() const {return m_key; }
    const Val &     val() const 
    {
        FGASSERT(valid());
        return m_val;
    }
};

template<class Key,class Val>
struct      Max
{
    void            update(Key const & key,Val const & val)
    {
        if (key > m_key) {
            m_key = key;
            m_val = val;
        }
    }
    bool            valid() const {return (m_key != std::numeric_limits<Key>::lowest()); }
    Key const &     key() const
    {
        FGASSERT(valid());
        return m_key;
    }
    Val const &     val() const
    {
        FGASSERT(valid());
        return m_val;
    }

private:
    Key             m_key = std::numeric_limits<Key>::lowest();
    Val             m_val {};   // default init to avoid gcc warning -Wmaybe-uninitialized
};

}

#endif
