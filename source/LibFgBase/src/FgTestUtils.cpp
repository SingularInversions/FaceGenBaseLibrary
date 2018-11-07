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
#include "FgNc.hpp"
#include "FgParse.hpp"

using namespace std::placeholders;

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

void
fgRegressFail(
    const FgString & testName,
    const FgString & refName)
{
    fgThrow("Regression failure",testName + " != " + refName);
}

void
fgRegressFile(const FgString & baselineRelPath,const FgString & queryPath,const FgFnRegressFiles & fnEqual)
{
    if (!fgExists(queryPath))
        fgThrow("Regression query file not found",queryPath);
    bool                    regressOverwrite = fgOverwriteBaselines();
    FgString                baselinePath = fgDataDir() + baselineRelPath;
    if (!fgExists(baselinePath)) {
        if (regressOverwrite) {
            fgCopyFile(queryPath,baselinePath);
            fgout << fgnl << "New regression baseline saved: " << baselineRelPath;
            // Don't return here, run the test to be sure it works.
        }
        else
            fgThrow("Regression baseline not found",baselinePath);
    }
    if (fnEqual(baselinePath,queryPath))
        fgDeleteFile(queryPath);        // Passed test
    else {                              // Failed test
        if (regressOverwrite) {
            fgCopyFile(queryPath,baselinePath,true);
            fgDeleteFile(queryPath);
        }
        fgThrow("Regression failure",queryPath+" != "+baselineRelPath);
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
    return fgImgApproxEqual(i1,i2,maxDelta);
}

void
fgRegressImage(
    const string &      testFile,
    const string &      refPath,
    uint                maxDelta)
{
    FgFnRegressFiles       rt = std::bind(compareImages,_1,_2,maxDelta);
    fgRegressFileRel(testFile,refPath,rt);
}

template<>
FgImgRgbaUb
fgRegressLoad(const FgString & path)
{return fgLoadImgAnyFormat(path); }

template<>
void
fgRegressSave(const FgString & path,const FgImgRgbaUb & img)
{fgSaveImgAnyFormat(path,img); }

void
fgRegressString(const string & data,const FgString & relPath)
{
    FgString        dd = fgDataDir();
    if (data == fgSlurp(dd+relPath))
        return;
    if (fgExists(dd+"_overwrite_baselines.flag"))
        fgDump(data,dd+relPath);
}
