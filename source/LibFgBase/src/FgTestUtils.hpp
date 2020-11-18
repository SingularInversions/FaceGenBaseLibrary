//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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
#include "FgCommand.hpp"
#include "FgImageIo.hpp"
#include "FgBuild.hpp"

namespace Fg {

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
            FGASSERT(beginsWith(e.no_tr_message(),e1));               \
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
regressFail(
    Ustring const & testName,
    Ustring const & refName);

// Returns corresponding regression baseline value:
template<class T>
T
getRegressionBaseline(
    T const &           val,
    const std::string & path,
    const std::string & name)
{
    if (fgKeepTempFiles())
        saveBsaXml(name+"_baseline.xml",val);
    T       base;
    loadBsaXml(dataDir()+path+name+"_baseline.xml",base);
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
regressImage(
    const std::string & name,
    const std::string & relDir,
    uint                maxDelta=2);

template<class T>
T
regressLoad(Ustring const & fname)
{
    T       ret;
    loadBsaXml(fname,ret);
    return ret;
}
template<>
ImgC4UC
regressLoad(Ustring const &);

template<class T>
void
regressSave(Ustring const & fname,T const & val)
{saveBsaXml(fname,val); }

template<>
inline void
regressSave(Ustring const & path,ImgC4UC const & img)
{saveImage(path,img); }

// Developers with source control create this (empty) flag file locally:
inline
bool
overwriteBaselines()
{return pathExists(dataDir()+"_overwrite_baselines.flag"); }

// Exact regression for all configurations:
template<class T>
void
regressTest(
    T const &           query,
    Ustring const &     baselinePath)
{
    // This flag should be set on a developer's machine (and ignored by source control) for
    // easy updates & change visualation. It should NOT be set of automated build machines:
    bool                regressOverwrite = overwriteBaselines();
    if (!pathExists(baselinePath)) {
        if (regressOverwrite) {
            regressSave(baselinePath,query);
            fgout << fgnl << "New regression baseline saved: " << baselinePath;
            // Don't return here, run the test to be sure it works:
        }
        else
            fgThrow("Regression baseline not found",baselinePath);
    }
    T       baseline = regressLoad<T>(baselinePath);
    if (!(query == baseline)) {
        if (regressOverwrite)
            regressSave(baselinePath,query);
        fgThrow("Regression failure: ",baselinePath);
    }
}

// Floating point optimizations for different configurations yield different (by a small amount) results:
template<class T>
void
regressTestApprox(
    T const &                       query,
    Ustring const &                 baselinePath,
    Sfun<bool(T const &,T const &)> compare,
    Sfun<T(Ustring const &)>        load=regressLoad<T>,
    Sfun<void(Ustring const &,T const &)> save=regressSave<T>,
    // Set equality if you want bitwise regression for primary configuration:
    Sfun<bool(T const &,T const &)> equality=Sfun<bool(T const &,T const &)>())
{
    bool                regressOverwrite = overwriteBaselines();
    if (!pathExists(baselinePath)) {
        if (regressOverwrite) {
            save(baselinePath,query);
            fgout << fgnl << "New regression baseline saved: " << baselinePath;
            // Don't return here, run the test to be sure it works:
        }
        else
            fgThrow("Regression baseline not found",baselinePath);
    }
    T       baseline = load(baselinePath);
    if (compare(query,baseline)) {  // Passed
        // If this is a developer machine primary config, test for exact equality and update the
        // regression file if not the case:
        if (equality && regressOverwrite && isPrimaryConfig() && (!equality(query,baseline))) {
            save(baselinePath,query);
            fgout << fgnl << "Regression baseline updated: " << baselinePath;
        }
    }
    else {                          // Failed
        if (regressOverwrite)
            save(baselinePath,query);
        fgThrow("Regression failure: ",baselinePath);
    }
}

// Regress a string against a data file. Throws if file is different.
// For dev instances (_overwrite_baselines.flag), also overwrites file if different.
void
regressString(String const & data,Ustring const & relPath);

}

#endif
