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

// Takes two filenames as input and returns true for regression passed and false for failure:
typedef boost::function<bool(const FgString &,const FgString &)> FgFnRegressFiles;

// Calls the given regression check and deletes the query file if successful. If unsuccessful then:
// 'overwrite_baselines.flag': overwrite base with regress and delete regress, otherwise:
// leave the query file in place.
void
fgRegressFile(
    const FgString &            baselineRelPath,    // Relative (to data dir) path to regression baseline file
    const FgString &            queryPath,          // Path to query file to be tested
    const FgFnRegressFiles &    fnEqual = fgBinaryFileCompare); // Defaults to binary equality test

// As above when query and baseline have same name:
inline
void
fgRegressFileRel(
    const FgString &    name,       // file name to be regressed. Must exist in current directory and in 'relDir'.
    const FgString &    relDir,     // Relative path (within data dir) of the baseline file of the same name.
    const FgFnRegressFiles & fnEqual = fgBinaryFileCompare)     // Defaults to binary equality test
{fgRegressFile(relDir+name,name,fnEqual); }

// As above but regression failure when max pixel diff greater than given:
void
fgRegressImage(
    const std::string & name,
    const std::string & relDir,
    uint                maxDelta=2);

#endif
