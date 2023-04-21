//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// regression tests work as follows:
// * the function to be tested generates the object <output>
// * the reference object is loaded from the file <reference>
// * the two are compared for regression:
//   - where the output is expected to be exact, simple equality is used
//   - otherwise, an approximate equality is used, to account for precision differences with different configurations
//   - except when running on the primary development configuration (os-compiler-bits-debrel), where exact equality is still used
// * if the equality fails:
//   - an error is thrown
//   - if this is the primary development config and also a developer system (~/data/_overwrite_baslines.flag)
//     <output> overwrites <reference> so the differences can easily be viewed or updated in source control
// * for easy viewing in source control, viewable formats are preferred for the <output>, eg. a mesh that contains
//   <output> as just its vertices

#ifndef INCLUDED_TEST_UTILS_HPP
#define INCLUDED_TEST_UTILS_HPP

#include "FgSerial.hpp"
#include "FgCommand.hpp"
#include "FgImageIo.hpp"
#include "FgBuild.hpp"

namespace Fg {

struct      TestDir
{
    PushDir         pd;
    Path            path;

    TestDir() {}
    // creates directory ~sdk/test-output/name/ if not already present, empties it, and sets it to current dir:
    explicit TestDir(String const & name);
    ~TestDir();
};

// Creates a test directory with a name formed from the breadcrumb of commands, changes the
// current directory to that, then reverts to the initial directory when it goes out of scope:
#define FGTESTDIR FGASSERT(!args.empty()); TestDir fgTestDir(toLower(args[0]));

// As above but regression failure when max pixel diff greater than given:
void                regressImage(String const & name,String const & relDir,uint maxDelta=2);

template<class T> T regressLoad(String8 const & fname) {return dsrlzText<T>(loadRawString(fname)); }
template<> ImgRgba8 regressLoad(String8 const &);

template<class T>
inline void         regressSave(T const & val,String8 const & fname) {saveRaw(srlzText(val),fname); }
template<>
inline void         regressSave(ImgRgba8 const & img,String8 const & path) {saveImage(path,img); }

// This flag should be set on a developer's machine (and ignored by source control) for easy regression test updates
// and change visualation. It should NOT be set of automated build machines:
inline bool         overwriteBaselines() {return pathExists(dataDir()+"_overwrite_baselines.flag"); }

template<class T> bool isEqualSrlz(T const & l,T const & r) {return (srlz(l) == srlz(r)); }

template<class T> bool  isEqual(T const & l,T const & r) {return (l==r); }

// Exact regression for all configurations:
template<class T>
void                testRegressExact(
    T const &           query,
    String const &      baselineRelPath,    // Relative to ~/data/  will be stored in TXT serialization.
    Sfun<bool(T const &,T const &)> const & eqFn = isEqual<T>)
{
    bool                overwrite = overwriteBaselines();
    String8             baselineAbsPath = dataDir() + baselineRelPath;
    if (!pathExists(baselineAbsPath)) {
        if (overwrite) {
            regressSave(query,baselineAbsPath);
            fgout << fgnl << "New regression baseline saved: " << baselineRelPath;
            // Don't return here, run the test to be sure it works:
        }
        else
            fgThrow("Regression baseline not found",baselineRelPath);
    }
    T                   baseline = regressLoad<T>(baselineAbsPath);
    if (!eqFn(query,baseline)) {
        if (overwrite)
            regressSave(query,baselineAbsPath);
        fgThrow("Regression failure: ",baselineRelPath);
    }
}

// throws if the test fails. Strangely, it can't infer the type from the first argument ...
template<class T>
void                testRegressApprox(
    T const &                       query,
    String const &                  baselineRelPath,        // must be relative to ~/data/
    Sfun<bool(T const &,T const &)> compareFn,
    Sfun<T(String8 const &)>        loadFn=regressLoad<T>,
    Sfun<void(T const &,String8 const &)> saveFn=regressSave<T>,
    // Set equality if you want bitwise regression for primary configuration:
    Sfun<bool(T const &,T const &)> isEqualFn=isEqualSrlz<T>)
{
    bool                overwrite = overwriteBaselines();
    String8             baselineAbsPath = dataDir() + baselineRelPath;
    if (!pathExists(baselineAbsPath)) {
        if (overwrite) {
            saveFn(query,baselineAbsPath);
            fgout << fgnl << "New regression baseline saved: " << baselineRelPath;
            // Don't return here, run the test to be sure it works:
        }
        else
            fgThrow("Regression baseline not found",baselineRelPath);
    }
    T                   baseline = loadFn(baselineAbsPath);
    if (compareFn(query,baseline)) {  // Passed
        // If this is a developer machine primary config, further test for exact equality and update the
        // regression file if not the case:
        if (isEqualFn && overwrite && isPrimaryConfig() && (!isEqualFn(query,baseline))) {
            saveFn(query,baselineAbsPath);
            fgout << fgnl << "Regression baseline updated: " << baselineRelPath;
        }
    }
    else {                          // Failed
        if (overwrite)
            saveFn(query,baselineAbsPath);
        else {                      // store for retrieval if required:
            Path                path {dataDir() + "../test-output/" + baselineRelPath};
            createPath(path.dir());
            saveFn(query,path.str());
        }
        fgThrow("Regression failure: ",baselineRelPath);
    }
}

// Takes two filenames as input and returns true for regression passed and false for failure:
typedef Sfun<bool(String8 const &,String8 const &)>     CmpFilesFn;

// as above when query is created as a temp file:
void                testRegressFile(
    String8 const &         baselineRelPath,    // Relative (to data dir) path to regression baseline file
    String8 const &         queryPath,          // Path to query file to be tested
    CmpFilesFn const &      cmpFilesFn = equateFilesBinary); // Defaults to binary equality test

// As above when query and baseline have same name:
inline void         regressFileRel(
    String8 const &         fname,      // Must exist relative to current dir (query) AND dataDir() + 'relDir' (base).
    String8 const &         relDir,     // Relative path (within data dir) of the baseline file of the same name.
    CmpFilesFn const &      cmpFilesFn = equateFilesBinary)     // Defaults to binary equality test
{
    testRegressFile(relDir+fname,fname,cmpFilesFn);
}

}

#endif
