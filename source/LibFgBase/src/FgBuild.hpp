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

#include "FgStdLibs.hpp"
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

// Fast insecure hash for generating UUIDs from unique strings of at least length 8 based on std::hash.
// Deterministic for same compiler / bit depth only for purposes of avoiding random seed issues,
// 64 and 32 bit versions will NOT generate the same value, nor will different compilers (per std::hash).
// In future may upgrade to use MD5 to ensure fully deterministic mapping:
uint64
fgUuidHash64(const string & uniqueString);

struct  FgUint128
{
    uchar       m[16];
};

// As above but requires unique string at least length 16:
FgUint128
fgUuidHash128(const string & uniqueString);

// Uses fgUuidHash128 above, filling UUID 'time' fields with random bits rather than actual time:
string
fgCreateMicrosoftGuid(
    const string &  name,   // Must be at least 16 bytes long.
    bool            withSquiglyBrackets=true);

#endif

// */
