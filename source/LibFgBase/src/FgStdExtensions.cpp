//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgStdExtensions.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

ThreadDispatcher::ThreadDispatcher(bool enable) :
    numThreads {enable ? thread::hardware_concurrency() : 0}
{
    reserve();
}

ThreadDispatcher::ThreadDispatcher(size_t maxThreads) :
    numThreads {(maxThreads<2) ? 0 : maxThreads}
{
    reserve();
}

ThreadDispatcher::~ThreadDispatcher()
{
    if (uncaught_exceptions() == 0)
        finish();
    else {  // unwinding stack from an exception. Join to avoid corruption and do not check for thread exceptions:
        for (thread & t : threads)
            t.join();
    }
}

void                ThreadDispatcher::dispatch(function<void()> const & fn)
{
    if (numThreads==0) {        // multithreading disabled
        fn();
        return;
    }
    // Add new tasks directly to new threads until full:
    if (threads.size() < numThreads) {
        size_t              tt = threads.size();
        errors.push_back({});
        dones.push_back(make_unique<atomic<bool>>(false));
        threads.emplace_back(&ThreadDispatcher::worker,this,fn,tt);
        return;
    }
    // Wait for free thread:
    for (;;) {
        for (size_t ii=0; ii<dones.size(); ++ii) {
            Uptr<atomic<bool>> const &  done = dones[ii];
            if (done->load()) {
                if (!errors[ii].empty())    // this check cannot be done while threads are running
                    fgThrow("ThreadDispatcher:\n"+errors[ii]);
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
    String              error;
    for (String const & e : errors) {
        if (!e.empty()) {
            error = e;      // just keep first one we find
            break;
        }
    }
    // must clear these before throwing since:
    // * destructor calls this function and will attempt to re-join and re-throw if it sees these
    // * leave valid state in case object is re-used after error
    threads.clear();        // leaves capacity unchanged
    dones.clear();
    errors.clear();
    if (!error.empty())
        fgThrow("ThreadDispatcher: "+error);
}

// must take all args by value as client values may go out of scope after dispatch ('this' must still exist):
void                ThreadDispatcher::worker(Sfun<void()> fn,size_t tt)
{
    try {fn();}
    catch (FgException const & e) {errors[tt] = e.englishMessage(); }
    catch (exception const & e) {errors[tt] = e.what(); }
    catch (...) {errors[tt] = "unknown exception"; }
    dones[tt]->store(true);
}

void                ThreadDispatcher::reserve()
{
    if (numThreads>0) {
        // must be reserved up front so they are not reallocated:
        threads.reserve(numThreads);
        dones.reserve(numThreads);
        errors.reserve(numThreads);
    }
}

void                testThreadDispatcher(CLArgs const &)
{
    PushIndent          pind {"ThreadDispatcher error"};
    int                 val;
    auto                fn = [&](){val = 7; fgThrow("test thread error"); };
    ThreadDispatcher    td;
    td.dispatch(fn);
    try {td.finish();}
    catch (FgException const & e) {fgout << fgnl << e.englishMessage(); }
}

}

// */
