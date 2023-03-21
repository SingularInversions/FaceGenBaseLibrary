//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Create build files for all platforms

#ifndef FGBUILD_HPP
#define FGBUILD_HPP

#include "FgSerial.hpp"
#include "FgFile.hpp"

namespace Fg {

// Build OS families are defined by their native build files
// (eg. Makefiles for *nix and VS files for Windows)
enum struct BuildOS { win, linux, macos, ios, android };
typedef Svec<BuildOS>   BuildOSs;
std::ostream &      operator<<(std::ostream &,BuildOS);
BuildOS             strToBuildOS(String const &);
BuildOSs            getAllBuildOss();
BuildOSs            getNativeBuildOSs();    // Native-build OS families (ie. not cross-compiled)
BuildOSs            getCrossBuildOSs();     // Supported cross-compile-build OS families; ios,android
BuildOS             getCurrentBuildOS();    // For currently running platform

// Instruction set architectures. To avoid a proliferation of makefiles we only do the major ISAs
// and users can adjust the build for extended instructions such as:
// x64: AVX, AVX2, AVX512
// arm8: 8.2, 8.3, 8.4, 8.5
// (x86 and arm7 are old 32-bit ISAs)
enum struct Arch { x86, x64, arm7, arm8 };
typedef Svec<Arch>     Archs;
std::ostream &      operator<<(std::ostream &,Arch);
Arch                strToArch(String const &);
Archs               getAllArchs();
Archs               getBuildArchs(BuildOS os);
Arch                getCurrentArch();

// Supported compilers (based on platform):
enum struct Compiler { vs19, vs22, gcc, clang, icpc };
typedef Svec<Compiler>     Compilers;
std::ostream &      operator<<(std::ostream &,Compiler);
Compiler            strToCompiler(String const &);
Compilers           getAllCompilers();
// Supported build compilers for given OS.
// The first listed compiler is the default for binary distribution:
Compilers           getBuildCompilers(BuildOS os);
inline Compiler     getDefaultCompiler(BuildOS os) {return getBuildCompilers(os)[0]; }
Compilers           getBuildCompilers();                // For current Build OS (starting with default)
Compiler            getCurrentCompiler();
String              getCurrentBuildConfig();
String              getCurrentBuildDescription();

enum struct Debrel { debug, release };
typedef Arr<Debrel,2>   Debrels;
std::ostream &      operator<<(std::ostream & os,Debrel d);
Debrels constexpr debrels {Debrel::debug,Debrel::release};

// The primary configuration is used in development for testing exact equality on tests with floating
// point results that can vary on different configs:
bool                isPrimaryConfig();

inline String       cNsOs(String const & path,BuildOS os)
{
    return (os == BuildOS::win) ? replaceAll(path,'/','\\') : replaceAll(path,'\\','/');
}

// Return bin directory for given configuration relative to the repo root using given file separator:
String              getRelBin(BuildOS,Arch,Compiler,Debrel debrel,bool backslash=false);
// Bin for pre-built binaries:
String              getRelBin(BuildOS,Arch,bool backslash=false);

// Fast insecure hash for generating UUIDs from unique strings of at least length 8 based on std::hash.
// Deterministic for same compiler / bit depth for regression testing.
// 64 and 32 bit versions will NOT generate the same value, nor will different compilers (per std::hash).
// In future may upgrade to MurmurHash3 to ensure fully deterministic mapping:
uint64              cUuidHash64(String const & uniqueString);

struct  uint128
{
    uchar       m[16];
};

// As above but requires unique string at least length 16:
uint128             cUuidHash128(String const & uniqueString);

// See comments on cUuidHash64. Fills UUID 'time' fields with random bits for deterministic
// regression testing; all randomness generated from 'name' argument:
String              createMicrosoftGuid(
    String const &  name,   // Must be at least 16 bytes long.
    bool            withSquiglyBrackets=true);

struct      ConsSrcDir
{
    String              dir;        // Relative to 'name/baseDir/' below. Can be empty.
    // Both compile targets and include files are listed below but the latter are only used
    // for display in IDEs:
    Strings             files;      // Bare filename relative to dir above.

    ConsSrcDir() {}
    ConsSrcDir(String const & d,Strings const & f) : dir(d), files(f) {}
    ConsSrcDir(String const & baseDir,String const & relDir);
};
typedef Svec<ConsSrcDir>    ConsSrcDirs;

struct      IncDir
{
    String              relPath;        // Relative to 'name/baseDir/' below. Can be empty.
    // The path of the actual include files relative to the above. This is NOT used as an
    // include path, but to locate the header files for build dependencies for code that
    // is being modified. For external libraries it doesn't matter since they seldom change,
    // and in fact many external libraries have a tree structure include so this wouldn't work anyway:
    String              relFiles;       // only .hpp files directly in this path are globbed for dependency updates.
    bool                transitive;     // True if the include path is public, false if private

    IncDir() {}
    IncDir(String const & r,bool t) : relPath(r), transitive(t) {}
    IncDir(String const & r,String const & f,bool t) : relPath(r), relFiles(f), transitive(t) {}
};
typedef Svec<IncDir>        IncDirs;

inline IncDirs      fgIncDirs(Strings const & dirs)
{
    IncDirs   ret;
    for (String const & d : dirs)
        ret.push_back(IncDir(d,true));
    return ret;
}

struct      ConsDef
{
    String      name;
    bool        transitive;

    ConsDef() {}
    ConsDef(String const & n,bool t) : name(n), transitive(t) {}
};
typedef Svec<ConsDef>   ConsDefs;

inline ConsDefs     fgConsDefs(Strings const & defs)
{
    ConsDefs  ret;
    for (String const & d : defs)
        ret.push_back(ConsDef(d,true));
    return ret;
}

struct      ProjDep
{
    String          name;
    // Are include directories and macro defs from this project and those it depends on passed on
    // to dependents ? (ie. are they used by this project's hpp files rather than just cpp files ?):
    bool            transitive;

    ProjDep() {}
    ProjDep(String const & n,bool t) : name(n), transitive(t) {}
};
typedef Svec<ProjDep>   ProjDeps;

struct      ConsProj
{
    String              name;
    String              baseDir;        // Relative to 'name' dir. Can be empty. Otherwise includes trailing '/'
    ConsSrcDirs         srcGroups;      // Can be empty for header-only libs (no library will be created)
    IncDirs             incDirs;        // Relative to 'name/baseDir/'
    ConsDefs            defs;
    // Projects on which this project directly depends, ie. directly uses include files from.
    // When those projects have outputs, they will be linked to. Indirect dependencies (either
    // via link-from-link or include-from-include) should not be put here as they are
    // automatically transitively determined:
    ProjDeps            projDeps;
    Strings             binDllDeps;     // Binary-only DLL dependencies (ie not in 'projDeps' above).
    uint                warn = 4;       // Warning level [0 - 4] (if project has compile targets)
    // 'lib' - static library
    // 'dll' - dynamic library
    // 'clp' - command-line program
    // 'gui' - GUI executable
    enum class Type { lib, dll, clp, gui };
    Type                type = Type::lib;

    ConsProj() {}
    ConsProj(String const & n,String const & sbd) : name(n), baseDir(sbd) {}

    bool                isStaticLib() const {return (type==Type::lib); }
    bool                isClExecutable() const {return (type==Type::clp); }
    bool                isGuiExecutable() const {return (type==Type::gui); }
    bool                isDynamicLib() const {return (type==Type::dll); }
    bool                isExecutable() const {return (isClExecutable() || isGuiExecutable()); }
    bool                isLinked() const {return (isExecutable() || isDynamicLib()); }
    void                addSrcDir(String const & relDir)
    {
        srcGroups.push_back(ConsSrcDir(name+'/'+baseDir,relDir));
    }
    void                addIncDir(String const & relDir,bool transitive)
    {
        incDirs.push_back(IncDir(relDir,transitive));
    }
    void                addIncDir(String const & relDir,String const & relFiles,bool transitive)
    {
        incDirs.push_back(IncDir(relDir,relFiles,transitive));
    }
    void                addDep(String const & depName,bool transitiveIncludesAndDefs)
    {
        projDeps.push_back(ProjDep(depName,transitiveIncludesAndDefs));
    }
    String              descriptor() const;             // Used to generate repeatable GUID
};
inline std::ostream &   operator<<(std::ostream & os,ConsProj::Type t) {return os << int(t); }
typedef Svec<ConsProj>  ConsProjs;

// The build construction data is diferent for the three cases of:
// win: Windows - extra Win-specific libraries
// nix: Linux, MacOS - extra nix-specific source folder within LibFgBase
// cross: Cross-compile platforms (iOS, Android, wasm) - no DLLs, treat as static lib
enum struct     ConsType { win, nix, cross };
inline std::ostream & operator<<(std::ostream & os,ConsType t)
{return os << size_t(t); }

ConsType                buildOSToConsType(BuildOS os);

struct  ConsSolution
{
    ConsType            type;
    ConsProjs           projects;

    explicit ConsSolution(ConsType t) : type(t) {}

    void                addAppClp(String const &  name,String const & lnkDep);
    void                addAppGui(String const &  name,String const & lnkDep);
    bool                contains(String const & projName) const;
    ConsProj const &    projByName(String const & projName) const;
    // Return transitive include directories, reverse topologically sorted. The 'fileDir'
    // option returns the directly directly containing the headers, rather than the expected
    // include directiry, if different:
    Strings             getIncludes(String const & projName,bool fileDir) const;
    Strings             getDefs(String const & projName) const;
    // Returns transitive required libraries (static and dynamic) for linking, reverse topologically sorted.
    // Library names not contained in 'projects' are binary-only DLLs.
    Strings             getLnkDeps(String const & projName) const;
    // Returns a list of all prerequisite projects including the given one, topologically sorted
    Strings             getAllDeps(String const & projName,bool dllSource=true) const;
    // Returns a list of all prerequisite projects including the given one, topologically sorted
    Strings             getAllDeps(Strings const & projNames,bool dllSource=true) const;

private:
    ConsProj            addApp(String const & name,String const & lnkDep);
    Strings             getTransitiveDefs(String const & projName,std::set<String> & done) const;
    Strings             getTransitiveIncludes(String const & projName,bool fileDir,std::set<String> & done) const;
    Strings             getTransitiveLnkDeps(String const & projName,std::set<String> & done) const;
    Strings             getAllDeps(String const & projName,std::set<String> & done,bool dllSource) const;
};

// Current dir must be ~repo/source/
ConsSolution            getConsData(ConsType type);
// Construct build files for given solution data (specific to platform type).
// Current dir must be ~repo/source/
// Returns true if any were modified
bool                    constructBuildFiles(ConsSolution const & sln);
// Construct build files for all platforms:
void                    constructBuildFiles();

}

#endif

// */
