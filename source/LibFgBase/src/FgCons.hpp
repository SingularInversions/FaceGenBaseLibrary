//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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

    FgConsSrcDir(String const & d,Strings const & f)
    : dir(d), files(f)
    {}

    FgConsSrcDir(String const & baseDir,String const & relDir);
};
typedef Svec<FgConsSrcDir>    FgConsSrcDirs;

struct  IncDir
{
    String      relPath;        // Relative to 'name/baseDir/' below. Can be empty.
    // The path of the actual include files relative to the above. This is NOT used as an
    // include path, but to locate the header files for build dependencies. Can be empty:
    String      relFiles;
    bool        transitive;     // True if the include path is public, false if private
    IncDir() {}
    IncDir(String const & r,bool t) : relPath(r), transitive(t) {}
    IncDir(String const & r,String const & f,bool t) : relPath(r), relFiles(f), transitive(t) {}
};
typedef Svec<IncDir>        IncDirs;

inline IncDirs fgIncDirs(Strings const & dirs)
{
    IncDirs   ret;
    for (String const & d : dirs)
        ret.push_back(IncDir(d,true));
    return ret;
}

struct  ConsDef
{
    String      name;
    bool        transitive;

    ConsDef() {}
    ConsDef(String const & n,bool t) : name(n), transitive(t) {}
};
typedef Svec<ConsDef>   ConsDefs;

inline ConsDefs fgConsDefs(Strings const & defs)
{
    ConsDefs  ret;
    for (String const & d : defs)
        ret.push_back(ConsDef(d,true));
    return ret;
}

struct  ProjDep
{
    String          name;
    // Are include directories and macro defs from this project and those it depends on passed on
    // to dependents ? (ie. are they used by this project's hpp files rather than just cpp files ?):
    bool            transitive;

    ProjDep() {}
    ProjDep(String const & n,bool t) : name(n), transitive(t) {}
};
typedef Svec<ProjDep>   ProjDeps;

struct  ConsProj
{
    String              name;
    String              baseDir;        // Relative to 'name' dir. Can be empty. Otherwise includes trailing '/'
    FgConsSrcDirs       srcGroups;      // Can be empty for header-only libs (no library will be created)
    IncDirs           incDirs;        // Relative to 'name/baseDir/'
    ConsDefs          defs;
    // Projects on which this project directly depends, ie. directly uses include files from.
    // When those projects have outputs, they will be linked to. Indirect dependencies (either
    // via link-from-link or include-from-include) should not be put here as they are
    // automatically transitively determined:
    ProjDeps          projDeps;
    Strings              binDllDeps;     // Binary-only DLL dependencies (ie not in 'projDeps' above).
    uint                warn = 4;       // Warning level [0 - 4] (if project has compile targets)
    // 'lib' - static library
    // 'dll' - dynamic library
    // 'clp' - command-line program
    // 'gui' - GUI executable
    enum class Type { lib, dll, clp, gui };
    Type                type = Type::lib;

    ConsProj() {}

    ConsProj(String const & n,String const & sbd) : name(n), baseDir(sbd) {}

    bool isStaticLib() const {return (type==Type::lib); }
    bool isClExecutable() const {return (type==Type::clp); }
    bool isGuiExecutable() const {return (type==Type::gui); }
    bool isDynamicLib() const {return (type==Type::dll); }
    bool isExecutable() const {return (isClExecutable() || isGuiExecutable()); }
    bool isLinked() const {return (isExecutable() || isDynamicLib()); }

    void
    addSrcDir(String const & relDir)
    {srcGroups.push_back(FgConsSrcDir(name+'/'+baseDir,relDir)); }

    void
    addIncDir(String const & relDir,bool transitive)
    {incDirs.push_back(IncDir(relDir,transitive)); }

    void
    addIncDir(String const & relDir,String const & relFiles,bool transitive)
    {incDirs.push_back(IncDir(relDir,relFiles,transitive)); }

    void
    addDep(String const & depName,bool transitiveIncludesAndDefs)
    {projDeps.push_back(ProjDep(depName,transitiveIncludesAndDefs)); }

    String
    descriptor() const;             // Used to generate repeatable GUID
};

inline std::ostream & operator<<(std::ostream & os,ConsProj::Type t) {return os << int(t); }

typedef Svec<ConsProj>  FgConsProjs;

// The build construction data is diferent for the three cases of:
// win: Windows - extra Win-specific libraries
// nix: Linux, MacOS - extra nix-specific source folder within LibFgBase
// cross: Cross-compile platforms (iOS, Android, wasm) - no DLLs, treat as static lib
enum struct ConsType { win, nix, cross };

inline std::ostream & operator<<(std::ostream & os,ConsType t)
{return os << size_t(t); }

struct  ConsSolution
{
    ConsType          type;
    FgConsProjs         projects;

    explicit ConsSolution(ConsType t) : type(t) {}

    void
    addAppClp(String const &  name,String const & lnkDep);

    void
    addAppGui(String const &  name,String const & lnkDep);

    bool
    contains(String const & projName) const;

    const ConsProj &
    at(String const & projName) const;

    // Return transitive include directories, reverse topologically sorted. The 'fileDir'
    // option returns the directly directly containing the headers, rather than the expected
    // include directiry, if different:
    Strings
    getIncludes(String const & projName,bool fileDir) const;

    Strings
    getDefs(String const & projName) const;

    // Returns transitive required libraries (static and dynamic) for linking, reverse topologically sorted.
    // Library names not contained in 'projects' are binary-only DLLs.
    Strings
    getLnkDeps(String const & projName) const;

    // Returns a list of all prerequisite projects including the given one, topologically sorted
    Strings
    getAllDeps(String const & projName,bool dllSource=true) const;

    // Returns a list of all prerequisite projects including the given one, topologically sorted
    Strings
    getAllDeps(Strings const & projNames,bool dllSource=true) const;

private:
    ConsProj
    addApp(String const & name,String const & lnkDep);

    Strings
    getTransitiveDefs(String const & projName,std::set<String> & done) const;

    Strings
    getTransitiveIncludes(String const & projName,bool fileDir,std::set<String> & done) const;

    Strings
    getTransitiveLnkDeps(String const & projName,std::set<String> & done) const;

    Strings
    getAllDeps(String const & projName,std::set<String> & done,bool dllSource) const;
};

// Current dir must be ~repo/source/
ConsSolution
fgGetConsData(ConsType type);

// Construct build files for given solution data (specific to platform type).
// Current dir must be ~repo/source/
// Returns true if any were modified
bool
fgConsBuildFiles(ConsSolution const & sln);

// Construct build files for all platforms:
void
fgConsBuildAllFiles();

}

#endif
