//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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

using namespace std;
using namespace std::placeholders;

namespace Fg {

void
regressFail(
    Ustring const & testName,
    Ustring const & refName)
{
    fgThrow("Regression failure",testName + " != " + refName);
}

void
regressFile(Ustring const & baselineRelPath,Ustring const & queryPath,const EquateFiles & fnEqual)
{
    if (!pathExists(queryPath))
        fgThrow("Regression query file not found",queryPath);
    bool                    regressOverwrite = overwriteBaselines();
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
        deleteFile(queryPath);        // Passed test
    else {                              // Failed test
        if (regressOverwrite) {
            fileCopy(queryPath,baselinePath,true);
            deleteFile(queryPath);
        }
        fgThrow("Regression failure",queryPath+" != "+baselineRelPath);
    }
}

static
bool
compareImages(
    Ustring const &    f1,
    Ustring const &    f2,
    uint                maxDelta)
{
    ImgC4UC         i1 = loadImage(f1),
                    i2 = loadImage(f2);
    return fgImgApproxEqual(i1,i2,maxDelta);
}

void
regressImage(
    string const &      testFile,
    string const &      refPath,
    uint                maxDelta)
{
    EquateFiles       rt = std::bind(compareImages,_1,_2,maxDelta);
    regressFileRel(testFile,refPath,rt);
}

template<>
ImgC4UC
regressLoad(Ustring const & path)
{return loadImage(path); }

void
regressString(string const & data,Ustring const & relPath)
{
    Ustring        dd = dataDir();
    if (data == loadRawString(dd+relPath))
        return;
    if (pathExists(dd+"_overwrite_baselines.flag"))
        saveRaw(data,dd+relPath);
}

}
