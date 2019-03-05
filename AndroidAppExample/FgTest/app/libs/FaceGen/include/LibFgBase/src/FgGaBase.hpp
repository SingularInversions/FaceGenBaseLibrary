//
// Copyright (c) 2018 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     18.07.27
//
// GUI API (GA) redesign:
// * Entirely GPU based
// * Fully dynamic

#ifndef FGGABASE_HPP
#define FGGABASE_HPP

#include "FgStdFunction.hpp"
#include "FgString.hpp"
#include "FgAny.hpp"
#include "FgLazy.hpp"
#include "FgMatrixC.hpp"

using namespace std;

namespace   FgGa {

// Set up this data structure for application error handling (eg. report to server):
struct  DiagHandler
{
    // Client-defined error reporting. Can be null.
    // Accepts error message, returns true if reported, false otherwise. If false is returned,
    // Client will be shown the system info:
    std::function<bool(FgString)> reportError;
    FgString                        reportSuccMsg;  // Displayed if 'reportError' returns true.
    // Prepended to error message and displayed if 'reportError' == NULL or 'reportError' returns false:
    FgString                        reportFailMsg;
};

struct  Event
{
    void *      handle;     // OS-specific handle to event for triggering main event-driven loop
    FgFunc      handler;    // Function to handle event
};

struct  KeyHandle
{
    char        key;        // Only visible keys handled for now
    FgFunc      handler;
};

struct  Options
{
    DiagHandler                 diagHandler;
    FgString                    storeDir;
    vector<Event>               events;
    vector<KeyHandle>           keyHandlers;
};

struct  Win
{
    virtual ~Win() {}

    // Return X,Y min,max dims
    virtual FgMat22UI getDims() const = 0;

    virtual void render(FgVect2UI region,const FgAny & osData) const = 0;
};

typedef shared_ptr<Win>     WinPtr;

WinPtr
winText(const FgAny & txtDat);

// Implementation is OS-specific:
void
startMain(
    WinPtr                  win,
    const FgString &        appNameVer);
    //const FgString &        storeDir,
    //const DiagHandler &     diagHandler=DiagHandler());

}

#endif
