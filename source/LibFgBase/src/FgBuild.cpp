//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgStdVector.hpp"
#include "FgStdString.hpp"
#include "FgBuild.hpp"
#include "FgRandom.hpp"
#include "FgHex.hpp"
#include "FgSyntax.hpp"

using namespace std;

namespace Fg {

const vector<pair<BuildOS,string> > &
fgBuildOSStrs()
{
    static vector<pair<BuildOS,string> >  ret =
    {
        {BuildOS::win,"win"},
        {BuildOS::linux,"ubuntu"},
        {BuildOS::macos,"macos"},
        {BuildOS::ios,"ios"},
        {BuildOS::android,"android"}
    };
    return ret;
}

std::ostream &
operator<<(std::ostream & os ,BuildOS bos)
{
    const vector<pair<BuildOS,string> > & lst = fgBuildOSStrs();
    for (const pair<BuildOS,string> & l : lst)
        if (l.first == bos)
            return os << l.second;
    fgThrow("ostream unhandled BuildOS",int(bos));
    FG_UNREACHABLE_RETURN(os);
}

BuildOS
strToBuildOS(string const & str)
{
    const vector<pair<BuildOS,string> > & lst = fgBuildOSStrs();
    for (const pair<BuildOS,string> & l : lst)
        if (l.second == str)
            return l.first;
    fgThrow("strToBuildOS unhandled string",str);
    FG_UNREACHABLE_RETURN(BuildOS::win);
}

BuildOS
getCurrentBuildOS()
{
#ifdef _WIN32
    return BuildOS::win;
#elif defined __APPLE__
    #ifdef FG_SANDBOX
        return BuildOS::ios;
    #else
        return BuildOS::macos;
    #endif
#else
    return BuildOS::linux;
#endif
}

const vector<pair<Arch,string> > &
fgArchStrs()
{
    static vector<pair<Arch,string> >  ret =
    {
        {Arch::x86,"x86"},
        {Arch::x64,"x64"},
        {Arch::armv7,"armv7"},
        {Arch::arm64,"arm64"},
        {Arch::arm64e,"arm64e"}
    };
    return ret;
}

std::ostream &
operator<<(std::ostream & os,Arch arch)
{
    const vector<pair<Arch,string> > &    lst = fgArchStrs();
    for (const pair<Arch,string> & l : lst)
        if (l.first == arch)
            return os << l.second;
    fgThrow("ostream unhandled Arch",int(arch));
    FG_UNREACHABLE_RETURN(os);
}

Arch
strToArch(string const & str)
{
    const vector<pair<Arch,string> > &    lst = fgArchStrs();
    for (const pair<Arch,string> & l : lst)
        if (l.second == str)
            return l.first;
    fgThrow("strToArch unhandled string",str);
    FG_UNREACHABLE_RETURN(Arch::x86);
}

Archs
getBuildArchs(BuildOS os)
{
    if ((os == BuildOS::win) || (os == BuildOS::linux))
        return fgSvec(Arch::x86,Arch::x64);
    else if (os == BuildOS::macos)
        return fgSvec(Arch::x64);
    else if (os == BuildOS::ios)
        // These are the iOS standard architectures (plus simulator) as of 2019.
        // They support iPhone 5s (not 5,5c) and later.
        return fgSvec(Arch::x64,Arch::armv7,Arch::arm64,Arch::arm64e);
    else if (os == BuildOS::android)
        return fgSvec(Arch::x86,Arch::x64,Arch::armv7,Arch::arm64);
    else
        fgThrow("fgBuildArchitecture unhandled OS",int(os));
    FG_UNREACHABLE_RETURN(Archs());
}

const vector<pair<Compiler,string> > &
compilerStrs()
{
    static vector<pair<Compiler,string> >  ret =
    {
        {Compiler::vs15,"vs15"},
        {Compiler::vs17,"vs17"},
        {Compiler::vs19,"vs19"},
        {Compiler::gcc,"gcc"},
        {Compiler::clang,"clang"},
        {Compiler::icpc,"icpc"}
    };
    return ret;
}

std::ostream &
operator<<(std::ostream & os,Compiler comp)
{
    for (auto const & l : compilerStrs())
        if (l.first == comp)
            return os << l.second;
    fgThrow("ostream unhandled Compiler",int(comp));
    FG_UNREACHABLE_RETURN(os);
}

Compiler strToCompiler(string const & str)
{
    for (auto const & l : compilerStrs())
        if (l.second == str)
            return l.first;
    fgThrow("strToCompiler unhandled string",str);
    FG_UNREACHABLE_RETURN(Compiler::vs15);
}

Compilers
getBuildCompilers(BuildOS os)
{
    if (os == BuildOS::win)
        return fgSvec(Compiler::vs19,Compiler::vs17,Compiler::vs15);
    else if (os == BuildOS::linux)
        return fgSvec(Compiler::clang,Compiler::gcc,Compiler::icpc);
    else if (os == BuildOS::macos)
        return fgSvec(Compiler::clang);
    else if (os == BuildOS::ios)
        return fgSvec(Compiler::clang);
    else if (os == BuildOS::android)
        return fgSvec(Compiler::clang);
    fgThrow("getBuildCompilers unhandled OS",int(os));
    FG_UNREACHABLE_RETURN(Compilers());
}

Compiler
getCurrentCompiler()
{
#if defined _MSC_VER
    #if(_MSC_VER == 1900)
        return Compiler::vs15;
    #elif((_MSC_VER >= 1910) && (_MSC_VER < 1920))
        return Compiler::vs17;
    #elif((_MSC_VER >= 1920) && (_MSC_VER < 1930))
        return Compiler::vs19;
    #else
        define_new_visual_studio_version_here
    #endif
#elif defined __INTEL_COMPILER
    return Compiler::icpc;
#elif defined __clang__
    return Compiler::clang;
#elif defined __GNUC__      // Must be second as it's also defined by CLANG
    return Compiler::gcc;
#else
    define_new_compiler_here
#endif
}

string
getCurrentBuildConfig()
{
#ifdef _DEBUG
    return "debug";
#else
    return "release";
#endif
}

string
getCurrentBuildDescription()
{
    return 
        toStr(getCurrentBuildOS()) + " " +
        toStr(getCurrentCompiler()) + " " +
        fgBitsString() + " " + getCurrentBuildConfig();
}

string
getRelBin(BuildOS os,Arch arch,Compiler comp,bool release,bool backslash)
{
    string          ds = backslash ? "\\" : "/",
                    debrel = release ? "release" : "debug";
    return "bin" + ds + toStr(os) + ds + toStr(arch) + ds + toStr(comp) + ds + debrel + ds;
}

uint64
cUuidHash64(string const & str)
{
    FGASSERT(str.size() >= 8);
    std::hash<string>   hf;         // Returns size_t
#ifdef FG_64    // size_t is 64 bits:
    return hf(str);
#else           // size_t is 32 bits:
    uint64      lo = hf(str),
                hi = hf(str+toStr(lo));
    return (lo | (hi << 32));
#endif
}

FgUint128
cUuidHash128(string const & str)
{
    FgUint128       ret;
    FGASSERT(str.size() >= 16);
    uint64          *pLo = reinterpret_cast<uint64*>(&ret.m[0]),
                    *pHi = reinterpret_cast<uint64*>(&ret.m[8]);
    *pLo = cUuidHash64(str);
    *pHi = cUuidHash64(str+toStr(*pLo));
    return ret;
}

// A UUID is composed of 32 hex digits (ie 16 bytes / 128 bits) in a specific hyphonated pattern,
// the first bits representing time values, although we just use all hash bits for repeatability
// (we don't want to force rebuilds every time we generate new solution/project files):
string
createMicrosoftGuid(string const & name,bool wsb)
{
    FgUint128       val = cUuidHash128(name);
    uchar           *valPtr = &val.m[0];
    string          ret;
    if (wsb) ret += '{';
    ret += 
        toHexString(valPtr,4) + '-' +
        toHexString(valPtr+4,2) + '-' +
        toHexString(valPtr+6,2) + '-' +
        toHexString(valPtr+8,2) + '-' +
        toHexString(valPtr+10,6);
    if (wsb) ret += '}';
    return ret;
}

void
fgTestmCreateMicrosoftGuid(CLArgs const &)
{
    fgout << fgnl << createMicrosoftGuid("This string should hash to a consistent value");
}

}
