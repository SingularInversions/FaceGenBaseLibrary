//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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
#include "FgCl.hpp"

using namespace std;

namespace Fg {

void test3d(CLArgs const &);
void fgBoostSerializationTest(CLArgs const &);
void fgCmdTestDfg(CLArgs const &);
void fgExceptionTest(CLArgs const &);
void fgFileSystemTest(CLArgs const &);
void fgOpenTest(CLArgs const &);
void testGeometry(CLArgs const &);
void fgGridTrianglesTest(CLArgs const &);
void fgImageTest(CLArgs const &);
void testKdTree(CLArgs const &);
void fgMatrixSolverTest(CLArgs const &);
void testMath(CLArgs const &);
void testMatrixC(CLArgs const &);
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

Cmd testSoftRenderInfo();   // Don't put these in a macro as it generates a clang warning about vexing parse.

Cmds
fgCmdBaseTests()
{
    Cmds      cmds {
        {test3d,"3d"},
        {fgBoostSerializationTest,"boostSerialization"},
        {fgCmdTestDfg,"dataflow"},
        {fgExceptionTest,"exception"},
        {fgFileSystemTest,"filesystem"},
        {fgOpenTest,"open"},
        {testGeometry,"geometry"},
        {fgGridTrianglesTest,"gridTriangles"},
        {fgImageTest,"image"},
        {testKdTree,"kd"},
        {fgMatrixSolverTest,"matSol","Matrix Solver"},
        {testMath,"math"},
        {testMatrixC,"matC","MatrixC"},
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
    };
    cmds.push_back(testSoftRenderInfo());
    return cmds;
}

void fgImgGuiTestm(CLArgs const &);
void fgTestmGuiMesh(CLArgs const &);
void fgTestmGui2(CLArgs const &);
void fgGuiTestmDialogSplashScreen(CLArgs const &);

static
void
testmGui(CLArgs const & args)
{
    Cmds      cmds {
        {fgImgGuiTestm,"image"},
        {fgTestmGui2,"gui2"},
        {fgTestmGuiMesh,"mesh"},
        {fgGuiTestmDialogSplashScreen,"splash"}
    };
    doMenu(args,cmds);
}

static
void
sysinfo(CLArgs const &)
{
    fgout
        << fgnl << "Computer name: " << fgComputerName()
        << fgnl << "OS: " << osDescription()
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

void testm3d(CLArgs const &);
void fgClusterTest(CLArgs const &);
void fgClusterTestm(CLArgs const &);
void fgClusterDeployTestm(CLArgs const &);
void fgCmdTestmCpp(CLArgs const &);
void fg3dReadWobjTest(CLArgs const &);
void fgRandomTest(CLArgs const &);
void testmGeometry(CLArgs const &);
void fgTextureImageMappingRenderTest(CLArgs const &);
void fgImageTestm(CLArgs const &);

Cmds
fgCmdBaseTestms()
{
    Cmds      cmds {
        {testmGui,"gui"},
        {testm3d,"3d"},
        {fgClusterTest,"cluster"},
        {fgClusterTestm,"clusterm"},
        {fgClusterDeployTestm,"clusterDeploy"},
        {fgCmdTestmCpp,"cpp","C++ behaviour tests"},
        {fg3dReadWobjTest,"readWobj"},
        {fgRandomTest,"random"},
        {testmGeometry,"geometry"},
        {fgTextureImageMappingRenderTest,"texturemap"},
        {fgImageTestm,"image"}
    };
    return cmds;
}

static
void
testm(CLArgs const & args)
{doMenu(args,fgCmdBaseTestms()); }

void
fgCmdBaseTest(CLArgs const & args)
{doMenu(args,fgCmdBaseTests(),true); }

void
view(CLArgs const & args)
{doMenu(args,getViewCmds()); }

/**
   \defgroup Base_Commands Base Library Command Line
   Commands in the program 'fgbl' demonstrating use of the FaceGen Base Library.
 */
void
fgCmdFgbl(CLArgs const & args)
{
    if (args.size() == 1)
        fgout << fgnl << "FaceGen Base Library CLI " << getSdkVersion(".") << " (" << getCurrentBuildDescription() << ")"; 
    Cmds        cmds {
        {getImgopsCmd()},
        {getMeshopsCmd()},
        {getMorphCmd()},
        {getRenderCmd()},
        {getTriExportCmd()},
        {cmdCons,"cons","Construct makefiles / solution file / project files"},
        {sysinfo,"sys","Show system info"},
        {fgCmdBaseTest,"test","Automated tests"},
        {testm,"testm","Manual tests"},
        {view,"view","Interactively view various file types"}
    };
#ifdef _WIN32
    cmds.push_back(getCompileShadersCmd());
#endif
    doMenu(args,cmds);
}

Cmd
getCompileShadersCmd()
{
    Cmd         ret;
#ifdef _WIN32
    ret = Cmd {
        [](CLArgs const &)
        {
            PushDir     pd(dataDir()+"base/shaders/");
            // Seems from online chat that the default optimization /O1 is no different than /O2 or /O3 ...
            // /WX - warnings as errors
            // CSO - compiled shader object (.fxc is legacy; effects compiler)
            clRun("fxc /T vs_5_0 /E VSTransform /WX /Fo dx11_shared_VS.cso dx11_shared.hlsl");
            clRun("fxc /T ps_5_0 /E PSOpaque /WX /Fo dx11_opaque_PS.cso dx11_opaque.hlsl");
            clRun("fxc /T ps_5_0 /E PSTransparent /WX /Fo dx11_transparent_PS.cso dx11_transparent.hlsl");
        //    clRun("fxc /T vs_5_0 /E VSResolve /WX /Fo dx11_resolve_VS.cso dx11_resolve.hlsl");
        //    clRun("fxc /T ps_5_0 /E PSResolve /WX /Fo dx11_resolve_PS.cso dx11_resolve.hlsl");
            clRun("fxc /T cs_5_0 /E CSResolve /WX /Fo dx11_resolve_CS.cso dx11_resolve.hlsl");
        },
        "d3d",
        "Compile Direct3D shaders"
    };
#else
    fgThrow("Shader compilation not supported on this platform");
#endif
    return ret;
}

}

// */
