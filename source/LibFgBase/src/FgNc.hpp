//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 7, 2012
//
// Network Computation

#ifndef FGNC_HPP
#define FGNC_HPP

#include "FgStdString.hpp"
#include "FgStdVector.hpp"
#include "FgSerialize.hpp"
#include "FgBuild.hpp"
#include "FgFileSystem.hpp"

// Location of network computing share root location as specified on given operating system:
string
fgNcShare(const string & os);

// As above for current host OS:
string
fgNcShare();

struct  FgNcScript
{
    // An HTML log of given commands and outputs will be appended to 'logFile' and a 32x32 image
    // will be written to <logFileBaseName>.jpg, green for success of all commands, red for a fail:
    string              logFile;
    string              title;      // Title line of log file
    // Each such command will be shell executed in order. In addition some builtin commands are supported:
    // fgPush <dir>     - push <dir> to current for this process
    // fgPop            - pop back to previous dir for this process
    vector<string>      cmds;
    FG_SERIALIZE3(logFile,title,cmds);
};

inline
uint16
fgNcServerPort()
{return 59405; }

inline
string
fgCiShareBoot()
{return fgNcShare() + fgNs("ci/boot/"); }

inline
string
fgCiShareBoot(const string & os)
{return fgNcShare(os) + fgNsOs("ci/boot/",os); }

inline
string
fgCiShareRepo()
{return fgNcShare() + fgNs("ci/root/"); }

inline
string
fgCiShareRepo(const string & os)
{return fgNcShare(os) + fgNsOs("ci/root/",os); }

#endif

// */
