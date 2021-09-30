//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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

namespace {

struct  ModelFiles
{
    string          triFilename;
    string          imgFilename;
    bool            shiny = false;

    FG_SERIALIZE3(triFilename,imgFilename,shiny)
};

struct  Pose
{
    QuaternionD         rotateToHcs;
    // Euler angles applied in FHCS in following order after 'rotateToHcs':
    double              rollRadians = 0.0;
    double              tiltRadians = 0.0;
    double              panRadians = 0.0;
    // Model translation parallel to image plane, relative to half max model bound:
    Vec2D               relTrans;
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
    Vec2UI                  imagePixelSize = Vec2UI(512,512);
    RenderOptions           options;

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

}   // namespace

/**
   \ingroup Base_Commands
   Command to render a mesh and colour map to an image.
 */
void
cmdRender(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<name> [-s <view>] [-l <view>] (<mesh>.tri [<image>.<ext>])*
    Render specified meshes [with texture images] using default render arguments.
    Saves render arguments to <name>.xml and rendered image to <name>.png
    -s     - Save the object pose and camera intrinsics in <view>_pose.xml and <view>_cam.xml
    -l     - Load the object pose and camera intrinsics from the above files, do not calculate from <name>.xml
    <ext> - )" + getImageFileExtCLDescriptions() + R"(
OUTPUT:
    <image>.<ext>       Rendered image
    [<name>.xml]        A configuation file for rendering with this command
NOTES:
    * If no mesh arguments are given, <name>.xml will be used for the arguments. Otherwise,
      <name>.xml will be created and can be modified for re-use.
    * Fields in <name>.xml can be modified as long as the XML structure is not changed.
      This can be automated with the 'substitute' command.
    * <name>.xml fields:
        <models>        the list of mesh and related color map files to be rendered.
        <pose>
            <rotateToHcs> quaternion specifying rotation from mesh original coordinates to
                head coordinate system (HCS): X - face's left, Y - face's up, Z - facing direction
            <rollRadians>,<tiltRadians>,<panRadians> euler angles applied from HCS
            <relTrans>  translation in HCS relative with unit scale equal to half max bound.
            <relScale>  scale from original mesh scale.
            <fovMaxDeg> field of view angle of camera in degrees (applied to larger image dimension).
        <imagePixelSize> width, height
        <options>
            <lighting>  all color component values below are in the range [0,1]:
                <ambient> light coming from all directions.
                <lights>
                    <colour> RGB order
                    <direction> in order of X,Y,Z in OpenGL Eye Coordinates.
            <backgroundColor> RGBA order, values in [0,1].
            <antiAliasBitDepth> 3 is high quality (equivalent to 8x FSAA) without running too slowly
            <renderSurfPoints> 0 - don't, 1 - when visible, 2 - always. They are rendered as single-pixel green dots over the image.
        <saveSurfPointFile> 0 - don't, 1 - save in <name>.csv written as <label>,<position>,<visible>
            where <position> is in image unit coordinates; [0,1]
        <outputFile> rendered image saved here.)"
    };
    string              renderName = syn.next();
    Options             opts;
    string              viewSave,    // If empty, option not selected
                        viewLoad;    // "
    while (syn.more() && (syn.peekNext()[0] == '-')) {
        string      arg = syn.next();
        if (arg == "-s")
            viewSave = syn.next();
        else if (arg == "-l")
            viewLoad = syn.next();
        else
            syn.error("Unrecognized option",arg);
    }
    if (syn.more()) {
        while (syn.more()) {
            //! Set up the default render options from the arguments:
            ModelFiles  mf;
            mf.triFilename = syn.next();
            if (syn.more() && hasImageFileExt(syn.peekNext()))
                mf.imgFilename = syn.next();
            opts.rend.models.push_back(mf);
        }
        opts.outputFile = renderName + ".png";
        saveBsaXml(renderName+".xml",opts);
    }
    else {
        // boost 1.58 introduced an XML deserialization bug on older compilers whereby std::vector
        // is appended rather than overwritten, so we must clear first:
        opts.rend.options.lighting.lights.clear();
        loadBsaXml(renderName+".xml",opts);
        if (!opts.rend.pose.rotateToHcs.normalize())
            fgThrow("rotateToHcs: quaternion cannot be zero magnitude");
    }

    //! Load data from files:
    Meshes              meshes(opts.rend.models.size());
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        const ModelFiles &  mf = opts.rend.models[ii];
        Mesh &              mesh = meshes[ii];
        mesh = loadTri(mf.triFilename);
        if (!mf.imgFilename.empty())
            loadImage_(String8(mf.imgFilename),mesh.surfaces[0].albedoMapRef());
        mesh.xform(SimilarityD{opts.rend.pose.rotateToHcs});
        mesh.surfaces[0].material.shiny = mf.shiny;
    }

    //! Calculate view transforms:
    Mat32F              bounds = cBounds(meshes);
    CameraParams        cps(fgF2D(bounds));
    cps.pose =
        cRotateY(opts.rend.pose.panRadians) *
        cRotateX(opts.rend.pose.tiltRadians) *
        cRotateZ(opts.rend.pose.rollRadians);
    cps.relTrans = opts.rend.pose.relTrans;
    cps.logRelScale = std::log(opts.rend.pose.relScale);
    cps.fovMaxDeg = opts.rend.pose.fovMaxDeg;
    Camera              cam;
    SimilarityD         mvm;
    if (!viewLoad.empty()) {
        loadBsaXml(viewLoad+"_cam.xml",cam);
        loadBsaXml(viewLoad+"_pose.xml",mvm);
    }
    else {
        cam = cps.camera(opts.rend.imagePixelSize);
        mvm = cam.modelview;
    }
    if (!viewSave.empty()) {
        saveBsaXml(viewSave+"_cam.xml",cam);
        saveBsaXml(viewSave+"_pose.xml",mvm);
    }

    //! Render:
    opts.rend.options.projSurfPoints = std::make_shared<ProjectedSurfPoints>();    // Receive surf point projection data
    Timer               timer;
    ImgRgba8             image = renderSoft(opts.rend.imagePixelSize,meshes,mvm,cam.itcsToIucs,opts.rend.options);
    fgout << fgnl << "Render time: " << timer.elapsedSeconds() << "s ";

    //! Save results:
    saveImage(String8(opts.outputFile),image);
    if (opts.saveSurfPointFile) {
        Ofstream            ofs(renderName+".csv");
        for (const ProjectedSurfPoint & psp : *opts.rend.options.projSurfPoints)
            ofs << psp.label << "," << psp.posIucs << "," << (psp.visible ? "true" : "false") << "\n";
    }
}

Cmd
getCmdRender()
{return Cmd(cmdRender,"render","Render meshes with color & specular maps to an image file"); }

static
bool
imgApproxEqual(String8 const & file0,String8 const & file1)
{
    ImgRgba8         img0 = loadImage(file0),
                    img1 = loadImage(file1);
    return fgImgApproxEqual(img0,img1,2);
}

void
testRenderCmd(CLArgs const & args)
{
    FGTESTDIR
    copyFileToCurrentDir("base/Jane.tri");
    copyFileToCurrentDir("base/Jane.jpg");
    Options             opts;
    opts.rend.imagePixelSize = Vec2UI(120,160);
    opts.rend.pose.panRadians = degToRad(55.0f);
    opts.outputFile = "render_test.png";
    opts.rend.options.renderSurfPoints = RenderSurfPoints::whenVisible;
    opts.saveSurfPointFile = true;
    ModelFiles          mf;
    mf.triFilename = "Jane.tri";
    mf.imgFilename = "Jane.jpg";
    opts.rend.models.push_back(mf);
    saveBsaXml("render_test.xml",opts);
    cmdRender(splitChar("render render_test"));
    regressFileRel("render_test.png","base/test/",imgApproxEqual);
    // TODO: make a struct and serialize to XML so an approx comparison can be done (debug has precision diffs):
    if ((getCurrentBuildOS() == BuildOS::win) && (getCurrentBuildConfig() == "release")) {
        regressFileRel("render_test.csv","base/test/");
    }
}

}
