//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Dec 18, 2009
//

#include "stdafx.h"

#include "Fg3dDisplay.hpp"
#include "FgGuiApi.hpp"
#include "FgFileSystem.hpp"
#include "FgTestUtils.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dTopology.hpp"
#include "FgCommand.hpp"

using namespace std;

static
FGLINK(linkLighting)
{
    FGLINKARGS(5,1);
    const vector<double> &  amb = inputs[0]->valueRef();
    const vector<double> &  l1 = inputs[1]->valueRef();
    const vector<double> &  l2 = inputs[2]->valueRef();
    const vector<double> &  d1 = inputs[3]->valueRef();
    const vector<double> &  d2 = inputs[4]->valueRef();
    FGASSERT(amb.size() == 3);
    FGASSERT(l1.size() == 3);
    FGASSERT(l2.size() == 3);
    FGASSERT(d1.size() == 3);
    FGASSERT(d2.size() == 3);
    FgLighting &            lighting = outputs[0]->valueRef();
    lighting.m_ambient = FgVect3F(amb[0],amb[1],amb[2]);
    lighting.m_lights.resize(2);    // Must always assign both lights since OGL will keep settings for both
    for (uint ll=0; ll<2; ++ll) {
        const vector<double> &  ls = (ll == 0) ? l1 : l2;
        const vector<double> &  ds = (ll == 0) ? d1 : d2;
        FgLight                 light;
        light.m_colour = FgVect3F(ls[0],ls[1],ls[2]);
        light.m_direction = fgNormalize(FgVect3F(ds[0],ds[1],ds[2]));
        lighting.m_lights[ll] = light;
    }
}

static
FgGuiWinVal<vector<double> >
colorSliders(const string & relStore,double init,double tickSpacing)
{
    FgGuiWinVal<vector<double> >    ret;
    vector<uint>                    valsN;
    FgGuiPtrs                       sliders;
    string                          colors[] = {"Red","Green","Blue"};
    for (uint ii=0; ii<3; ++ii) {
        FgDgn<double>   val = g_gg.addInput(init,relStore+colors[ii]);
        valsN.push_back(val);
        sliders.push_back(fgGuiSplit(true,
            fgGuiText(colors[ii],45),       // Fixed width larger than all colors to align sliders
            fgGuiSlider(val,"",FgVectD2(0,1),tickSpacing)));
    }
    ret.valN = g_gg.addNode(vector<double>(),"RGB");
    g_gg.addLink(fgLinkCollate<double>,valsN,ret.valN);
    ret.win = fgGuiSplit(false,sliders);
    return ret;
}

struct  FgLightDir
{
    FgGuiPtr                        win;
    FgDgn<vector<double> >          dirN;
    FgMatrixC<FgDgn<double>,3,1>    dirAxisNs;
};

static
FgLightDir
lightDir(const string & relLight)
{
    FgLightDir      ret;
    ret.dirAxisNs[0] = g_gg.addInput(0.0,"Light"+relLight+"X"),
    ret.dirAxisNs[1] = g_gg.addInput(0.0,"Light"+relLight+"Y"),
    ret.dirAxisNs[2] = g_gg.addInput(1.0,"Light"+relLight+"Z");
    FgGuiPtr        x = fgGuiTextEditFloat(ret.dirAxisNs[0],FgVect2D(-100000,100000)),
                    y = fgGuiTextEditFloat(ret.dirAxisNs[1],FgVect2D(-100000,100000)),
                    z = fgGuiTextEditFloat(ret.dirAxisNs[2],FgVect2D(-100000,100000));
    ret.dirN = g_gg.addNode(vector<double>(),"Light"+relLight+"Dir");
    g_gg.addLink(fgLinkCollate<double>,fgUints(ret.dirAxisNs[0],ret.dirAxisNs[1],ret.dirAxisNs[2]),ret.dirN);
    ret.win = fgGuiSplit(false,
        fgGuiText("Direction vector to light (from viewer's point of view):"),
        fgGuiSplit(true,fgSvec<FgGuiPtr>(fgGuiText("Right:"),x,fgGuiText("Up:"),y,fgGuiText("Backward:"),z)));
    return ret;
}

void
resetLighting(uint lightingIdx)
{g_gg.setInputsToDefault(lightingIdx); }

FgGuiLighting
fgGuiLighting()
{
    FgGuiLighting   ret;
    ret.outN = g_gg.addNode(FgLighting(),"Lighting");
    FgGuiWinVal<vector<double> >    ambient = colorSliders("Ambient",0.4,0.1);
    FgGuiWinVal<vector<double> >    light1 = colorSliders("Light1",0.6,0.1);
    FgGuiWinVal<vector<double> >    light2 = colorSliders("Light2",0.0,0.1);
    FgLightDir                      ld1 = lightDir("1"),
                                    ld2 = lightDir("2");
    g_gg.addLink(linkLighting,fgUints(ambient.valN,light1.valN,light2.valN,ld1.dirN,ld2.dirN),ret.outN);
    ret.dirAxisNs[0] = ld1.dirAxisNs;
    ret.dirAxisNs[1] = ld2.dirAxisNs;
    ret.win = fgGuiSplitScroll(fgSvec(
        fgGuiGroupbox("Ambient",ambient.win),
        fgGuiGroupbox("Light 1",fgGuiSplit(false,light1.win,ld1.win)),
        fgGuiGroupbox("Light 2",fgGuiSplit(false,light2.win,ld2.win)),
        fgGuiText("\nInteractively adjust light direction:\n"
            "Light 1: hold down left and right mouse buttons while dragging\n"
            "Light 2: As above but also hold down shift key"),
        fgGuiButton("Reset Lighting",boost::bind(resetLighting,ret.outN.idx()))
    ));
    return ret;
}

static
FGLINK(linkRenderOpts)
{
    FGLINKARGS(10,1);
    bool                facets = inputs[0]->valueRef();
    bool                useTexture = inputs[1]->valueRef();
    bool                shiny = inputs[2]->valueRef();
    bool                wireframe = inputs[3]->valueRef();
    bool                flatShaded = inputs[4]->valueRef();
    bool                surfPoints = inputs[5]->valueRef();
    bool                markedVerts = inputs[6]->valueRef();
    bool                allVerts = inputs[7]->valueRef();
    bool                twoSided = inputs[8]->valueRef();
    FgVect3F            bcolor = inputs[9]->valueRef();
    Fg3dRenderOptions & ro = outputs[0]->valueRef();
    ro.facets = facets;
    ro.useTexture = useTexture;
    ro.shiny = shiny;
    ro.wireframe = wireframe;
    ro.flatShaded = flatShaded;
    ro.surfPoints = surfPoints;
    ro.markedVerts = markedVerts;
    ro.allVerts = allVerts;
    ro.twoSided = twoSided;
    ro.backgroundColor = bcolor;
}

static
void
bgImageLoad(FgDgn<FgImgRgbaUb> imgN,FgDgn<FgVect2UI> dimsN)
{
    FgOpt<FgString>     fname = fgGuiDialogFileLoad(fgImgCommonFormatsDescription(),fgImgCommonFormats());
    if (!fname.valid())
        return;
    FgImgRgbaUb         orig;
    try {
        orig = fgLoadImgAnyFormat(fname.val());
    }
    catch (const FgException & e) {
        fgGuiDialogMessage("Unable to load image",e.no_tr_message());
        return;
    }
    g_gg.setVal(dimsN,orig.dims());
    FgImgRgbaUb &       img = g_gg.getRef(imgN);
    img.resize(FgVect2UI(1024));
    fgImgResize(orig,img);
}

static
void
bgImageClear(FgDgn<FgImgRgbaUb> imgN)
{g_gg.setVal(imgN,FgImgRgbaUb()); }

static
FGLINK(linkColSel)
{
    FGLINKARGS(1,1);
    const vector<double> &  cv = inputs[0]->valueRef();
    FgVect3F &              col = outputs[0]->valueRef();
    col = FgVect3F(cv[0],cv[1],cv[2]);
}

static
FGLINK(lnkSetAlpha)
{
    FGLINKARGS(2,1);
    const FgImgRgbaUb &     imgIn = inputs[0]->valueRef();
    double                  alpha = inputs[1]->valueRef();
    FgImgRgbaUb &           imgOut = outputs[0]->valueRef();
    uchar                   a = uchar(alpha * 255.0 + 0.5);
    imgOut.resize(imgIn.dims());
    for (size_t ii=0; ii<imgOut.numPixels(); ++ii) {
        FgRgbaUB    p = imgIn[ii];
        p.alpha() = a;
        imgOut[ii] = p;
    }
}

FgRenderCtrls
fgRenderCtrls(uint simple)
{
    FgRenderCtrls       ret;
    string              uid = "RenderCtrls";
    FgDgn<bool>         useTexture = g_gg.addInput((simple != 3),uid+"UseTexture"),
                        shiny = g_gg.addInput(false,uid+"Shiny"),
                        wireframe = g_gg.addInput(false,uid+"Wireframe"),
                        flatShaded = g_gg.addInput(false,uid+"FlatShaded"),
                        facets = g_gg.addInput(true,uid+"Facets"),
                        surfPoints = g_gg.addInput(false,uid+"SurfPoints"),
                        markedVerts = g_gg.addInput(false,uid+"MarkedVerts"),
                        allVerts = g_gg.addInput(false,uid+"AllVerts"),
                        twoSidedN = g_gg.addInput(true,uid+"TwoSided");
    FgGuiWinVal<vector<double> >    bgColor = colorSliders(uid+"Background",0.3,0.1);
    FgDgn<double>       bgAlphaN = g_gg.addInput(0.0,uid+"BgImageAlpha");
    FgDgn<FgVect3F>     bgColorN(g_gg,"bgColor");
    g_gg.addLink(linkColSel,bgColor.valN,bgColorN);
    ret.optsN = g_gg.addNode(Fg3dRenderOptions(),"renderOptions");
    ret.bgImgN = g_gg.addNode(FgImgRgbaUb(),"bgImage");
    ret.bgImgDimsN = g_gg.addNode(FgVect2UI());
    g_gg.addLink(linkRenderOpts,
        fgSvec<uint>(facets,useTexture,shiny,wireframe,flatShaded,surfPoints,markedVerts,allVerts,twoSidedN,bgColorN),
        ret.optsN);
    FgGuiPtr            rcColor = fgGuiCheckbox("Color maps",useTexture),
                        rcShiny = fgGuiCheckbox("Shiny",shiny),
                        rcWire = fgGuiCheckbox("Wireframe",wireframe),
                        rcFlat = fgGuiCheckbox("Flat shaded",flatShaded),
                        rcFacet = fgGuiCheckbox("Facets",facets),
                        rcSurf = fgGuiCheckbox("Surface points",surfPoints),
                        rcMark = fgGuiCheckbox("Marked vertices",markedVerts),
                        rcAll = fgGuiCheckbox("All vertices",allVerts),
                        rcTwoSided = fgGuiCheckbox("Two sided",twoSidedN),
                        bgImageSlider = fgGuiSlider(bgAlphaN,"Foreground transparency",FgVectD2(0,1),0.1);
    FgGuiPtr            renderCtls;
    if (simple == 0)
        renderCtls = fgGuiSplit(true,
            fgGuiSplit(false,rcColor,rcShiny,rcWire),
            fgGuiSplit(false,rcFlat,rcFacet,rcSurf),
            fgGuiSplit(false,rcMark,rcAll,rcTwoSided));
    else if (simple == 1)
        renderCtls = fgGuiSplit(false,rcColor,rcShiny,rcWire,rcFlat);
    else if (simple == 3)
        renderCtls = fgGuiSplit(false,rcShiny,rcWire,rcFlat);
    else    // simple == 2
        renderCtls = fgGuiSplit(false,rcColor,rcShiny);
    FgDgn<FgImgRgbaUb>  bgImgPreAlphaN(g_gg,"bgImgPreAlpha");
    g_gg.addLink(lnkSetAlpha,fgUints(bgImgPreAlphaN,bgAlphaN),ret.bgImgN);
    FgGuiPtr        viewCtlRender = fgGuiGroupboxTr("Render",renderCtls),
                    viewCtlColor = fgGuiGroupboxTr("Background Color",bgColor.win),
                    bgImageButtons = fgGuiSplit(true,
                        fgGuiButton("Load Image",boost::bind(bgImageLoad,bgImgPreAlphaN,ret.bgImgDimsN)),
                        fgGuiButton("Clear Image",boost::bind(bgImageClear,bgImgPreAlphaN))),
                    bgImageBox = fgGuiGroupbox("Background Image",fgGuiSplit(false,bgImageButtons,bgImageSlider));
    ret.win = fgGuiSplit(false,viewCtlRender,viewCtlColor,bgImageBox);
    return ret;
}

static
FGLINK(linkNorms)
{
    FGLINKARGS(2,1);
    const vector<Fg3dMesh> &    meshes = inputs[0]->valueRef();
    const vector<FgVerts> &     vertss = inputs[1]->valueRef();
    vector<Fg3dNormals> &       normss = outputs[0]->valueRef();
    FGASSERT(meshes.size() == vertss.size());
    normss.resize(meshes.size());
    for (size_t ii=0; ii<normss.size(); ++ii)
        fgCalcNormals(meshes[ii].surfaces,vertss[ii],normss[ii]);
}

static
FGLINK(linkXform)
{
    FGLINKARGS(8,1);
    Fg3dCameraParams    cps;
    cps.modelBounds = inputs[0]->valueRef();
    size_t              panTiltMode = inputs[1]->valueRef();
    FgVect2D            panTilt = inputs[2]->valueRef();
    cps.pose = inputs[3]->valueRef();
    cps.relTrans = inputs[4]->valueRef();
    cps.logRelScale = inputs[5]->valueRef();
    cps.fovMaxDeg= inputs[6]->valueRef();
    FgVect2UI           viewport = inputs[7]->valueRef();
    if (panTiltMode == 0) {
        FgQuaternionD   pan(fgDegToRad(panTilt[0]),1),
                        tilt(fgDegToRad(panTilt[1]),0);
        cps.pose = pan*tilt;
    }
    Fg3dCamera &        camera = outputs[0]->valueRef();
    camera = cps.camera(viewport);
}

static
void
resetView(
    FgDgn<double>           lensFovDeg,
    FgDgn<FgVect2D>         panTilt,
    FgDgn<FgQuaternionD>    pose,
    FgDgn<FgVect2D>         translate,
    FgDgn<double>           logRelSize)
{
    Fg3dCameraParams        defaultCps;
    defaultCps.logRelScale = -0.3;
    g_gg.setVal(lensFovDeg,defaultCps.fovMaxDeg);
    g_gg.setVal(panTilt,FgVect2D(0));
    g_gg.setVal(pose,FgQuaternionD());
    g_gg.setVal(translate,FgVect2D(0));
    g_gg.setVal(logRelSize,defaultCps.logRelScale);
}

static
FGLINK(linkMorphNames)
{
    FGLINKARGS(1,1);
    const vector<Fg3dMesh> &    meshes = inputs[0]->valueRef();
    vector<FgString> &          morphNames = outputs[0]->valueRef();
    set<FgString>               ns = fgMorphs(meshes);
    morphNames.clear();
    morphNames.assign(ns.begin(),ns.end());
}

static
FGLINK(linkMorphs)
{
    FGLINKARGS(4,1);
    const vector<Fg3dMesh> &    meshes = inputs[0]->valueRef();
    const vector<FgVerts> &     allVertss = inputs[1]->valueRef();
    const vector<FgString> &    morphNames = inputs[2]->valueRef();
    vector<double>              morphVals = inputs[3]->valueRef();
    FGASSERT(meshes.size() == allVertss.size());
    // If morph list has changed but morphVals not yet updated (by GUI), assume all zero.
    // TODO: Keep morphs ls as a label:val dictionary.
    if (morphNames.size() != morphVals.size()) {
        morphVals.clear();
        morphVals.resize(morphNames.size(),0.0);
    }
    vector<FgVerts> &           vertss = outputs[0]->valueRef();
    vertss.resize(meshes.size());
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        const Fg3dMesh &        mesh = meshes[ii];
        const FgVerts &         allVerts = allVertss[ii];
        vector<float>           morphCoord(mesh.numMorphs(),0.0f);
        for (size_t jj=0; jj<morphNames.size(); ++jj) {
            FgValid<size_t>     idx = mesh.findMorph(morphNames[jj]);
            if (idx.valid())
                morphCoord[idx.val()] = morphVals[jj];
        }
        mesh.morph(allVerts,morphCoord,vertss[ii]);
    }
}

static
FGLINK(linkMeshStats2)
{
    FGLINKARGS(1,1);
    const vector<Fg3dMesh> &    meshes = inputs[0]->valueRef();
    FgString &                  text = outputs[0]->valueRef();
    text.clear();
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        ostringstream   os;
        const Fg3dMesh &    mesh = meshes[ii];
        os  << "\nMesh " << ii << ":"
            << fgpush << mesh << fgpop;
        text += FgString(os.str());
    }
}

static
void
saveMesh(FgDgn<vector<Fg3dMesh> > meshesN,FgDgn<vector<FgVerts> > vertssN)
{
    const vector<Fg3dMesh> &    meshes = g_gg.getVal(meshesN);
    const vector<FgVerts> &     vertss = g_gg.getVal(vertssN);
    FGASSERT(!meshes.empty());
    FGASSERT(meshes.size() == vertss.size());
    FgOpt<FgString>        fname = fgGuiDialogFileSave("FaceGen TRI","tri");
    if (fname.valid()) {
        Fg3dMesh                mesh = meshes[0];
        mesh.updateAllVerts(vertss[0]);
        fgSaveTri(fname.val(),mesh);
    }
}

static
double
getInput(FgDgn<vector<double> > n,size_t idx)
{
    const vector<double> &  ls = g_gg.getVal(n);
    return ls[idx];
}

static
void
setOutput(FgDgn<vector<double> > n,size_t idx,double val)
{
    vector<double> &        ls = g_gg.dg.nodeValRef(n);
    ls[idx] = val;
}

static
void
setAllZero(FgDgn<vector<double> > n)
{
    vector<double> &        ls = g_gg.dg.nodeValRef(n);
    for (size_t ii=0; ii<ls.size(); ++ii)
        ls[ii] = 0.0;
}

static 
FgGuiPtrs
getPanes(
    FgDgn<vector<FgString> >    in,
    FgDgn<vector<double> >      out)
{
    vector<FgString>    labels = g_gg.getVal(in);
    vector<double> &    output = g_gg.dg.nodeValRef(out);
    if (output.size() != labels.size()) {   // past value is stale
        output.clear();
        output.resize(labels.size(),0.0);
    }
    FgGuiPtrs        sliders;
    sliders.push_back(fgGuiButton("Set All To Zero",boost::bind(setAllZero,out)));
    sliders.push_back(fgGuiSpacer(0,7));
    for (size_t ii=0; ii<labels.size(); ++ii) {
        FgGuiApiSlider      slider;
        slider.updateFlagIdx = g_gg.addUpdateFlag(out);     // Memory leak but what can you do.
        slider.getInput = boost::bind(getInput,out,ii);
        slider.setOutput = boost::bind(setOutput,out,ii,_1);
        slider.label = labels[ii];
        slider.range = FgVectD2(0,1);
        slider.tickSpacing = 0.1;
        sliders.push_back(fgGuiPtr(slider));
    }
    return sliders;
}

static
void
markAllSeams(FgDgn<vector<Fg3dMesh> > meshesN)
{
    vector<Fg3dMesh> &      meshes = g_gg.getRef(meshesN);
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        Fg3dMesh &          mesh = meshes[ii];
        Fg3dTopology        topo(mesh.verts,mesh.getTriEquivs().vertInds);
        vector<set<uint> >  seams = topo.seams();
        for (size_t ss=0; ss<seams.size(); ++ss) {
            const set<uint> &   seam = seams[ss];
            for (set<uint>::const_iterator it=seam.begin(); it != seam.end(); ++it)
                mesh.markedVerts.push_back(FgMarkedVert(*it));
        }
    }
}

static
void
clearAllSurfPts(FgDgn<vector<Fg3dMesh> > meshesN)
{
    vector<Fg3dMesh> &      meshes = g_gg.getRef(meshesN);
    for (size_t ii=0; ii<meshes.size(); ++ii)
        for (size_t ss=0; ss<meshes[ii].surfaces.size(); ++ss)
            meshes[ii].surfaces[ss].surfPoints.clear();
}

static
void
clearAllMarkedVerts(FgDgn<vector<Fg3dMesh> > meshesN)
{
    vector<Fg3dMesh> &      meshes = g_gg.getRef(meshesN);
    for (size_t ii=0; ii<meshes.size(); ++ii)
        meshes[ii].markedVerts.clear();
}

FgGui3dCtls
fgGui3dCtls(
    FgDgn<vector<Fg3dMesh> >    meshesN,
    FgDgn<vector<FgVerts> >     allVertssN,
    FgDgn<vector<FgImgs> >      texssN,
    FgDgn<FgMat32D>             viewBoundsN,
    FgRenderCtrls               renderCtrls,
    FgDgn<FgLighting>           lightingN,
    boost::function<void(bool,FgVect2I)>    bothButtonsDrag,
    uint                        simple)
{
    FgGui3dCtls                 ret;
    FgDgn<vector<FgString> >    morphNamesN(g_gg,"morphNames");
    g_gg.addLink(linkMorphNames,meshesN,morphNamesN);
    FgDgn<vector<double> >      morphValsN = g_gg.addInput(vector<double>(),"morphVals");
    ret.morphedVertssN = g_gg.addNode(vector<FgVerts>(),"morphedVertss");
    g_gg.addLink(linkMorphs,fgUints(meshesN,allVertssN,morphNamesN,morphValsN),ret.morphedVertssN);
    ret.morphCtls = fgGuiSplitScroll(morphNamesN,boost::bind(getPanes,morphNamesN,morphValsN),3);
    vector<Fg3dMesh>            meshes = g_gg.getVal(meshesN);

    string                  uid = "3d";
    FgGuiApi3d              api;
    api.meshesN = meshesN;
    api.vertssN = ret.morphedVertssN;
    api.normssN = g_gg.addNode(vector<Fg3dNormals>(),"normss");
    api.texssN = texssN;
    api.viewBounds = viewBoundsN;
    api.panTiltMode = g_gg.addInput(size_t(0),uid+"PanTiltMode");
    api.light = lightingN;
    api.xform = g_gg.addNode(Fg3dCamera(),"xform");
    api.renderOptions = renderCtrls.optsN;
    api.vertMarkModeN = g_gg.addNode(size_t(0));
    api.pointLabel = g_gg.addNode(FgString());
    if (simple == 2)
        api.panTiltLimits = true;
    api.panTiltDegrees = g_gg.addInput(FgVect2D(0),uid+"PanTilt");
    api.pose = g_gg.addInput(FgQuaternionD(),uid+"Pose");
    api.trans = g_gg.addInput(FgVect2D(0),uid+"Trans");
    api.logRelSize = g_gg.addInput(-0.3,uid+"LogRelSize");
    api.viewportDims = g_gg.addNode(FgVect2UI(0),"viewportDims");
    api.bgImgN = renderCtrls.bgImgN;
    api.bgImgUpdateFlag = g_gg.addUpdateFlag(api.bgImgN);
    api.bgImgOrigDimsN = renderCtrls.bgImgDimsN;
    // Including api.viewBounds, api.panTiltDegrees and api.logRelSize in the below caused a feedback
    // issue that broke updates of the other sliders:
    api.updateFlagIdx = g_gg.addUpdateFlag(fgUints(
        api.meshesN,api.vertssN,api.normssN,api.texssN,api.light,api.xform,renderCtrls.optsN,api.bgImgN));
    // The OGL texture update process depends on the meshes as well as the textures:
    api.updateTexFlagIdx = g_gg.addUpdateFlag(fgUints(api.texssN,api.meshesN));
    api.bothButtonsDragAction = bothButtonsDrag;
    g_gg.addLink(linkNorms,fgUints(meshesN,ret.morphedVertssN),api.normssN);
    Fg3dCameraParams    defaultCps;     // Use default for lensFovDeg:
    FgDgn<double>       lensFovDeg = g_gg.addInput(defaultCps.fovMaxDeg,uid+"LensFovDeg");
    FgVectD2            logScaleRange(log(1.0/5.0),log(5.0));
    g_gg.addLink(linkXform,
        fgUints(viewBoundsN,api.panTiltMode,api.panTiltDegrees,api.pose,api.trans,
                api.logRelSize,lensFovDeg,api.viewportDims),
        api.xform);
    ret.viewport = fgsp(api);
    FgGuiPtr        viewCtlText =
        fgGuiText(
            "  Rotate: left-click-drag (or touch-drag)\n"
            "  Scale: right-click-drag up/down (or pinch-to-zoom)\n"
            "  Move: shift-left-click-drag (or middle-click-drag)");
    FgGuiPtr        viewCtlRotation =
        fgGuiGroupboxTr("Object rotation",
            fgGuiRadio(api.panTiltMode,fgSvec<FgString>("Pan / tilt","Unconstrained")));
    FgGuiPtr        viewCtlFov =
        fgGuiGroupboxTr(
            "Lens field of view (degrees)",
            fgGuiSlider(
                lensFovDeg,
                FgString(),
                FgVectD2(0.0,60.0),
                15.0,
                fgGuiApiTickLabels(FgVectD2(0.0,60.0),15.0,0.0),
                fgSvec(
                    FgGuiApiTickLabel(0.0,fgTr("Orthographic")),
                    FgGuiApiTickLabel(30.0,fgTr("Normal")),
                    FgGuiApiTickLabel(60.0,fgTr("Wide"))),
                30      // Lots of room required for "Orthographic" at end of slider
            )
        );
    FgGuiPtr        viewCtlScale =
        fgGuiGroupboxTr(
            "Object relative scale",
            fgGuiSlider(
                api.logRelSize,
                FgString(),
                logScaleRange,
                (logScaleRange[1]-logScaleRange[0])/4.0,
                fgSvec(
                    FgGuiApiTickLabel(logScaleRange[0],"0.2"),
                    FgGuiApiTickLabel(0.0,"1.0"),
                    FgGuiApiTickLabel(logScaleRange[1],"5.0")),
                vector<FgGuiApiTickLabel>(),
                30      // Match width of slider above
            )
        );
    FgGuiPtr        viewCtlReset = fgGuiButtonTr("Reset Camera",boost::bind(
        resetView,lensFovDeg,api.panTiltDegrees,api.pose,api.trans,api.logRelSize));
    if (simple == 2)
        ret.cameraGui = fgGuiSplit(false,viewCtlText,viewCtlScale,viewCtlReset);
    else
        ret.cameraGui = fgGuiSplit(false,fgSvec(
            viewCtlText,viewCtlRotation,viewCtlFov,viewCtlScale,viewCtlReset));
    ret.editCtls =
        fgGuiSplit(false,fgSvec(
            fgGuiText(
                "Mark Surface Point: ctrl-shift-left-click on surface.\n"
                "A point with an identical name will be overwritten"
            ),
            fgGuiSplit(true,
                fgGuiText("Name:"),
                fgGuiTextEdit(api.pointLabel)),
            fgGuiText("Mark Vertex: ctrl-shift-right-click on surface near vertex"),
            fgGuiGroupbox("Vertex Selection Mode",
                fgGuiRadio(api.vertMarkModeN,fgSvec<FgString>("Single","Edge Seam","Fold Seam"))),
            fgGuiButtonTr("Mark all seam vertices",boost::bind(markAllSeams,meshesN)),
            fgGuiButtonTr("Clear all surface points",boost::bind(clearAllSurfPts,meshesN)),
            fgGuiButtonTr("Clear all marked vertices",boost::bind(clearAllMarkedVerts,meshesN)),
            fgGuiButtonTr("Save Pre-Morphed Mesh 0",boost::bind(saveMesh,meshesN,allVertssN))
        ));
    return ret;
}

static
FGLINK(linkSelect)
{
    FGLINKARGS(2,1);
    const vector<Fg3dMesh> &    meshes = inputs[0]->valueRef();
    size_t                      sel = inputs[1]->valueRef();
    FGASSERT(sel < meshes.size());
    vector<FgBool>              sels(meshes.size(),false);
    sels[sel] = true;
    outputs[0]->set(sels);
}

static
FGLINK(linkParts)
{
    FGLINKARGS(2,3);
    const vector<Fg3dMesh> &    meshesIn = inputs[0]->valueRef();
    const vector<FgBool> &      selections = inputs[1]->valueRef();
    FGASSERT(meshesIn.size() == selections.size());
    vector<Fg3dMesh>            meshes;
    vector<FgVerts>             vertss;
    vector<FgImgs>              imgss;
    for (size_t ii=0; ii<selections.size(); ++ii) {
        if (selections[ii]) {
            const Fg3dMesh &    mesh = meshesIn[ii];
            meshes.push_back(mesh);
            vertss.push_back(mesh.allVerts());
            imgss.push_back(mesh.texImages);
        }
    }
    outputs[0]->set(meshes);
    outputs[1]->set(vertss);
    outputs[2]->set(imgss);
}

void
fgDisplayMeshes(
    const std::vector<Fg3dMesh> &   meshes,
    bool                            compare)
{
    FGASSERT(meshes.size() > 0);
    FgString                    store = fgDirUserAppDataLocalFaceGen("SDK","fgDisplayMeshes");
    g_gg = FgGuiGraph(store);
    FgDgn<vector<Fg3dMesh> >    meshesSrcN = g_gg.addNode(meshes,"meshesSrc"),
                                meshesN(g_gg,"meshes");
    FgDgn<vector<FgBool> >      selsN = g_gg.addNode(vector<FgBool>(meshes.size(),true),"sels");
    FgDgn<vector<FgVerts> >     allVertsN(g_gg,"allVerts");
    FgDgn<vector<FgImgs> >      texssN(g_gg,"texss");

    FgGuiTab                    meshSelect;
    if (meshes.size() == 1) {   // No selection pane in this case so edit controls will work
        g_gg.setVal(meshesN,meshes);
        vector<FgVerts>         vertss;
        vector<FgImgs>          texss;
        for (size_t ii=0; ii<meshes.size(); ++ii) {
            vertss.push_back(meshes[ii].allVerts());
            texss.push_back(meshes[ii].texImages);
        }
        g_gg.setVal(allVertsN,vertss);
        g_gg.setVal(texssN,texss);
    }
    else {
        g_gg.addLink(linkParts,fgUints(meshesSrcN,selsN),fgUints(meshesN,allVertsN,texssN));
        if (compare) {
            vector<FgString>    meshNames;
            for (size_t ii=0; ii<meshes.size(); ++ii)
                meshNames.push_back(meshes[ii].name);
            FgDgn<size_t>       selN = g_gg.addNode(size_t(0),"selectMesh");
            g_gg.addLink(linkSelect,fgUints(meshesSrcN,selN),selsN);
            meshSelect = fgGuiTab("Select",true,fgGuiRadio(selN,meshNames));
        }
        else {
            meshSelect = fgGuiTab("Select",true,fgGuiText("TODO: part selections"));
        }
    }
    FgMat32F                 viewBounds = fgBounds(meshes[0].verts);
    for (size_t ii=1; ii<meshes.size(); ++ii)
        viewBounds = fgBounds(viewBounds,fgBounds(meshes[ii].verts));
    FgDgn<FgMat32D>          viewBoundsN = g_gg.addNode(fgF2D(viewBounds),"viewBounds");
    FgDgn<FgString>             meshStatsN(g_gg,"meshStats");
    g_gg.addLink(linkMeshStats2,meshesN,meshStatsN);
    FgRenderCtrls       renderOpts = fgRenderCtrls(0);
    FgGui3dCtls         ga3 = fgGui3dCtls(
        meshesN,allVertsN,texssN,viewBoundsN,renderOpts,
        g_gg.addNode(FgLighting(),"Lighting"),NULL);
    vector<FgGuiTab>    viewTabs = fgSvec(
        fgGuiTab("Camera",true,ga3.cameraGui),
        fgGuiTab("Render",true,renderOpts.win));
    vector<FgGuiTab>    tabDefs = fgSvec(
        fgGuiTab("View",false,fgGuiTabs(viewTabs)),
        fgGuiTab("Morphs",true,ga3.morphCtls)//,
        //fgGuiTab("Info",true,fgGuiSplitScroll(fgSvec(fgGuiText(meshStatsN))))
    );
    if (meshes.size() == 1)
        tabDefs.push_back(fgGuiTab("Edit",true,ga3.editCtls));
    else
        tabDefs.push_back(meshSelect);
    fgGuiImplStart(
        FgString("FaceGen SDK fgDisplayMeshes"),
        //fgGuiSplitAdj(true,ga3.viewport,ga3.morphCtls),
        fgGuiSplitAdj(true,ga3.viewport,fgGuiTabs(tabDefs)),
        store);
}

Fg3dMesh
fgDisplayForEdit(const Fg3dMesh & mesh)
{
    FgString                    store = fgDirUserAppDataLocalFaceGen("SDK","fgDisplayForEdit");
    g_gg = FgGuiGraph(store);
    FgDgn<vector<Fg3dMesh> >    meshesN = g_gg.addNode(fgSvec(mesh),"meshes");
    FgDgn<vector<FgVerts> >     allVertsN = g_gg.addNode(vector<FgVerts>(1,mesh.allVerts()),"allVerts");
    FgDgn<vector<FgImgs> >      texssN = g_gg.addNode(vector<FgImgs>(1,mesh.texImages),"texss");
    FgGuiTab                    meshSelect;
    FgDgn<FgMat32D>             viewBoundsN = g_gg.addNode(fgF2D(fgBounds(mesh.verts)),"viewBounds");
    FgDgn<FgString>             meshStatsN(g_gg,"meshStats");
    g_gg.addLink(linkMeshStats2,meshesN,meshStatsN);
    FgRenderCtrls               renderOpts = fgRenderCtrls(0);
    FgGui3dCtls         ga3 = fgGui3dCtls(
        meshesN,allVertsN,texssN,viewBoundsN,renderOpts,
        g_gg.addNode(FgLighting(),"Lighting"),NULL);
    vector<FgGuiTab>    viewTabs = fgSvec(
        fgGuiTab("Camera",true,ga3.cameraGui),
        fgGuiTab("Render",true,renderOpts.win));
    vector<FgGuiTab>    tabDefs = fgSvec(
        fgGuiTab("View",false,fgGuiTabs(viewTabs)),
        fgGuiTab("Morphs",true,ga3.morphCtls),
        fgGuiTab("Info",true,fgGuiText(meshStatsN)));
        tabDefs.push_back(fgGuiTab("Edit",true,ga3.editCtls));
    fgGuiImplStart(
        FgString("FaceGen SDK fgDisplayForEdit"),
        fgGuiSplitAdj(true,ga3.viewport,fgGuiTabs(tabDefs)),
        store);
    return g_gg.getVal(meshesN)[0];
}

void
fgTestmGuiMesh(const FgArgs &)
{
    FgString    dir = fgDataDir() + "base/";
    Fg3dMesh    mesh = fgLoadTri(dir+"JaneLoresFace.tri",dir+"JaneLoresFace.jpg");
    fgDisplayMesh(mesh);
}

// */
