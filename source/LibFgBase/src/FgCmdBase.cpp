//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: July 15, 2015
//
/**
   \defgroup Base_Commands
   Commands in the program 'fgbl' demonstrating use of the FaceGen Base Library.
 */

#include "stdafx.h"

#include "FgCmd.hpp"
#include "FgSyntax.hpp"
#include "FgMetaFormat.hpp"
#include "FgImage.hpp"
#include "FgTestUtils.hpp"
#include "FgBuild.hpp"
#include "FgVersion.hpp"

using namespace std;

vector<FgCmd>
fgCmdBaseTests()
{
    vector<FgCmd>   cmds;
    //FGADDCMD1(fgApproxFuncTest,"approxFunc");
    FGADDCMD1(fg3dTest,"3d");
    FGADDCMD1(fgBoostSerializationTest,"boostSerialization");
    FGADDCMD1(fgClusterTest,"cluster");
    FGADDCMD1(fgDepGraphTest,"depGraph");
    FGADDCMD1(fgExceptionTest,"exception");
    FGADDCMD1(fgFileSystemTest,"filesystem");
    FGADDCMD1(fgGeometryTest,"geometry");
    FGADDCMD1(fgGridTrianglesTest,"gridTriangles");
    FGADDCMD1(fgImageTest,"image");
    FGADDCMD1(fgMatrixSolverTest,"matrixSolver");
    FGADDCMD1(fgMathTest,"math");
    FGADDCMD1(fgMatrixCTest,"matrixC");
    FGADDCMD1(fgMetaFormatTest,"metaFormat");
    FGADDCMD1(fgMorphTest,"morph");
    FGADDCMD1(fgPathTest,"path");
    FGADDCMD1(fgQuaternionTest,"quaternion");
    FGADDCMD1(fgRenderTest,"render");
    FGADDCMD1(fgSerializeTest,"serialize");
    FGADDCMD1(fgSharedPtrTest,"sharedPtr");
    FGADDCMD1(fgSimilarityTest,"similarity");
    FGADDCMD1(fgSimilarityApproxTest,"similarityApprox");
    FGADDCMD1(fgStdVectorTest,"vector");
    FGADDCMD1(fgStringTest,"string");
    FGADDCMD1(fgTensorTest,"tensor");
    FGADDCMD1(fgVariantTest,"variant");
    return cmds;
}

static
void
test(const FgArgs & args)
{fgMenu(args,fgCmdBaseTests(),true); }

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

vector<FgCmd>
fgCmdBaseTestms()
{
    vector<FgCmd>   cmds;
    cmds.push_back(FgCmd(testmGui,"gui"));
    FGADDCMD1(fg3dTestMan,"3d");
    FGADDCMD1(fgClusterTestm,"cluster");
    FGADDCMD1(fgClusterDeployTestm,"clusterDeploy");
    FGADDCMD(fgCmdTestmCpp,"cpp","C++ behaviour tests");
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
    cmds.push_back(FgCmd(test,"test","Automated tests"));
    cmds.push_back(FgCmd(testm,"testm","Manual tests"));
    cmds.push_back(FgCmd(view,"view","Interactively view various file types"));
    fgMenu(args,cmds);
}

// */
