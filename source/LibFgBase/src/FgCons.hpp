//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov 29, 2011
//
// Construct build files (makefiles, VS sln/vcxproj), aka build file generator.
//

#ifndef FGCONS_HPP
#define FGCONS_HPP

#include "FgStdLibs.hpp"
#include "FgTypes.hpp"
#include "FgDefaultVal.hpp"
#include "FgStdString.hpp"
#include "FgStdVector.hpp"
#include "FgStdSet.hpp"

struct  FgConsSrcDir
{
    string          dir;        // Relative to 'name/baseDir/' below. Can be empty.
    // Both compile targets and include files are listed below but the latter are only used
    // for display in IDEs:
    FgStrs          files;      // Bare filename relative to dir above.

    FgConsSrcDir() {}

    FgConsSrcDir(const string & d,const FgStrs & f)
    : dir(d), files(f)
    {}

    FgConsSrcDir(const string & baseDir,const string & relDir);
};
typedef vector<FgConsSrcDir>    FgConsSrcDirs;

struct  FgIncDir
{
    string      relPath;        // Relative to 'name/baseDir/' below. Can be empty.
    // The path of the actual include files relative to the above. This is NOT used as an
    // include path, but to locate the header files for build dependencies. Can be empty:
    string      relFiles;
    bool        transitive;     // True if the include path is public, false if private
    FgIncDir() {}
    FgIncDir(const string & r,bool t) : relPath(r), transitive(t) {}
    FgIncDir(const string & r,const string & f,bool t) : relPath(r), relFiles(f), transitive(t) {}
};
typedef vector<FgIncDir>        FgIncDirs;

inline FgIncDirs fgIncDirs(const FgStrs & dirs)
{
    FgIncDirs   ret;
    for (const string & d : dirs)
        ret.push_back(FgIncDir(d,true));
    return ret;
}

struct  FgConsDef
{
    string      name;
    bool        transitive;

    FgConsDef() {}
    FgConsDef(const string & n,bool t) : name(n), transitive(t) {}
};
typedef vector<FgConsDef>   FgConsDefs;

inline FgConsDefs fgConsDefs(const FgStrs & defs)
{
    FgConsDefs  ret;
    for (const string & d : defs)
        ret.push_back(FgConsDef(d,true));
    return ret;
}

struct  FgProjDep
{
    string          name;
    // Are include directories and macro defs from this project and those it depends on passed on
    // to dependents ? (ie. are they used by this project's hpp files rather than just cpp files ?):
    bool            transitive;

    FgProjDep() {}
    FgProjDep(const string & n,bool t) : name(n), transitive(t) {}
};
typedef vector<FgProjDep>   FgProjDeps;

struct  FgConsProj
{
    string              name;
    string              baseDir;        // Relative to 'name' dir. Can be empty. Otherwise includes trailing '/'
    FgConsSrcDirs       srcGroups;      // Can be empty for header-only libs (no library will be created)
    FgIncDirs           incDirs;        // Relative to 'name/baseDir/'
    FgConsDefs          defs;
    // Projects on which this project directly depends, ie. directly uses include files from.
    // When those projects have outputs, they will be linked to. Indirect dependencies (either
    // via link-from-link or include-from-include) should not be put here as they are
    // automatically transitively determined:
    FgProjDeps          projDeps;
    FgStrs              binDllDeps;     // Binary-only DLL dependencies (ie not in 'projDeps' above).
    uint                warn = 4;       // Warning level [0 - 4] (if project has compile targets)
    // 'lib' - static library
    // 'dll' - dynamic library
    // 'clp' - command-line program
    // 'gui' - GUI executable
    enum class Type { lib, dll, clp, gui };
    Type                type = Type::lib;

    FgConsProj() {}

    FgConsProj(const string & n,const string & sbd) : name(n), baseDir(sbd) {}

    bool isStaticLib() const {return (type==Type::lib); }
    bool isClExecutable() const {return (type==Type::clp); }
    bool isGuiExecutable() const {return (type==Type::gui); }
    bool isDynamicLib() const {return (type==Type::dll); }
    bool isExecutable() const {return (isClExecutable() || isGuiExecutable()); }
    bool isLinked() const {return (isExecutable() || isDynamicLib()); }

    void
    addSrcDir(const string & relDir)
    {srcGroups.push_back(FgConsSrcDir(name+'/'+baseDir,relDir)); }

    void
    addIncDir(const string & relDir,bool transitive)
    {incDirs.push_back(FgIncDir(relDir,transitive)); }

    void
    addIncDir(const string & relDir,const string & relFiles,bool transitive)
    {incDirs.push_back(FgIncDir(relDir,relFiles,transitive)); }

    void
    addDep(const string & depName,bool transitiveIncludesAndDefs)
    {projDeps.push_back(FgProjDep(depName,transitiveIncludesAndDefs)); }

    string
    descriptor() const;             // Used to generate repeatable GUID
};

inline std::ostream & operator<<(std::ostream & os,FgConsProj::Type t) {return os << int(t); }

typedef vector<FgConsProj>  FgConsProjs;

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
    addAppClp(const string &  name,const string & lnkDep);

    void
    addAppGui(const string &  name,const string & lnkDep);

    bool
    contains(const string & projName) const;

    const FgConsProj &
    at(const string & projName) const;

    // Return transitive include directories, reverse topologically sorted. The 'fileDir'
    // option returns the directly directly containing the headers, rather than the expected
    // include directiry, if different:
    FgStrs
    getIncludes(const string & projName,bool fileDir) const;

    FgStrs
    getDefs(const string & projName) const;

    // Returns transitive required libraries (static and dynamic) for linking, reverse topologically sorted.
    // Library names not contained in 'projects' are binary-only DLLs.
    FgStrs
    getLnkDeps(const string & projName) const;

    // Returns a list of all prerequisite projects including the given one, topologically sorted
    FgStrs
    getAllDeps(const string & projName,bool dllSource=true) const;

    // Returns a list of all prerequisite projects including the given one, topologically sorted
    FgStrs
    getAllDeps(const FgStrs & projNames,bool dllSource=true) const;

private:
    FgConsProj
    addApp(const string & name,const string & lnkDep);

    FgStrs
    getTransitiveDefs(const string & projName,set<string> & done) const;

    FgStrs
    getTransitiveIncludes(const string & projName,bool fileDir,set<string> & done) const;

    FgStrs
    getTransitiveLnkDeps(const string & projName,set<string> & done) const;

    FgStrs
    getAllDeps(const string & projName,set<string> & done,bool dllSource) const;
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

#endif
