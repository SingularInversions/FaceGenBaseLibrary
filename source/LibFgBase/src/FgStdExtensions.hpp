//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGSTDEXTENSIONS_HPP
#define FGSTDEXTENSIONS_HPP

#include "FgStdLibs.hpp"
#include "FgStdVector.hpp"
#include "FgStdArray.hpp"
#include "FgStdMap.hpp"
#include "FgStdPair.hpp"
#include "FgStdSet.hpp"
#include "FgStdStream.hpp"
#include "FgStdString.hpp"

namespace Fg {

template<class T>
using Sfun = std::function<T>;

template<class T>
using Sptr = std::shared_ptr<T>;

template<class T>
using Uptr = std::unique_ptr<T>;

// A useful default:
template<class T>
std::ostream &
operator<<(std::ostream & ss,std::shared_ptr<T> const & p)
{
    if (p)
        return ss << *p;
    else
        return ss << "NULL";
}

// Like C++17 std::data() but better named:
template <class _Elem>
static
constexpr const _Elem* dataPtr(std::initializer_list<_Elem> _Ilist) noexcept
{return _Ilist.begin(); }

template<class T,class U>
Svec<U>
lookupLs(Svec<std::pair<T,U>> const & table,T const & val)
{
    Svec<U>                 ret;
    for (auto const & row : table)
        if (row.first == val)
            ret.push_back(row.second);
    return ret;
}
template<class T,class U>
Svec<T>
lookupRs(Svec<std::pair<T,U>> const & table,U const & val)
{
    Svec<T>                 ret;
    for (auto const & row : table)
        if (row.second == val)
            ret.push_back(row.first);
    return ret;
}
template<class T,class U>
Sizes
lookupIndsL(Svec<std::pair<T,U>> const & table,T const & val)
{
    Sizes               ret;
    for (size_t ii=0; ii<table.size(); ++ii)
        if (table[ii].first == val)
            ret.push_back(ii);
    return ret;
}
template<class T,class U>
Sizes
lookupIndsR(Svec<std::pair<T,U>> const & table,U const & val)
{
    Sizes               ret;
    for (size_t ii=0; ii<table.size(); ++ii)
        if (table[ii].second == val)
            ret.push_back(ii);
    return ret;
}

// Throws if not found:
template<class T,class U>
U const &
lookupFirstL(Svec<std::pair<T,U>> const & table,T const & val)
{
    auto        it=table.begin();
    for (; it!=table.end(); ++it)
        if (it->first == val)
            return it->second;
    FGASSERT_FALSE;
    return it->second;          // avoid warning
}
template<class T,class U>
T const &
lookupFirstR(Svec<std::pair<T,U>> const & table,U const & val)
{
    auto        it=table.begin();
    for (; it!=table.end(); ++it)
        if (it->second == val)
            return it->first;
    FGASSERT_FALSE;
    return it->first;           // avoid warning
}

// Simple blocking thread dispatcher - limits running threads to hardware capacity.
struct  ThreadDispatcher
{
    ThreadDispatcher()
    {
        threads.reserve(std::thread::hardware_concurrency());
        dones.reserve(std::thread::hardware_concurrency());
    }
    ~ThreadDispatcher() {finish(); }      // thread terminates if destructed before join()

    void
    dispatch(std::function<void()> const & fn);

    void
    finish();

private:
    Svec<std::thread>               threads;
    // thread provides no non-blocking way if testing if it's done so use flags.
    // vector requires copyable which atomic is not so use shared pointer to flags.
    Svec<Sptr<std::atomic<bool>>>   dones;      // 1-1 with above

    void
    worker(Sfun<void()> const & fn,Sptr<std::atomic<bool> > done)
    {
        fn();
        done->store(true);
    }
};

}

#endif
