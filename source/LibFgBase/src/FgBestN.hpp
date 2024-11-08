//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Keep sorted list of best objects based on an associated metric

#ifndef FGBESTN_HPP
#define FGBESTN_HPP

#include "FgArray.hpp"

namespace Fg {

template<class O,class M>
struct      ObjMetric
{
    O               object;
    M               metric;     // must support operator<()

    ObjMetric() {}
    ObjMetric(O const & o,M m) : object{o}, metric{m} {}

    bool            operator<(ObjMetric const & r) const {return metric < r.metric; }
};

template<class M,class O,size_t N>
struct      BestN               // stack-based list of N largest (metric) objects
{
    VArray<ObjMetric<O,M>,N> om;

    bool                update(M metric,O const & object)   // returns true if object was added to the best list:
    {
        for (size_t ii=0; ii<om.size(); ++ii) {
            if (metric > om[ii].metric) {
                om.insertOverflow(ii,{object,metric});
                return true;
            }
        }
        if (om.size() < N) {
            om.append({object,metric});
            return true;
        }
        return false;
    }
    ObjMetric<O,M> const & operator[](size_t idx) const {return om[idx]; }
    bool                empty() const {return om.empty(); }
    size_t              size() const {return om.size(); }
    auto                begin() const {return om.begin(); }
    auto                end() const {return om.end(); }
};

template<class Key,class Val,class Cmp=std::less<Key> >
struct      BestV               // Svec based list with variable maximum size
{
    size_t                      maxNum;     // max value for 'best.size()'
    Svec<std::pair<Key,Val> >   best;

    BestV(size_t maxNum_) : maxNum(maxNum_) {}

    bool                full() const {return best.size() == maxNum; }
    size_t              size() const {return best.size(); }
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
    Svec<Val>           vals() const {return mapMember(best,&std::pair<Key,Val>::second); }
};

// Just keep the min or max value (duplication easier than templating comparison and limits):
template<class Key,class Val>
class           Min
{
    Key             mKey;
    Val             mVal;

public:
    Min() : mKey{lims<Key>::max()}, mVal{} {}            // default init val to avoid gcc maybe-uninitialized warnings
    Min(Key k,Val const & v) : mKey{k}, mVal{v} {}

    void            update(Key key,const Val & val)
    {
        if (key < mKey) {
            mKey = key;
            mVal = val;
        }
    }
    bool            valid() const {return (mKey != lims<Key>::max()); }
    Key             key() const {return mKey; }
    const Val &     val() const 
    {
        FGASSERT(valid());
        return mVal;
    }
};
template<class Key,class Val>
class           Max
{
    Key                 mKey;
    Val                 mVal;

public:
    Max() : mKey{lims<Key>::lowest()}, mVal{} {}            // default init val to avoid gcc maybe-uninitialized warnings
    Max(Key k,Val const & v) : mKey{k}, mVal{v} {}

    void            update(Key const & key,Val const & val)
    {
        if (key > mKey) {
            mKey = key;
            mVal = val;
        }
    }
    bool            valid() const {return (mKey != lims<Key>::lowest()); }
    Key const &     key() const {return mKey; }
    Val const &     val() const
    {
        FGASSERT(valid());
        return mVal;
    }
};

}

#endif
