//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
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
#include "FgSystemInfo.hpp"
#include "FgCl.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgParse.hpp"
#include "Fg3dDisplay.hpp"
#include "FgGui.hpp"

using namespace std;
using json = nlohmann::json;

namespace Fg {

void                cmd3dmmImport(CLArgs const & args)
{
    Syntax              syn {args,R"(<mean>.<ext> <fileList>.txt <out>.MatV3F
    <mean>          - file containing the mean shape base mesh with V vertices
    <ext>           - )" + getMeshLoadExtsCLDescription() + R"(
    <fileList>.txt  - list of mesh filename with V verts for each of M linear basis modes (each added to the mean shape)
OUTPUT:
    <out>.MatV3F    - FaceGen binary serialized matrix of Vec3F with V rows and M columns
NOTES:
    the output file can be viewed using the command 'fgbl 3dmm view')"
    };
    Mesh                base = loadMesh(syn.next());
    Strings             modeFiles = splitWhitespace(loadRawString(syn.next()));
    Vec3Fss             modesMV;
    for (String const & mf : modeFiles) {
        Vec3Fs              verts = loadMesh(mf).verts;
        if (verts.size() != base.verts.size())
            syn.error("base and mode meshes must have identical size vertex lists");
        modesMV.push_back(verts-base.verts);
    }
    MatV<Vec3F>         modes {transpose(modesMV)};
    saveMessage(modes,syn.next());
}

void                cmd3dmmView(CLArgs const & args)
{
    Syntax              syn {args,R"(<mean>.<ext> <modes>.MatV3F
    <mean>          - file containing the mean shape base mesh with V vertices
    <ext>           - )" + getMeshLoadExtsCLDescription() + R"(
    <modes>.MatV3F  - FaceGen binary serialized matrix of Vec3F with V rows and M columns)"
    };
    Mesh                base = loadMesh(syn.next());
    MatV<Vec3F>         modes = loadMessage<MatV<Vec3F>>(syn.next());
    size_t              V = base.verts.size(),
                        M = modes.numCols();
    if (modes.numRows() != V)
        syn.error("vertex count of base and modes differs",toStr(V)+"!="+toStr(modes.numRows()));
    // GUI:
    String8                 store = getDirUserAppDataLocalFaceGen({"SDK","3dmm view"});
    GuiPosedMeshes          gpms;
    Mat32F                  bounds = cBounds(base.verts);
    IPT<Mesh>               meshN(base);
    Svec<IPT<double>>       coeffNs = makeIPTs(Doubles(M,0.0));
    OPT<Doubles>            coordN = linkCollate(coeffNs);
    auto                    randFn = [coeffNs,M]()
    {
        for (size_t mm=0; mm<M; ++mm)
            coeffNs[mm].set(randNormal());
    };
    GuiPtr                  randW = guiSplitH({
        guiButton("Random",randFn),
        guiText("iid standard normals to each coeff")});
    Img<GuiPtr>             sliderWs = guiSliders(coeffNs,numberedLabels("mode",M),VecD2{-5,5},1);
    GuiPtr                  slidersW = guiSplitScroll(sliderWs);
    auto                    identFn = [&,V,M](Doubles const & coord)
    {
        Vec3Fs                  ret; ret.reserve(V);
        for (size_t vv=0; vv<V; ++vv) {
            Vec3F                   acc = base.verts[vv];
            Vec3F const *           rowPtr = modes.rowPtr(vv);
            for (size_t mm=0; mm<M; ++mm)
                acc += rowPtr[mm] * scast<float>(coord[mm]);
            ret.push_back(acc);
        }
        return ret;
    };
    OPT<Vec3Fs>             identVertsN = link1<Doubles,Vec3Fs>(coordN,identFn);
    gpms.addMesh(meshN,identVertsN,base.surfaces.size());
    IPT<Mat32D>             viewBoundsN {Mat32D(bounds)};
    Gui3d                   gui3d {makeIPT(gpms.rendMeshes)};
    GuiTabDefs              tabs = {
        {"Expression",true,gpms.makePoseCtrls(true)},
        {"Basis",guiSplitV({randW,slidersW})},
        {"View",false,makeViewCtrls(gui3d,viewBoundsN,store+"View") },
    };
    guiStartImpl(
        makeIPT<String8>("FaceGen SDK 3dmm view"),
        guiSplitAdj(true,make_shared<Gui3d>(gui3d),guiTabs(tabs)),
        store);
}

void                cmd3dmm(CLArgs const & args)
{
    Cmds                cmds {
        {cmd3dmmImport,"import","import 3DMM modes from a list of mesh files"},
        {cmd3dmmView,"view","view a base mesh with compatible 3DMM modes"},
    };
    doMenu(args,cmds);
}

Cmd testSoftRenderInfo();   // Don't put these in a macro as it generates a clang warning about vexing parse.

void testmGuiImage(CLArgs const &);
void testGui2(CLArgs const &);
void testGuiDialogSplashScreen(CLArgs const &);

static void         testmGui(CLArgs const & args)
{
    Cmds            cmds {
        {testmGuiImage,"image"},
        {testGui2,"gui2"},
        {testGuiDialogSplashScreen,"splash"}
    };
    doMenu(args,cmds);
}

static void         sysinfo(CLArgs const &)
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
void cmdTestmCpp(CLArgs const &);
void fg3dReadWobjTest(CLArgs const &);
void testmRandom(CLArgs const &);
void testmGeometry(CLArgs const &);
void fgTextureImageMappingRenderTest(CLArgs const &);
void testmImage(CLArgs const &);

static void         testmBase(CLArgs const & args)
{
    Cmds            cmds {
        {testmGui,"gui"},
        {testm3d,"3d"},
        {cmdTestmCpp,"cpp","C++ behaviour tests"},
        {fg3dReadWobjTest,"readWobj"},
        {testmRandom,"random"},
        {testmGeometry,"geometry"},
        {fgTextureImageMappingRenderTest,"texturemap"},
        {testmImage,"image"}
    };
    doMenu(args,cmds);
}

// Test nlohmann/json library
// * this is a very large include file with loads of templated operator overloads
// * it is over-complex and hard to debug but well supported
// * do not use brace initializers with these objects; you will get gcc errors
void                testNlohmannJson(CLArgs const &)
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
    json                    j0 = nlohmann::json::parse(str),
                            j1 = j0.at("InstanceStatuses");
    size_t                  idx {0};
    // Arrays iterate in given order:
    for (auto const & it1 : j1.items()) {
        PushIndent              pi {toStr(idx++)};
        json                    j2 = it1.value();
        // Containers iterate in key-alphabetical order:
        for (auto const & it2 : j2.items())
            fgout << fgnl << it2.key() << " : " << it2.value();
        // But we don't need to iterate we can just find:
        auto                    itf = j2.find("InstanceId");
        json                    id = itf.value();
        String                  idStr = id.get<String>();
        fgout << fgnl << "InstanceId: " << idStr;
    }
}

void    test3d(CLArgs const &);
void    testDataflow(CLArgs const &);
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
void    testSampler(CLArgs const &);
void    testSerial(CLArgs const &);
void    testSimilarity(CLArgs const &);
void    testSurfTopo(CLArgs const &);
void    testString(CLArgs const &);

void                testBase(CLArgs const & args)
{
    Cmds            cmds {
        {test3d,"3d"},
        {testDataflow,"dataflow"},
        {testFilesystem,"filesystem"},
        {testFopen,"fopen"},
        {testGeometry,"geometry"},
        {testGridTriangles,"gridTriangles"},
        {testHash,"hash"},
        {testImage,"image"},
        {testNlohmannJson,"json"},
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
        {testSampler,"sampler"},
        {testSerial,"serial"},
        {testSimilarity,"sim","similarity transform and solver"},
        {testSurfTopo,"topo","surface topology analysis"},
        {testString,"string"},
    };
    cmds.push_back(testSoftRenderInfo());
    doMenu(args,cmds,true);
}

void                view(CLArgs const & args) {doMenu(args,getViewCmds()); }

Cmd                 getCmdGraph();

void                cmdRenExt(CLArgs const & args)
{
    Syntax              syn {args,R"([-s] <extOld> <baseAppend> <extNew>
    -s              - apply rename in each subdirectory of the current dir
    <extOld>        - apply rename to all files ending in this extension
    <baseAppend>    - add this string to the base name of matching file
    <extNew>        - change the extension to this string
OUTPUT:
    every file of the form <base>.<extOld> is renamed to <base><baseAppend>.<extNew>
NOTES:
    throws if the rename results in a filename collision
)"
    };
    bool                subdirs = false;
    if (syn.peekNext() == "-s") {
        subdirs = true;
        syn.next();
    }
    String              extOld = syn.next(),
                        baseAppend = syn.next(),
                        extNew = syn.next();
    auto                fn = [=]() {
        DirContents         dirContents = getDirContents(".");
        for (String8 const & fname : dirContents.filenames) {
            Path                path {fname};
            if (path.ext == extOld) {       // it's a match
                String8             newName = path.dirBase() + baseAppend + "." + extNew;
                fgout << fgnl << fname << " -> " << newName << flush;
                fileMove(fname,newName);
            }
        }
    };
    if (subdirs) {
        DirContents         dirContents = getDirContents(".");
        for (String8 const & dname : dirContents.dirnames) {
            PushDir             pdir {dname};
            PushIndent          pind {dname.m_str};
            fn();
        }
    }
    else {
        PushIndent          pint {"renaming"};
        fn();
    }
}

void                cmdRenDir(CLArgs const & args)
{
    Syntax              syn {args,R"(start
OUTPUT:
    for every directory <dir> in the current directory, renames / moves all files of the form <dir>/<file> to
    <dir>-<file> in the current directory.
NOTES:
    throws if the rename / move results in a filename collision)"
    };
    if (syn.next() == "start") {
        DirContents         dirContents = getDirContents(".");
        for (String8 const & dname : dirContents.dirnames) {
            PushIndent          pind {dname.m_str};
            DirContents         dc = getDirContents(dname);
            for (String8 const & fname : dc.filenames) {
                String8             newName = dname + "-" + fname;
                fgout << fgnl << fname << flush;
                fileMove(dname + "/" + fname,newName);
            }
        }
    }
}

void                cmdRename(CLArgs const & args)
{
    Cmds            cmds {
        {cmdRenExt,"ext","rename files based on extension"},
        {cmdRenDir,"dir","rename files based on directory"}
    };
    doMenu(args,cmds);
}

/**
   \ingroup Main_Commands
   Command to substitute strings for automated paramter setting in .xml parameter files.
 */
void                cmdSubstitute(CLArgs const & args)
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

Cmd                 getCmdImage();
void                cmdMesh(CLArgs const &);
void                cmdMorph(CLArgs const &);
void                cmdRender(CLArgs const &);

Cmds                getFgblCmds()
{
    Cmds        cmds {
        {cmd3dmm,"3dmm","3D morphable model commands"},
        {getCmdGraph()},
        {getCmdImage()},
        {cmdMesh,"mesh","3D Mesh IO and manipulation tools"},
        {cmdMorph,"morph","List, apply or create animation morphs for 3D meshes"},
        {cmdRender,"render","Render meshes with color & specular maps to an image file"},
        {cmdCons,"cons","Construct makefiles / solution file / project files"},
        {cmdSubstitute,"substitute","Substitute first instance of exact strings in a text file"},
        {cmdRename,"rename","rename files according to a pattern"},
        {sysinfo,"sys","Show system info"},
        {testBase,"test","Automated tests"},
        {testmBase,"testm","Manual tests"},
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
void                cmdFgbl(CLArgs const & args)
{
    if (args.size() == 1)
        fgout << fgnl << "FaceGen Base Library CLI " << getSdkVersion(".") << " (" << getCurrentBuildDescription() << ")"; 
    doMenu(args,getFgblCmds());
}

Cmd                 getCompileShadersCmd()
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

// */
