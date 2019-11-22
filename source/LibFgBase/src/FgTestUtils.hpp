//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef INCLUDED_TEST_UTILS_HPP
#define INCLUDED_TEST_UTILS_HPP

#include "FgStdLibs.hpp"
#include "FgBoostLibs.hpp"
#include "FgDiagnostics.hpp"
#include "FgOut.hpp"
#include "FgMetaFormat.hpp"
#include "FgImageBase.hpp"
#include "FgCommand.hpp"

namespace Fg {

struct FgMemoryLeakDetector
{
    FgMemoryLeakDetector();
    ~FgMemoryLeakDetector();
    void throw_if_leaked(const char * whichfn);
private:
    struct impl;
    impl * m_p;
};

// Ensure that an expression throws a specific FgException by checking that the 
// beginning of untranslated strings match. Don't use the full string as the state
// information can vary even within a test (eg. thread race).
#define FG_TEST_CHECK_THROW_1(expr,e1)                                  \
    {                                                                   \
        bool fg_test_check_threw = false;                               \
        try                                                             \
        {                                                               \
            expr;                                                       \
        }                                                               \
        catch(FgException const &e)                                     \
        {                                                               \
            fgout << fgnl << e.no_tr_message();                         \
            fgout << fgnl << e1;                                        \
            FGASSERT(fgBeginsWith(e.no_tr_message(),e1));               \
            fg_test_check_threw = true;                                 \
        }                                                               \
        catch(...){fg_test_check_threw = true;}                         \
        FGASSERT1(fg_test_check_threw,"An exception was expected but none was thrown"); \
    }
    

// An easy way to throw an error with a format string:
// \code
//  if(!foo)
//    TEST_THROW(runtime_error,"Foo is wrong: " << foo);
// \endcode
#define TEST_THROW(stream_expr)                 \
    {                                           \
        std::ostringstream str;                 \
        str << stream_expr;                     \
        throw std::runtime_error(str.str());  \
    }

void
fgRegressFail(
    Ustring const & testName,
    Ustring const & refName);

// Returns corresponding regression baseline value:
template<class T>
T
fgRegressionBaseline(
    const T &           val,
    const std::string & path,
    const std::string & name)
{
    if (fgKeepTempFiles())
        fgSaveXml(name+"_baseline.xml",val);
    T       base;
    fgLoadXml(dataDir()+path+name+"_baseline.xml",base);
    return base;
}

// Takes two filenames as input and returns true for regression passed and false for failure:
typedef std::function<bool(Ustring const &,Ustring const &)> EquateFiles;

// Calls the given regression check and deletes the query file if successful. If unsuccessful then:
// '_overwrite_baselines.flag': overwrite base with regress and delete regress, otherwise:
// leave the query file in place.
void
regressFile(
    Ustring const &         baselineRelPath,    // Relative (to data dir) path to regression baseline file
    Ustring const &         queryPath,          // Path to query file to be tested
    EquateFiles const &     equateFiles = equateFilesBinary); // Defaults to binary equality test

// As above when query and baseline have same name:
inline
void
regressFileRel(
    Ustring const &         fname,      // Must exist relative to current dir (query) AND dataDir() + 'relDir' (base).
    Ustring const &         relDir,     // Relative path (within data dir) of the baseline file of the same name.
    EquateFiles const &     equateFiles = equateFilesBinary)     // Defaults to binary equality test
{regressFile(relDir+fname,fname,equateFiles); }

// As above but regression failure when max pixel diff greater than given:
void
fgRegressImage(
    const std::string & name,
    const std::string & relDir,
    uint                maxDelta=2);

// Default comparison is equality but this should be overidden when small differences between platforms are
// acceptable:
template<class T>
bool
fgRegressCompare(const T & lhs,const T & rhs)
{return (lhs == rhs); }

template<class T>
T
fgRegressLoad(Ustring const & fname)
{
    T       ret;
    fgLoadXml(fname,ret);
    return ret;
}

template<class T>
void
fgRegressSave(Ustring const & fname,const T & val)
{fgSaveXml(fname,val); }

template<>
ImgC4UC
fgRegressLoad(Ustring const &);
template<>
void
fgRegressSave(Ustring const &,const ImgC4UC &);

// Developers with source control create this (empty) flag file locally:
inline
bool
fgOverwriteBaselines()
{return pathExists(dataDir()+"_overwrite_baselines.flag"); }

template<class T>
void
fgRegress(
    const T &           query,
    Ustring const &    baselinePath,
    const std::function<bool(const T &,const T &)> & regressCompare=fgRegressCompare<T>)
{
    // This flag should be set on a developer's machine (and ignored by source control) for
    // easy updates & change visualation. It should NOT be set of automated build machines:
    bool                regressOverwrite = fgOverwriteBaselines();
    if (!pathExists(baselinePath)) {
        if (regressOverwrite) {
            fgRegressSave(baselinePath,query);
            fgout << fgnl << "New regression baseline saved: " << baselinePath;
            // Don't return here, run the test to be sure it works:
        }
        else
            fgThrow("Regression baseline not found",baselinePath);
    }

    // Regression file exists, do the test:
    T       baseline = fgRegressLoad<T>(baselinePath);
    if (regressCompare(query,baseline)) {   // Passed
        // If this is the developer machine, further test for exact equality and update the
        // regression file if not the case:
        if (regressOverwrite && (!(query == baseline))) {
            fgRegressSave(baselinePath,query);
            fgout << fgnl << "Regression baseline updated: " << baselinePath;
        }
    }
    else {      // The test failed:
        if (regressOverwrite)
            fgRegressSave(baselinePath,query);
        fgThrow("Regression failure: ",baselinePath);
    }
}

// Regress a string against a data file. Throws if file is different.
// For dev instances (_overwrite_baselines.flag), also overwrites file if different.
void
fgRegressString(String const & data,Ustring const & relPath);

}

#endif
