//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Dec 10, 2011
//
// Notes:
// * Each command line of a makefile runs in a separate shell.
// * Multithreaded make can be achieved using 'make -j<numthreads>'

#include "stdafx.h"
#include "FgCons.hpp"
#include "FgOut.hpp"
#include "FgException.hpp"
#include "FgFileSystem.hpp"

using namespace std;

static const char lf = 0x0A;

static
void
targets(
    ostream &              ofs,
    const string &          prjName,
    const FgConsSrcGroup &  grp)
{
    vector<pair<string,string> >    srcNames;
    for (size_t ii=0; ii<grp.files.size(); ++ii)
    {
        FgPath      p(grp.files[ii]);
        string      ext = p.ext.ascii();
        if ((ext == "cpp") || (ext == "c"))
            srcNames.push_back(make_pair(p.base.ascii(),ext));
    }
    string  odir = "$(ODIR" + prjName + ")" + fgReplace(grp.dir,'/','_');
    for (size_t ii=0; ii<srcNames.size(); ++ii)
        ofs << odir << srcNames[ii].first << ".o ";
}

static
void
group(
    ostream &              ofs,
    const string &          prjName,
    const FgConsSrcGroup &  grp)
{
    string  odir = "$(ODIR" + prjName + ")" + fgReplace(grp.dir,'/','_'),
            sdir = "$(SDIR" + prjName + ")" + grp.dir;
    for (size_t ii=0; ii<grp.files.size(); ++ii) {
        FgPath      path(grp.files[ii]);
        string      ext = path.ext.ascii();
        if ((ext == "cpp") || (ext == "c")) {
            string      srcName = grp.files[ii],
                        objName = path.base.ascii() + ".o",
                        compile = (ext == "cpp") ? "$(CPPC)" : "$(CC)";
            ofs << odir << objName << ": " << sdir << srcName << " $(INCS" << prjName << ")" << lf
                << "\t" << compile << " -o " << odir << objName << " -c $(CFLAGS" << prjName << ") "
                << sdir << srcName << lf;
        }
    }
}

static
string
dllExt(const string & os)
{
    if (os == "osx")
        return ".dylib";
    else if (os == "ubuntu")
        return ".so";
    else
        fgThrow("Don't know DLL extension for OS",os);
    return string();
}

static
string
targetPath(const FgConsProj & prj,const string & os)
{
    string  path = "$(BIN)" + prj.name;
    if (!prj.app) {
        if (prj.dll)
            path += dllExt(os);
        else
            path += ".a";
    }
    return path;
}

static
void
linkLibs(
    ostream &              ofs,
    const FgConsProj &      prj,
    const FgConsSolution &  sln,
    const string &          os)
{
    // For unix, order of linkage is actually important, since compiling on unix
    // is a miserable PITA:
    for (size_t ii=prj.dllDeps.size(); ii>0; --ii)
        ofs << "$(BIN)" << prj.dllDeps[ii-1] << dllExt(os) << " ";
    for (size_t ii=prj.lnkDeps.size(); ii>0; --ii)
        for (size_t jj=0; jj<sln.projects.size(); ++jj)
            if (sln.projects[jj].name == prj.lnkDeps[ii-1])
                ofs << targetPath(sln.projects[jj],os) << " ";
}

// Collapse common case of 'ThisLib/../OtherLib':
static
string
collapse(
    const string &  prjName,
    const string &  incDir)
{
    if (fgStartsWith(incDir,"../"))
        return string(incDir.begin()+3,incDir.end());
    else
        return prjName + "/" + incDir;
}

static
void
proj(
    ostream &          ofs,
    const FgConsProj &  prj,
    const FgConsSolution & sln,
    const string &      os,
    const string &      compiler)
{
    ofs << "CFLAGS" << prj.name << " = $(CFLAGS)";
    if (prj.warn < 3)
        ofs << " -w";                   // disables warnings (for all compilers)
    else
        ofs << " -Wextra";
    if (prj.warn == 4) {
        if (compiler == "clang") {
            ofs << " -Wno-unused-local-typedef";    // boost static assert issue with clang
        }
        else if (compiler == "gcc") {
            ofs << " -Wno-unused-result";
        }
    }
    for (size_t ii=0; ii<prj.defs.size(); ++ii)
        ofs << " -D" << prj.defs[ii];
    for (size_t ii=0; ii<prj.incDirs.size(); ++ii) {
        string      libName = collapse(prj.name,prj.incDirs[ii]);
        if (fgStartsWith(libName,"LibTpBoost"))
            // Tell gcc/clang/icpc to treat as system lib and not give warnings. Doesn't work so well.
            ofs << " -isystem" << libName;
        else
            ofs << " -I" << libName;
    }
    ofs << lf
        << "SDIR" << prj.name << " = " << prj.name << '/' << prj.srcBaseDir << lf
        << "ODIR" << prj.name << " = " << prj.name << "/$(CONFIG)" << lf
        << "$(shell mkdir -p $(ODIR" << prj.name << "))" << lf;
    // Make compiles dependent on ALL possible .hpp files since we're not about
    // to go parsing the source code to find the specific dependencies:
    ofs << "INCS" << prj.name << " := ";
    for (size_t ii=0; ii<prj.incDirs.size(); ++ii)
        ofs << "$(wildcard " << collapse(prj.name,prj.incDirs[ii]) << "*.hpp) ";
    ofs << "$(wildcard " << prj.name << '/' << prj.srcBaseDir << "*.hpp)" << lf;
    ofs << targetPath(prj,os) << ": ";
    for (size_t ii=0; ii<prj.srcGroups.size(); ++ii)
        targets(ofs,prj.name,prj.srcGroups[ii]);
    if (prj.app || prj.dll)
        linkLibs(ofs,prj,sln,os);
    ofs << lf;
    if (prj.app || prj.dll)
    {
        ofs << "\t$(LINK) -o $(BIN)" << prj.name;
        if (prj.dll) {
            if (os == "osx")
                // -Wl, passes the args on to the linker
                // -install_name tells binaries linking with this dynlib where to look for it at
                //    runtime. Without this the linker stupidly use the path given in the CL:
                ofs << ".dylib -dynamiclib -Wl,-install_name,@executable_path/" << prj.name << ".dylib";
            else if (os == "ubuntu")
                // -shared is necessary here even though it's supposedly the default.
                // -soname is the name that will be used when dependent binaries are linked to
                //    this dll to search for at run-time. The $ORIGIN method below requires it.
                ofs << ".so -shared -Wl,-soname," << prj.name << ".so";
            else
                fgThrow("Don't know DLL creation args for OS",os);
        }
        ofs << " ";
        if (os == "ubuntu")     // both gcc and clang use g++ linker on ubuntu:
            // -pthread sets gcc flags for both preprocessor and linker for use of pthreads library.
            //     Not used by clang.
            // -z origin is idiotically necessary for -rpath to work at all:
            // $ORIGIN tells the runtime linker that the soname is relative to the directory
            //     of the currently executing binary (we must escape $ with $$ for make and
            //     escape it all with single-quotes for the shell).
            ofs << "-pthread -z origin -Wl,-rpath='$$ORIGIN' ";
        for (size_t ii=0; ii<prj.srcGroups.size(); ++ii)
            targets(ofs,prj.name,prj.srcGroups[ii]);
        linkLibs(ofs,prj,sln,os);
        if (os == "osx")
            ofs << " -framework Carbon ";
        else if (os == "ubuntu")
            // -lrt links to the librt.so 'real-time' shared library (contains clock_gettime)
            ofs << " -lrt ";
        else
            fgThrow("Don't know standard lib linkage for OS",os);
    }
    else    // Static library
    {
        // ar options: r - replace .o files in archive, c - create if doesn't exist
        ofs << "\tar rc " << targetPath(prj,os) << " ";
        for (size_t ii=0; ii<prj.srcGroups.size(); ++ii)
            targets(ofs,prj.name,prj.srcGroups[ii]);
        ofs << lf << "\tranlib " << targetPath(prj,os);
    }
    ofs << lf;
    for (size_t ii=0; ii<prj.srcGroups.size(); ++ii)
        group(ofs,prj.name,prj.srcGroups[ii]);
}

static
bool
consMakefile(
    const FgConsSolution &  sln,
    const string &          os,
    const string &          compiler,
    bool                    x64,        // 32 bit if false
    bool                    debug)      // release if false
{
    string          bits = x64 ? "64" : "32",
                    config = debug ? "debug" : "release",
                    makefile = fgCat(fgSvec<string>("Makefile",os,compiler,bits,config),".");
    ostringstream   ofs(ios::binary);    // ios::binary keeps newline = LF for *nix
    if (compiler == "clang") {
        // Will be changing to 'clang++ -Wno-c++11-extensions' if I can get ImageMagick to compile
        // without errors (or get rid of it). As of Yosemite (10.10) g++ is just clang++ and 
        // uses the same libc++, however there's some kind of residual difference causing assignements
        // from void* to <type>* to yield errors if clang++ is used directly:
        ofs << "CPPC = clang -std=c++11 " << lf
            << "CC = clang " << lf
            << "LINK = g++ " << lf
            << "CFLAGS = "
                // Ensures no common symbols across all libraries, required for all files which
                // are part of a DLL so we just always use it:
                "-fno-common "
                // clang defaults to 256 which can cause problems with boost::serialization exceeding
                // template depth limit, so set to same limit used by gcc and vs:
                "-ftemplate-depth=1024 ";
        if (os == "ubuntu")
            ofs << "-fPIC ";
        if (debug)
            ofs << "-g -O0 -D_DEBUG ";  // -g: generate debug info
        else
            ofs << "-Ofast ";   // == O3 plus fast floating point opts.
    }
    else if (compiler == "gcc") {
        // gcc will call gnu c compiler or g++ depending on source type:
        ofs << "CPPC = gcc -std=c++11 " << lf
            << "CC = gcc " << lf
            // g++ calls ld but it adds arguments for the appropriate standard
            // libraries so it's easier. It uses shared libaries by default and
            // static linking is apparently quite difficult to get right:
            << "LINK = g++" << lf
            // -fPIC: (position indepdent code) is required for all object files which are 
            // part of a shared library, so we just always use it (except on OSX where it's the default)
            << "CFLAGS = -fPIC ";
        if (x64)
            ofs << "-m64 ";
        else
            ofs << "-m32 ";
        if (debug)
            // -g: generate debug info
            ofs << "-g -O0 -D_DEBUG ";
        else
            ofs << "-O3 "           // -march=corei7 actually slowed down nrrjohnverts a smidgen.
                "-ffast-math ";     // Needed for auto-vectorization as well as minor speedup.
                                    // -msse3 made no difference on some speed tests.
    }
    else if (compiler == "icpc") {
        ofs << "CPPC = icpc -std=c++11 " << lf
            << "CC = icc " << lf
            << "LINK = icpc" << lf
            << "CFLAGS = -fPIC "
               "-diag-disable=11074,"   // Disable remark: Inlining inhibited by limit max-size / max-total-size
               "11076 ";                // Disable remark: To get full report use -qopt-report=4 -qopt-report-phase ipo
        if (x64)
            ofs << "-mavx ";            // Use AVX and all of SSE. This build is for server-side only !
        else
            ofs << "-m32 ";
        if (debug)
            ofs << "-g -D_DEBUG";
        else
            ofs << "-Ofast ";           // -fast-transcendentals made no diff on model corr speed test
    }
    else
        fgThrow("Don't know how to create makefile for compiler",compiler);
    ofs << lf
        << "CONFIG = " << compiler << "/" << bits <<  "/" << config << "/" << lf
        << "BIN = ../bin/" << os << "/$(CONFIG)" << lf
        << "$(shell mkdir -p $(BIN))" << lf
        << "all: ";
    for (size_t ii=0; ii<sln.projects.size(); ++ii)
        if (sln.projects[ii].app)
            ofs << targetPath(sln.projects[ii],os) << " ";
    ofs << lf;
    for (size_t ii=0; ii<sln.projects.size(); ++ii)
        proj(ofs,sln.projects[ii],sln,os,compiler);
    ofs << ".PHONY: clean" << lf
        << "clean: cleanObjs cleanTargs" << lf
        << "cleanObjs:" << lf;
    for (size_t ii=0; ii<sln.projects.size(); ++ii)
        ofs << "\trm -r " << sln.projects[ii].name << "/" << compiler << lf;
    ofs << "cleanTargs:" << lf
        << "\trm -r $(BIN)" << lf;
    string      body = ofs.str();
    if (fgExists(makefile) && (body == fgSlurp(makefile)))
        return false;
    fgDump(body,makefile);
    return true;
}

bool
fgConsMakefiles(FgConsSolution sln)
{
    bool        anyChanged = false,
                change;
    // Control flow to ensure 'consMakefile' is always run regardless of previous changes is a little awkward:
    change = consMakefile(sln,"ubuntu","clang",true,true); anyChanged = anyChanged || change;
    change = consMakefile(sln,"ubuntu","clang",true,false); anyChanged = anyChanged || change;
    change = consMakefile(sln,"ubuntu","gcc",true,true); anyChanged = anyChanged || change;
    change = consMakefile(sln,"ubuntu","gcc",true,false); anyChanged = anyChanged || change;
    change = consMakefile(sln,"ubuntu","icpc",true,true); anyChanged = anyChanged || change;
    change = consMakefile(sln,"ubuntu","icpc",true,false); anyChanged = anyChanged || change;
    change = consMakefile(sln,"osx","clang",true,true); anyChanged = anyChanged || change;
    change = consMakefile(sln,"osx","clang",true,false); anyChanged = anyChanged || change;
    return anyChanged;
}

// */
