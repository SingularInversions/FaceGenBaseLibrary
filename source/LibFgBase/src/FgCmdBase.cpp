//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgImage.hpp"
#include "FgTestUtils.hpp"
#include "FgBuild.hpp"
#include "FgSystem.hpp"
#include "FgCl.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgParse.hpp"
#include "Fg3dDisplay.hpp"
#include "FgGuiApi.hpp"

using namespace std;

namespace Fg {

void                cmd3dmmImport(CLArgs const & args)
{
    Syntax              syn {args,R"(<mean>.<ext> <fileList>.txt <out>.MatV3F
    <mean>          - file containing the mean shape base mesh with V vertices
    <ext>           - )" + getMeshLoadExtsCLDescription() + R"(
    <fileList>.txt  - list of mesh filenames with V verts for each of M linear basis modes (each added to the mean shape)
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
    GuiMorphMeshes          gpms;
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
    Img<GuiPtr>             sliderWs = guiSliders(coeffNs,cNumberedLabels(String8{"mode"},M),VecD2{-5,5},1);
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
    OPT<Vec3Fs>             identVertsN = link1(coordN,identFn);
    gpms.addMesh(meshN,identVertsN,base.surfaces.size());
    IPT<Mat32D>             viewBoundsN {Mat32D(bounds)};
    Gui3d                   gui3d {makeIPT(gpms.rendMeshes)};
    GuiTabDefs              tabs = {
        {"Expression",gpms.makePoseCtrls(true),true},
        {"Basis",guiSplitV({randW,slidersW})},
        {"View",makeViewCtrls(gui3d,viewBoundsN,store+"View") },
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


static void         sysinfo(CLArgs const &)
{
    fgout
        << fgnl << "Computer name: " << getComputerName()
        << fgnl << "OS: " << getOSString()
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

void                testBase(CLArgs const & args);
void                cmdView(CLArgs const & args);

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

void                cmdReplace(CLArgs const & args)
{
    Syntax              syn {args,R"(<search> <replace> <glob>
    <search>        - text to find
    <replace>       - text to replace it with
    <glob>          - simple file glob (* can only be used for entire base name or extension)
NOTES:
    'sed' for windows will not write in place
    'perl' for windows will not expand CL globs
    'awk' for windows ... I gave up and wrote this.)"
    };
    String              srch = syn.next(),
                        repl = syn.next();
    String8s            fileNames = globFiles(Path{syn.next()});
    if (fileNames.empty())
        syn.error("no matching files found");
    for (String8 const & fileName : fileNames) {
        String              in = loadRawString(fileName),
                            out;
        if (replaceAll_(in,srch,repl,out))
            saveRaw(out,fileName);
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

void                cmdTools(CLArgs const & args)
{
    Cmds                cmds {
        {cmdRename,"rename","rename files according to a pattern"},
        {cmdReplace,"replace","search and replace simple text in multiple files"},
        {sysinfo,"system","Show system info"},
    };
    doMenu(args,cmds);
}

void                cmdCons(CLArgs const &);
void                cmdGraph(CLArgs const &);
void                cmdImgops(CLArgs const &);
void                cmdMesh(CLArgs const &);
void                cmdMorph(CLArgs const &);
void                cmdRender(CLArgs const &);

void                cmdCompileShaders(CLArgs const &)
{
#ifdef _WIN32
    PushDir             pd {dataDir()+"base/shaders/"};
    // Seems from online chat that the default optimization /O1 is no different than /O2 or /O3 ...
    // /WX - warnings as errors
    // CSO - compiled shader object (.fxc is legacy; effects compiler)
    clRun("fxc /T vs_5_0 /E VSTransform /WX /Fo dx11_shared_VS.cso dx11_shared.hlsl");
    clRun("fxc /T vs_5_0 /E VSTransparentPass2 /WX /Fo dx11_transparent_VS.cso dx11_transparent.hlsl");
    clRun("fxc /T ps_5_0 /E PSOpaque /WX /Fo dx11_opaque_PS.cso dx11_opaque.hlsl");
    clRun("fxc /T ps_5_0 /E PSTransparentPass1 /WX /Fo dx11_transparent_PS1.cso dx11_transparent.hlsl");
    clRun("fxc /T ps_5_0 /E PSTransparentPass2 /WX /Fo dx11_transparent_PS2.cso dx11_transparent.hlsl");
#else
    fgThrow("Shader compilation not supported on this platform");
#endif
}

Cmds                getFgblCmds()
{
    Cmds        cmds {
        {cmd3dmm,"3dmm","3D morphable model commands"},
        {cmdCons,"cons","Construct makefiles / solution file / project files"},
        {cmdGraph,"graph","Create simple bar graphs from text data"},
        {cmdImgops,"image","Image operations"},
        {cmdMesh,"mesh","3D Mesh IO and manipulation tools"},
        {cmdMorph,"morph","List, apply or create animation morphs for 3D meshes"},
        {cmdRender,"render","Render meshes with color & specular maps to an image file"},
        {testBase,"test","test suite"},
        {cmdTools,"tools","Command-line tools"},
        {cmdView,"view","Interactive GUI view of images and meshes (Windows only)"},
#ifdef _WIN32
        {cmdCompileShaders,"d3d","Compile D3D shaders (Windows only)"},
#endif
    };
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

}

// */
