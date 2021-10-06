//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgBuild.hpp"
#include "FgOut.hpp"
#include "FgException.hpp"
#include "FgFileSystem.hpp"
#include "FgStdStream.hpp"
#include "FgStdSet.hpp"

using namespace std;

namespace Fg {

namespace {

char const lf = 0x0A;       // Unix line ending

void
targets(
    ostream &               ofs,
    string const &          prjName,
    const ConsSrcDir &    grp)
{
    vector<pair<string,string> >    srcNames;
    for (size_t ii=0; ii<grp.files.size(); ++ii) {
        Path      p(grp.files[ii]);
        string      ext = p.ext.ascii();
        if ((ext == "cpp") || (ext == "c"))
            srcNames.push_back(make_pair(p.base.ascii(),ext));
    }
    string  odir = "$(ODIR" + prjName + ")" + replaceAll(grp.dir,'/','_');
    for (size_t ii=0; ii<srcNames.size(); ++ii)
        ofs << odir << srcNames[ii].first << ".o ";
}

void
group(
    ostream &               ofs,
    string const &          prjName,
    const ConsSrcDir &  grp)
{
    string  odir = "$(ODIR" + prjName + ")" + replaceAll(grp.dir,'/','_'),
            sdir = "$(SDIR" + prjName + ")" + grp.dir;
    for (size_t ii=0; ii<grp.files.size(); ++ii) {
        Path      path(grp.files[ii]);
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
targetPath(ConsProj const & prj)
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
    ConsProj const &      prj,
    ConsSolution const &  sln,
    bool                    binaryOnly)     // Include binary-only deps
{
    // gcc link is single-pass; order of linkage is important:
    Strings      libDeps = sln.getLnkDeps(prj.name);
    for (string const & name : libDeps) {
        if (sln.contains(name)) {
            ConsProj const &  p = sln.projByName(name);
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
    string const &  prjName,
    string const &  incDir)
{
    if (beginsWith(incDir,"../"))
        return string(incDir.begin()+3,incDir.end());
    else
        return prjName + "/" + incDir;
}

void
consProj(
    ostream &               ofs,
    ConsProj const &        prj,
    ConsSolution const &    sln)
{
    Strings                 incDirs = sln.getIncludes(prj.name,false),
                            headerDirs = sln.getIncludes(prj.name,true),
                            defs = sln.getDefs(prj.name);
    ofs << "FLAGS" << prj.name << " = ";
    if (prj.warn < 3)
        ofs << " -w";                   // disables warnings (for all compilers)
    else
        ofs << " -Wall -Wextra";        // Amazingly, the latter does not encompass all of the former
    for (string const & def : defs)
        ofs << " -D" << def;
    for (string const & id : incDirs) {
        string      libName = collapse(prj.name,id);
        if (beginsWith(libName,"LibFg"))
            ofs << " -I" << libName;
        else 
            ofs << " -isystem " << libName;      // Tell compiler not to emit warnings for these includes
    }
    ofs << lf
        << "SDIR" << prj.name << " = " << prj.name << '/' << prj.baseDir << lf
        << "ODIR" << prj.name << " = $(BUILDIR)" << prj.name << "/" << lf
        << "$(shell mkdir -p $(ODIR" << prj.name << "))" << lf;
    // Make compiles dependent on ALL possible .hpp files since we're not about
    // to go parsing the source code to find the specific dependencies:
    ofs << "INCS" << prj.name << " := ";
    for (string const & id : headerDirs)
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
        ofs << "\t$(AR) rc " << targetPath(prj) << " ";
        for (size_t ii=0; ii<prj.srcGroups.size(); ++ii)
            targets(ofs,prj.name,prj.srcGroups[ii]);
        ofs << lf << "\t$(RANLIB) " << targetPath(prj);
    }
    ofs << lf;
    for (size_t ii=0; ii<prj.srcGroups.size(); ++ii)
        group(ofs,prj.name,prj.srcGroups[ii]);
}

bool
constIncludeFileNative(ConsSolution const & sln,string const & fname)
{
    set<string>         prereqs;        // All project names which are depended on
    for (ConsProj const & p : sln.projects)
        for (ProjDep const & pd : p.projDeps)
            prereqs.insert(pd.name);
    ostringstream       ofs;
    ofs
        << "$(shell mkdir -p $(BINDIR))" << lf
        // .PHONY ensures 'make' won't check for a file named 'all' so will always update:
        << ".PHONY: all" << lf
        << "all: ";
    for (ConsProj const & p : sln.projects)
        if (!contains(prereqs,p.name) && !p.srcGroups.empty())
            ofs << targetPath(p) << " ";
    ofs << lf;
    for (ConsProj const & proj : sln.projects)
        if (!proj.srcGroups.empty())
            consProj(ofs,proj,sln);
    ofs << ".PHONY: clean cleanObjs cleanTargs" << lf
        << "clean: cleanObjs cleanTargs" << lf
        << "cleanObjs:" << lf
        << "\trm -r $(BUILDIR)" << lf
        << "cleanTargs:" << lf
        << "\trm -r $(BINDIR)" << lf;
    return saveRaw(ofs.str(),fname);
}

bool
constIncludeFileBox(ConsSolution const & sln,string const & fname)
{
    ostringstream       ofs;
    ofs
        // .PHONY ensures 'make' won't check for a file named 'all' so will always update:
        << ".PHONY: all" << lf
        << "all: ";
    for (ConsProj const & p : sln.projects)
        if (!p.srcGroups.empty())
            ofs << targetPath(p) << " ";
    ofs << lf;
    for (ConsProj const & proj : sln.projects)
        if (!proj.srcGroups.empty())
            consProj(ofs,proj,sln);
    ofs << ".PHONY: clean cleanObjs cleanTargs" << lf
        << "clean:" << lf
        << "\trm -r $(BUILDIR)" << lf;
    return saveRaw(ofs.str(),fname);
}

bool
consMakefileOsArch(
    BuildOS             os,
    Compiler            compiler,
    Arch                arch,
    Debrel              debrel,
    string const &      fnameBuild)         // includes this makefile
{
    string              osStr = (os == BuildOS::linux) ? "linux" : toStr(os),
                        makefile = cat(svec<string>("Makefile",osStr,toStr(arch),toStr(compiler),toStr(debrel)),"_"),
                        cc,
                        cxx,
                        link,
                        dllext,             // DLL file extension (including dot)
                        dllarg;             // DLL link args up to and adjacent to target name
    Strings             cflags,
                        cxxflags,           // cpp flags in *addition* to 'cflags'
                        lflags;
    // NOTE that warning and include flags are added per-project, not here.
    if (os == BuildOS::ios) {
        if (arch == Arch::x64)
            cflags.push_back("-arch x86_64");
        else if (arch == Arch::arm8_0)
            cflags.push_back("-arch arm64");
        else if (arch == Arch::arm8_3)
            cflags.push_back("-arch arm64e");       // Apple's A12
        else if (arch == Arch::armv7)
            cflags.push_back("-arch armv7");
        else
            fgThrow("architecture not supported on iOS",arch);
    }
    else {
        if (arch == Arch::x86)
            cflags.push_back("-m32");
        else if (arch == Arch::x64)
            cflags.push_back("-m64");
        else if (arch == Arch::arm8_2)
            cflags.push_back("-march=armv8.2-a");
        else
            fgThrow("architecture not supported on native *nix",arch);
    }
    if (debrel == Debrel::debug) {
        cflags.push_back("-g");             // generate debug info
        cflags.push_back("-O1");            // minimal optimization
        cflags.push_back("-D_DEBUG");
    }
    else {
        cflags.push_back("-DNDEBUG");
    }
    cxxflags.push_back("-std=c++14");
    if (compiler == Compiler::clang) {
        // uses libc++. On Linux, g++ is the GNU linker and uses libstdc++.
        cc = "clang";                   // CLANG C compiler
        cxx = "clang++";                // CLANG C++ compiler
        // On Linux, clang++ will dispatch to g++ (GNU linker) which uses libstdc++ (GNU std libs),
        // after ensuring appropriate linkage clang libs (eg. AddressSanitizer).
        // On MacOS, clang++ will link with clang/Apple's non-GPL 'libc++'.
        link = "clang++";
        // Ensures no common symbols across all libraries, required for all files which
        // are part of a DLL so we just always use it:
        cflags.push_back("-fno-common");
        // clang defaults to 256 which can cause errors with boost::iarchive exceeding
        // template depth limit. gcc limit of 1024 too small due to large ACS thread sigs:
        cxxflags.push_back("-ftemplate-depth=4096");
        if (debrel == Debrel::release)
            cflags.push_back("-Ofast");         // O3 plus fast floating point opts.
        if ((debrel == Debrel::debug) && (os == BuildOS::linux)) {
            cflags.push_back("-fsanitize=address");
            lflags.push_back("-fsanitize=address");
        }
    }
    else if (compiler == Compiler::gcc) {
        // gcc will call gnu c compiler or g++ depending on source type:
        cc = "gcc";
        cxx = "gcc";
        // g++ calls ld but it adds arguments for the appropriate standard libraries so it's easier.
        // It uses shared libaries by default and static linking appears quite difficult to get right:
        link = "g++";
        // gcc defaults to 1024. Too small due to large ACS thread sigs:
        cxxflags.push_back("-ftemplate-depth=4096");
        if (debrel == Debrel::release) {
            // -march=corei7 actually slowed down nrrjohnverts a smidgen.
            // -msse3 made no difference on some speed tests.
            cflags.push_back("-O3");
            cflags.push_back("-ffast-math");    // Needed for auto-vectorization as well as minor speedup.
        }
    }
    else if (compiler == Compiler::icpc) {
        cc = "icc";
        cxx = "icpc";
        link = "icpc";
        // Disable remark: Inlining inhibited by limit max-size / max-total-size
        // Disable remark: To get full report use -qopt-report=4 -qopt-report-phase ipo
        cflags.push_back("-diag-disable=11074,11076");
        if (debrel == Debrel::release)
            cflags.push_back("-Ofast");     // -fast-transcendentals made no diff on model corr speed test
        if (arch == Arch::x64)
            cflags.push_back("-xavx");      // Use AVX (and SSE).
    }
    else
        fgThrow("Don't know how to create makefile for compiler",compiler);
    if (os == BuildOS::macos) {
        lflags.push_back("-framework Carbon");
        // -Wl, passes the args on to the linker
        // -install_name tells binaries linking with this dynlib where to look for it at
        //    runtime. Without this the linker stupidly use the path given in the CL:
        dllext = ".dylib";
        dllarg = "-dynamiclib -Wl,-install_name,@executable_path/";
    }
    else if (os == BuildOS::ios) {
        bool                isSim = (arch == Arch::x64);
        string              osSim = isSim ? "Simulator" : "OS",
                            sys =
            "/Applications/Xcode.app/Contents/Developer/Platforms/iPhone"+osSim+".platform/Developer/SDKs/iPhone"+osSim+".sdk";
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
    bool                    native = contains(getNativeBuildOSs(),os);
    ostringstream   ofs;
    ofs << "CC = " << cc << lf
        << "CXX = " << cxx << lf
        << "LINK = " << link << lf
        << "AR = ar" << lf
        << "RANLIB = ranlib" << lf
        << "CFLAGS = " << cat(cflags," ") << lf
        << "CCFLAGS = $(CFLAGS)" << lf
        << "CXXFLAGS = $(CFLAGS) " << cat(cxxflags," ") << lf;
    if (native)
        ofs
            << "LFLAGS = " << cat(lflags," ") << lf
            << "DLLEXT = " << dllext << lf
            << "DLLARG = " << dllarg << lf
            << "BINDIR = ../" << getRelBin(os,arch,compiler,debrel) << lf
            << "BBINDIR = ../" << "bin/" << os << "/" << arch << "/" << lf;
    ofs
        << "BUILDIR = ../build_" << os << "/" << arch << "/" << compiler << "/" << toStr(debrel) << "/" << lf
        << "include " << fnameBuild << lf;
    return saveRaw(ofs.str(),makefile);
}

bool
consMakefileAndroidArch(
    BuildOS             host,       // Host OS for cross-compilation
    Arch                arch,
    bool                debug=false)
{
    // https://developer.android.com/ndk/guides/other_build_systems
    string          hostStr;
    if (host == BuildOS::win)
        hostStr = "windows";
    else if (host == BuildOS::linux)
        hostStr = "linux";
    else
        fgThrow("consMakefileAndroidArch unhandled host OS",host);
    string          archCMakeStr;
    if (arch == Arch::x86)
        archCMakeStr = "x86";
    else if (arch == Arch::x64)
        archCMakeStr = "x86_64";
    else if (arch == Arch::armv7)
        archCMakeStr = "armeabi-v7a";
    else if (arch == Arch::arm8_0)
        archCMakeStr = "arm64-v8a";
    else
        fgThrow("consMakefileAndroidArch unhandled arch",arch);
    string          debrel = debug ? "debug" : "release",
                    ar,ranlib;
    if (arch == Arch::x86) {
        string      base = "${NDK_BIN}/i686-" + hostStr + "-android-";
        ar = base + "ar";
        ranlib = base + "ranlib";
    }
    else if (arch == Arch::x64) {
        string      base = "${NDK_BIN}/x86_64-" + hostStr + "-android-";
        ar = base + "ar";
        ranlib = base + "ranlib";
    }
    else if (arch == Arch::armv7) {
        string      base = "${NDK_BIN}/arm-" + hostStr + "-androideabi-";
        ar = base + "ar";
        ranlib = base + "ranlib";
    }
    else if (arch == Arch::arm8_0) {
        string      base = "${NDK_BIN}/aarch64-" + hostStr + "-android-";
        ar = base + "ar";
        ranlib = base + "ranlib";
    }
    else
        fgThrow("consMakefileAndroidArch unhandled arch for ar/ranlib",arch);
    Strings          cflags,
                    cxxflags;               // cpp flags in *addition* to 'cflags'
    if (debug) {
        cflags.push_back("-g");             // generate debug info
        cflags.push_back("-O1");            // minimal optimization
        cflags.push_back("-D_DEBUG");
    }
    else {
        cflags.push_back("-DNDEBUG");
        cflags.push_back("-Ofast");         // O3 plus fast floating point opts.
    }
    if (arch == Arch::x86)
        cflags.push_back("-mstackrealign");     // From android toolchain
    cflags.push_back("-fno-addrsig");           // From android toolchain
    cflags.push_back("-fPIC");                  // In case used with shared libs
    string          targetStr;                  // The so-called 'target triple':
    if (arch == Arch::x86)
        targetStr = "i686-" + hostStr + "-android";
    else if (arch == Arch::x64)
        targetStr = "x86_64-" + hostStr + "-android";
    else if (arch == Arch::armv7)
        targetStr = "armv7a-" + hostStr + "-androideabi";
    else if (arch == Arch::arm8_0)
        targetStr = "aarch64-" + hostStr + "-android";
    cflags.push_back("--target="+targetStr+"${API_VERSION}");
    cflags.push_back("--sysroot=${NDK_ROOT}/toolchains/llvm/prebuilt/"+hostStr+"-x86_64/sysroot");
    cxxflags.push_back("-std=c++14");
    cxxflags.push_back("-ftemplate-depth=1024");    // boost annoyance
    cxxflags.push_back("-stdlib=libc++");           // From android toolchain
    ostringstream   ofs;
    ofs << "ifndef NDK_ROOT" << lf
        << "$(error NDK_ROOT is not set)" << lf
        << "endif" << lf
        << "API_VERSION ?= 23" << lf
        << "NDK_BIN = ${NDK_ROOT}/toolchains/llvm/prebuilt/" + hostStr + "-x86_64/bin" << lf
        << "CC = ${NDK_BIN}/clang" << lf
        << "CXX = ${NDK_BIN}/clang++" << lf
        << "LINK = ${NDK_BIN}/clang++" << lf
        << "AR = " << ar << lf
        << "RANLIB = " << ranlib << lf
        << "CFLAGS = " << cat(cflags," ") << lf
        << "CCFLAGS = $(CFLAGS)" << lf
        << "CXXFLAGS = $(CFLAGS) " << cat(cxxflags," ") << lf
        << "BUILDIR = ../build_android/" << archCMakeStr << "/clang/" << debrel << "/" << lf
        << "include make_libs.mk" << lf;
    return saveRaw(ofs.str(),"Makefile_android_" + toStr(arch) + "_clang_" + debrel);
}

bool
consMakefileIos(ConsSolution const & sln)
{
    Archs             archs = getBuildArchs(BuildOS::ios);
    ostringstream       oss;
    oss << "OUTDIR = ../build_ios/" << lf;
        oss << ".PHONY: all FORCE clean" << lf
        // Rely on 'make' following order given for targets to ensure recurse make calls.
        // Could perhaps have thin libs depend on phony architecture targets ("%.a : arch")
        // to trigger recursive makes:
        << "all: ";
    for (ConsProj const & p : sln.projects)
        if (!p.srcGroups.empty())
            oss << "$(OUTDIR)"+p.name+".a" << " ";
    oss << lf;
    for (ConsProj const & p : sln.projects) {
        if (!p.srcGroups.empty()) {
            string          fatLib = "$(OUTDIR)"+p.name+".a";
            string          srcLibs;
            for (Arch arch : archs)
                srcLibs += " $(OUTDIR)" + toStr(arch) + "/clang/release/" + p.name + ".a";
            oss << fatLib << ":" << srcLibs << lf;
            // Using 'xcrun' here avoid problems according to some sources.
            oss << "\txcrun -sdk iphoneos lipo -create -output " << fatLib << srcLibs << lf;
        }
    }
    for (Arch arch : archs) {
        // The % character is a glob which indicates that all such files depend on the same command:
        oss << "$(OUTDIR)" << arch << "/clang/release/%.a: FORCE" << lf
            // The recursive make command is smart enough to handle parent CL options such as -n
            << "\t$(MAKE) -j4 -f Makefile_ios_" << arch << "_clang_release" << lf;
    }
    oss << "FORCE:" << lf
        << "clean:" << lf
        << "\trm -r $(OUTDIR)" << lf;
    return saveRaw(oss.str(),"Makefile_ios");
}

bool
consMakefileAndroid(ConsSolution const &)
{
    Archs             archs = getBuildArchs(BuildOS::android);
    ostringstream       oss;
    oss << "OUTDIR = ../build_android/" << lf
        << ".PHONY: all clean" << lf
        << "all:" << lf;
    for (Arch arch : archs)
        oss << "\t$(MAKE) -j4 -f Makefile_android_" << arch << "_clang_release" << lf;
    oss << "clean:" << lf
        << "\trm -r $(OUTDIR)" << lf;
    return saveRaw(oss.str(),"Makefile_android");
}

}

bool
fgConsNativeMakefiles(ConsSolution const & sln)
{
    FGASSERT(sln.type == ConsType::nix);
    bool            changed = false;
    string          fnameAll = "make_all.mk";
    changed = constIncludeFileNative(sln,fnameAll) || changed;
    for (Debrel debrel : getDebrels()) {
        changed = consMakefileOsArch(BuildOS::linux,Compiler::clang,Arch::x64,debrel,fnameAll) || changed;
        changed = consMakefileOsArch(BuildOS::linux,Compiler::gcc,Arch::x64,debrel,fnameAll) || changed;
        changed = consMakefileOsArch(BuildOS::linux,Compiler::gcc,Arch::arm8_2,debrel,fnameAll) || changed;
        changed = consMakefileOsArch(BuildOS::linux,Compiler::icpc,Arch::x64,debrel,fnameAll) || changed;
        changed = consMakefileOsArch(BuildOS::macos,Compiler::clang,Arch::x64,debrel,fnameAll) || changed;
    }
    return changed;
}

bool
fgConsCrossMakefiles(ConsSolution const & sln)
{
    FGASSERT(sln.type == ConsType::cross);
    // Only build static SDK libraries for cross-compile targets:
    ConsSolution      slnXC(ConsType::cross);
    for (ConsProj const & p : sln.projects)
        if (p.isStaticLib())
            slnXC.projects.push_back(p);
    // We build separate thin libs then combine with lipo. It appears possible to build
    // fat object files and 'libtool' those, which would make for simpler makefiles, however
    // we need different compile arguments for simulator vs. deployment compiles:
    bool            changed = false;
    string          fnameLibs = "make_libs.mk";
    changed = constIncludeFileBox(slnXC,fnameLibs) || changed;

    // IOS on MacOS:
    for (Arch arch : getBuildArchs(BuildOS::ios))
        changed = consMakefileOsArch(BuildOS::ios,getBuildCompilers(BuildOS::ios)[0],arch,Debrel::release,fnameLibs) || changed;
    changed = consMakefileIos(slnXC) || changed;

    // Android on Linux:
    for (Arch arch : getBuildArchs(BuildOS::android))
        changed = consMakefileAndroidArch(BuildOS::linux,arch) || changed;
    changed = consMakefileAndroid(slnXC) || changed;

    return changed;
}

}

// */
