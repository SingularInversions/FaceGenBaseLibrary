//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgStdExtensions.hpp"

using namespace std;

namespace Fg {

void                ThreadDispatcher::dispatch(function<void()> const & fn)
{
    // Add new tasks directly to new threads until full:
    if (threads.size() < thread::hardware_concurrency()) {
        dones.push_back(make_shared<atomic<bool> >(false));
        threads.emplace_back(&ThreadDispatcher::worker,this,fn,dones.back());
        return;
    }
    // Wait for free thread:
    for (;;) {
        for (size_t ii=0; ii<dones.size(); ++ii) {
            Sptr<atomic<bool> > const & done = dones[ii];
            if (done->load()) {
                done->store(false);
                threads[ii].join();
                threads[ii] = thread(&ThreadDispatcher::worker,this,fn,done);
                return;
            }
        }
        this_thread::yield();   // hopefully switches context for a bit
    }
}

void                ThreadDispatcher::finish()
{
    for (thread & t : threads) {
        t.join();
    }
    threads.clear();
    dones.clear();
}

void                ThreadDispatcher::worker(Sfun<void()> const & fn,Sptr<std::atomic<bool> > done)
{
    fn();
    done->store(true);
}

}

// */
