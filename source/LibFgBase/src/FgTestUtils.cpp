//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
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

namespace Fg {

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
    const Ustring & testName,
    const Ustring & refName)
{
    fgThrow("Regression failure",testName + " != " + refName);
}

void
regressFile(const Ustring & baselineRelPath,const Ustring & queryPath,const EquateFiles & fnEqual)
{
    if (!pathExists(queryPath))
        fgThrow("Regression query file not found",queryPath);
    bool                    regressOverwrite = fgOverwriteBaselines();
    Ustring                baselinePath = dataDir() + baselineRelPath;
    if (!pathExists(baselinePath)) {
        if (regressOverwrite) {
            fileCopy(queryPath,baselinePath);
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
            fileCopy(queryPath,baselinePath,true);
            fgDeleteFile(queryPath);
        }
        fgThrow("Regression failure",queryPath+" != "+baselineRelPath);
    }
}

static
bool
compareImages(
    const Ustring &    f1,
    const Ustring &    f2,
    uint                maxDelta)
{
    ImgC4UC         i1,i2;
    imgLoadAnyFormat(f1,i1);
    imgLoadAnyFormat(f2,i2);
    return fgImgApproxEqual(i1,i2,maxDelta);
}

void
fgRegressImage(
    const string &      testFile,
    const string &      refPath,
    uint                maxDelta)
{
    EquateFiles       rt = std::bind(compareImages,_1,_2,maxDelta);
    regressFileRel(testFile,refPath,rt);
}

template<>
ImgC4UC
fgRegressLoad(const Ustring & path)
{return imgLoadAnyFormat(path); }

template<>
void
fgRegressSave(const Ustring & path,const ImgC4UC & img)
{imgSaveAnyFormat(path,img); }

void
fgRegressString(const string & data,const Ustring & relPath)
{
    Ustring        dd = dataDir();
    if (data == fgSlurp(dd+relPath))
        return;
    if (pathExists(dd+"_overwrite_baselines.flag"))
        fgDump(data,dd+relPath);
}

}
