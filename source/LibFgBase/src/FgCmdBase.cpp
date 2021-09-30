//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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

void    test3d(CLArgs const &);
void    testBaseJson(CLArgs const &);
void    testBoostSer(CLArgs const &);
void    testDataflow(CLArgs const &);
void    testExceptions(CLArgs const &);
void    testFilesystem(CLArgs const &);
void    testFopen(CLArgs const &);
void    testGeometry(CLArgs const &);
void    testGridTriangles(CLArgs const &);
void    testHash(CLArgs const &);
void    testImage(CLArgs const &);
void    testKdTree(CLArgs const &);
void    testMatrixSolver(CLArgs const &);
void    testMath(CLArgs const &);
void    testMatrixC(CLArgs const &);
void    testMatrixV(CLArgs const &);
void    testMetaFormat(CLArgs const &);
void    testMorph(CLArgs const &);
void    testPath(CLArgs const &);
void    testQuaternion(CLArgs const &);
void    testRenderCmd(CLArgs const &);
void    testSerial(CLArgs const &);
void    testSerialize(CLArgs const &);
void    testSimilarity(CLArgs const &);
void    testSimilaritySolve(CLArgs const &);
void    testStdVector(CLArgs const &);
void    testString(CLArgs const &);

Cmd testSoftRenderInfo();   // Don't put these in a macro as it generates a clang warning about vexing parse.

Cmds
getBaseTests()
{
    Cmds            cmds {
        {test3d,"3d"},
        {testBoostSer,"boostSerialization"},
        {testDataflow,"dataflow"},
        {testExceptions,"exception"},
        {testFilesystem,"filesystem"},
        {testFopen,"fopen"},
        {testGeometry,"geometry"},
        {testGridTriangles,"gridTriangles"},
        {testHash,"hash"},
        {testImage,"image"},
        {testBaseJson,"json"},
        {testKdTree,"kd"},
        {testMatrixSolver,"matSol","Matrix Solver"},
        {testMath,"math"},
        {testMatrixC,"matC","MatrixC"},
        {testMatrixV,"matV","MatrixV"},
        {testMetaFormat,"metaFormat"},
        {testMorph,"morph"},
        {testPath,"path"},
        {testQuaternion,"quaternion"},
        {testRenderCmd,"rendc","render command"},
        {testSerial,"serial"},
        {testSerialize,"serialize"},
        {testSimilarity,"similarity"},
        {testSimilaritySolve,"solveSimilarity"},
        {testStdVector,"vector"},
        {testString,"string"},
    };
    cmds.push_back(testSoftRenderInfo());
    return cmds;
}

void testmGuiImage(CLArgs const &);
void fgTestmGuiMesh(CLArgs const &);
void fgTestmGui2(CLArgs const &);
void fgGuiTestmDialogSplashScreen(CLArgs const &);

static
void
testmGui(CLArgs const & args)
{
    Cmds            cmds {
        {testmGuiImage,"image"},
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

static void testPopen(CLArgs const &)
{
    Opt<String>     out = clPopen("dir");
    if (out.valid())
        fgout << fgnl << "Command 'dir' output " << out.val().size() << " characters:\n" << out.val();
    else
        fgout << fgnl << "Command 'dir' failed";
}

void testm3d(CLArgs const &);
void fgClusterTest(CLArgs const &);
void fgClusterTestm(CLArgs const &);
void fgClusterDeployTestm(CLArgs const &);
void fgCmdTestmCpp(CLArgs const &);
void fg3dReadWobjTest(CLArgs const &);
void testmRandom(CLArgs const &);
void testmGeometry(CLArgs const &);
void fgTextureImageMappingRenderTest(CLArgs const &);
void testmImage(CLArgs const &);

Cmds
getBaseTestms()
{
    Cmds      cmds {
        {testmGui,"gui"},
        {testm3d,"3d"},
        {fgClusterTest,"cluster"},
        {fgClusterTestm,"clusterm"},
        {fgClusterDeployTestm,"clusterDeploy"},
        {fgCmdTestmCpp,"cpp","C++ behaviour tests"},
        {testPopen,"popen"},
        {fg3dReadWobjTest,"readWobj"},
        {testmRandom,"random"},
        {testmGeometry,"geometry"},
        {fgTextureImageMappingRenderTest,"texturemap"},
        {testmImage,"image"}
    };
    return cmds;
}

static
void
testm(CLArgs const & args)
{doMenu(args,getBaseTestms()); }

void
testBase(CLArgs const & args)
{doMenu(args,getBaseTests(),true); }

void
view(CLArgs const & args)
{doMenu(args,getViewCmds()); }

Cmd         getCmdGraph();

/**
   \ingroup Main_Commands
   Command to substitute strings for automated paramter setting in .xml parameter files.
 */
void
cmdSubstitute(CLArgs const & args)
{
    Syntax        syntax(args,
        "<inFile> <outFile> (<stringIn> <stringOut>)+\n"
        "    Substitute first occurence of each <stringIn> with respective <stringOut>");
    Ifstream      ifs(syntax.next());
    Ofstream      ofs(syntax.next());
    string          body(
        (istreambuf_iterator<char>(ifs)),   // Construct from beginning, brackets for parser
        istreambuf_iterator<char>());       // Default constructor is end of stream
    while (syntax.more()) {
        string      findStr(syntax.next()),
                    replStr(syntax.next());
        FGASSERT(!findStr.empty());
        size_t      pos;
        if ((pos = body.find(findStr)) != string::npos)
            body.replace(pos,findStr.size(),replStr);
    }
    ofs << body;
}

Cmds
getFgblCmds()
{
    Cmds        cmds {
        {getCmdGraph()},
        {getImgopsCmd()},
        {getCmdMesh()},
        {getMorphCmd()},
        {getCmdRender()},
        {cmdCons,"cons","Construct makefiles / solution file / project files"},
        {cmdSubstitute,"substitute","Substitute first instance of exact strings in a text file"},
        {sysinfo,"sys","Show system info"},
        {testBase,"test","Automated tests"},
        {testm,"testm","Manual tests"},
        {view,"view","Interactive GUI view of images and meshes (Windows only)"}
    };
#ifdef _WIN32
    cmds.push_back(getCompileShadersCmd());
#endif
    return cmds;
}

/**
   \defgroup Base_Commands Base Library Command Line
   Commands in the program 'fgbl' demonstrating use of the FaceGen Base Library.
 */
void
cmdFgbl(CLArgs const & args)
{
    if (args.size() == 1)
        fgout << fgnl << "FaceGen Base Library CLI " << getSdkVersion(".") << " (" << getCurrentBuildDescription() << ")"; 
    doMenu(args,getFgblCmds());
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
            clRun("fxc /T vs_5_0 /E VSTransparentPass2 /WX /Fo dx11_transparent_VS.cso dx11_transparent.hlsl");
            clRun("fxc /T ps_5_0 /E PSOpaque /WX /Fo dx11_opaque_PS.cso dx11_opaque.hlsl");
            clRun("fxc /T ps_5_0 /E PSTransparentPass1 /WX /Fo dx11_transparent_PS1.cso dx11_transparent.hlsl");
            clRun("fxc /T ps_5_0 /E PSTransparentPass2 /WX /Fo dx11_transparent_PS2.cso dx11_transparent.hlsl");
        },
        "d3d",
        "Compile Direct3D shaders (Windows only)"
    };
#else
    fgThrow("Shader compilation not supported on this platform");
#endif
    return ret;
}

}

// This include file causes link problem with gcc debug so quarantine to Windows:

#ifdef _WIN32

#include "json.hpp"

namespace Fg {

void
testBaseJson(CLArgs const &)
{
    String          str {
R"(
{
    "InstanceStatuses": [
        {
            "AvailabilityZone": "us-east",
            "InstanceId": "i-12345",
            "InstanceState": {
                "Code": 16,
                "Name": "running"
            },
            "InstanceStatus": {
                "Details": [
                    {
                        "Name": "reachability",
                        "Status": "passed"
                    }
                ],
                "Status": "ok"
            }
        },
        {
            "AvailabilityZone": "us-west",
            "InstanceId": "i-67890",
            "InstanceState": {
                "Code": 16,
                "Name": "running"
            },
            "InstanceStatus": {
                "Details": [
                    {
                        "Name": "reachability",
                        "Status": "passed"
                    }
                ],
                "Status": "ok"
            }
        }
    ]
})"
    };
    // The call to 'parse' is required:
    nlohmann::json          j0 {nlohmann::json::parse(str)},
                            j1 = j0.find("InstanceStatuses").value();
    // The .key() and .get<String>() calls below fail with gcc:
    size_t                  idx {0};
    // Lists iterate in given order:
    for (auto it1=j1.begin(); it1!=j1.end(); ++it1) {
        PushIndent              pi {toStr(idx++)};
        nlohmann::json          j2 = it1.value();
        // Containers iterate in key-alphabetical order.
        for (auto it2=j2.begin(); it2!=j2.end(); ++it2)
            fgout << fgnl << it2.key() << " : " << it2.value();
        // But we don't need to iterate we can just find:
        auto                    itf = j2.find("InstanceId");
        nlohmann::json          id = itf.value();
        String                  idStr = id.get<String>();
        fgout << fgnl << "InstanceId: " << idStr;
    }
}

}

#else

namespace Fg {
void testBaseJson(CLArgs const &) {}
}

#endif

// */
