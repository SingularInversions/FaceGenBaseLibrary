//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgBuild.hpp"
#include "FgFileSystem.hpp"
#include "FgRandom.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

const vector<pair<BuildOS,string> > & fgBuildOSStrs()
{
    static vector<pair<BuildOS,string> >  ret =
    {
        {BuildOS::win,"win"},
        {BuildOS::linux,"linux"},
        {BuildOS::macos,"macos"},
        {BuildOS::ios,"ios"},
        {BuildOS::android,"android"}
    };
    return ret;
}

std::ostream &      operator<<(std::ostream & os ,BuildOS bos)
{
    const vector<pair<BuildOS,string> > & lst = fgBuildOSStrs();
    for (const pair<BuildOS,string> & l : lst)
        if (l.first == bos)
            return os << l.second;
    fgThrow("ostream unhandled BuildOS",int(bos));
    FG_UNREACHABLE_RETURN(os);
}

BuildOS             strToBuildOS(string const & str)
{
    const vector<pair<BuildOS,string> > & lst = fgBuildOSStrs();
    for (const pair<BuildOS,string> & l : lst)
        if (l.second == str)
            return l.first;
    fgThrow("Not a recognized Build OS name",str);
    FG_UNREACHABLE_RETURN(BuildOS::win);
}

BuildOSs            getAllBuildOss()
{
    return sliceMember(fgBuildOSStrs(),&pair<BuildOS,string>::first);
}

BuildOSs            getNativeBuildOSs()
{
    return {BuildOS::win,BuildOS::linux,BuildOS::macos};
}

BuildOSs            getCrossBuildOSs()
{
    return {BuildOS::ios,BuildOS::android};
}

BuildOS             getCurrentBuildOS()
{
#ifdef _WIN32
    return BuildOS::win;
#elif defined __APPLE__
    #ifdef FG_SANDBOX
        return BuildOS::ios;
    #else
        return BuildOS::macos;
    #endif
#elif defined __ANDROID__
    return BuildOS::android;
#else
    return BuildOS::linux;
#endif
}

Svec<pair<Arch,string> > getArchStrs()
{
    return {
        {Arch::x86,"x86"},
        {Arch::x64,"x64"},
        {Arch::arm7,"arm7"},
        {Arch::arm8,"arm8"},
    };
}

std::ostream &      operator<<(std::ostream & os,Arch arch)
{
    const vector<pair<Arch,string> > &    lst = getArchStrs();
    for (const pair<Arch,string> & l : lst)
        if (l.first == arch)
            return os << l.second;
    fgThrow("ostream unhandled Arch",int(arch));
    FG_UNREACHABLE_RETURN(os);
}

Arch                strToArch(string const & str)
{
    const vector<pair<Arch,string> > &    lst = getArchStrs();
    for (const pair<Arch,string> & l : lst)
        if (l.second == str)
            return l.first;
    fgThrow("strToArch unhandled string",str);
    FG_UNREACHABLE_RETURN(Arch::x86);
}

Archs               getBuildArchs(BuildOS os)
{
    if (os == BuildOS::win)
        return { Arch::x86, Arch::x64 };
    else if (os == BuildOS::linux)
        return { Arch::x64, Arch::arm8 };
    else if (os == BuildOS::macos)
        return { Arch::x64 };
    else if (os == BuildOS::ios)
        // iOS required architectures (plus x64 for simulator):
        return { Arch::x64, Arch::arm7, Arch::arm8 };
    else if (os == BuildOS::android)
        // Android runs all all kids of devices:
        return { Arch::x86, Arch::x64, Arch::arm7, Arch::arm8 };
    else
        fgThrow("getBuildArchs unhandled OS",toStr(os));
    FG_UNREACHABLE_RETURN(Archs());
}

Arch                getCurrentArch()
{
#if defined (__x86_64__) || defined(_M_X64)     // nix , win
    return Arch::x64;
#elif defined (i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)    // nix*3 , win
    return Arch::x86;
#elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__) || (__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)  || defined(__ARM_ARCH_7M__)  || defined(__ARM_ARCH_7S__)
    return Arch::arm7;
#elif defined(__aarch64__) || defined(_M_ARM64)
    return Arch::arm8;
#else
    current architecture not detected by pre-defined macros
#endif
}

Archs               getAllArchs()
{
    return { Arch::x86, Arch::x64, Arch::arm7, Arch::arm8 };
}

const vector<pair<Compiler,string> > & compilerStrs()
{
    static vector<pair<Compiler,string> >  ret =
    {
        {Compiler::vs19,"vs19"},
        {Compiler::vs22,"vs22"},
        {Compiler::gcc,"gcc"},
        {Compiler::clang,"clang"},
        {Compiler::icpc,"icpc"}
    };
    return ret;
}

std::ostream &      operator<<(std::ostream & os,Compiler comp)
{
    for (auto const & l : compilerStrs())
        if (l.first == comp)
            return os << l.second;
    fgThrow("ostream unhandled Compiler",int(comp));
    FG_UNREACHABLE_RETURN(os);
}

Compilers           getAllCompilers()
{
    return sliceMember(compilerStrs(),&pair<Compiler,string>::first);
}

Compiler strToCompiler(string const & str)
{
    for (auto const & l : compilerStrs())
        if (l.second == str)
            return l.first;
    fgThrow("strToCompiler unhandled string",str);
    FG_UNREACHABLE_RETURN(Compiler::vs19);
}

Compilers           getBuildCompilers(BuildOS os)
{
    if (os == BuildOS::win)
        return { Compiler::vs22,Compiler::vs19};
    else if (os == BuildOS::linux)
        return {Compiler::gcc,Compiler::clang,Compiler::icpc};
    else if (os == BuildOS::macos)
        return {Compiler::clang};
    else if (os == BuildOS::ios)
        return {Compiler::clang};
    else if (os == BuildOS::android)
        return {Compiler::clang};
    fgThrow("getBuildCompilers unhandled OS",int(os));
    FG_UNREACHABLE_RETURN(Compilers());
}

Compilers           getBuildCompilers() {return getBuildCompilers(getCurrentBuildOS()); }

Compiler            getCurrentCompiler()
{
#if defined _MSC_VER
    #if(_MSC_VER < 1920)
        visual_studio_2017_and_earlier_not_supprted;
    #elif(_MSC_VER < 1930)
        return Compiler::vs19;
    #elif(_MSC_VER < 1940)
        return Compiler::vs22;
    #else
        visual_studio_version_not_yet_supported;
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

string              getCurrentBuildConfig()
{
#ifdef _DEBUG
    return "debug";
#else
    return "release";
#endif
}

string              getCurrentBuildDescription()
{
    return 
        toStr(getCurrentBuildOS()) + " " +
        toStr(getCurrentArch()) + " " +
        toStr(getCurrentCompiler()) + " " +
        getCurrentBuildConfig();
}

ostream &           operator<<(ostream & os,Debrel d)
{
    if (d == Debrel::debug)
        return os << "debug";
    else
        return os << "release";
}

bool                isPrimaryConfig()
{
    return (is64Bit() && isRelease() && (getCurrentCompiler() == getBuildCompilers()[0]));
}

string              getRelBin(BuildOS os,Arch arch,Compiler comp,Debrel debrel,bool backslash)
{
    string          ds = backslash ? "\\" : "/";
    return "bin" + ds + toStr(os) + ds + toStr(arch) + ds + toStr(comp) + ds + toStr(debrel) + ds;
}
string              getRelBin(BuildOS os,Arch arch,bool backslash)
{
    string          ds = backslash ? "\\" : "/";
    return "bin" + ds + toStr(os) + ds + toStr(arch) + ds;
}

uint64              cUuidHash64(string const & str)
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

uint128             cUuidHash128(string const & str)
{
    uint128       ret;
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
string              createMicrosoftGuid(string const & name,bool wsb)
{
    uint128       val = cUuidHash128(name);
    uchar           *valPtr = &val.m[0];
    string          ret;
    if (wsb) ret += '{';
    ret += 
        bytesToHexString(valPtr,4) + '-' +
        bytesToHexString(valPtr+4,2) + '-' +
        bytesToHexString(valPtr+6,2) + '-' +
        bytesToHexString(valPtr+8,2) + '-' +
        bytesToHexString(valPtr+10,6);
    if (wsb) ret += '}';
    return ret;
}

void                testmCreateMicrosoftGuid(CLArgs const &)
{
    fgout << fgnl << createMicrosoftGuid("This string should hash to a consistent value");
}


static Strings      glob(String const & dir)
{
    Strings      ret;
    DirContents dc = getDirContents(dir);
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

ConsSrcDir::ConsSrcDir(String const & baseDir,String const & relDir)
    : dir(relDir), files(glob(baseDir+relDir))
    {}

String              ConsProj::descriptor() const
{
    string          ret = name + ":" + baseDir + ":";
    for (const ConsSrcDir & csg : srcGroups)
        ret += csg.dir + ";";
    return ret;
}

ConsType            buildOSToConsType(BuildOS os)
{
    ConsType            type {ConsType::nix};
    if (os == BuildOS::win)
        type = ConsType::win;
    else if (contains(getCrossBuildOSs(),os))
        type = ConsType::cross;
    return type;
}

ConsProj            ConsSolution::addApp(String const & name,String const & lnkDep)
{
    if (!pathExists(name))
        fgThrow("Unable to find directory",name);
    ConsProj const &  dp = projByName(lnkDep);
    if (!dp.isStaticLib())
        fgThrow("App must depend on static lib",name,lnkDep);
    ConsProj          proj(name,"");
    proj.addSrcDir("");
    proj.addDep(lnkDep,false);
    return proj;
}

void                ConsSolution::addAppClp(String const & name,String const & lnkDep)
{
    ConsProj          proj = addApp(name,lnkDep);
    proj.type = ConsProj::Type::clp;
    projects.push_back(proj);
}

void                ConsSolution::addAppGui(String const & name,String const & lnkDep)
{
    ConsProj          proj = addApp(name,lnkDep);
    proj.type = ConsProj::Type::gui;
    projects.push_back(proj);
}

bool                ConsSolution::contains(String const & projName) const
{
    for (ConsProj const & p : projects)
        if (p.name == projName)
            return true;
    return false;
}

ConsProj const &    ConsSolution::projByName(String const & projName) const
{
    for (ConsProj const & p : projects)
        if (p.name == projName)
            return p;
    fgThrow("projByName() project not found",projName);
    FG_UNREACHABLE_RETURN(projects[0]);
}

// Topological sort of transitive includes:
Strings             ConsSolution::getTransitiveIncludes(String const & projName,bool fileDir,set<String> & done) const
{
    ConsProj const &  p = projByName(projName);
    Strings              ret;
    // DLLs are not transitive - related include file must be a separate explicit lnkDep:
    if (p.isDynamicLib())
        return ret;
    for (ProjDep const & pd : p.projDeps) {
        if (!Fg::contains(done,pd.name) && pd.transitive) {
            cat_(ret,getTransitiveIncludes(pd.name,fileDir,done));
            done.insert(pd.name);
        }
    }
    for (IncDir const & id : p.incDirs) {
        if (id.transitive) {
            if (fileDir)
                ret.push_back("../"+p.name+"/"+p.baseDir+id.relPath+id.relFiles);
            else
                ret.push_back("../"+p.name+"/"+p.baseDir+id.relPath);
        }
    }
    return ret;
}

Strings             ConsSolution::getIncludes(String const & projName,bool fileDir) const
{
    ConsProj const &        p = projByName(projName);
    Strings                 ret;
    set<string>             done;
    for (ProjDep const & pd : p.projDeps)
        cat_(ret,getTransitiveIncludes(pd.name,fileDir,done));
    for (IncDir const & id : p.incDirs) {
        if (fileDir)
            ret.push_back(p.baseDir+id.relPath+id.relFiles);
        else
            ret.push_back(p.baseDir+id.relPath);
    }
    return cReverse(ret);      // Includes need to be search from most proximal to least
}

// Topological sort of transitive defines:
Strings             ConsSolution::getTransitiveDefs(String const & projName,set<String> & done) const
{
    ConsProj const &  p = projByName(projName);
    Strings              ret;
    // DLLs are not transitive - related include file must be a separate explicit lnkDep:
    if (p.isDynamicLib())
        return ret;
    for (ProjDep const & pd : p.projDeps) {
        if (!Fg::contains(done,pd.name) && pd.transitive) {
            setwiseAdd_(ret,getTransitiveDefs(pd.name,done));
            done.insert(pd.name);
        }
    }
    for (const ConsDef & d : p.defs)
        if (d.transitive)
            ret.push_back(d.name);
    return ret;
}

Strings             ConsSolution::getDefs(String const & projName) const
{
    ConsProj const &  p = projByName(projName);
    Strings              ret;
    set<string>         done;
    for (ProjDep const & pd : p.projDeps)
        setwiseAdd_(ret,getTransitiveDefs(pd.name,done));
    for (const ConsDef & d : p.defs)
        ret.push_back(d.name);
    return ret;
}

Strings             ConsSolution::getTransitiveLnkDeps(String const & projName,set<String> & done) const
{
    Strings              ret;
    if (Fg::contains(done,projName))
        return ret;
    ConsProj const &  p = projByName(projName);
    if (p.isStaticLib())                // Only static libs link transitively, not DLLs:
        for (ProjDep const & pd : p.projDeps)
            cat_(ret,getTransitiveLnkDeps(pd.name,done));
    cat_(ret,p.binDllDeps);           // Assume binary link deps are most derived (hack)
    if (!p.srcGroups.empty())           // Header-only libs don't link:
        ret.push_back(projName);
    done.insert(projName);
    return ret;
}

Strings             ConsSolution::getLnkDeps(String const & projName) const
{
    ConsProj const &  p = projByName(projName);
    Strings              ret;
    set<string>         done;
    for (ProjDep const & pd : p.projDeps)
        cat_(ret,getTransitiveLnkDeps(pd.name,done));
    return cReverse(ret);
}

Strings             ConsSolution::getAllDeps(String const & projName,set<String> & done,bool dllSource) const
{
    Strings      ret;
    if (Fg::contains(done,projName))
        return ret;
    ConsProj const &  p = projByName(projName);
    if (!dllSource && p.isDynamicLib())
        return ret;
    for (ProjDep const & pd : p.projDeps)
        cat_(ret,getAllDeps(pd.name,done,dllSource));
    ret.push_back(projName);
    done.insert(projName);
    return ret;
}

Strings             ConsSolution::getAllDeps(String const & projName,bool dllSource) const
{
    set<string>     done;
    return getAllDeps(projName,done,dllSource);
}

Strings             ConsSolution::getAllDeps(Strings const & projNames,bool dllSource) const
{
    set<string>     done;
    Strings          ret;
    for (string const & projName : projNames)
        cat_(ret,getAllDeps(projName,done,dllSource));
    return ret;
}

ConsSolution        getConsData(ConsType type)
{
    ConsSolution        ret(type);

    ConsProj            eigen("LibTpEigen","");
    eigen.addIncDir("","Eigen/",true);
    ret.projects.push_back(eigen);

    ConsProj            stb("LibTpStb","");
    stb.addIncDir("",true);
    ret.projects.push_back(stb);

    ConsProj            dlib {"LibTpDlib",""};
    dlib.addIncDir("",true);
    dlib.addSrcDir("dlib/all/");
    dlib.warn = 0;
    dlib.defs.emplace_back("DLIB_ISO_CPP_ONLY",false);    // don't build stuff we won't use
    ret.projects.push_back(dlib);

    ConsProj            base("LibFgBase","src/");
    base.addSrcDir("");
    base.addIncDir("",true);
    if (type != ConsType::win)
        base.addSrcDir("nix/");
    base.addDep(eigen.name,false);
    base.addDep(stb.name,false);
    base.addDep(dlib.name,true);
    ret.projects.push_back(base);
    string              depName = base.name;
    if (type == ConsType::win) {
        ConsProj      basewin("LibFgWin","");
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
bool                writeVisualStudioSolutionFiles(ConsSolution const & sln);
// Create native-build OS makefiles for given solution in current directory
// Returns true if different from existing (useful for source control & CI):
bool                consNativeMakefiles(ConsSolution const & sln);
// Create cross-compile-build OS makefiles for given solution in current directory
// Returns true if different from existing (useful for source control & CI):
//bool                consCrossMakefiles(ConsSolution const & sln);

bool                constructBuildFiles(ConsSolution const & sln)
{
    PushDir             pd;
    if (pathExists("source"))
        pd.push("source");
    bool                changed = false;
    if (sln.type == ConsType::win)
        changed = writeVisualStudioSolutionFiles(sln);
    else if (sln.type == ConsType::nix)
        changed = consNativeMakefiles(sln);
    //else if (sln.type == ConsType::cross)
    //    changed = consCrossMakefiles(sln);
    else
        fgThrow("constructBuildFiles unhandled OS build family",sln.type);
    return changed;
}

void                constructBuildFiles()
{
    PushDir             pd;
    if (pathExists("source"))
        pd.push("source");
    constructBuildFiles(getConsData(ConsType::win));
    constructBuildFiles(getConsData(ConsType::nix));
    //constructBuildFiles(getConsData(ConsType::cross));
}

void                cmdCons(CLArgs const & args)
{
    Syntax              syn(args,
        "(sln | make) <option>*\n"
        "    sln  - Visual Studio SLN and VCXPROJ files.\n"
        "    make - Makefiles (all non-windows platforms).\n"
    );
    string              type = syn.next();
    set<string>         options;
    while (syn.more())
        options.insert(syn.next());
    if (type == "sln")
        constructBuildFiles(getConsData(ConsType::win));
    else if (type == "make") {
        constructBuildFiles(getConsData(ConsType::nix));
        //constructBuildFiles(getConsData(ConsType::cross));
    }
    else
        syn.error("Invalid option",type);
}

}
