//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgStdExtensions.hpp"

using namespace std;

namespace Fg {

ThreadDispatcher::ThreadDispatcher()
{
    size_t              T = thread::hardware_concurrency();
    threads.reserve(T);
    dones.reserve(T);
    errors.resize(T);
}

void                ThreadDispatcher::dispatch(function<void()> const & fn)
{
    // Add new tasks directly to new threads until full:
    if (threads.size() < thread::hardware_concurrency()) {
        size_t              tt = threads.size();
        dones.push_back(make_unique<atomic<bool>>(false));
        threads.emplace_back(&ThreadDispatcher::worker,this,fn,tt);
        return;
    }
    // Wait for free thread:
    for (;;) {
        for (size_t ii=0; ii<dones.size(); ++ii) {
            Uptr<atomic<bool>> const &  done = dones[ii];
            if (done->load()) {
                threads[ii].join();
                done->store(false);
                threads[ii] = thread(&ThreadDispatcher::worker,this,fn,ii);
                return;
            }
        }
        this_thread::yield();   // hopefully switches context for a bit
    }
}

void                ThreadDispatcher::finish()
{
    for (thread & t : threads)
        t.join();
    threads.clear();
    dones.clear();
    for (size_t ii=0; ii<errors.size(); ++ii)
        if (!errors[ii].empty())
            fgThrow("ThreadDispatcher:\n" + errors[ii]);    // throw the first one we come across
}

void                ThreadDispatcher::worker(Sfun<void()> const & fn,size_t tt)
{
    try {fn();}
    catch (FgException const & e) {errors[tt] = e.englishMessage(); }
    catch (exception const & e) {errors[tt] = e.what(); }
    catch (...) {errors[tt] = "unknown exception"; }
    dones[tt]->store(true);
}

}

// */
