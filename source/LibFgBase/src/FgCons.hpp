//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov 29, 2011
//
// Construct makefiles / solution files / project files
//
// Release versions use fast floating point optimizations to ensure SSE2 vectorization of doubles can be done
// (SSE2 introduced double precision ops but only has 64bit registers).
//

#ifndef FGCONS_HPP
#define FGCONS_HPP

#include "FgStdLibs.hpp"
#include "FgTypes.hpp"
#include "FgDefaultVal.hpp"

using namespace std;

struct  FgConsSrcGroup
{
    string              dir;        // Relative to srcBaseDir (below). Empty means '.'
    vector<string>      files;      // Relative to dir above.

    FgConsSrcGroup() {}

    FgConsSrcGroup(const string & d,const vector<string> & f)
    : dir(d), files(f)
    {}

    FgConsSrcGroup(const string & baseDir,const string & relDir);
};

struct  FgConsProj
{
    string              name;
    string              guid;       // Format is platform-specific
    string              srcBaseDir; // Relative to project dir. Empty string means '.'
    vector<FgConsSrcGroup>    srcGroups;
    vector<string>      incDirs;    // Relative to project dir.
    vector<string>      defs;       // Define statements specific to this project
    // Dependencies on other projects by name. Must be in order of dependencies, from base lib
    // to most dependent lib, so they can be put in right order in makefiles for gcc:
    vector<string>      lnkDeps;
    vector<string>      dllDeps;    // Dependencies on binary DLLs by name (no ext)
    uint                warn;       // Warning level [0 - 4]
    FgBoolF             app;        // Is there a main() ?
    FgBoolF             pureGui;    // Only for apps: false = console, true = pure GUI
    FgBoolF             dll;        // Only for libs: false = static lib, true = DLL
    FgBoolT             unicode;    // Old FG3 stuff uses MBCS

    FgConsProj() {}

    FgConsProj(
        const string &          name_,
        const string &          srcBaseDir_,
        const vector<string> &  incDirs_,
        const vector<string> &  defs_,
        const vector<string> &  lnkDeps_,
        uint                    warn_ = 4);

    void
    addSrcGroup(const string & relDir)
    {srcGroups.push_back(FgConsSrcGroup(name+'/'+srcBaseDir,relDir)); }
};

struct  FgConsSolution
{
    bool                        win;    // true: Windows, false: *nix
    vector<FgConsProj>          projects;

    void
    addDll(
        const string &          name,
        const vector<string> &  incDirs,
        const vector<string> &  lnkDeps,
        const vector<string> &  defs);

    void
    addLib(
        const string &          name,
        const vector<string> &  incDirs,
        const vector<string> &  defs);

    void
    addLib(
        const string &          name,
        const string &          srcBaseDir,
        const vector<string> &  srcDirs,
        const vector<string> &  incDirs,
        const vector<string> &  defs,
        uint                    warn = 4);

    void
    addApp(
        const string &          name,
        const vector<string> &  incDirs,
        const vector<string> &  lnkDeps,
        const vector<string> &  defs);
};

struct  FgConsBase
{
    FgConsSolution      sln;
    vector<string>      defs;
    vector<string>      incs;
    vector<string>      lnks;
};

// Current dir must be ~repo/source/
// Use of both flags below is for creating distros, not constructing build files:
FgConsBase
fgConsBase(
    bool    win,    // Include projects needed for Windows
    bool    nix);   // Include projects needed for *nix

// Create makefiles for given solution in current directory:
void
fgConsMakefiles(FgConsSolution sln);

// Create Visual Studio 201x solution & project files for given solution in current directory tree:
void
fgConsVs201x(FgConsSolution sln);

#endif
