//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgCmd.hpp"
#include "FgSyntax.hpp"
#include "FgMetaFormat.hpp"
#include "FgImage.hpp"
#include "FgTestUtils.hpp"
#include "FgBuild.hpp"
#include "FgVersion.hpp"
#include "FgSystemInfo.hpp"

using namespace std;

namespace Fg {

void fg3dTest(CLArgs const &);
void fgBoostSerializationTest(CLArgs const &);
void fgCmdTestDfg(CLArgs const &);
void fgExceptionTest(CLArgs const &);
void fgFileSystemTest(CLArgs const &);
void fgOpenTest(CLArgs const &);
void fgGeometryTest(CLArgs const &);
void fgGridTrianglesTest(CLArgs const &);
void fgImageTest(CLArgs const &);
void fgMatrixSolverTest(CLArgs const &);
void fgMathTest(CLArgs const &);
void fgMatrixCTest(CLArgs const &);
void fgMatrixVTest(CLArgs const &);
void fgMetaFormatTest(CLArgs const &);
void fgMorphTest(CLArgs const &);
void fgPathTest(CLArgs const &);
void fgQuaternionTest(CLArgs const &);
void fgCmdRenderTest(CLArgs const &);
void fgSerializeTest(CLArgs const &);
void fgSimilarityTest(CLArgs const &);
void fgSimilarityApproxTest(CLArgs const &);
void fgStdVectorTest(CLArgs const &);
void fgStringTest(CLArgs const &);
void fgTensorTest(CLArgs const &);

Cmd fgSoftRenderTestInfo();   // Don't put these in a macro as it generates a clang warning about vexing parse.

vector<Cmd>
fgCmdBaseTests()
{
    Cmds      cmds {
        {fg3dTest,"3d"},
        {fgBoostSerializationTest,"boostSerialization"},
        {fgCmdTestDfg,"dataflow"},
        {fgExceptionTest,"exception"},
        {fgFileSystemTest,"filesystem"},
        {fgOpenTest,"open"},
        {fgGeometryTest,"geometry"},
        {fgGridTrianglesTest,"gridTriangles"},
        {fgImageTest,"image"},
        {fgMatrixSolverTest,"matSol","Matrix Solver"},
        {fgMathTest,"math"},
        {fgMatrixCTest,"matC","MatrixC"},
        {fgMatrixVTest,"matV","MatrixV"},
        {fgMetaFormatTest,"metaFormat"},
        {fgMorphTest,"morph"},
        {fgPathTest,"path"},
        {fgQuaternionTest,"quaternion"},
        {fgCmdRenderTest,"rendc","render command"},
        {fgSerializeTest,"serialize"},
        {fgSimilarityTest,"similarity"},
        {fgSimilarityApproxTest,"similarityApprox"},
        {fgStdVectorTest,"vector"},
        {fgStringTest,"string"},
        {fgTensorTest,"tensor"}
    };
    cmds.push_back(fgSoftRenderTestInfo());
    return cmds;
}

void fgImgGuiTestm(CLArgs const &);
void fgTestmGuiMesh(CLArgs const &);
void fgTestmGui2(CLArgs const &);
void fgGuiTestmDialogSplashScreen(CLArgs const &);

static
void
testmGui(const CLArgs & args)
{
    Cmds      cmds {
        {fgImgGuiTestm,"image"},
        {fgTestmGui2,"gui2"},
        {fgTestmGuiMesh,"mesh"},
        {fgGuiTestmDialogSplashScreen,"splash"}
    };
    fgMenu(args,cmds);
}

static
void
sysinfo(const CLArgs &)
{
    fgout
        << fgnl << "Computer name: " << fgComputerName()
        << fgnl << "OS: " << fgOsName() << (fg64bitOS() ? " (64 bit)" : " (32 bit)")
        << fgnl << "CPU hardware threads: " << std::thread::hardware_concurrency()
        << fgnl << "Executable:" << fgpush
#ifdef __APPLE__
        << fgnl << "__APPLE__: " << __APPLE__
#endif
#ifdef _MSC_VER
        << fgnl << "_MSC_VER: " << _MSC_VER
#endif
        << fgpop;
}

void fg3dTestMan(CLArgs const &);
void fgClusterTest(CLArgs const &);
void fgClusterTestm(CLArgs const &);
void fgClusterDeployTestm(CLArgs const &);
void fgCmdTestmCpp(CLArgs const &);
void fg3dReadWobjTest(CLArgs const &);
void fgRandomTest(CLArgs const &);
void fgGeometryManTest(CLArgs const &);
void fgSubdivisionTest(CLArgs const &);
void fgTextureImageMappingRenderTest(CLArgs const &);
void fgImageTestm(CLArgs const &);

Cmds
fgCmdBaseTestms()
{
    Cmds      cmds {
        {testmGui,"gui"},
        {fg3dTestMan,"3d"},
        {fgClusterTest,"cluster"},
        {fgClusterTestm,"clusterm"},
        {fgClusterDeployTestm,"clusterDeploy"},
        {fgCmdTestmCpp,"cpp","C++ behaviour tests"},
        {fg3dReadWobjTest,"readWobj"},
        {fgRandomTest,"random"},
        {fgGeometryManTest,"geometry"},
        {fgSubdivisionTest,"subdivision"},
        {fgTextureImageMappingRenderTest,"texturemap"},
        {fgImageTestm,"image"}
    };
    return cmds;
}

static
void
testm(const CLArgs & args)
{fgMenu(args,fgCmdBaseTestms()); }

void
fgCmdBaseTest(const CLArgs & args)
{fgMenu(args,fgCmdBaseTests(),true); }

void
view(const CLArgs & args)
{fgMenu(args,fgCmdViewInfos()); }

/**
   \defgroup Base_Commands Base Library Command Line
   Commands in the program 'fgbl' demonstrating use of the FaceGen Base Library.
 */
void
fgCmdFgbl(const CLArgs & args)
{
    if (args.size() == 1)
        fgout << fgnl << "FaceGen Base Library CLI " << fgVersion(".") << " (" << fgCurrentBuildDescription() << ")"; 
    vector<Cmd>   cmds;
    cmds.push_back(fgCmdImgopsInfo());
    cmds.push_back(fgCmdMeshopsInfo());
    cmds.push_back(fgCmdMorphInfo());
    cmds.push_back(fgCmdRenderInfo());
    cmds.push_back(fgCmdTriexportInfo());
    cmds.push_back(Cmd(fgCmdCons,"cons","Construct makefiles / solution file / project files"));
    cmds.push_back(Cmd(sysinfo,"sys","Show system info"));
    cmds.push_back(Cmd(fgCmdBaseTest,"test","Automated tests"));
    cmds.push_back(Cmd(testm,"testm","Manual tests"));
    cmds.push_back(Cmd(view,"view","Interactively view various file types"));
    fgMenu(args,cmds);
}

}

// */
