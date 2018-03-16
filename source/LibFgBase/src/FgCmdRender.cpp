//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Sept 27, 2010
//
// Software render meshes to image files.
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

using namespace std;

struct  ModelFiles
{
    string          triFilename;
    string          imgFilename;
    bool            shiny;

    ModelFiles() : shiny(false) {}

    FG_SERIALIZE3(triFilename,imgFilename,shiny)
};

struct  Camera
{
    FgQuaternionD       rotateToHcs;
    double              rollRadians;
    double              tiltRadians;
    double              panRadians;
    // Model translation parallel to image plane, relative to half max model bound:
    FgVect2D            relTrans;
    // Scale relative to automatically determined size of object in image:
    double              relScale;
    // Field of view of larger image dimension (degrees). Must be > 0 but can set as low as 0.0001
    // to simulate orthographic projection:
    double              fovMaxDeg;

    Camera() : rollRadians(0.0),tiltRadians(0.0),panRadians(0.0),relScale(0.9), fovMaxDeg(17) {}

    FG_SERIALIZE7(rotateToHcs,rollRadians,tiltRadians,panRadians,relTrans,relScale,fovMaxDeg)
};

struct  RenderArgs
{
    vector<ModelFiles>      models;
    Camera                  cam;
    FgLighting              lighting;
    FgRgbaF                 backgroundColor;
    FgVect2UI               imagePixelSize;
    uint                    antiAliasBitDepth;
    bool                    showSurfPoints;     // Show marked surface points on models as red dots over final render
    bool                    saveSurfPointFile;  // Write a text file of image position of marked surface points on models, along with visibility.
    bool                    saveSurfBoundBoxes; // Write a file for the bounding box of the vertices of each rendered surface
    string                  outputFile;

    RenderArgs() :
        imagePixelSize(512,512),
        antiAliasBitDepth(3),
        showSurfPoints(false),
        saveSurfPointFile(false)
    {}

    FG_SERIALIZE10(models,cam,lighting,backgroundColor,imagePixelSize,antiAliasBitDepth,showSurfPoints,saveSurfPointFile,saveSurfBoundBoxes,outputFile)
};

/**
   \ingroup Base_Commands
   Command to render a mesh and colour map to an image.
 */
void
fgCmdRender(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<name> (<mesh>.tri [<image>.<ext1>])+\n"
        "    Render specified meshes [with texture images] using default render arguments.\n"
        "    Saves render arguments to <name>.xml and rendered image to <name>.png\n"
        "    <ext1>     - " + fgImgCommonFormatsDescription() + "\n"
        "render <name>\n"
        "    Render using the arguments in <name>.xml (including the output image file name and type)");

    string          renderName = syntax.next();
    RenderArgs      renderArgs;
    if (syntax.more()) {
        while (syntax.more()) {
            //! Set up the default render options from the arguments:
            ModelFiles  mf;
            mf.triFilename = syntax.next();
            if (syntax.more())
                mf.imgFilename = syntax.next();
            renderArgs.models.push_back(mf);
        }
        renderArgs.outputFile = renderName + ".png";
        fgSaveXml(renderName+".xml",renderArgs);
    }
    else {
        // boost 1.58 introduced an XML deserialization bug on older compilers whereby std::vector
        // is appended rather than overwritten, so we must clear first:
        renderArgs.lighting.m_lights.clear();
        fgLoadXml(renderName+".xml",renderArgs);
        if (!renderArgs.cam.rotateToHcs.normalize())
            fgThrow("rotateToHcs: quaternion cannot be zero magnitude");
    }

    //! Load data from files:
    vector<Fg3dMesh>    meshes(renderArgs.models.size());
    FgMat33F            rotMatrix = FgMat33F(renderArgs.cam.rotateToHcs.asMatrix());
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        const ModelFiles &  mf = renderArgs.models[ii];
        meshes[ii] = fgLoadTri(mf.triFilename);
        if (!mf.imgFilename.empty())
            fgLoadImgAnyFormat(FgString(mf.imgFilename),meshes[ii].surfaces[0].albedoMapRef());
        meshes[ii].transform(rotMatrix);
        meshes[ii].material.shiny = mf.shiny;
    }

    //! Calculate view transforms:
    FgMat32F            bounds = fgBounds(meshes);
    Fg3dCameraParams    cps(fgF2D(bounds));
    cps.pose =
        fgRotateY(renderArgs.cam.panRadians) *
        fgRotateX(renderArgs.cam.tiltRadians) *
        fgRotateZ(renderArgs.cam.rollRadians);
    cps.relTrans = renderArgs.cam.relTrans;
    cps.logRelScale = std::log(renderArgs.cam.relScale);
    cps.fovMaxDeg = renderArgs.cam.fovMaxDeg;
    Fg3dCamera          cam = cps.camera(renderArgs.imagePixelSize);
    FgAffine3F          mvm(cam.modelview);

    //! Render:
    FgTimer             timer;
    FgImgRgbaUb         image =
        fgSoftRender(
            renderArgs.imagePixelSize,
            meshes,
            renderArgs.lighting,
            mvm,
            cam.itcsToIucs,
            renderArgs.backgroundColor,
            renderArgs.antiAliasBitDepth);
    FgMat44F        projectionH = cam.projectIpcs(renderArgs.imagePixelSize);
    if (renderArgs.showSurfPoints) {
        for (size_t mm=0; mm<meshes.size(); ++mm) {
            const Fg3dMesh &    mesh = meshes[mm];
            for (size_t ii=0; ii<mesh.surfPointNum(); ++ii) {
                FgVect4F    p0 = projectionH * fgAsHomogVec(mesh.surfPointPos(ii));
                FgVect2F    p1(p0[0]/p0[3],p0[1]/p0[3]);
                image.paint(FgVect2I(fgFloor(p1)),FgRgbaUB(255,0,0,255));
            }
        }
    }
    fgout << fgnl << "Render time: " << timer.read() << "s ";
    fgSaveImgAnyFormat(FgString(renderArgs.outputFile),image);
    if (renderArgs.saveSurfPointFile) {
        FgOfstream      ofs(renderArgs.outputFile+".txt");
        ofs <<
            "Image coordinates of marked surface points in render, and visibility (true/false by normal)\n"
            "(origin at top left of image, X right, Y down, units are pixels):" << endl;
        for (size_t mm=0; mm<meshes.size(); ++mm) {
            ofs << "Mesh " << mm << endl;
            const Fg3dMesh &    mesh = meshes[mm];
            Fg3dNormals         norms = fgNormals(mesh);
            for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
                ofs << "  Surf " << ss << endl;
                const Fg3dSurface &     surf = mesh.surfaces[ss];
                for (size_t ii=0; ii<surf.surfPoints.size(); ++ii) {
                    FgSurfPoint     sp = surf.surfPoints[ii];
                    FgVect3F        spc = surf.surfPointPos(mesh.verts,ii);
                    FgVect4F        p0 = projectionH * fgAsHomogVec(spc);
                    FgVect2F        p1(p0[0]/p0[3],p0[1]/p0[3]);
                    ofs << "    " << ii << "(" << sp.label << ")" << ": " << p1 << " ";
                    FgVect3UI       triVertInds = surf.getTriEquiv(sp.triEquivIdx);
                    FgVect3F        spNorm = 
                        norms.vert[triVertInds[0]] * sp.weights[0] +
                        norms.vert[triVertInds[1]] * sp.weights[1] +
                        norms.vert[triVertInds[2]] * sp.weights[2];
                    FgVect4F        spnH(spNorm[0],spNorm[1],spNorm[2],0),
                                    spnT = projectionH * spnH;
                    ofs << ((spnT[3] > 0) ? "false" : "true") << endl;
                }
            }
        }
    }
    if (renderArgs.saveSurfBoundBoxes) {
        for (const Fg3dMesh & mesh : meshes) {
            FgVect2Fs       ppts;
            for (FgVect3F v : mesh.verts) {
                FgVect4F    p0 = projectionH * fgAsHomogVec(v);
                ppts.push_back(FgVect2F(p0[0]/p0[3],p0[1]/p0[3]));
            }
            fgSaveXml(renderName+"_"+mesh.name+"_bbox.xml",fgBounds(ppts));
        }
    }
}

FgCmd
fgCmdRenderInfo()
{return FgCmd(fgCmdRender,"render","Render TRI files with optional texture images to an image file"); }

static
bool
imgApproxEqual(const FgString & file0,const FgString & file1)
{
    FgImgRgbaUb     img0 = fgLoadImgAnyFormat(file0),
                    img1 = fgLoadImgAnyFormat(file1);
    return fgImgApproxEqual(img0,img1,2);
}

void
fgRenderTest(const FgArgs & args)
{
    FGTESTDIR
    fgTestCopy("base/Jane.tri");
    fgTestCopy("base/Jane.jpg");
    RenderArgs  ra;
    ra.imagePixelSize = FgVect2UI(120,160);
    ra.cam.panRadians = fgDegToRad(55.0f);
    ra.outputFile = "render_test.png";
    ra.showSurfPoints = true;
    ra.saveSurfPointFile = true;
    ra.saveSurfBoundBoxes = true;
    ModelFiles  mf;
    mf.triFilename = "Jane.tri";
    mf.imgFilename = "Jane.jpg";
    ra.models.push_back(mf);
    fgSaveXml("render_test.xml",ra);
    fgCmdRender(fgSplitChar("render render_test"));
    fgRegressFileRel("render_test.png","base/test/",imgApproxEqual);
    // TODO: make a struct and serialize to XML so an approx comparison can be done (debug has precision diffs):
    if ((fgCurrentCompiler() == "vs15") && (fgCurrentBuildConfig() == "release")) {
        fgRegressFileRel("render_test.png.txt","base/test/");
        fgRegressFileRel("render_test_Jane_bbox.xml","base/test/");
    }
}
