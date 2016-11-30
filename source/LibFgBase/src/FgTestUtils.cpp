//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani
// Created: 2008
//

#include "stdafx.h"
#include "FgTestUtils.hpp"
#include "FgOut.hpp"
#include "FgFileSystem.hpp"
#include "FgCommand.hpp"
#include "FgStdString.hpp"
#include "FgImage.hpp"
#include "FgBuild.hpp"

#if defined(_MSC_VER) && defined(_DEBUG)

#include <crtdbg.h>

struct FgMemoryLeakDetector::impl
{
    _CrtMemState before;
};

FgMemoryLeakDetector::FgMemoryLeakDetector():
    m_p(new impl)
{
    _CrtMemCheckpoint(&m_p->before);
}

FgMemoryLeakDetector::~FgMemoryLeakDetector()
{
    delete m_p;
}

void
FgMemoryLeakDetector::throw_if_leaked(const char * whichfn)
{
    _CrtMemState after;
    _CrtMemCheckpoint(&after);
    _CrtMemState diff;
    if(_CrtMemDifference(&diff,
                         &m_p->before,
                         &after))
    {
        TEST_THROW("The test " << whichfn << " has a memory leak");
    }
}

#else

FgMemoryLeakDetector::FgMemoryLeakDetector(){}
FgMemoryLeakDetector::~FgMemoryLeakDetector(){}
void FgMemoryLeakDetector::throw_if_leaked(const char * /*whichfn*/){}

#endif

using namespace std;

bool
fgIsRegressBuild()
{
    if (fgCurrentOS() != "win")
        return false;
    if (fgCurrentCompiler() != "vs12")
        return false;
    if (fgCurrentBuildBits() != "64")
        return false;
    if (fgCurrentBuildConfig() != "release")
        return false;
    return true;
}

bool
fgCompareImages(
    const FgImgRgbaUb & test,
    const FgImgRgbaUb & ref,
    uint                maxDelta)
{
    if (test.dims() != ref.dims())
        return false;
    int             lim = int(maxDelta * maxDelta);
    for (FgIter2UI it(test.dims()); it.valid(); it.next())
    {
        FgVect4I delta =
            FgVect4I(test[it()].m_c) -
            FgVect4I(ref[it()].m_c);
        if ((fgSqr(delta[0]) > lim) ||
            (fgSqr(delta[1]) > lim) ||
            (fgSqr(delta[2]) > lim) ||
            (fgSqr(delta[3]) > lim))
            return false;
    }
    return true;
}

void
fgRegressFail(
    const FgString & testName,
    const FgString & refName)
{
    fgThrow("Regression failure",testName + " != " + refName);
}

void
fgRegressUpdateQuery(const std::string & relPath)
{
    fgout << fgnl << "REGRESS FAILURE: " << relPath;
    FGASSERT(!fgCommandAutomated);      // Fail if non-interactive
    fgout << fgnl << "UPDATE FILE (Y/N) ?" << flush;
    char        choice;
    cin >> choice;
    if (std::tolower(choice) == 'y') {
        FgPath      path(fgDataDir()+relPath);
        fgCopyFile(path.baseExt(),path.str(),true);
    }
}

void
fgRegressFile(
    const FgString &        name,
    const FgString &        relDir,
    const FgRegressTest &   testFunc)
{
    FgString        baselinePath = fgDataDir()+relDir+name;
    if (!fgExists(name))
        fgThrow("Regression file not created by test",name);
    if (fgRegressOverwrite()) {
        if (fgExists(baselinePath)) {       // Otherwise we are creating a new test
            bool    succ = testFunc(name,baselinePath);
            fgCopyFile(name,baselinePath,true);
            if (!succ) 
                // Don't throw in dev mode so remaining regressions still get run:
                fgout << fgnl << "WARNING: Regression failure: " << relDir << name;
        }
        else {
            fgCopyFile(name,baselinePath);
            fgout << fgnl << "New regression baseline saved: " << relDir+name;
        }
    }
    else {
        if (!testFunc(name,baselinePath))
            fgThrow("Regression failure",fgGetCurrentDir()+name+" != "+relDir+name);
    }
}

static
bool
compareImages(
    const FgString &    f1,
    const FgString &    f2,
    uint                maxDelta)
{
    FgImgRgbaUb         i1,i2;
    fgLoadImgAnyFormat(f1,i1);
    fgLoadImgAnyFormat(f2,i2);
    return fgCompareImages(i1,i2,maxDelta);
}

void
fgRegressImage(
    const string &      testFile,
    const string &      refPath,
    uint                maxDelta)
{
    FgRegressTest       rt = boost::bind(compareImages,_1,_2,maxDelta);
    fgRegressFile(testFile,refPath,rt);
}
