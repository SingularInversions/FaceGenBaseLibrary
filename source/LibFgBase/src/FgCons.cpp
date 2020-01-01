//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgStdVector.hpp"
#include "FgStdSet.hpp"
#include "FgOut.hpp"
#include "FgException.hpp"
#include "FgFileSystem.hpp"
#include "FgCommand.hpp"
#include "FgMetaFormat.hpp"
#include "FgCons.hpp"
#include "FgSyntax.hpp"

using namespace std;

namespace Fg {

static
vector<string>
glob(string const & dir)
{
    vector<string>      ret;
    DirectoryContents dc = directoryContents(dir);
    for (size_t ii=0; ii<dc.filenames.size(); ++ii) {
        string      fn = dc.filenames[ii].as_utf8_string();
        Path      p(fn);
        string      ext = p.ext.ascii(),
                    base = p.base.ascii();
        if (((ext == "cpp") || (ext == "c") || (ext == "hpp") || (ext == "h")) &&
            (base[0] != '_'))
            ret.push_back(fn);
    }
    return ret;
}

FgConsSrcDir::FgConsSrcDir(string const & baseDir,string const & relDir)
    : dir(relDir), files(glob(baseDir+relDir))
    {}

string
FgConsProj::descriptor() const
{
    string          ret = name + ":" + baseDir + ":";
    for (const FgConsSrcDir & csg : srcGroups)
        ret += csg.dir + ";";
    return ret;
}

FgConsProj
FgConsSolution::addApp(string const & name,string const & lnkDep)
{
    if (!pathExists(name))
        fgThrow("Unable to find directory",name);
    const FgConsProj &  dp = at(lnkDep);
    if (!dp.isStaticLib())
        fgThrow("App must depend on static lib",name,lnkDep);
    FgConsProj          proj(name,"");
    proj.addSrcDir("");
    proj.addDep(lnkDep,false);
    return proj;
}

void
FgConsSolution::addAppClp(string const & name,string const & lnkDep)
{
    FgConsProj          proj = addApp(name,lnkDep);
    proj.type = FgConsProj::Type::clp;
    projects.push_back(proj);
}

void
FgConsSolution::addAppGui(string const & name,string const & lnkDep)
{
    FgConsProj          proj = addApp(name,lnkDep);
    proj.type = FgConsProj::Type::gui;
    projects.push_back(proj);
}

bool
FgConsSolution::contains(string const & projName) const
{
    for (const FgConsProj & p : projects)
        if (p.name == projName)
            return true;
    return false;
}

const FgConsProj &
FgConsSolution::at(string const & projName) const
{
    for (const FgConsProj & p : projects)
        if (p.name == projName)
            return p;
    fgThrow("at() project not found",projName);
    FG_UNREACHABLE_RETURN(projects[0]);
}

// Topological sort of transitive includes:
Strings
FgConsSolution::getTransitiveIncludes(string const & projName,bool fileDir,set<string> & done) const
{
    const FgConsProj &  p = at(projName);
    Strings              ret;
    // DLLs are not transitive - related include file must be a separate explicit lnkDep:
    if (p.isDynamicLib())
        return ret;
    for (const FgProjDep & pd : p.projDeps) {
        if (!Fg::contains(done,pd.name) && pd.transitive) {
            cat_(ret,getTransitiveIncludes(pd.name,fileDir,done));
            done.insert(pd.name);
        }
    }
    for (const FgIncDir & id : p.incDirs) {
        if (id.transitive) {
            if (fileDir)
                ret.push_back("../"+p.name+"/"+p.baseDir+id.relPath+id.relFiles);
            else
                ret.push_back("../"+p.name+"/"+p.baseDir+id.relPath);
        }
    }
    return ret;
}

Strings
FgConsSolution::getIncludes(string const & projName,bool fileDir) const
{
    const FgConsProj &  p = at(projName);
    Strings              ret;
    set<string>         done;
    for (const FgProjDep & pd : p.projDeps)
        cat_(ret,getTransitiveIncludes(pd.name,fileDir,done));
    for (const FgIncDir & id : p.incDirs) {
        if (fileDir)
            ret.push_back(p.baseDir+id.relPath+id.relFiles);
        else
            ret.push_back(p.baseDir+id.relPath);
    }
    return fgReverse(ret);      // Includes need to be search from most proximal to least
}

// Topological sort of transitive defines:
Strings
FgConsSolution::getTransitiveDefs(string const & projName,set<string> & done) const
{
    const FgConsProj &  p = at(projName);
    Strings              ret;
    // DLLs are not transitive - related include file must be a separate explicit lnkDep:
    if (p.isDynamicLib())
        return ret;
    for (const FgProjDep & pd : p.projDeps) {
        if (!Fg::contains(done,pd.name) && pd.transitive) {
            fgSetwiseAdd_(ret,getTransitiveDefs(pd.name,done));
            done.insert(pd.name);
        }
    }
    for (const FgConsDef & d : p.defs)
        if (d.transitive)
            ret.push_back(d.name);
    return ret;
}

Strings
FgConsSolution::getDefs(string const & projName) const
{
    const FgConsProj &  p = at(projName);
    Strings              ret;
    set<string>         done;
    for (const FgProjDep & pd : p.projDeps)
        fgSetwiseAdd_(ret,getTransitiveDefs(pd.name,done));
    for (const FgConsDef & d : p.defs)
        ret.push_back(d.name);
    return ret;
}

Strings
FgConsSolution::getTransitiveLnkDeps(string const & projName,set<string> & done) const
{
    Strings              ret;
    if (Fg::contains(done,projName))
        return ret;
    const FgConsProj &  p = at(projName);
    if (p.isStaticLib())                // Only static libs link transitively, not DLLs:
        for (const FgProjDep & pd : p.projDeps)
            cat_(ret,getTransitiveLnkDeps(pd.name,done));
    cat_(ret,p.binDllDeps);           // Assume binary link deps are most derived (hack)
    if (!p.srcGroups.empty())           // Header-only libs don't link:
        ret.push_back(projName);
    done.insert(projName);
    return ret;
}

Strings
FgConsSolution::getLnkDeps(string const & projName) const
{
    const FgConsProj &  p = at(projName);
    Strings              ret;
    set<string>         done;
    for (const FgProjDep & pd : p.projDeps)
        cat_(ret,getTransitiveLnkDeps(pd.name,done));
    return fgReverse(ret);
}

Strings
FgConsSolution::getAllDeps(string const & projName,set<string> & done,bool dllSource) const
{
    Strings      ret;
    if (Fg::contains(done,projName))
        return ret;
    const FgConsProj &  p = at(projName);
    if (!dllSource && p.isDynamicLib())
        return ret;
    for (const FgProjDep & pd : p.projDeps)
        cat_(ret,getAllDeps(pd.name,done,dllSource));
    ret.push_back(projName);
    done.insert(projName);
    return ret;
}

Strings
FgConsSolution::getAllDeps(string const & projName,bool dllSource) const
{
    set<string>     done;
    return getAllDeps(projName,done,dllSource);
}

Strings
FgConsSolution::getAllDeps(Strings const & projNames,bool dllSource) const
{
    set<string>     done;
    Strings          ret;
    for (string const & projName : projNames)
        cat_(ret,getAllDeps(projName,done,dllSource));
    return ret;
}

FgConsSolution
fgGetConsData(FgConsType type)
{
    FgConsSolution  ret(type);

    FgConsProj      boost("LibTpBoost","boost_1_67_0/");
    boost.addSrcDir("libs/filesystem/src/");
    boost.addSrcDir("libs/serialization/src/");
    boost.addSrcDir("libs/system/src/");
    boost.addIncDir("","boost/",true);
    // This stops boost from automatically flagging the compiler to link to it's default library names.
    // Without this you'll see link errors looking for libs like libboost_filesystem-vc90-mt-gd-1_48.lib:
    boost.defs.push_back(FgConsDef("BOOST_ALL_NO_LIB",true));
    // Suppress command-line warning if the compiler version is more recent than this boost version recognizes:
    boost.defs.push_back(FgConsDef("BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE",true));
    boost.warn = 2;
    ret.projects.push_back(boost);

    // This library is set up such that you run a config script to adapt the source code to
    // the platform (eg. generate config.h and remove other-platform .c files). So a bit of work
    // is required to make it properly source-compatible cross-platform:
    FgConsProj      jpeg("LibJpegIjg6b","");
    jpeg.addSrcDir("");
    jpeg.addIncDir("",true);
    jpeg.warn = 2;
    ret.projects.push_back(jpeg);

    FgConsProj      eigen("LibTpEigen","");
    eigen.addIncDir("","Eigen/",true);
    ret.projects.push_back(eigen);

    FgConsProj      stb("LibTpStb","stb/");
    stb.addIncDir("",true);
    ret.projects.push_back(stb);

    FgConsProj      base("LibFgBase","src/");
    base.addSrcDir("");
    base.addIncDir("",true);
    if (type != FgConsType::win)
        base.addSrcDir("nix/");
    base.addDep(boost.name,true);
    base.addDep(stb.name,false);
    base.addDep(jpeg.name,false);
    base.addDep(eigen.name,false);
    ret.projects.push_back(base);

    string          depName = base.name;

    if (type == FgConsType::win) {
        FgConsProj      basewin("LibFgWin","");
        basewin.addSrcDir("");
        basewin.addIncDir("",true);
        basewin.addDep(base.name,true);
        ret.projects.push_back(basewin);
        depName = basewin.name;
    }

    if (pathExists("fgbl"))
        ret.addAppClp("fgbl",depName);

    return ret;
}

// Create Visual Studio 2015/17/19 solution & project files for given solution in current directory tree:
bool
fgConsVs201x(const FgConsSolution & sln);

// Create native-build OS makefiles for given solution in current directory
// Returns true if different from existing (useful for source control & CI):
bool
fgConsNativeMakefiles(const FgConsSolution & sln);

// Create cross-compile-build OS makefiles for given solution in current directory
// Returns true if different from existing (useful for source control & CI):
bool
fgConsCrossMakefiles(const FgConsSolution & sln);

bool
fgConsBuildFiles(const FgConsSolution & sln)
{
    PushDir       pd;
    if (pathExists("source"))
        pd.push("source");
    bool        changed = false;
    if (sln.type == FgConsType::win)
        changed = fgConsVs201x(sln);
    else if (sln.type == FgConsType::nix)
        changed = fgConsNativeMakefiles(sln);
    else if (sln.type == FgConsType::cross)
        changed = fgConsCrossMakefiles(sln);
    else
        fgThrow("fgConsBuildFiles unhandled OS build family",sln.type);
    return changed;
}

void
fgConsBuildAllFiles()
{
    PushDir       pd;
    if (pathExists("source"))
        pd.push("source");
    fgConsBuildFiles(fgGetConsData(FgConsType::win));
    fgConsBuildFiles(fgGetConsData(FgConsType::nix));
    fgConsBuildFiles(fgGetConsData(FgConsType::cross));
}

void
cmdCons(CLArgs const & args)
{
    Syntax        syntax(args,
        "(sln | make) <option>*\n"
        "    sln  - Visual Studio SLN and VCXPROJ files.\n"
        "    make - Makefiles (all non-windows platforms).\n"
    );
    string          type = syntax.next();
    set<string>     options;
    while (syntax.more())
        options.insert(syntax.next());
    if (type == "sln")
        fgConsBuildFiles(fgGetConsData(FgConsType::win));
    else if (type == "make") {
        fgConsBuildFiles(fgGetConsData(FgConsType::nix));
        fgConsBuildFiles(fgGetConsData(FgConsType::cross));
    }
    else
        syntax.error("Invalid option",type);
}

}
