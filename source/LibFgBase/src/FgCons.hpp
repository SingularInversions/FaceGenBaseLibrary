//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Construct build files (makefiles, VS sln/vcxproj), aka build file generator.
//

#ifndef FGCONS_HPP
#define FGCONS_HPP

#include "FgStdLibs.hpp"
#include "FgTypes.hpp"
#include "FgStdString.hpp"
#include "FgStdVector.hpp"
#include "FgStdSet.hpp"

namespace Fg {

struct  FgConsSrcDir
{
    String          dir;        // Relative to 'name/baseDir/' below. Can be empty.
    // Both compile targets and include files are listed below but the latter are only used
    // for display in IDEs:
    Strings          files;      // Bare filename relative to dir above.

    FgConsSrcDir() {}

    FgConsSrcDir(const String & d,Strings const & f)
    : dir(d), files(f)
    {}

    FgConsSrcDir(const String & baseDir,const String & relDir);
};
typedef Svec<FgConsSrcDir>    FgConsSrcDirs;

struct  FgIncDir
{
    String      relPath;        // Relative to 'name/baseDir/' below. Can be empty.
    // The path of the actual include files relative to the above. This is NOT used as an
    // include path, but to locate the header files for build dependencies. Can be empty:
    String      relFiles;
    bool        transitive;     // True if the include path is public, false if private
    FgIncDir() {}
    FgIncDir(const String & r,bool t) : relPath(r), transitive(t) {}
    FgIncDir(const String & r,const String & f,bool t) : relPath(r), relFiles(f), transitive(t) {}
};
typedef Svec<FgIncDir>        FgIncDirs;

inline FgIncDirs fgIncDirs(Strings const & dirs)
{
    FgIncDirs   ret;
    for (const String & d : dirs)
        ret.push_back(FgIncDir(d,true));
    return ret;
}

struct  FgConsDef
{
    String      name;
    bool        transitive;

    FgConsDef() {}
    FgConsDef(const String & n,bool t) : name(n), transitive(t) {}
};
typedef Svec<FgConsDef>   FgConsDefs;

inline FgConsDefs fgConsDefs(Strings const & defs)
{
    FgConsDefs  ret;
    for (const String & d : defs)
        ret.push_back(FgConsDef(d,true));
    return ret;
}

struct  FgProjDep
{
    String          name;
    // Are include directories and macro defs from this project and those it depends on passed on
    // to dependents ? (ie. are they used by this project's hpp files rather than just cpp files ?):
    bool            transitive;

    FgProjDep() {}
    FgProjDep(const String & n,bool t) : name(n), transitive(t) {}
};
typedef Svec<FgProjDep>   FgProjDeps;

struct  FgConsProj
{
    String              name;
    String              baseDir;        // Relative to 'name' dir. Can be empty. Otherwise includes trailing '/'
    FgConsSrcDirs       srcGroups;      // Can be empty for header-only libs (no library will be created)
    FgIncDirs           incDirs;        // Relative to 'name/baseDir/'
    FgConsDefs          defs;
    // Projects on which this project directly depends, ie. directly uses include files from.
    // When those projects have outputs, they will be linked to. Indirect dependencies (either
    // via link-from-link or include-from-include) should not be put here as they are
    // automatically transitively determined:
    FgProjDeps          projDeps;
    Strings              binDllDeps;     // Binary-only DLL dependencies (ie not in 'projDeps' above).
    uint                warn = 4;       // Warning level [0 - 4] (if project has compile targets)
    // 'lib' - static library
    // 'dll' - dynamic library
    // 'clp' - command-line program
    // 'gui' - GUI executable
    enum class Type { lib, dll, clp, gui };
    Type                type = Type::lib;

    FgConsProj() {}

    FgConsProj(const String & n,const String & sbd) : name(n), baseDir(sbd) {}

    bool isStaticLib() const {return (type==Type::lib); }
    bool isClExecutable() const {return (type==Type::clp); }
    bool isGuiExecutable() const {return (type==Type::gui); }
    bool isDynamicLib() const {return (type==Type::dll); }
    bool isExecutable() const {return (isClExecutable() || isGuiExecutable()); }
    bool isLinked() const {return (isExecutable() || isDynamicLib()); }

    void
    addSrcDir(const String & relDir)
    {srcGroups.push_back(FgConsSrcDir(name+'/'+baseDir,relDir)); }

    void
    addIncDir(const String & relDir,bool transitive)
    {incDirs.push_back(FgIncDir(relDir,transitive)); }

    void
    addIncDir(const String & relDir,const String & relFiles,bool transitive)
    {incDirs.push_back(FgIncDir(relDir,relFiles,transitive)); }

    void
    addDep(const String & depName,bool transitiveIncludesAndDefs)
    {projDeps.push_back(FgProjDep(depName,transitiveIncludesAndDefs)); }

    String
    descriptor() const;             // Used to generate repeatable GUID
};

inline std::ostream & operator<<(std::ostream & os,FgConsProj::Type t) {return os << int(t); }

typedef Svec<FgConsProj>  FgConsProjs;

// The build construction data is diferent for the three cases of:
// win: Windows - extra Win-specific libraries
// nix: Linux, MacOS - extra nix-specific source folder within LibFgBase
// cross: Cross-compile platforms (iOS, Android, wasm) - no DLLs, treat as static lib
enum struct FgConsType { win, nix, cross };

inline std::ostream & operator<<(std::ostream & os,FgConsType t)
{return os << size_t(t); }

struct  FgConsSolution
{
    FgConsType          type;
    FgConsProjs         projects;

    explicit FgConsSolution(FgConsType t) : type(t) {}

    void
    addAppClp(const String &  name,const String & lnkDep);

    void
    addAppGui(const String &  name,const String & lnkDep);

    bool
    contains(const String & projName) const;

    const FgConsProj &
    at(const String & projName) const;

    // Return transitive include directories, reverse topologically sorted. The 'fileDir'
    // option returns the directly directly containing the headers, rather than the expected
    // include directiry, if different:
    Strings
    getIncludes(const String & projName,bool fileDir) const;

    Strings
    getDefs(const String & projName) const;

    // Returns transitive required libraries (static and dynamic) for linking, reverse topologically sorted.
    // Library names not contained in 'projects' are binary-only DLLs.
    Strings
    getLnkDeps(const String & projName) const;

    // Returns a list of all prerequisite projects including the given one, topologically sorted
    Strings
    getAllDeps(const String & projName,bool dllSource=true) const;

    // Returns a list of all prerequisite projects including the given one, topologically sorted
    Strings
    getAllDeps(Strings const & projNames,bool dllSource=true) const;

private:
    FgConsProj
    addApp(const String & name,const String & lnkDep);

    Strings
    getTransitiveDefs(const String & projName,std::set<String> & done) const;

    Strings
    getTransitiveIncludes(const String & projName,bool fileDir,std::set<String> & done) const;

    Strings
    getTransitiveLnkDeps(const String & projName,std::set<String> & done) const;

    Strings
    getAllDeps(const String & projName,std::set<String> & done,bool dllSource) const;
};

// Current dir must be ~repo/source/
FgConsSolution
fgGetConsData(FgConsType type);

// Construct build files for given solution data (specific to platform type).
// Current dir must be ~repo/source/
// Returns true if any were modified
bool
fgConsBuildFiles(const FgConsSolution & sln);

// Construct build files for all platforms:
void
fgConsBuildAllFiles();

}

#endif
