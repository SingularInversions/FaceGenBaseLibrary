//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: July 15, 2015
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgSyntax.hpp"
#include "FgMetaFormat.hpp"
#include "FgImage.hpp"
#include "FgTestUtils.hpp"

using namespace std;

#define ADDCMD1(fn,handle) void fn(const FgArgs &); cmds.push_back(FgCmd(fn,handle))

vector<FgCmd>
fgCmdBaseTests()
{
    vector<FgCmd>   cmds;
    ADDCMD1(fgApproxFuncTest,"approxFunc");
    ADDCMD1(fgBoostSerializationTest,"boostSerialization");
    ADDCMD1(fgDepGraphTest,"depGraph");
    ADDCMD1(fgExceptionTest,"exception");
    ADDCMD1(fgFileSystemTest,"filesystem");
    ADDCMD1(fgGeometryTest,"geometry");
    ADDCMD1(fgGridTrianglesTest,"gridTriangles");
    ADDCMD1(fgImageTest,"image");
    ADDCMD1(fgMatrixSolverTest,"matrixSolver");
    ADDCMD1(fgMathTest,"math");
    ADDCMD1(fgMatrixCTest,"matrixC");
    ADDCMD1(fgMatrixVTest,"matrixV");
    ADDCMD1(fgMetaFormatTest,"metaFormat");
    ADDCMD1(fgMorphTest,"morph");
    ADDCMD1(fgPathTest,"path");
    ADDCMD1(fgQuaternionTest,"quaternion");
    ADDCMD1(fgRenderTest,"render");
    ADDCMD1(fgSerializeTest,"serialize");
    ADDCMD1(fgSharedPtrTest,"sharedPtr");
    ADDCMD1(fgSimilarityTest,"similarity");
    ADDCMD1(fgSimilarityApproxTest,"similarityApprox");
    ADDCMD1(fgStringTest,"string");
    ADDCMD1(fgVariantTest,"variant");
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
    ADDCMD1(fgImgGuiTestm,"image");
    ADDCMD1(fgTestmGuiMesh,"mesh");
    ADDCMD1(fgGuiTestmText,"text");
    ADDCMD1(fgGuiTestmScroll,"scroll");
    ADDCMD1(fgGuiTestmDialogSplashScreen,"splash");
    fgMenu(args,cmds);
}

vector<FgCmd>
fgCmdBaseTestms()
{
    vector<FgCmd>   cmds;
    cmds.push_back(FgCmd(testmGui,"gui"));
    ADDCMD1(fg3dTest,"3d");
    ADDCMD1(fg3dReadWobjTest,"readWobj");
    ADDCMD1(fgRandomTest,"random");
    ADDCMD1(fgGeometryManTest,"geometry");
    ADDCMD1(fgSubdivisionTest,"subdivision");
    ADDCMD1(fgTextureImageMappingRenderTest,"texturemap");
    ADDCMD1(fgImageTestm,"image");
    return cmds;
}

static
void
testm(const FgArgs & args)
{fgMenu(args,fgCmdBaseTestms()); }

void
fgCmdBaseTest(const FgArgs & args)
{fgMenu(args,fgCmdBaseTests(),true); }

FgCmd   fgCmdImgopsInfo();
FgCmd   fgCmdMeshopsInfo();
FgCmd   fgCmdMorphInfo();
FgCmd   fgCmdRenderInfo();
void    fgCmdCons(const FgArgs &);

vector<FgCmd> fgCmdViewInfos();

void
view(const FgArgs & args)
{fgMenu(args,fgCmdViewInfos()); }

void
fgCmdFgbl(const FgArgs & args)
{
    vector<FgCmd>   cmds;
    cmds.push_back(fgCmdImgopsInfo());
    cmds.push_back(fgCmdMeshopsInfo());
    cmds.push_back(fgCmdMorphInfo());
    cmds.push_back(fgCmdRenderInfo());
    cmds.push_back(FgCmd(fgCmdCons,"cons","Construct makefiles / solution file / project files"));
    cmds.push_back(FgCmd(test,"test","Automated tests"));
    cmds.push_back(FgCmd(testm,"testm","Manual tests"));
    cmds.push_back(FgCmd(view,"view","Interactively view various file types"));
    fgMenu(args,cmds);
}

// */
