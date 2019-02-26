//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Dec 2, 2011
//

#include "stdafx.h"
#include "FgStdVector.hpp"
#include "FgStdString.hpp"
#include "FgBuild.hpp"
#include "FgRandom.hpp"
#include "FgHex.hpp"
#include "FgSyntax.hpp"

using namespace std;

const vector<pair<FgBuildOS,string> > &
fgBuildOSStrs()
{
    static vector<pair<FgBuildOS,string> >  ret =
    {
        {FgBuildOS::win,"win"},
        {FgBuildOS::linux,"ubuntu"},
        {FgBuildOS::macos,"macos"},
        {FgBuildOS::ios,"ios"},
        {FgBuildOS::android,"android"}
    };
    return ret;
}

std::ostream &
operator<<(std::ostream & os ,FgBuildOS bos)
{
    const vector<pair<FgBuildOS,string> > & lst = fgBuildOSStrs();
    for (const pair<FgBuildOS,string> & l : lst)
        if (l.first == bos)
            return os << l.second;
    fgThrow("ostream unhandled FgBuildOS",int(bos));
    FG_UNREACHABLE_RETURN(os);
}

FgBuildOS
fgStrToBuildOS(const string & str)
{
    const vector<pair<FgBuildOS,string> > & lst = fgBuildOSStrs();
    for (const pair<FgBuildOS,string> & l : lst)
        if (l.second == str)
            return l.first;
    fgThrow("fgStrToBuildOS unhandled string",str);
    FG_UNREACHABLE_RETURN(FgBuildOS::win);
}

FgBuildOS
fgCurrentBuildOS()
{
#ifdef _WIN32
    return FgBuildOS::win;
#elif defined __APPLE__
    #ifdef FG_SANDBOX
        return FgBuildOS::ios;
    #else
        return FgBuildOS::macos;
    #endif
#else
    return FgBuildOS::linux;
#endif
}

const vector<pair<FgArch,string> > &
fgArchStrs()
{
    static vector<pair<FgArch,string> >  ret =
    {
        {FgArch::x86,"x86"},
        {FgArch::x64,"x64"},
        {FgArch::armv7,"armv7"},
        {FgArch::arm64,"arm64"},
        {FgArch::arm64e,"arm64e"},
        {FgArch::armv7_a,"armv7-a"},
        {FgArch::arm64_v8a,"arm64-v8a"}
    };
    return ret;
}

std::ostream &
operator<<(std::ostream & os,FgArch arch)
{
    const vector<pair<FgArch,string> > &    lst = fgArchStrs();
    for (const pair<FgArch,string> & l : lst)
        if (l.first == arch)
            return os << l.second;
    fgThrow("ostream unhandled FgArch",int(arch));
    FG_UNREACHABLE_RETURN(os);
}

FgArch
fgStrToArch(const string & str)
{
    const vector<pair<FgArch,string> > &    lst = fgArchStrs();
    for (const pair<FgArch,string> & l : lst)
        if (l.second == str)
            return l.first;
    fgThrow("fgStrToArch unhandled string",str);
    FG_UNREACHABLE_RETURN(FgArch::x86);
}

FgArchs
fgBuildArchitectures(FgBuildOS os)
{
    if ((os == FgBuildOS::win) || (os == FgBuildOS::linux))
        return fgSvec(FgArch::x86,FgArch::x64);
    else if (os == FgBuildOS::macos)
        return fgSvec(FgArch::x64);
    else if (os == FgBuildOS::ios)
        // These are the iOS standard architectures (plus simulator) as of 2019.
        // They support iPhone 5s (not 5,5c) and later.
        return fgSvec(FgArch::armv7,FgArch::arm64,FgArch::arm64e,FgArch::x64);
    else if (os == FgBuildOS::android)
        return fgSvec(FgArch::armv7_a,FgArch::arm64_v8a,FgArch::x86);
    else
        fgThrow("fgBuildArchitecture unhandled OS",int(os));
    FG_UNREACHABLE_RETURN(FgArchs());
}

const vector<pair<FgCompiler,string> > &
fgCompilerStrs()
{
    static vector<pair<FgCompiler,string> >  ret =
    {
        {FgCompiler::vs13,"vs13"},
        {FgCompiler::vs15,"vs15"},
        {FgCompiler::vs17,"vs17"},
        {FgCompiler::gcc,"gcc"},
        {FgCompiler::clang,"clang"},
        {FgCompiler::icpc,"icpc"}
    };
    return ret;
}

std::ostream &
operator<<(std::ostream & os,FgCompiler comp)
{
    const vector<pair<FgCompiler,string> > &    lst = fgCompilerStrs();
    for (const pair<FgCompiler,string> & l : lst)
        if (l.first == comp)
            return os << l.second;
    fgThrow("ostream unhandled FgCompiler",int(comp));
    FG_UNREACHABLE_RETURN(os);
}

FgCompiler fgStrToCompiler(const string & str)
{
    const vector<pair<FgCompiler,string> > &    lst = fgCompilerStrs();
    for (const pair<FgCompiler,string> & l : lst)
        if (l.second == str)
            return l.first;
    fgThrow("fgStrToCompiler unhandled string",str);
    FG_UNREACHABLE_RETURN(FgCompiler::vs13);
}

FgCompilers
fgBuildCompilers(FgBuildOS os)
{
    if (os == FgBuildOS::win)
        return fgSvec(FgCompiler::vs17,FgCompiler::vs15,FgCompiler::vs13);
    else if (os == FgBuildOS::linux)
        return fgSvec(FgCompiler::clang,FgCompiler::gcc,FgCompiler::icpc);
    else if (os == FgBuildOS::macos)
        return fgSvec(FgCompiler::clang);
    else if (os == FgBuildOS::ios)
        return fgSvec(FgCompiler::clang);
    else if (os == FgBuildOS::android)
        return fgSvec(FgCompiler::clang);
    fgThrow("fgBuildCompilers unhandled OS",int(os));
    FG_UNREACHABLE_RETURN(FgCompilers());
}

FgCompiler
fgCurrentCompiler()
{
#if defined _MSC_VER
    #if (_MSC_VER == 1800)
		return FgCompiler::vs13;
    #elif(_MSC_VER == 1900)
        return FgCompiler::vs15;
    #elif((_MSC_VER >= 1910) && (_MSC_VER < 1920))
        return FgCompiler::vs17;
    #else
        define_new_visual_studio_version_here
    #endif
#elif defined __INTEL_COMPILER
    return FgCompiler::icpc;
#elif defined __clang__
    return FgCompiler::clang;
#elif defined __GNUC__      // Must be second as it's also defined by CLANG
    return FgCompiler::gcc;
#else
    define_new_compiler_here
#endif
}

string
fgCurrentBuildConfig()
{
#ifdef _DEBUG
    return "debug";
#else
    return "release";
#endif
}

string
fgCurrentBuildDescription()
{
    return 
        fgToStr(fgCurrentBuildOS()) + " " +
        fgToStr(fgCurrentCompiler()) + " " +
        fgBitsString() + " " + fgCurrentBuildConfig();
}

string
fgRelBin(FgBuildOS os,FgArch arch,FgCompiler comp,bool release,bool backslash)
{
    string          ds = backslash ? "\\" : "/",
                    debrel = release ? "release" : "debug";
    return "bin" + ds + fgToStr(os) + ds + fgToStr(arch) + ds + fgToStr(comp) + ds + debrel + ds;
}

uint64
fgUuidHash64(const string & str)
{
    FGASSERT(str.size() >= 8);
    std::hash<string>   hf;         // Returns size_t
#ifdef FG_64    // size_t is 64 bits:
    return hf(str);
#else           // size_t is 32 bits:
    uint64      lo = hf(str),
                hi = hf(str+fgToStr(lo));
    return (lo | (hi << 32));
#endif
}

FgUint128
fgUuidHash128(const string & str)
{
    FgUint128       ret;
    FGASSERT(str.size() >= 16);
    uint64          *pLo = reinterpret_cast<uint64*>(&ret.m[0]),
                    *pHi = reinterpret_cast<uint64*>(&ret.m[8]);
    *pLo = fgUuidHash64(str);
    *pHi = fgUuidHash64(str+fgToStr(*pLo));
    return ret;
}

// A UUID is composed of 32 hex digits (ie 16 bytes / 128 bits) in a specific hyphonated pattern,
// the first bits representing time values, although we just use all hash bits for repeatability
// (we don't want to force rebuilds every time we generate new solution/project files):
string
fgCreateMicrosoftGuid(const string & name,bool wsb)
{
    FgUint128       val = fgUuidHash128(name);
    uchar           *valPtr = &val.m[0];
    string          ret;
    if (wsb) ret += '{';
    ret += 
        fgAsHex(valPtr,4) + '-' +
        fgAsHex(valPtr+4,2) + '-' +
        fgAsHex(valPtr+6,2) + '-' +
        fgAsHex(valPtr+8,2) + '-' +
        fgAsHex(valPtr+10,6);
    if (wsb) ret += '}';
    return ret;
}

void
fgTestmCreateMicrosoftGuid(const FgArgs &)
{
    fgout << fgnl << fgCreateMicrosoftGuid("This string should hash to a consistent value");
}
