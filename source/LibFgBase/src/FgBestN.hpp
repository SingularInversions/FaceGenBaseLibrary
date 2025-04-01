//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Keep min/max or sorted list of min/max objects based on an associated metric

#ifndef FGBESTN_HPP
#define FGBESTN_HPP

#include "FgArray.hpp"

namespace Fg {

template<class O,class M>
struct      ObjMetricMin        // smallest to largest metric
{
    O               object;
    M               metric;     // must support operator<()
    FG_SER(object,metric)

    ObjMetricMin() : object{}, metric{lims<M>::max()} {}
    ObjMetricMin(O const & o,M m) : object{o}, metric{m} {}

    bool            valid() const {return metric != lims<M>::max(); }
    bool            operator<(ObjMetricMin<O,M> const & r) const {return metric < r.metric; }
    bool            isBefore(M rm) const {return metric<rm; }
    bool            isBefore(ObjMetricMin<O,M> const & r) const {return metric<r.metric; }
    void            update(M met,O const & obj)
    {
        if (met<metric) {
            object = obj;
            metric = met;
        }
    }
};

template<class O,class M>
struct      ObjMetricMax        // largest to smallest metric
{
    O               object;
    M               metric;     // must support operator<()

    ObjMetricMax() : object{}, metric{lims<M>::lowest()} {}
    ObjMetricMax(O const & o,M m) : object{o}, metric{m} {}

    bool            valid() const {return metric != lims<M>::lowest(); }
    bool            operator<(ObjMetricMin<O,M> const & r) const {return metric < r.metric; }
    bool            isBefore(M rm) const {return rm<metric; }
    bool            isBefore(ObjMetricMin<O,M> const & r) const {return r.metric<metric; }
    void            update(M met,O const & obj)
    {
        if (metric<met) {
            object = obj;
            metric = met;
        }
    }
};

template<class M,class O> using Min = ObjMetricMin<O,M>;
template<class M,class O> using Max = ObjMetricMax<O,M>;
template<class M,class O> using Mins = Svec<ObjMetricMin<O,M>>;
template<class M,class O> using Maxs = Svec<ObjMetricMax<O,M>>;

// keep the N best objects based on their score in a stack array:
template<class M,class O,size_t N>
struct      BestN               // stack-based list of N largest (metric) objects
{
    VArray<ObjMetricMax<O,M>,N> om;

    bool                update(M metric,O const & object)   // returns true if object was added to the best list:
    {
        for (size_t ii=0; ii<om.size(); ++ii) {
            if (!om[ii].isBefore(metric)) {
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
    ObjMetricMax<O,M> const & operator[](size_t idx) const {return om[idx]; }
    bool                empty() const {return om.empty(); }
    size_t              size() const {return om.size(); }
    auto                begin() const {return om.begin(); }
    auto                end() const {return om.end(); }
};

// keep the N best objects based on a metric, in sorted order, using std::vector:
template<class OM>
struct      BestV
{
    size_t              maxNum;         // max number to keep (max size of 'best' below)
    Svec<OM>            best;

    BestV(size_t m) : maxNum{m} {}

    bool                full() const {return best.size() == maxNum; }
    size_t              size() const {return best.size(); }
    bool                update(OM const & om)   // returns true if an update was made
    {
        for (size_t ii=0; ii<best.size(); ++ii) {
            if (om.isBefore(best[ii])) {
                best.insert(best.begin()+ii,om);
                if (best.size() > maxNum)
                    best.resize(maxNum);
                return true;
            }
        }
        if (best.size() < maxNum) {
            best.push_back(om);
            return true;
        }
        return false;
    }
};

}

#endif
