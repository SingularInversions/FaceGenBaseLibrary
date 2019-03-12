//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: July 15, 2015
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

FgCmd fgSoftRenderTestInfo();   // Don't put these in a macro as it generates a clang warning about vexing parse.

vector<FgCmd>
fgCmdBaseTests()
{
    vector<FgCmd>   cmds;
    //FGADDCMD1(fgApproxFuncTest,"approxFunc");
    FGADDCMD1(fg3dTest,"3d");
    FGADDCMD1(fgBoostSerializationTest,"boostSerialization");
    FGADDCMD1(fgDepGraphTest,"depGraph");
    FGADDCMD1(fgExceptionTest,"exception");
    FGADDCMD1(fgFileSystemTest,"filesystem");
    FGADDCMD1(fgOpenTest,"open");
    FGADDCMD1(fgGeometryTest,"geometry");
    FGADDCMD1(fgGridTrianglesTest,"gridTriangles");
    FGADDCMD1(fgImageTest,"image");
    FGADDCMD(fgMatrixSolverTest,"matSol","Matrix Solver");
    FGADDCMD1(fgMathTest,"math");
    FGADDCMD(fgMatrixCTest,"matC","MatrixC");
    FGADDCMD(fgMatrixVTest,"matV","MatrixV");
    FGADDCMD1(fgMetaFormatTest,"metaFormat");
    FGADDCMD1(fgMorphTest,"morph");
    FGADDCMD1(fgPathTest,"path");
    FGADDCMD1(fgQuaternionTest,"quaternion");
    FGADDCMD(fgCmdRenderTest,"rendc","render command");
    FGADDCMD1(fgSerializeTest,"serialize");
    FGADDCMD1(fgSharedPtrTest,"sharedPtr");
    FGADDCMD1(fgSimilarityTest,"similarity");
    FGADDCMD1(fgSimilarityApproxTest,"similarityApprox");
    FGADDCMD1(fgStdVectorTest,"vector");
    FGADDCMD1(fgStringTest,"string");
    FGADDCMD1(fgTensorTest,"tensor");
    FGADDCMD1(fgVariantTest,"variant");
    cmds.push_back(fgSoftRenderTestInfo());
    return cmds;
}

static
void
testmGui(const FgArgs & args)
{
    vector<FgCmd>   cmds;
    FGADDCMD1(fgImgGuiTestm,"image");
    FGADDCMD1(fgTestmGuiMesh,"mesh");
    FGADDCMD1(fgGuiTestmText,"text");
    FGADDCMD1(fgGuiTestmScroll,"scroll");
    FGADDCMD1(fgGuiTestmDialogSplashScreen,"splash");
    fgMenu(args,cmds);
}

static
void
sysinfo(const FgArgs &)
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

vector<FgCmd>
fgCmdBaseTestms()
{
    vector<FgCmd>   cmds;
    cmds.push_back(FgCmd(testmGui,"gui"));
    FGADDCMD1(fg3dTestMan,"3d");
    FGADDCMD1(fgClusterTest,"cluster");
    FGADDCMD1(fgClusterTestm,"clusterm");
    FGADDCMD1(fgClusterDeployTestm,"clusterDeploy");
    FGADDCMD(fgCmdTestmCpp,"cpp","C++ behaviour tests");
    FGADDCMD(fgGaTestm,"ga","GUI API");
    FGADDCMD1(fg3dReadWobjTest,"readWobj");
    FGADDCMD1(fgRandomTest,"random");
    FGADDCMD1(fgGeometryManTest,"geometry");
    FGADDCMD1(fgSubdivisionTest,"subdivision");
    FGADDCMD1(fgTextureImageMappingRenderTest,"texturemap");
    FGADDCMD1(fgImageTestm,"image");
    return cmds;
}

static
void
testm(const FgArgs & args)
{fgMenu(args,fgCmdBaseTestms()); }

void
fgCmdBaseTest(const FgArgs & args)
{fgMenu(args,fgCmdBaseTests(),true); }

void
view(const FgArgs & args)
{fgMenu(args,fgCmdViewInfos()); }

/**
   \defgroup Base_Commands Base Library Command Line
   Commands in the program 'fgbl' demonstrating use of the FaceGen Base Library.
 */
void
fgCmdFgbl(const FgArgs & args)
{
    if (args.size() == 1)
        fgout << fgnl << "FaceGen Base Library CLI " << fgVersion(".") << " (" << fgCurrentBuildDescription() << ")"; 
    vector<FgCmd>   cmds;
    cmds.push_back(fgCmdImgopsInfo());
    cmds.push_back(fgCmdMeshopsInfo());
    cmds.push_back(fgCmdMorphInfo());
    cmds.push_back(fgCmdRenderInfo());
    cmds.push_back(fgCmdTriexportInfo());
    cmds.push_back(FgCmd(fgCmdCons,"cons","Construct makefiles / solution file / project files"));
    cmds.push_back(FgCmd(sysinfo,"sys","Show system info"));
    cmds.push_back(FgCmd(fgCmdBaseTest,"test","Automated tests"));
    cmds.push_back(FgCmd(testm,"testm","Manual tests"));
    cmds.push_back(FgCmd(view,"view","Interactively view various file types"));
    fgMenu(args,cmds);
}

// */
