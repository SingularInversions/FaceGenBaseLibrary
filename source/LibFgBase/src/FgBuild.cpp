//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Dec 2, 2011
//

#include "stdafx.h"
#include "FgBuild.hpp"
#include "FgStdVector.hpp"
#include "FgRandom.hpp"
#include "FgHex.hpp"

using namespace std;

std::vector<std::string>
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

std::vector<std::string>
fgBuildCompilers(const std::string & os)
{
    if (os == "win")
        return fgSvec<string>("vs10","vs08","vs12","vs13");    // First is default
    else if (os == "ubuntu")
        return fgSvec<string>("gcc");
    else if (os == "osx")
        return fgSvec<string>("gcc");
    FGASSERT_FALSE;
    return vector<string>();
}

std::string
fgCurrentCompiler()
{
#if defined _MSC_VER
    #if (_MSC_VER == 1500)
        return "vs08";
    #elif (_MSC_VER == 1600)
        return "vs10";
    #elif (_MSC_VER == 1700)
        return "vs12";
	#elif (_MSC_VER == 1800)
		return "vs13";
    #endif
#elif defined __GNUC__
    return "gcc";
#else
    FGASSERT_FALSE;
    return "";
#endif
}

string
fgCompilersSyntax()
{
    vector<string>  compilers = fgCompilers();
    string  ret = "    <compiler> = (";
    ret += compilers[0];
    for (size_t ii=1; ii<compilers.size(); ++ii)
        ret = ret + " | " + compilers[ii];
    ret = ret + ")\n";
    return ret;
}

std::vector<std::string>
fgBuildBits(const std::string & compiler)
{
    if (compiler == "gcc")
        return fgSvec<string>("64");
    else
        return fgSvec<string>("32","64");
}

std::string
fgCurrentBuildBits()
{
#ifdef FG_64
    return "64";
#else
    return "32";
#endif
}

std::vector<std::string>
fgBuildConfigs()
{return fgSvec<string>("debug","release"); }

std::string
fgCurrentBuildConfig()
{
#ifdef _DEBUG
    return "debug";
#else
    return "release";
#endif
}

std::string
fgCurrentBuildDescription()
{
    return 
        fgCurrentOS() + " " + fgCurrentCompiler() + " " +
        fgCurrentBuildBits() + " " + fgCurrentBuildConfig();
}

// An MS GUID is composed of 32 hex digits (ie 16 bytes / 128 bits).
// The hex digits must have letters capitalized and dashes are inserted at
// certain stupid points:
string
fgCreateMicrosoftGuid(const string & name,bool wsb)
{
    boost::hash<string>     hashFunc;       // returns size_t
    size_t                  hashVal = hashFunc(name);
    fgRandSeedRepeatable(uint(hashVal));
    uchar           val[16],
                    *valPtr = &val[0];
    uint64          *p0 = (uint64*)(&val[0]),
                    *p1 = (uint64*)(&val[8]);
    *p0 = fgRandUint64();
    *p1 = fgRandUint64();
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
