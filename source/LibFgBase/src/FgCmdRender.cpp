//
// Copyright (c) 2018 Singular Inversions Inc. (facegen.com)
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
#include "FgSyntax.hpp"
#include "FgMetaFormat.hpp"
#include "FgImgDisplay.hpp"
#include "FgParse.hpp"
#include "FgTestUtils.hpp"
#include "FgBuild.hpp"
#include "FgGridTriangles.hpp"

using namespace std;

namespace Fg {

namespace FgCmdRender {     // Avoid global namespace visibility of local types below

struct  ModelFiles
{
    string          triFilename;
    string          imgFilename;
    bool            shiny = false;

    FG_SERIALIZE3(triFilename,imgFilename,shiny)
};

struct  Pose
{
    QuaternionD       rotateToHcs;
    // Euler angles applied in HCS in following order after 'rotateToHcs':
    double              rollRadians = 0.0;
    double              tiltRadians = 0.0;
    double              panRadians = 0.0;
    // Model translation parallel to image plane, relative to half max model bound:
    Vec2D            relTrans;
    // Scale relative to automatically determined size of object in image:
    double              relScale = 0.9;
    // Field of view of larger image dimension (degrees). Must be > 0 but can set as low as 0.0001
    // to simulate orthographic projection:
    double              fovMaxDeg = 17.0;

    FG_SERIALIZE7(rotateToHcs,rollRadians,tiltRadians,panRadians,relTrans,relScale,fovMaxDeg)
};

struct  RenderArgs
{
    vector<ModelFiles>      models;
    Pose                    pose;
    Vec2UI               imagePixelSize = Vec2UI(512,512);
    FgRenderOptions         options;

    FG_SERIALIZE4(models,pose,imagePixelSize,options);
};

struct  Options
{
    RenderArgs              rend;
    // Write a CSV file of label, image position (IUCS) and visibility of projected surface points,
    // ordered by mesh->surface->point:
    bool                    saveSurfPointFile = false;
    string                  outputFile;

    FG_SERIALIZE3(rend,saveSurfPointFile,outputFile);
};

}   // namespace FgCmdRender

using namespace FgCmdRender;

/**
   \ingroup Base_Commands
   Command to render a mesh and colour map to an image.
 */
void
fgCmdRender(CLArgs const & args)
{
    Syntax    syntax(args,
        "<name> [-s <view>] [-l <view>] (<mesh>.tri [<image>.<ext1>])*\n"
        "    - Render specified meshes [with texture images] using default render arguments.\n"
        "    - Saves render arguments to <name>.xml and rendered image to <name>.png\n"
        "    -s     - Save the object pose and camera intrinsics in <view>_pose.xml and <view>_cam.xml\n"
        "    -l     - Load the object pose and camera intrinsics from the above files, "
                     "do not calculate from <name>.xml\n"
        "    <ext1> - " + imgFileExtensionsDescription() + "\n"
        "NOTES:\n"
        "    - If no mesh arguments are given, <name>.xml will be used for the arguments.\n"
        "ARGUMENTS XML:\n"
        "<name>.xml :\n"
        "    <models> - The list of mesh filenames to be rendered.\n"
        "    <pose>\n"
        "        <rotateToHcs> - Quaternion specifying rotation from mesh original coordinates to\n"
        "            head coordinate system (HCS) with head facing in Z direction, X to face's left.\n"
        "        <rollRadians>,<tiltRadians>,<panRadians> - Euler angles applied from HCS\n"
        "        <relTrans> - Translation in HCS relative with unit scale equal to half max bound.\n"
        "        <relScale> - Scale from original mesh scale.\n"
        "        <fovMaxDeg> - Maximum field of view angle of camera in degrees (applied to larger image dimension).\n"
        "    <imagePixelSize>\n"
        "    <options>\n"
        "        <lighting> - All color component values below are in the range [0,1]:\n"
        "            <ambient> - Light coming from all directions.\n"
        "            <lights>\n"
        "                <colour> - In order of Red, Greeen, Blue.\n"
        "                <direction> - In order of X,Y,Z in OpenGL Eye Coordinates.\n"
        "        <backgroundColor> - In order of R,G,B,A where all values in range [0,1].\n"
        "        <antiAliasBitDepth> - Defaults to 3, use 4 or 5 for higher quality (slower).\n"
        "        <renderSurfPoints> - 0 means don't, 1 means when visible and 2 means always.\n"
        "            They are rendered as single-pixel green dots over the image.\n"
        "    <saveSurfPointFile> - 0 means don't, 1 means save <name>.csv with a list of surface points\n"
        "        written as <label>,<position>,<visible>, where <position> is in image unit coordinates; [0,1]\n"
        "    <outputFile> - Name of image output file."
    );
    string          renderName = syntax.next();
    Options         opts;
    string          viewSave,    // If empty, option not selected
                    viewLoad;    // "
    while (syntax.more() && (syntax.peekNext()[0] == '-')) {
        string      arg = syntax.next();
        if (arg == "-s")
            viewSave = syntax.next();
        else if (arg == "-l")
            viewLoad = syntax.next();
        else
            syntax.error("Unrecognized option",arg);
    }
    if (syntax.more()) {
        while (syntax.more()) {
            //! Set up the default render options from the arguments:
            ModelFiles  mf;
            mf.triFilename = syntax.next();
            if (syntax.more() && hasImgExtension(syntax.peekNext()))
                mf.imgFilename = syntax.next();
            opts.rend.models.push_back(mf);
        }
        opts.outputFile = renderName + ".png";
        fgSaveXml(renderName+".xml",opts);
    }
    else {
        // boost 1.58 introduced an XML deserialization bug on older compilers whereby std::vector
        // is appended rather than overwritten, so we must clear first:
        opts.rend.options.lighting.lights.clear();
        fgLoadXml(renderName+".xml",opts);
        if (!opts.rend.pose.rotateToHcs.normalize())
            fgThrow("rotateToHcs: quaternion cannot be zero magnitude");
    }

    //! Load data from files:
    vector<Mesh>    meshes(opts.rend.models.size());
    Mat33F            rotMatrix = Mat33F(opts.rend.pose.rotateToHcs.asMatrix());
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        const ModelFiles &  mf = opts.rend.models[ii];
        Mesh &          mesh = meshes[ii];
        mesh = loadTri(mf.triFilename);
        if (!mf.imgFilename.empty())
            imgLoadAnyFormat(Ustring(mf.imgFilename),mesh.surfaces[0].albedoMapRef());
        mesh.transform(rotMatrix);
        mesh.surfaces[0].material.shiny = mf.shiny;
    }

    //! Calculate view transforms:
    Mat32F            bounds = cBounds(meshes);
    CameraParams    cps(fgF2D(bounds));
    cps.pose =
        fgRotateY(opts.rend.pose.panRadians) *
        fgRotateX(opts.rend.pose.tiltRadians) *
        fgRotateZ(opts.rend.pose.rollRadians);
    cps.relTrans = opts.rend.pose.relTrans;
    cps.logRelScale = std::log(opts.rend.pose.relScale);
    cps.fovMaxDeg = opts.rend.pose.fovMaxDeg;
    Camera          cam;
    Affine3F        mvm;
    if (!viewLoad.empty()) {
        fgLoadXml(viewLoad+"_cam.xml",cam);
        fgLoadXml(viewLoad+"_pose.xml",mvm);
    }
    else {
        cam = cps.camera(opts.rend.imagePixelSize);
        mvm = Affine3F(cam.modelview);
    }
    if (!viewSave.empty()) {
        fgSaveXml(viewSave+"_cam.xml",cam);
        fgSaveXml(viewSave+"_pose.xml",mvm);
    }

    //! Render:
    opts.rend.options.projSurfPoints = std::make_shared<FgProjSurfPoints>();    // Receive surf point projection data
    FgTimer         timer;
    ImgC4UC          image = renderSoft(opts.rend.imagePixelSize,meshes,mvm,cam.itcsToIucs,opts.rend.options);
    fgout << fgnl << "Render time: " << timer.read() << "s ";

    //! Save results:
    imgSaveAnyFormat(Ustring(opts.outputFile),image);
    if (opts.saveSurfPointFile) {
        Ofstream      ofs(renderName+".csv");
        for (const FgProjSurfPoint & psp : *opts.rend.options.projSurfPoints)
            ofs << psp.label << "," << psp.posIucs << "," << (psp.visible ? "true" : "false") << "\n";
    }
}

Cmd
fgCmdRenderInfo()
{return Cmd(fgCmdRender,"render","Render TRI files with optional texture images to an image file"); }

static
bool
imgApproxEqual(Ustring const & file0,Ustring const & file1)
{
    ImgC4UC     img0 = imgLoadAnyFormat(file0),
                    img1 = imgLoadAnyFormat(file1);
    return fgImgApproxEqual(img0,img1,2);
}

void
fgCmdRenderTest(CLArgs const & args)
{
    FGTESTDIR
    fgTestCopy("base/Jane.tri");
    fgTestCopy("base/Jane.jpg");
    Options             opts;
    opts.rend.imagePixelSize = Vec2UI(120,160);
    opts.rend.pose.panRadians = fgDegToRad(55.0f);
    opts.outputFile = "render_test.png";
    opts.rend.options.renderSurfPoints = FgRenderSurfPoints::whenVisible;
    opts.saveSurfPointFile = true;
    ModelFiles          mf;
    mf.triFilename = "Jane.tri";
    mf.imgFilename = "Jane.jpg";
    opts.rend.models.push_back(mf);
    fgSaveXml("render_test.xml",opts);
    fgCmdRender(splitChar("render render_test"));
    regressFileRel("render_test.png","base/test/",imgApproxEqual);
    // TODO: make a struct and serialize to XML so an approx comparison can be done (debug has precision diffs):
    if ((fgCurrentCompiler() == FgCompiler::vs15) && (fgCurrentBuildConfig() == "release")) {
        regressFileRel("render_test.csv","base/test/");
    }
}

}
