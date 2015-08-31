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
#include "FgSerialize.hpp"
#include "FgBuild.hpp"
#include "FgFileSystem.hpp"

// Path to the build server share:

inline
std::string
fgBuildShareWin()
{return "N:\\"; }

inline
std::string
fgBuildShareOsx()
{return "/Volumes/share/"; }

inline
std::string
fgBuildShareUbuntu()
{return "/mnt/share/"; }

inline
std::string
fgNcShare()
{
#if defined _WIN32
    return fgBuildShareWin();
#elif defined __APPLE__
    return fgBuildShareOsx();
#else
    return fgBuildShareUbuntu();
#endif
}

std::string
fgNcShare(const std::string & os);

struct  FgNcScript
{
    std::string                 logFile;
    std::vector<std::string>    cmds;
    FG_SERIALIZE2(logFile,cmds);
};

inline
uint16
fgNcServerPort()
{return 59405; }

inline
std::string
fgCiShareBoot()
{return fgNcShare() + fgNs("ci/boot/"); }

inline
std::string
fgCiShareBoot(const std::string & os)
{return fgNcShare(os) + fgNsOs("ci/boot/",os); }

inline
std::string
fgCiShareRepo()
{return fgNcShare() + fgNs("ci/root/"); }

inline
std::string
fgCiShareRepo(const std::string & os)
{return fgNcShare(os) + fgNsOs("ci/root/",os); }

#endif

// */
