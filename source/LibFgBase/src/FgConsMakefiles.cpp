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

const char  lf = 0x0A;

static
void
targets(
    ofstream &              ofs,
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
    ofstream &              ofs,
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
    string  odir = "$(ODIR" + prjName + ")" + fgReplace(grp.dir,'/','_'),
            sdir = "$(SDIR" + prjName + ")" + grp.dir;
    for (size_t ii=0; ii<srcNames.size(); ++ii)
    {
        string  srcName = srcNames[ii].first + '.' + srcNames[ii].second,
                objName = srcNames[ii].first + ".o";
        ofs << odir << objName << ": " << sdir << srcName << " $(INCS" << prjName << ")" << lf
            << "\t$(CC) -o " << odir << objName << " -c $(CFLAGS" << prjName << ") "
            << sdir << srcName << lf;
    }
}

static
string
target(const FgConsProj & prj,bool osx)
{
    string  path = "$(BIN)" + prj.name;
    if (prj.app)
        return path;
    else {
        if (prj.dll) {
            if (osx)
                return path + ".dylib";
            else
                return path + ".so"; }
        else
            return path + ".a"; }
}

static
void
linkLibs(
    ofstream &              ofs,
    const FgConsProj &      prj,
    const FgConsSolution &  sln,
    bool                    osx)
{
    // For unix, order of linkage is actually important, since compiling on unix
    // is a miserable PITA:
    for (size_t ii=prj.dllDeps.size(); ii>0; --ii) {
        ofs << "$(BIN)" << prj.dllDeps[ii-1];
        if (osx)
            ofs << ".dylib ";
        else
            ofs << ".so "; }
    for (size_t ii=prj.lnkDeps.size(); ii>0; --ii)
        for (size_t jj=0; jj<sln.projects.size(); ++jj)
            if (sln.projects[jj].name == prj.lnkDeps[ii-1])
                ofs << target(sln.projects[jj],osx) << " ";
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
    ofstream &          ofs,
    const FgConsProj &  prj,
    const FgConsSolution & sln,
    bool                osx)    // false = ubuntu
{
    ofs << "CFLAGS" << prj.name << " = $(CFLAGS)";
    if (prj.warn < 3)
        ofs << " -w";
    else
        ofs << " -Wall";
    if (prj.warn == 4) {
        if (!osx) {
            ofs << " -Wextra";              // causes boost 'unused-parameter' warnings with clang
            ofs << " -Wno-unused-result";   // This flag doesn't exist in gcc4.2 (OSX)
        }
    }
    for (size_t ii=0; ii<prj.defs.size(); ++ii)
        ofs << " -D" << prj.defs[ii];
    ofs << " -DBOOST_THREAD_POSIX";
    for (size_t ii=0; ii<prj.incDirs.size(); ++ii) {
        string      libName = collapse(prj.name,prj.incDirs[ii]);
        if (fgStartsWith(libName,"LibTpBoost"))
            // Tell gcc/clang to treat as system lib and not give warnings. Doesn't work so well.
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
    ofs << target(prj,osx) << ": ";
    for (size_t ii=0; ii<prj.srcGroups.size(); ++ii)
        targets(ofs,prj.name,prj.srcGroups[ii]);
    if (prj.app || prj.dll)
        linkLibs(ofs,prj,sln,osx);
    ofs << lf;
    if (prj.app || prj.dll)
    {
        ofs << "\t$(LINK) -o $(BIN)" << prj.name;
        if (prj.dll) {
            if (osx)
                // -Wl, passes the args on to the linker
                // -install_name tells binaries linking with this dynlib where to look for it at
                //    runtime. Without this the linker stupidly use the path given in the CL:
                ofs << ".dylib -dynamiclib -Wl,-install_name,@executable_path/" << prj.name << ".dylib";
            else
                // -shared is necessary here even though it's supposedly the default.
                // -soname is the name that will be used when dependent binaries are linked to
                //    this dll to search for at run-time. The $ORIGIN method below requires it.
                ofs << ".so -shared -Wl,-soname," << prj.name << ".so"; }
        ofs << " ";
        if (!osx)
            // -pthread sets gcc flags for both preprocessor and linker for use of pthreads library.
            //     Not used by clang.
            // -z origin is idiotically necessary for -rpath to work at all:
            // $ORIGIN tells the runtime linker that the soname is relative to the directory
            //     of the currently executing binary (we must escape $ with $$ for make and
            //     escape it all with single-quotes for the shell).
            ofs << "-pthread -z origin -Wl,-rpath='$$ORIGIN' ";
        for (size_t ii=0; ii<prj.srcGroups.size(); ++ii)
            targets(ofs,prj.name,prj.srcGroups[ii]);
        linkLibs(ofs,prj,sln,osx);
        if (osx)
            ofs << " -framework Carbon ";
        else
            // -lrt links to the librt.so 'real-time' shared library (contains clock_gettime)
            ofs << " -lrt ";
    }
    else    // Static library
    {
        // ar options: r - replace .o files in archive, c - create if doesn't exist
        ofs << "\tar rc " << target(prj,osx) << " ";
        for (size_t ii=0; ii<prj.srcGroups.size(); ++ii)
            targets(ofs,prj.name,prj.srcGroups[ii]);
        ofs << lf << "\tranlib " << target(prj,osx);
    }
    ofs << lf;
    for (size_t ii=0; ii<prj.srcGroups.size(); ++ii)
        group(ofs,prj.name,prj.srcGroups[ii]);
}

static
void
consGcc(
    const FgConsSolution &  sln,
    const string &          mfName,
    bool                    x64,
    bool                    debug,
    bool                    osx)        // false = ubuntu
{
    ofstream    ofs(mfName.c_str(),ios::binary);    // ios::binary keeps newline = LF for *nix
    if (osx) {
        // Will be changing to 'clang++ -Wno-c++11-extensions' if I can get ImageMagick to compile
        // without errors (or get rid of it). As of Yosemite (10.10) g++ is just clang++ and 
        // uses the same libc++, however there's some kind of residual difference causing assignements
        // from void* to <type>* to yield errors if clang++ is used directly:
        ofs << "CC = gcc " << lf
            << "LINK = g++ " << lf
            // Avoid the verbose errors from boost when compiling without C++11 support:
            << "CFLAGS = -Wno-c++11-extensions ";
        // Ensures no common symbols across all libraries, required for all files which
        // are part of a DLL so we just always use it (OSX specific)
        ofs << "-fno-common ";
        // clang default template depth of 256 doesn't work with boost serializtion:
        ofs << "-ftemplate-depth=512 ";
    }
    else {
        // gcc will call gnu c compiler or g++ depending on source type:
        ofs << "CC = gcc" << lf
            // g++ calls ld but it adds arguments for the appropriate standard
            // libraries so it's easier. It uses shared libaries by default and
            // static linking is apparently quite difficult to get right:
            << "LINK = g++" << lf
            << "CFLAGS = ";
        // -fPIC: (position indepdent code) is required for all object files which are 
        // part of a DLL, so we just always use it (except on OSX where it's the default)
        ofs << "-fPIC ";
    }
    if (x64)
        ofs << "-m64 ";
    else
        ofs << "-m32 ";
    if (debug)
        // -g: generate debug info
        ofs << "-g -O0 -D_DEBUG ";
    else
        ofs << "-O3 ";  // -march=corei7 actually slowed down nrrjohnverts a smidgen.
    string os = (osx ? "osx" : "ubuntu");
    ofs << lf
        << "CONFIG = gcc/"
        << (x64 ? "64/" : "32/")
        << (debug ? "debug/" : "release/") << lf
        << "BIN = ../bin/" << os << "/$(CONFIG)" << lf
        << "$(shell mkdir -p $(BIN))" << lf
        << "all: ";
    for (size_t ii=0; ii<sln.projects.size(); ++ii)
        if (sln.projects[ii].app)
            ofs << target(sln.projects[ii],osx) << " ";
    ofs << lf;
    for (size_t ii=0; ii<sln.projects.size(); ++ii)
        proj(ofs,sln.projects[ii],sln,osx);
    ofs << ".PHONY: clean" << lf
        << "clean: cleanObjs cleanTargs" << lf
        << "cleanObjs:" << lf;
    for (size_t ii=0; ii<sln.projects.size(); ++ii)
        ofs << "\trm -r " << sln.projects[ii].name << "/gcc" << lf;
    ofs << "cleanTargs:" << lf
        << "\trm -r $(BIN)" << lf;
}

void
fgConsMakefiles(FgConsSolution sln)
{
    // gcc4.6 has some problem with 32 bit. Ignore for now:
    //cons(sln,"Makefile.ubuntu.gcc.32.release",false,false);
    //cons(sln,"Makefile.ubuntu.gcc.32.debug",false,true);
    consGcc(sln,"Makefile.ubuntu.gcc.64.debug",true,true,false);
    consGcc(sln,"Makefile.ubuntu.gcc.64.release",true,false,false);
    consGcc(sln,"Makefile.osx.gcc.64.debug",true,true,true);
    consGcc(sln,"Makefile.osx.gcc.64.release",true,false,true);
    //consClang(sln,"Makefile.osx.clang.64.debug",true,true,true);
    //consClang(sln,"Makefile.osx.clang.64.release",true,false,true);
}

// */
