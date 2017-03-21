//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Dec 7, 2011
//

#ifndef FGBUILD_HPP
#define FGBUILD_HPP

#include "FgStdString.hpp"

// Supported build OSes:
std::vector<std::string>
fgBuildOSes();

// Returns "win", "osx" or "ubuntu":
std::string
fgCurrentOS();

// Supported build compilers for given OS.
// The first listed compiler is the default for binary distribution:
std::vector<std::string>
fgBuildCompilers(const std::string & os);

// Supported build compilers for current OS:
inline
std::vector<std::string>
fgCompilers()
{return fgBuildCompilers(fgCurrentOS()); }

std::string
fgCurrentCompiler();

// Supported build bit sizes for given compiler:
std::vector<std::string>
fgBuildBits(const std::string & compiler);

std::string
fgCurrentBuildBits();

// Supported build configurations ("debug" and "release"):
std::vector<std::string>
fgBuildConfigs();

std::string
fgCurrentBuildConfig();

std::string
fgCurrentBuildDescription();

inline
char
fgDs(const std::string os)
{return ((os == "win") ? '\\' : '/'); }

inline
std::string
fgNsOs(
    const std::string & path,
    const std::string & os)
{return (os == "win") ? fgReplace(path,'/','\\') : fgReplace(path,'\\','/'); }

string
fgCreateMicrosoftGuid(const string & name,bool withSquiglyBrackets=true);

#endif

// */
