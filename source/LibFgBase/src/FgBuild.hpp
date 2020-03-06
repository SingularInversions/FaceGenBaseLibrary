//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGBUILD_HPP
#define FGBUILD_HPP

#include "FgStdLibs.hpp"
#include "FgStdString.hpp"

namespace Fg {

// Build OS families are identical with respect to build files:
enum struct BuildOS { win, linux, macos, ios, android };
typedef Svec<BuildOS>      BuildOSs;

std::ostream & operator<<(std::ostream &,BuildOS);

BuildOS         strToBuildOS(String const &);

// Supported native-build OS families (ie. not cross-compiled):
inline BuildOSs
getNativeBuildOSs()
{return {BuildOS::win,BuildOS::linux,BuildOS::macos}; }

// Supported cross-compile-build OS families:
inline BuildOSs
getCrossBuildOSs()
{return {BuildOS::ios,BuildOS::android}; }

BuildOS         getCurrentBuildOS();

// Instruction set architectures:
enum struct Arch { x86, x64, armv7, arm64, arm64e };
typedef Svec<Arch>     Archs;
std::ostream & operator<<(std::ostream &,Arch);
Arch strToArch(String const &);

Archs           getBuildArchs(BuildOS os);

// Supported compilers (based on platform):
enum struct Compiler { vs15, vs17, vs19, gcc, clang, icpc };
typedef Svec<Compiler>     Compilers;
std::ostream & operator<<(std::ostream &,Compiler);
Compiler strToCompiler(String const &);

// Supported build compilers for given OS.
// The first listed compiler is the default for binary distribution:
Compilers
getBuildCompilers(BuildOS os);

Compilers       getBuildCompilers();                // For current Build OS (starting with default)
Compiler        getCurrentCompiler();
String          getCurrentBuildConfig();
String          getCurrentBuildDescription();

// The primary configuration is used in development for testing exact equality on tests with floating
// point results that can vary on different configs. It is currently win/x64/vs19/release:
bool            isPrimaryConfig();

inline
String
cNsOs(String const & path,BuildOS os)
{return (os == BuildOS::win) ? replaceAll(path,'/','\\') : replaceAll(path,'\\','/'); }

// Return bin directory for given configuration relative to the repo root using given file separator:
String
getRelBin(BuildOS,Arch,Compiler,bool release,bool backslash=false);

// Fast insecure hash for generating UUIDs from unique strings of at least length 8 based on std::hash.
// Deterministic for same compiler / bit depth for regression testing.
// 64 and 32 bit versions will NOT generate the same value, nor will different compilers (per std::hash).
// In future may upgrade to MurmurHash3 to ensure fully deterministic mapping:
uint64
cUuidHash64(String const & uniqueString);

struct  FgUint128
{
    uchar       m[16];
};

// As above but requires unique string at least length 16:
FgUint128
cUuidHash128(String const & uniqueString);

// See comments on cUuidHash64. Fills UUID 'time' fields with random bits for deterministic
// regression testing; all randomness generated from 'name' argument:
String
createMicrosoftGuid(
    String const &  name,   // Must be at least 16 bytes long.
    bool            withSquiglyBrackets=true);

}

#endif

// */
