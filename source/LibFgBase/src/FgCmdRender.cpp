//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Command to software-render meshes to image files.
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dCamera.hpp"
#include "FgSoftRender.hpp"
#include "FgTime.hpp"
#include "FgImgDisplay.hpp"
#include "FgParse.hpp"
#include "FgTestUtils.hpp"
#include "FgBuild.hpp"
#include "FgGridTriangles.hpp"

using namespace std;

namespace Fg {

namespace {

struct      ModelFile
{
    String              meshFilename;
    Strings             imgFilenames;
    bool                shiny = false;
    FG_SER3(meshFilename,imgFilenames,shiny)
};
typedef Svec<ModelFile>     ModelFiles;

struct      RenderArgs
{
    ModelFiles              models;
    double                  roll = 0,       // Z axis rotation, radians, in order:
                            tilt = 0,       // X axis rotation
                            pan = 0;        // Y axis rotation
    SimilarityD             modelview;      // Transform world to openGL eye coordinate system (OECS)
    // transform from projected coordinates (image tangent CS) to image unit CS (x,y in [0,1]):
    AffineEw2D              itcsToIucs;
    Vec2UI                  imagePixelSize = Vec2UI(512,512);
    RenderOptions           options;
    FG_SER8(models,roll,tilt,pan,modelview,itcsToIucs,imagePixelSize,options)
};

Meshes              loadMeshes(ModelFiles const & mfs)
{
    Meshes          meshes;
    for (ModelFile const & mf : mfs) {
        meshes.push_back(loadMesh(mf.meshFilename));
        for (size_t ss=0; ss<mf.imgFilenames.size(); ++ss) {
            if (ss < meshes.back().surfaces.size()) {
                meshes.back().surfaces[ss].setAlbedoMap(loadImage(mf.imgFilenames[ss]));
                meshes.back().surfaces[ss].material.shiny = mf.shiny;
            }
            else
                fgout << "WARNING: more images specified than surfaces for mesh " << mf.meshFilename;
        }
    }
    return meshes;
}

void                cmdRenderSetup(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<name>.txt (-<option> <value>)* (<mesh>.<ext> [<albedo>.<img>])*
    <option>        - (roll | tilt | pan) - specify object pose, with <value> given in degrees
    <ext>           - )" + getMeshLoadExtsCLDescription() + R"(
    <img>           - )" + clOptionsStr(getImgExts()) + R"(
OUTPUT:
    <name>.txt      - render configuration file
NOTES:
    * creates default rendering parameters for the given models
    * fields in <name>.txt can be modified as long as the XML structure remains valid
    <name>.txt description:
        models:
            The list of mesh and related albedo map files to be rendered.
        roll:,tilt:,pan:
            Euler angle rotations in radians applied to the models, in this order, before
            application of <modelview> below. Can be used to orient your meshes for proper viewing
            (FG head coordinate system is X - head's left, Y - head's up, Z - head's forward),
            or to orient the head as desired. Defaults are 0.
        modelview:
            The similarity transform from mesh coordinates to openGL eye coordinates,
            consisting of scale, rotation (as a quaternion) and translation, applied in that order.
            Default places subject at a viewing distance from the camera to mostly fill the frame.
        itcsToIucs:
            The 2D transform from projected coordinates ("image tangent CS") to the
            "image unit CS" in which x,y are in [0,1] with origin at top left of image.
        imagePixelSize:
            Pixel width, height
        options:
            lighting: all color component values below are in the range [0,1]:
                ambient: light coming from all directions.
                lights:
                    colour: RGB order
                    direction: in order of X,Y,Z in OpenGL Eye Coordinates.
            backgroundColor: RGBA order, values in [0,1].
            antiAliasBitDepth: 3 is high quality (equivalent to 8x FSAA) without running too slowly
            renderSurfPoints: 0 - don't, 1 - when visible, 2 - always.
                They are rendered as single-pixel green dots over the image.)"
    };
    string              name = syn.next();
    RenderArgs          rend;
    while (syn.more() && (syn.peekNext()[0] == '-')) {          // options
        String          opt = syn.next();
        if (opt == "-roll")
            rend.roll = degToRad(syn.nextAs<float>());
        else if (opt == "-tilt")
            rend.tilt = degToRad(syn.nextAs<float>());
        else if (opt == "-pan")
            rend.pan = degToRad(syn.nextAs<float>());
        else
            syn.error("Unrecognized option",opt);
    }
    while (syn.more()) {                                        // meshes & maps
        ModelFile           mf;
        mf.meshFilename = syn.next();
        while (syn.more() && hasImgFileExt(syn.peekNext()))
            mf.imgFilenames.push_back(syn.next());
        rend.models.push_back(mf);
    }
    // Load data from files to validate and set modelview:
    Meshes              meshes = loadMeshes(rend.models);
    Mat32F              bounds = cBounds(meshes);
    CameraParams        cps {Mat32D{bounds}};
    cps.logRelScale = std::log(0.9);
    cps.fovMaxDeg = 17.0;
    Camera              cam = cps.camera(rend.imagePixelSize);
    rend.modelview = cam.modelview;
    rend.itcsToIucs = cam.itcsToIucs;
    saveRaw(srlzText(rend),name);
}

void                renderRun(String const & rendFile,String const & outName)
{
    RenderArgs          rend = dsrlzText<RenderArgs>(loadRawString(rendFile));
    Meshes              meshes = loadMeshes(rend.models);
    // Receive surf point projection data:
    rend.options.projSurfPoints = std::make_shared<ProjectedSurfPoints>();
    SimilarityD         mvm = rend.modelview * cRotateY(rend.pan) * cRotateX(rend.tilt) * cRotateZ(rend.roll);
    ImgRgba8            image = renderSoft(rend.imagePixelSize,meshes,mvm,rend.itcsToIucs,rend.options);
    saveImage(outName,image);
    String8             outBase = pathToBase(outName);
    Ofstream            ofs {outBase+"-landmarks.csv"};
    for (ProjectedSurfPoint const & psp : *rend.options.projSurfPoints)
        ofs << psp.label << "," << psp.posIucs << "," << (psp.visible ? "true" : "false") << "\n";
    Camera              cam {mvm,{},rend.itcsToIucs};
    saveRaw(srlzText(cam.projectIpcs(rend.imagePixelSize)),outBase+"-matrix.txt");
}

void                cmdRenderRun(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<in>.txt <out>.<img>
    <in>.txt    - the render configuration file created using 'fgbl render setup'
    <img>       - )" + clOptionsStr(getImgExts()) + R"(
OUTPUT:
    <out>.<img>         -   the rendered image
    <out>-matrix.txt    -   the homogeneous combined projection*modelview matrix resulting from the camera model
    <out>-landmarks.csv -   label, image position (IUCS) and visibility of projected surface points,
                            ordered by mesh then surface then point. Surface points can be interactively placed
                            on a mesh (Windoows only) using 'fgbl view mesh' on a single mesh,
                            from the 'Edit' -> 'Points' tab, and saved in either TRI or FGMESH formats.)"
    };
    String              inFile = syn.next(),
                        outFile = syn.next();
    PushTimer           pt {"Rendering and writing files"};
    renderRun(inFile,outFile);
}

void                cmdRenderBatch(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<files>.txt <img>
    <img>       - )" + clOptionsStr(getImgExts()) + R"(
OUTPUT:
    for each render parameter file <name>.xml listed in <files>.txt, the following files are created:
    <name>.<img>             the rendered image
    <name>-matrix.txt        the homogeneous combined projection*modelview matrix resulting from the camera model
    <name>-landmarks.csv     label, image position (IUCS) and visibility of projected surface points,
                             ordered by mesh then surface then point.
NOTES:
    renders are done in parallel using all physical cores on the current machine)"
    };
    Strings             rendFiles = splitWhitespace(loadRawString(syn.next()));
    String              imgExt = syn.next();
    if (!contains(getImgExts(),toLower(imgExt)))
        syn.error("Unrecognized image format",imgExt);
    auto                runFn = [](String const & rendFile,String const & imgFile)
    {
        try { renderRun(rendFile,imgFile); }
        catch (FgException & e) {fgout << "ERROR: " << rendFile << ": " << e.tr_message(); }
        catch (std::exception & e) { fgout << "ERROR: " << rendFile << ": " << e.what(); }
    };
    PushTimer           pt {"Dispatching renders "};
    ThreadDispatcher    td;
    for (String const & rendFile : rendFiles) {
        String              imgFile = pathToDirBase(rendFile).m_str+"."+imgExt;
        td.dispatch(bind(runFn,rendFile,imgFile));
        fgout << ".";
    }
}

bool                imgApproxEqual(String8 const & file0,String8 const & file1)
{
    ImgRgba8        img0 = loadImage(file0),
                    img1 = loadImage(file1);
    return isApproxEqual(img0,img1,2);
}

}   // namespace

void                cmdRender(CLArgs const & args)
{
    Cmds            cmds {
        {cmdRenderBatch,"batch","batch render from a text file list of configuration file names"},
        {cmdRenderSetup,"setup","setup a render configuration file"},
        {cmdRenderRun,"run","run a render from a configuration file"},
    };
    doMenu(args,cmds);
}

void                testRenderCmd(CLArgs const & args)
{
    FGTESTDIR
    copyDataFileToCurr("base/Jane.tri");
    copyDataFileToCurr("base/Jane.jpg");
    copyDataFileToCurr("base/test/cmd-render.txt");
    RenderArgs          rend = dsrlzText<RenderArgs>(loadRawString("cmd-render.txt"));
    cmdRenderRun(splitChar("run cmd-render.txt cmd-render.png"));
    regressFileRel("cmd-render.png","base/test/",imgApproxEqual);
}

}
