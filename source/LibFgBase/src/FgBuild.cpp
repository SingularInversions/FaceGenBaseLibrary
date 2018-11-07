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

FgStrs
fgBuildOSes()
{return fgSvec<string>("win","osx","ubuntu"); }

string
fgCurrentOS()
{
#if defined _WIN32
    return "win";
#elif defined __APPLE__
    return "osx";
#else
    return "ubuntu";
#endif
}

FgStrs
fgBuildCompilers(const string & os)
{
    if (os == "win")
        return fgSvec<string>("vs17","vs15","vs13");    // First is default for releases
    else if (os == "ubuntu")
        return fgSvec<string>("clang","gcc","icpc");    // clang is faster than gcc and can run using the same shared libs
    else if (os == "osx")
        return fgSvec<string>("clang");
    FGASSERT_FALSE;
    return FgStrs();
}

string
fgCurrentCompiler()
{
#if defined _MSC_VER
    #if (_MSC_VER == 1600)
        return "vs10";
    #elif (_MSC_VER == 1700)
        return "vs12";
	#elif (_MSC_VER == 1800)
		return "vs13";
    #elif(_MSC_VER == 1900)
        return "vs15";
    #elif((_MSC_VER >= 1910) && (_MSC_VER < 1920))
        return "vs17";
    #endif                  // If you get an error, define the new Visual Studio version here
#elif defined _INTEL_COMPILER
    return "icpc";
#elif defined __clang__
    return "clang";
#elif defined __GNUC__      // Must be second as it's also defined by CLANG
    return "gcc";
#endif                      // If you get an error, define your compiler here
}

FgStrs
fgBuildBits(const string & compiler)
{
    if (fgStartsWith(compiler,"vs"))
        return fgSvec<string>("32","64");
    else
        return fgSvec<string>("64");
}

string
fgCurrentBuildBits()
{
#ifdef FG_64
    return "64";
#else
    return "32";
#endif
}

FgStrs
fgBuildConfigs()
{return fgSvec<string>("debug","release"); }

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
        fgCurrentOS() + " " + fgCurrentCompiler() + " " +
        fgCurrentBuildBits() + " " + fgCurrentBuildConfig();
}

uint64
fgUuidHash64(const string & str)
{
    FGASSERT(str.size() >= 8);
    std::hash<string>   hf;         // Returns size_t
#ifdef FG_64    // size_t is 64 bits:
    return hf(str);
#else           // size_t is 32 bits:
    size_t      split = str.size() / 2;
    uint64      lo = hf(str.substr(0,split)),
                hi = hf(str.substr(split));
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
    size_t          split = str.length() / 2;
    *pLo = fgUuidHash64(str.substr(0,split));
    *pHi = fgUuidHash64(str.substr(split));
    return ret;
}

// A UUID is composed of 32 hex digits (ie 16 bytes / 128 bits) in a specific hyphonated pattern,
// in which some of the first bits represent time values, which we ignore and replace with has bits:
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
    fgout << fgCreateMicrosoftGuid("FaceGen library test");
}
