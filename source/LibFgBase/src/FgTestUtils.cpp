//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgTestUtils.hpp"
#include "FgFileSystem.hpp"
#include "FgCommand.hpp"
#include "FgImage.hpp"
#include "FgBuild.hpp"
#include "FgNc.hpp"
#include "FgParse.hpp"

using namespace std;
using namespace std::placeholders;

namespace Fg {

TestDir::TestDir(String const & name) : path {getTestDir()}
{
    FGASSERT(!name.empty());
    path.dirs.push_back(name);
    createPath(path.dir());                         // create if it doesn't already exist
    pd.push(path.dir());                            // make current
    deleteDirectoryContentsRecursive(path.str());   // remove any previous output for clean test
}

TestDir::~TestDir()
{
    if (!pd.orig.empty()) {
        pd.pop();
    }
}

void                testRegressFile(String8 const & baselineRelPath,String8 const & queryPath,CmpFilesFn const & fnEqual)
{
    if (!pathExists(queryPath))
        fgThrow("Regression query file not found",queryPath);
    bool                regressOverwrite = overwriteBaselines();
    String8             baselinePath = dataDir() + baselineRelPath;
    if (!pathExists(baselinePath)) {
        if (regressOverwrite) {
            copyFile(queryPath,baselinePath);
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
            copyFile(queryPath,baselinePath,true);
            deleteFile(queryPath);
        }
        fgThrow("Regression failure",queryPath+" != "+baselineRelPath);
    }
}

static bool         compareImages(String8 const & f1,String8 const & f2,uint maxDelta)
{
    ImgRgba8            i1 = loadImage(f1),
                        i2 = loadImage(f2);
    return isApproxEqual(i1,i2,maxDelta);
}

void                regressImage(string const & testFile,string const & refPath,uint maxDelta)
{
    CmpFilesFn         rt = bind(compareImages,_1,_2,maxDelta);
    regressFileRel(testFile,refPath,rt);
}

template<>
ImgRgba8            regressLoad(String8 const & path) {return loadImage(path); }

}
