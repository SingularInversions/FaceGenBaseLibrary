//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGBUILD_HPP
#define FGBUILD_HPP

#include "FgStdLibs.hpp"
#include "FgStdString.hpp"

namespace Fg {

// Build OS families are identical with respect to build files:
enum struct FgBuildOS { win, linux, macos, ios, android };
typedef Svec<FgBuildOS>      FgBuildOSs;
std::ostream & operator<<(std::ostream &,FgBuildOS);
FgBuildOS fgStrToBuildOS(const String &);

// Supported native-build OS families (ie. not cross-compiled):
inline FgBuildOSs
fgBuildNativeOSs()
{return fgSvec(FgBuildOS::win,FgBuildOS::linux,FgBuildOS::macos); }

// Supported cross-compile-build OS families:
inline FgBuildOSs
fgBuildCrossCompileOSs()
{return fgSvec(FgBuildOS::ios,FgBuildOS::android); }

FgBuildOS
fgCurrentBuildOS();

// Instruction set architectures:
enum struct FgArch { x86, x64, armv7, arm64, arm64e };
typedef Svec<FgArch>     FgArchs;
std::ostream & operator<<(std::ostream &,FgArch);
FgArch fgStrToArch(const String &);

FgArchs
fgBuildArchitectures(FgBuildOS os);

// Supported compilers (based on platform):
enum struct FgCompiler { vs13, vs15, vs17, vs19, gcc, clang, icpc };
typedef Svec<FgCompiler>     FgCompilers;
std::ostream & operator<<(std::ostream &,FgCompiler);
FgCompiler fgStrToCompiler(const String &);

// Supported build compilers for given OS.
// The first listed compiler is the default for binary distribution:
FgCompilers
fgBuildCompilers(FgBuildOS os);

FgCompiler
fgCurrentCompiler();

String
fgCurrentBuildConfig();

String
fgCurrentBuildDescription();

inline
String
fgNsOs(const String & path,FgBuildOS os)
{return (os == FgBuildOS::win) ? fgReplace(path,'/','\\') : fgReplace(path,'\\','/'); }

// Return bin directory for given configuration relative to the repo root:
String
fgRelBin(FgBuildOS,FgArch,FgCompiler,bool release,bool backslash=false);

// Fast insecure hash for generating UUIDs from unique strings of at least length 8 based on std::hash.
// Deterministic for same compiler / bit depth for regression testing.
// 64 and 32 bit versions will NOT generate the same value, nor will different compilers (per std::hash).
// In future may upgrade to MurmurHash3 to ensure fully deterministic mapping:
uint64
fgUuidHash64(const String & uniqueString);

struct  FgUint128
{
    uchar       m[16];
};

// As above but requires unique string at least length 16:
FgUint128
fgUuidHash128(const String & uniqueString);

// See comments on fgUuidHash64. Fills UUID 'time' fields with random bits for deterministic
// regression testing; all randomness generated from 'name' argument:
String
fgCreateMicrosoftGuid(
    const String &  name,   // Must be at least 16 bytes long.
    bool            withSquiglyBrackets=true);

}

#endif

// */
