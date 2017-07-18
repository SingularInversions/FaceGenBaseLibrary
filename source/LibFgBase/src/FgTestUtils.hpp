//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:Sohail Somani
// Created: 2008
//

#ifndef INCLUDED_TEST_UTILS_HPP
#define INCLUDED_TEST_UTILS_HPP

#include "FgStdLibs.hpp"
#include "FgDiagnostics.hpp"
#include "FgTempFile.hpp"
#include "FgOut.hpp"
#include "FgMath.hpp"
#include "FgMetaFormat.hpp"
#include "FgImageBase.hpp"
#include "FgMain.hpp"
#include "FgCommand.hpp"

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
            FGASSERT(e.no_tr_message().beginsWith(FgString(e1)));       \
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

// Returns true if this is the OS + compiler + bits + config used for binary regression.
// This is necessary since floating point optimizations will differ otherwise:
bool
fgIsRegressBuild();

bool
fgCompareImages(
    const FgImgRgbaUb & test,
    const FgImgRgbaUb & ref,
    uint                maxDelta=0);

void
fgRegressFail(
    const FgString & testName,
    const FgString & refName);

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
    fgLoadXml(fgDataDir()+path+name+"_baseline.xml",base);
    return base;
}

// Returns true if the user updates the regression baseline, false otherwise:
void
fgRegressUpdateQuery(const std::string & relPath);

// Useful to automatically overwite baselines when version control present, for fast
// comparison and updates. Note that this flag file is excluded from version control:
inline
bool
fgRegressOverwrite()
{return fgExists(fgDataDir()+"overwrite_baselines.flag"); }

// Takes two filenames as input and returns true for regression passed and false for failure:
typedef boost::function<bool(const FgString &,const FgString &)> FgFuncRegressFiles;

// Best used with FGTESTDIR macro to create and make current a test dir:
void
fgRegressFile(
    const FgString &    name,       // The test-created file to be regressed. Must exist in current directory.
    const FgString &    relDir,     // Relative path (within data dir) of the baseline file of the same name.
    // File regression defaults to binary file compare:
    const FgFuncRegressFiles & testFunc = fgBinaryFileCompare);

// Calls the given regression check and removes the regress file if successful. If unsuccessful then:
// 'overwrite_baselines.flag': overwrite base with regress and delete regress, otherwise:
// 'ci_build_server.flag': move regress file to CI server in relative path, otherwise:
// leave the regress file in place.
void
fgRegressFiles(
    const FgString &    base,
    const FgString &    regress,
    // Directory containing 'baes' and 'regress' relative to ~/data (for feedback and CI server copy):
    const string &      relDir,
    // File regression defaults to binary file compare:
    const FgFuncRegressFiles & testFunc = fgBinaryFileCompare);

// As above but regression failure when max pixel diff greater than given:
void
fgRegressImage(
    const std::string & name,
    const std::string & relDir,
    uint                maxDelta=2);

#endif
