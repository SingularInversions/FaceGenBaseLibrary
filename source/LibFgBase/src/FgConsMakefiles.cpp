//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Dec 10, 2011
//

#include "stdafx.h"
#include "FgBuild.hpp"
#include "FgCons.hpp"
#include "FgOut.hpp"
#include "FgException.hpp"
#include "FgFileSystem.hpp"
#include "FgStdStream.hpp"
#include "FgStdSet.hpp"

using namespace std;

namespace {

const char lf = 0x0A;       // Unix line ending

void
targets(
    ostream &               ofs,
    const string &          prjName,
    const FgConsSrcDir &    grp)
{
    vector<pair<string,string> >    srcNames;
    for (size_t ii=0; ii<grp.files.size(); ++ii) {
        FgPath      p(grp.files[ii]);
        string      ext = p.ext.ascii();
        if ((ext == "cpp") || (ext == "c"))
            srcNames.push_back(make_pair(p.base.ascii(),ext));
    }
    string  odir = "$(ODIR" + prjName + ")" + fgReplace(grp.dir,'/','_');
    for (size_t ii=0; ii<srcNames.size(); ++ii)
        ofs << odir << srcNames[ii].first << ".o ";
}

void
group(
    ostream &               ofs,
    const string &          prjName,
    const FgConsSrcDir &  grp)
{
    string  odir = "$(ODIR" + prjName + ")" + fgReplace(grp.dir,'/','_'),
            sdir = "$(SDIR" + prjName + ")" + grp.dir;
    for (size_t ii=0; ii<grp.files.size(); ++ii) {
        FgPath      path(grp.files[ii]);
        string      ext = path.ext.ascii();
        if ((ext == "cpp") || (ext == "c")) {
            string      srcName = grp.files[ii],
                        objName = path.base.ascii() + ".o",
                        compile = (ext == "cpp") ? "$(CXX)" : "$(CC)",
                        cflags = (ext == "cpp") ? "$(CXXFLAGS)" : "$(CCFLAGS)";
            ofs << odir << objName << ": " << sdir << srcName << " $(INCS" << prjName << ")" << lf
                << "\t" << compile << " -o " << odir << objName << " -c " << cflags
                << " $(FLAGS" << prjName << ") " << sdir << srcName << lf;
        }
    }
}

string
targetPath(const FgConsProj & prj)
{
    if (prj.isDynamicLib())
        return "$(BINDIR)" + prj.name + "$(DLLEXT)";
    else if (prj.isStaticLib())
        return "$(BUILDIR)" + prj.name + ".a";
    else
        return "$(BINDIR)" + prj.name;
}

void
linkLibs(
    ostream &               ofs,
    const FgConsProj &      prj,
    const FgConsSolution &  sln,
    bool                    binaryOnly)     // Include binary-only deps
{
    // Unix link is single-pass; order of linkage is important:
    FgStrs      libDeps = sln.getLnkDeps(prj.name);
    for (const string & name : libDeps) {
        if (sln.contains(name)) {
            const FgConsProj &  p = sln.at(name);
            if (p.isDynamicLib())
                ofs << "$(BINDIR)" << name << "$(DLLEXT) ";
            else
                ofs << "$(BUILDIR)" << name << ".a ";
        }
        else if (binaryOnly)
            ofs << "$(BBINDIR)" << name << "$(DLLEXT) ";
    }
}

// Collapse common case of 'ThisLib/../OtherLib':
string
collapse(
    const string &  prjName,
    const string &  incDir)
{
    if (fgBeginsWith(incDir,"../"))
        return string(incDir.begin()+3,incDir.end());
    else
        return prjName + "/" + incDir;
}

void
consProj(
    ostream &           ofs,
    const FgConsProj &  prj,
    const FgConsSolution & sln)
{
    FgStrs          incDirs = sln.getIncludes(prj.name,false),
                    headerDirs = sln.getIncludes(prj.name,true),
                    defs = sln.getDefs(prj.name);
    ofs << "FLAGS" << prj.name << " = ";
    if (prj.warn < 3)
        ofs << " -w";                   // disables warnings (for all compilers)
    else
        ofs << " -Wall -Wextra";        // Amazingly, the latter does not encompass all of the former
    for (const string & def : defs)
        ofs << " -D" << def;
    for (const string & id : incDirs) {
        string      libName = collapse(prj.name,id);
        ofs << " -I" << libName;
    }
    ofs << lf
        << "SDIR" << prj.name << " = " << prj.name << '/' << prj.baseDir << lf
        << "ODIR" << prj.name << " = $(BUILDIR)" << prj.name << "/" << lf
        << "$(shell mkdir -p $(ODIR" << prj.name << "))" << lf;
    // Make compiles dependent on ALL possible .hpp files since we're not about
    // to go parsing the source code to find the specific dependencies:
    ofs << "INCS" << prj.name << " := ";
    for (const string & id : headerDirs)
        ofs << "$(wildcard " << collapse(prj.name,id) << "*.hpp) ";
    ofs << lf << targetPath(prj) << ": ";
    for (size_t ii=0; ii<prj.srcGroups.size(); ++ii)
        targets(ofs,prj.name,prj.srcGroups[ii]);
    if (prj.isLinked())
        linkLibs(ofs,prj,sln,false);
    ofs << lf;
    if (prj.isLinked()) {
        ofs << "\t$(LINK) $(LFLAGS) -o $(BINDIR)" << prj.name;
        if (prj.isDynamicLib())
            ofs  << "$(DLLEXT) " << "$(DLLARG)" << prj.name << "$(DLLEXT)";
        ofs << " ";
        for (size_t ii=0; ii<prj.srcGroups.size(); ++ii)
            targets(ofs,prj.name,prj.srcGroups[ii]);
        linkLibs(ofs,prj,sln,true);
    }
    else {  // Static library
        // We put libraries in the 'bin' path rather than the 'build' path as that simplifies
        // the issue 
        // ar options: r - replace .o files in archive, c - create if doesn't exist
        ofs << "\tar rc " << targetPath(prj) << " ";
        for (size_t ii=0; ii<prj.srcGroups.size(); ++ii)
            targets(ofs,prj.name,prj.srcGroups[ii]);
        ofs << lf << "\tranlib " << targetPath(prj);
    }
    ofs << lf;
    for (size_t ii=0; ii<prj.srcGroups.size(); ++ii)
        group(ofs,prj.name,prj.srcGroups[ii]);
}

bool
constIncludeFileNative(const FgConsSolution & sln,const string & fname)
{
    set<string>         prereqs;        // All project names which are depended on
    for (const FgConsProj & p : sln.projects)
        for (const FgProjDep & pd : p.projDeps)
            prereqs.insert(pd.name);
    ostringstream       ofs;
    ofs
        << "$(shell mkdir -p $(BINDIR))" << lf
        // .PHONY ensures 'make' won't check for a file named 'all' so will always update:
        << ".PHONY: all" << lf
        << "all: ";
    for (const FgConsProj & p : sln.projects)
        if (!fgContains(prereqs,p.name) && !p.srcGroups.empty())
            ofs << targetPath(p) << " ";
    ofs << lf;
    for (const FgConsProj & proj : sln.projects)
        if (!proj.srcGroups.empty())
            consProj(ofs,proj,sln);
    ofs << ".PHONY: clean cleanObjs cleanTargs" << lf
        << "clean: cleanObjs cleanTargs" << lf
        << "cleanObjs:" << lf
        << "\trm -r $(BUILDIR)" << lf
        << "cleanTargs:" << lf
        << "\trm -r $(BINDIR)" << lf;
    return fgDump(ofs.str(),fname);
}

bool
constIncludeFileBox(const FgConsSolution & sln,const string & fname)
{
    ostringstream       ofs;
    ofs
        // .PHONY ensures 'make' won't check for a file named 'all' so will always update:
        << ".PHONY: all" << lf
        << "all: ";
    for (const FgConsProj & p : sln.projects)
        if (!p.srcGroups.empty())
            ofs << targetPath(p) << " ";
    ofs << lf;
    for (const FgConsProj & proj : sln.projects)
        if (!proj.srcGroups.empty())
            consProj(ofs,proj,sln);
    ofs << ".PHONY: clean cleanObjs cleanTargs" << lf
        << "clean:" << lf
        << "\trm -r $(BUILDIR)" << lf;
    return fgDump(ofs.str(),fname);
}

bool
consMakefile(
    FgBuildOS               os,
    FgCompiler              compiler,
    FgArch                  arch,
    bool                    debug,      // release if false
    const string &          fnameBuild) // includes this makefile
{
    string          debrel = debug ? "debug" : "release",
                    osStr = (os == FgBuildOS::linux) ? "linux" : fgToStr(os),
                    makefile = fgCat(fgSvec<string>("Makefile",osStr,fgToStr(arch),fgToStr(compiler),debrel),"_"),
                    cc,
                    cxx,
                    link,
                    dllext,             // DLL file extension (including dot)
                    dllarg;             // DLL link args up to and adjacent to target name
    FgStrs          cflags,
                    cxxflags,           // cpp flags in *addition* to 'cflags'
                    lflags;
    // NOTE that warning and include flags are added per-project, not here.
    if (os == FgBuildOS::ios) {
        if (arch == FgArch::x64)
            cflags.push_back("-arch x86_64");
        else if (arch == FgArch::arm64)
            cflags.push_back("-arch arm64");
        else if (arch == FgArch::arm64e)
            cflags.push_back("-arch arm64e");
        else if (arch == FgArch::armv7)
            cflags.push_back("-arch armv7");
        else
            fgThrow("architecture not supported on iOS",arch);
    }
    else if (os == FgBuildOS::android) {
        if (arch == FgArch::x86)
            cflags.push_back("-arch x86");
        else if (arch == FgArch::x64)
            cflags.push_back("-arch x86_64");
        else if (arch == FgArch::armv7_a)
            cflags.push_back("-arch armv7-a");
        else if (arch == FgArch::arm64_v8a)
            cflags.push_back("-arch arm64-v8a");
        else
            fgThrow("architecture not supported on Android",arch);
    }
    else {
        if (arch == FgArch::x86)
            cflags.push_back("-m32");
        else if (arch == FgArch::x64)
            cflags.push_back("-m64");
        else
            fgThrow("architecture not supported on native *nix",arch);
    }
    if (debug) {
        cflags.push_back("-g");             // generate debug info
        cflags.push_back("-O1");            // minimal optimization
        cflags.push_back("-D_DEBUG");
    }
    else {
        cflags.push_back("-DNDEBUG");
    }
    cxxflags.push_back("-std=c++11");
    if (compiler == FgCompiler::clang) {
        // uses libc++. On Linux, g++ is the GNU linker and uses libstdc++.
        cc = "clang";                   // CLANG C compiler
        cxx = "clang++";                // CLANG C++ compiler
        // On Ubuntu, clang++ will dispatch to g++ (GNU linker) which uses libstdc++ (GNU std libs),
        // after ensuring appropriate linkage clang libs (eg. AddressSanitizer).
        // On MacOS, clang++ will link with clang/Apple's non-GPL 'libc++'.
        link = "clang++";
        // Ensures no common symbols across all libraries, required for all files which
        // are part of a DLL so we just always use it:
        cflags.push_back("-fno-common");
        // clang defaults to 256 which can cause problems with boost::serialization exceeding
        // template depth limit, so set to same limit used by gcc and vs:
        cxxflags.push_back("-ftemplate-depth=1024");
        if (!debug)
            cflags.push_back("-Ofast");         // O3 plus fast floating point opts.
        if (debug && (os == FgBuildOS::linux)) {
            cflags.push_back("-fsanitize=address");
            lflags.push_back("-fsanitize=address");
        }
    }
    else if (compiler == FgCompiler::gcc) {
        // gcc will call gnu c compiler or g++ depending on source type:
        cc = "gcc";
        cxx = "gcc";
        // g++ calls ld but it adds arguments for the appropriate standard libraries so it's easier.
        // It uses shared libaries by default and static linking appears quite difficult to get right:
        link = "g++";
        if (!debug) {
            // -march=corei7 actually slowed down nrrjohnverts a smidgen.
            // -msse3 made no difference on some speed tests.
            cflags.push_back("-O3");
            cflags.push_back("-ffast-math");    // Needed for auto-vectorization as well as minor speedup.
        }
    }
    else if (compiler == FgCompiler::icpc) {
        cc = "icc";
        cxx = "icpc";
        link = "icpc";
        // Disable remark: Inlining inhibited by limit max-size / max-total-size
        // Disable remark: To get full report use -qopt-report=4 -qopt-report-phase ipo
        cflags.push_back("-diag-disable=11074,11076");
        if (!debug)
            cflags.push_back("-Ofast");     // -fast-transcendentals made no diff on model corr speed test
        if (arch == FgArch::x64)
            cflags.push_back("-xavx");      // Use AVX (and SSE).
    }
    else
        fgThrow("Don't know how to create makefile for compiler",compiler);
    if (os == FgBuildOS::macos) {
        lflags.push_back("-framework Carbon");
        // -Wl, passes the args on to the linker
        // -install_name tells binaries linking with this dynlib where to look for it at
        //    runtime. Without this the linker stupidly use the path given in the CL:
        dllext = ".dylib";
        dllarg = "-dynamiclib -Wl,-install_name,@executable_path/";
    }
    else if (os == FgBuildOS::ios) {
        bool        isSim = (arch == FgArch::x64);
        string      osSim = isSim ? "Simulator" : "OS",
                    sys = "/Applications/Xcode.app/Contents/Developer/Platforms/iPhone"+osSim+".platform/Developer/SDKs/iPhone"+osSim+".sdk";
        if (isSim)
            cflags.push_back("-mios-simulator-version-min=7.1");
        else
            cflags.push_back("-miphoneos-version-min=7.1");         // Minimum version with arm64 support
        cxxflags.push_back("-stdlib=libc++");
        cflags.push_back("-isysroot " + sys);
        // DO NOT add sysroot/usr/include as this is C-only and will cause errors with C++:
        // BAD: cflags.push_back("-iwithsysroot /usr/include/");
        // BAD: cflags.push_back("-I"+sys+"/usr/include/");
        cflags.push_back("-fembed-bitcode");
        cflags.push_back("-DENABLE_BITCODE");
        cflags.push_back("-fPIC");
        cflags.push_back("-fno-strict-aliasing");
    }
    else if (os == FgBuildOS::android) {
        cflags.push_back("-DANDROID");
        cflags.push_back("-fPIE");
    }
    else {      // Linux
        // -fPIC: (position indepdent code) is required for all object files which are 
        // part of a shared library, so we just always use it (except MacOS where it's the default)
        cflags.push_back("-fPIC");
        lflags.push_back("-pthread");       // Link to unix 'pthread' library
        lflags.push_back("-z origin");      // Appears necessary for -rpath to work
        // Tells the runtime linker that the soname is relative to the directory of the currently executing
        // binary (we must escape $ with $$ for make and escape it all with single-quotes for the shell):
        lflags.push_back("-Wl,-rpath='$$ORIGIN'");
        dllext = ".so";
        // -shared is necessary here even though it's supposedly the default.
        // -soname is the name that will be used when dependent binaries are linked to
        //    this dll to search for at run-time. The $ORIGIN method below requires it.
        dllarg = "-shared -Wl,-soname,";
    }
    bool    native = fgContains(fgBuildNativeOSs(),os);
    ostringstream   ofs;
    ofs << "CC = " << cc << lf
        << "CXX = " << cxx << lf
        << "LINK = " << link << lf
        << "CFLAGS = " << fgCat(cflags," ") << lf
        << "CCFLAGS = $(CFLAGS)" << lf
        << "CXXFLAGS = $(CFLAGS) " << fgCat(cxxflags," ") << lf;
    if (native)
        ofs
            << "LFLAGS = " << fgCat(lflags," ") << lf
            << "DLLEXT = " << dllext << lf
            << "DLLARG = " << dllarg << lf
            << "BINDIR = ../" << fgRelBin(os,arch,compiler,!debug) << lf
            << "BBINDIR = ../" << "bin/" << os << "/" << arch << "/" << lf;
    ofs
        << "BUILDIR = ../build_" << os << "/" << arch << "/" << compiler << "/" << debrel << "/" << lf
        << "include " << fnameBuild << lf;
    return fgDump(ofs.str(),makefile);
}

bool
consMakefileCross(const FgConsSolution & sln,FgBuildOS os)
{
    FgCompiler          compiler = fgBuildCompilers(os)[0];
    FgArchs             archs = fgBuildArchitectures(os);
    ostringstream       oss;
    oss << "OUTDIR = ../build_" << os << "/" << lf
        << ".PHONY: all FORCE clean" << lf
        // Rely on 'make' following order given for targets to ensure recurse make calls.
        // Could perhaps have thin libs depend on phony architecture targets ("%.a : arch")
        // to trigger recursive makes:
        << "all: ";
    for (const FgConsProj & p : sln.projects)
        if (!p.srcGroups.empty())
            oss << "$(OUTDIR)"+p.name+".a" << " ";
    oss << lf;
    for (const FgConsProj & p : sln.projects) {
        if (!p.srcGroups.empty()) {
            if (os == FgBuildOS::ios) {
                string          fatLib = "$(OUTDIR)"+p.name+".a";
                string          srcLibs;
                for (FgArch arch : archs)
                    srcLibs += " $(OUTDIR)" + fgToStr(arch) + "/" + fgToStr(compiler) + "/release/" + p.name + ".a";
                oss << fatLib << ":" << srcLibs << lf;
                // Using 'xcrun' here avoid problems according to some sources.
                oss << "\txcrun -sdk iphoneos lipo -create -output " << fatLib << srcLibs << lf;
            }
            else if (os == FgBuildOS::android) {
                string          fatLib = "$(OUTDIR)"+p.name+".aar";
                string          srcLibs;
                for (FgArch arch : archs)
                    srcLibs += " $(OUTDIR)" + fgToStr(arch) + "/" + fgToStr(compiler) + "/release/" + p.name + ".aar";
                oss << fatLib << ":" << srcLibs << lf;
                // TODO: build android AAR from .o files here:
                oss << "\techo This Makefile is not yet working" << lf;
            }
            else
                fgThrow("consMakefileCross unexpected OS",fgToStr(os));
        }
    }
    for (FgArch arch : archs) {
        // The % character is a glob which indicates that all such files depend on the same command:
        oss << "$(OUTDIR)" << arch << "/" << compiler << "/release/%.a: FORCE" << lf
            // The recursive make command is smart enough to handle parent CL options such as -n
            << "\t$(MAKE) -j4 -f Makefile_" << os << "_" << arch << "_" << compiler << "_release" << lf;
    }
    oss << "FORCE:" << lf
        << "clean:" << lf
        << "\trm -r $(OUTDIR)" << lf;
    return fgDump(oss.str(),"Makefile_"+fgToStr(os));
}

}

bool
fgConsNativeMakefiles(const FgConsSolution & sln)
{
    FGASSERT(sln.type == FgConsType::nix);
    bool            changed = false;
    string          fnameAll = "make_all.mk";
    changed = constIncludeFileNative(sln,fnameAll) || changed;
    changed = consMakefile(FgBuildOS::linux,FgCompiler::clang,FgArch::x64,true,fnameAll) || changed;
    changed = consMakefile(FgBuildOS::linux,FgCompiler::clang,FgArch::x64,false,fnameAll) || changed;
    changed = consMakefile(FgBuildOS::linux,FgCompiler::gcc,FgArch::x64,true,fnameAll) || changed;
    changed = consMakefile(FgBuildOS::linux,FgCompiler::gcc,FgArch::x64,false,fnameAll) || changed;
    changed = consMakefile(FgBuildOS::linux,FgCompiler::icpc,FgArch::x64,true,fnameAll) || changed;
    changed = consMakefile(FgBuildOS::linux,FgCompiler::icpc,FgArch::x64,false,fnameAll) || changed;
    changed = consMakefile(FgBuildOS::macos,FgCompiler::clang,FgArch::x64,true,fnameAll) || changed;
    changed = consMakefile(FgBuildOS::macos,FgCompiler::clang,FgArch::x64,false,fnameAll) || changed;
    return changed;
}

bool
fgConsCrossMakefiles(const FgConsSolution & sln)
{
    FGASSERT(sln.type == FgConsType::cross);
    // Only build static SDK libraries for cross-compile targets:
    FgConsSolution      slnXC(FgConsType::cross);
    for (const FgConsProj & p : sln.projects)
        if (p.isStaticLib())
            slnXC.projects.push_back(p);
    // We build separate thin libs then combine with lipo. It appears possible to build
    // fat object files and 'libtool' those, which would make for simpler makefiles, however
    // we need different compile arguments for simulator vs. deployment compiles:
    bool            changed = false;
    string          fnameLibs = "make_libs.mk";
    changed = constIncludeFileBox(slnXC,fnameLibs) || changed;
    for (FgBuildOS bs : fgBuildCrossCompileOSs()) {
        for (FgArch arch : fgBuildArchitectures(bs))
            changed = consMakefile(bs,fgBuildCompilers(bs)[0],arch,false,fnameLibs) || changed;
        changed = consMakefileCross(slnXC,bs) || changed;
    }
    return changed;
}

// */
