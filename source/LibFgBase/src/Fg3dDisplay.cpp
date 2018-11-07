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
using namespace std::placeholders;

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
    lighting.ambient = FgVect3F(amb[0],amb[1],amb[2]);
    lighting.lights.resize(2);    // Must always assign both lights since OGL will keep settings for both
    for (uint ll=0; ll<2; ++ll) {
        const vector<double> &  ls = (ll == 0) ? l1 : l2;
        const vector<double> &  ds = (ll == 0) ? d1 : d2;
        FgLight                 light;
        light.colour = FgVect3F(ls[0],ls[1],ls[2]);
        light.direction = fgNormalize(FgVect3F(ds[0],ds[1],ds[2]));
        lighting.lights[ll] = light;
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
    FgVectD2        bounds(-100000,100000);
    FgGuiPtr        x = fgGuiTextEditFloat(ret.dirAxisNs[0],bounds),
                    y = fgGuiTextEditFloat(ret.dirAxisNs[1],bounds),
                    z = fgGuiTextEditFloat(ret.dirAxisNs[2],bounds);
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
        fgGuiButton("Reset Lighting",std::bind(resetLighting,ret.outN.idx()))
    ));
    return ret;
}

static
FGLINK(linkRenderOpts)
{
    FGLINKARGS(11,1);
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
    bool                showAxes = inputs[10]->valueRef();
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
    ro.showAxes = showAxes;
}

static
void
bgImageLoad(FgDgn<FgImgRgbaUb> imgN,FgGuiApiBgImage bgImg)
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
    g_gg.setVal(bgImg.origDimsN,orig.dims());
    FgImgRgbaUb &       img = g_gg.getRef(imgN);
    img.resize(FgVect2UI(1024));
    fgImgResize(orig,img);
    g_gg.setVal(bgImg.offset,FgVect2F(0));
    g_gg.setVal(bgImg.lnScale,0.0);
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

FgBgImageCtrls
fgBgImageCtrls()
{
    FgBgImageCtrls      ret;
    ret.api.imgN = g_gg.addNode(FgImgRgbaUb(),"bgImage");
    ret.api.origDimsN = g_gg.addNode(FgVect2UI());
    ret.api.lnScale = g_gg.addNode(0.0);
    ret.api.offset = g_gg.addNode(FgVect2F(0));
    FgDgn<FgImgRgbaUb>  bgImgPreAlphaN = g_gg.addNode(FgImgRgbaUb(),"bgImgPreAlpha");
    FgDgn<double>       bgAlphaN = g_gg.addInput(0.0,"BgImageAlpha");
    g_gg.addLink(lnkSetAlpha,fgUints(bgImgPreAlphaN,bgAlphaN),ret.api.imgN);
    FgGuiPtr            bgImageSlider = fgGuiSlider(bgAlphaN,"Foreground transparency",FgVectD2(0,1),0.1),
                        bgImageButtons = fgGuiSplit(true,
                            fgGuiButton("Load Image",std::bind(bgImageLoad,bgImgPreAlphaN,ret.api)),
                            fgGuiButton("Clear Image",std::bind(bgImageClear,bgImgPreAlphaN))),
                        text = fgGuiText(
                            "Move: Ctrl-Shift-Left-Drag\n"
                            "Scale: Ctrl-Shift-Right-Drag");
    ret.win = fgGuiGroupbox("Background Image",fgGuiSplit(false,bgImageButtons,bgImageSlider,text));
    ret.changeFlag = g_gg.addUpdateFlag(fgUints(ret.api.imgN,ret.api.origDimsN,ret.api.lnScale,ret.api.offset));
    return ret;
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
                        surfPoints = g_gg.addInput((simple == 0),uid+"SurfPoints"),
                        markedVerts = g_gg.addInput((simple == 0),uid+"MarkedVerts"),
                        allVerts = g_gg.addInput(false,uid+"AllVerts"),
                        twoSidedN = g_gg.addInput(true,uid+"TwoSided"),
                        showAxesN = g_gg.addInput(false,uid+"ShowAxes");
    FgGuiWinVal<vector<double> >    bgColor = colorSliders(uid+"Background",0.0,0.1);
    FgDgn<FgVect3F>     bgColorN = g_gg.addNode(FgVect3F(),"bgColor");
    g_gg.addLink(linkColSel,bgColor.valN,bgColorN);
    ret.optsN = g_gg.addNode(Fg3dRenderOptions(),"renderOptions");
    g_gg.addLink(linkRenderOpts,
        fgSvec<uint>(facets,useTexture,shiny,wireframe,flatShaded,surfPoints,markedVerts,allVerts,twoSidedN,bgColorN,showAxesN),
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
                        rcAxes = fgGuiCheckbox("Show origin and axes (red:X green:Y blue: Z)",showAxesN);
    FgGuiPtr            renderCtls;
    if (simple == 1)
        renderCtls = fgGuiSplit(true,
            fgGuiSplit(false,rcColor,rcShiny,rcWire),
            fgGuiSplit(false,rcFlat,rcFacet),
            fgGuiSplit(false,rcAll,rcTwoSided));
    else if (simple == 2)
        renderCtls = fgGuiSplit(false,rcColor,rcShiny);
    else if (simple == 3)
        renderCtls = fgGuiSplit(false,rcShiny,rcWire,rcFlat);
    else    // simple == 0 or default to all controls:
        renderCtls = fgGuiSplit(false,fgGuiSplit(true,
            fgGuiSplit(false,rcColor,rcShiny,rcWire),
            fgGuiSplit(false,rcFlat,rcFacet,rcSurf),
            fgGuiSplit(false,rcMark,rcAll,rcTwoSided)),rcAxes);
    FgGuiPtr        viewCtlRender = fgGuiGroupboxTr("Render",renderCtls),
                    viewCtlColor = fgGuiGroupboxTr("Background Color",bgColor.win);
    FgBgImageCtrls  bgImageCtrls = fgBgImageCtrls();
    ret.bgImgApi = bgImageCtrls.api;
    ret.win = fgGuiSplit(false,viewCtlRender,viewCtlColor,bgImageCtrls.win);
    return ret;
}

static
FGLINK(linkNorms)
{
    FGLINKARGS(2,1);
    const vector<Fg3dMesh> &    meshes = inputs[0]->valueRef();
    const FgVertss &            vertss = inputs[1]->valueRef();
    vector<Fg3dNormals> &       normss = outputs[0]->valueRef();
    FGASSERT(meshes.size() == vertss.size());
    normss.resize(meshes.size());
    for (size_t ii=0; ii<normss.size(); ++ii)
        fgNormals_(meshes[ii].surfaces,vertss[ii],normss[ii]);
}

static
FGLINK(linkXform)
{
    FGLINKARGS(9,1);
    Fg3dCameraParams    cps;
    cps.modelBounds = inputs[0]->valueRef();
    size_t              panTiltMode = inputs[1]->valueRef();
    FgVect2D            panTilt;
    panTilt[0] = inputs[2]->valueRef();
    panTilt[1] = inputs[3]->valueRef();
    cps.pose = inputs[4]->valueRef();
    cps.relTrans = inputs[5]->valueRef();
    cps.logRelScale = inputs[6]->valueRef();
    cps.fovMaxDeg= inputs[7]->valueRef();
    FgVect2UI           viewport = inputs[8]->valueRef();
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
    FgDgn<double>           pan,
    FgDgn<double>           tilt,
    FgDgn<FgQuaternionD>    pose,
    FgDgn<FgVect2D>         translate,
    FgDgn<double>           logRelSize)
{
    Fg3dCameraParams        defaultCps;
    defaultCps.logRelScale = -0.3;
    g_gg.setVal(lensFovDeg,defaultCps.fovMaxDeg);
    g_gg.setVal(pan,0.0);
    g_gg.setVal(tilt,0.0);
    g_gg.setVal(pose,FgQuaternionD());
    g_gg.setVal(translate,FgVect2D(0));
    g_gg.setVal(logRelSize,defaultCps.logRelScale);
}

static
FGLINK(lnkPoses)
{
    FGLINKARGS(1,1);
    const vector<Fg3dMesh> &    meshes = inputs[0]->valueRef();
    FgPoses &                   poses = outputs[0]->valueRef();
    poses = fgPoses(meshes);
}

static
FGLINK(lnkPoseShape)
{
    FGLINKARGS(4,1);
    const vector<Fg3dMesh> &    meshes = inputs[0]->valueRef();
    const FgVertss &            allVertss = inputs[1]->valueRef();
    const FgPoses &             poses = inputs[2]->valueRef();
    vector<double>              poseVals = inputs[3]->valueRef();
    FgVertss &                  vertss = outputs[0]->valueRef();
    FGASSERT(meshes.size() == allVertss.size());
    // If pose list has changed but poseVals not yet updated (by GUI), assume all zero.
    // TODO: Keep poses as a label:val dictionary.
    if (poses.size() != poseVals.size()) {
        poseVals.clear();
        poseVals.resize(poses.size(),0.0);
    }
    map<FgString,float>     poseMap;
    for (size_t ii=0; ii<poses.size(); ++ii)
        poseMap[poses[ii].name] = poseVals[ii];
    vertss.resize(meshes.size());
    for (size_t ii=0; ii<meshes.size(); ++ii)
        vertss[ii] = meshes[ii].poseShape(allVertss[ii],poseMap);
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
saveMesh(string format,FgDgn<vector<Fg3dMesh> > meshesN,FgDgn<FgVertss > vertssN)
{
    const vector<Fg3dMesh> &    meshes = g_gg.getVal(meshesN);
    const FgVertss &     vertss = g_gg.getVal(vertssN);
    FGASSERT(!meshes.empty());
    FGASSERT(meshes.size() == vertss.size());
    FgOpt<FgString>        fname = fgGuiDialogFileSave("FaceGen",format);
    if (fname.valid()) {
        Fg3dMesh                mesh = meshes[0];
        mesh.updateAllVerts(vertss[0]);
        fgSaveMeshAnyFormat(mesh,fname.val());
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
FgString
morphGetTxt(FgDgn<vector<double> > valsN,size_t idx)
{
    const vector<double> &  vals = g_gg.getVal(valsN);
    return fgToFixed(vals[idx],2);
}

static
void
morphSetTxt(FgDgn<vector<double> > valsN,size_t idx,const FgString & str)
{
    double              val = fgFromString<double>(str.as_ascii());
    vector<double> &    vals = g_gg.getRef(valsN);
    vals[idx] = fgClip(val,-1.0,2.0);
}

static
void
expr_load(FgDgn<FgPoses> posesN,FgDgn<vector<double> > valsN)
{
    FgOpt<FgString> fname = fgGuiDialogFileLoad("FaceGen XML expression file",fgSvec<string>("xml"));
    if (fname.valid()) {
        const FgPoses &         poses = g_gg.getVal(posesN);
        vector<double> &        vals = g_gg.getRef(valsN);
        FGASSERT(poses.size() == vals.size());
        vector<pair<FgString,double> >  lvs;
        fgLoadXml(fname.val(),lvs);
        for (size_t ii=0; ii<lvs.size(); ++ii) {
            size_t      idx = fgFindFirstIdx(poses,lvs[ii].first);
            if (idx < poses.size())
                vals[idx] = lvs[ii].second;
        }
    }
}

static
void
expr_save(FgDgn<FgPoses> posesN,FgDgn<vector<double> > valsN)
{
    FgOpt<FgString> fname = fgGuiDialogFileSave("FaceGen XML expression file","xml");
    if (fname.valid()) {
        const FgPoses &             poses = g_gg.getVal(posesN);
        const vector<double> &      vals = g_gg.getVal(valsN);
        FGASSERT(poses.size() == vals.size());
        vector<pair<FgString,double> >  lvs;
        lvs.reserve(poses.size());
        for (size_t ii=0; ii<poses.size(); ++ii)
            lvs.push_back(make_pair(poses[ii].name,vals[ii]));
        fgSaveXml(fname.val(),lvs);
    }
}

// Called by FgGuiSplitScroll when 'labelsN' changes:
static 
FgGuiPtrs
getPanes(FgDgn<FgPoses> posesN,FgDgn<vector<double> > valsN,bool textEditBoxes)
{
    // Since this window is dynamic, its values are stored as a vector in a single node, to
    // avoid having to edit the depgraph with each update.
    // Do not use const ref as DG may re-allocate:
    const FgPoses &         poses = g_gg.getVal(posesN);
    vector<double> &        output = g_gg.dg.nodeValRef(valsN);
    if (output.size() != poses.size()) {   // past value is stale
        output.clear();
        output.resize(poses.size());
        for (size_t ii=0; ii<output.size(); ++ii)
            output[ii] = poses[ii].neutral;
    }
    FgGuiPtrs       buttons;
    buttons.push_back(fgGuiButton("Set All Zero",std::bind(setAllZero,valsN)));
    buttons.push_back(fgGuiButton("Load File",std::bind(expr_load,posesN,valsN)));
    buttons.push_back(fgGuiButton("Save File",std::bind(expr_save,posesN,valsN)));
    FgGuiPtrs       sliders;
    sliders.push_back(fgGuiSplit(true,buttons));
    sliders.push_back(fgGuiSpacer(0,7));
    for (size_t ii=0; ii<poses.size(); ++ii) {
        FgGuiApiSlider      slider;
        slider.updateFlagIdx = g_gg.addUpdateFlag(valsN);     // Node leak but what can you do.
        slider.getInput = std::bind(getInput,valsN,ii);
        slider.setOutput = std::bind(setOutput,valsN,ii,_1);
        slider.label = poses[ii].name;
        slider.range[0] = poses[ii].bounds[0];
        slider.range[1] = poses[ii].bounds[1];
        slider.tickSpacing = 0.1;
        if (textEditBoxes) {
            FgGuiApiTextEdit    te;
            te.updateFlagIdx = g_gg.addUpdateFlag(valsN);
            te.minWidth = 50;
            te.wantStretch = false;
            te.getInput = std::bind(morphGetTxt,valsN,ii);
            te.setOutput = std::bind(morphSetTxt,valsN,ii,_1);
            sliders.push_back(fgGuiSplit(true,fgGuiPtr(slider),fgGuiPtr(te)));
        }
        else
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

static
void
assignTri(      // Re-assign a tri (or quad) to a different surface if intersected
    FgDgn<Fg3dMeshes> meshesN,FgDgn<FgVertss> allVertssN,FgDgn<size_t> surfIdx,
    FgVect2UI winSize,FgVect2I pos,FgMat44F toOics)
{
    if (g_gg.dg.sinkNode(meshesN))      // feature disabled if node not modifiable
        return;
    vector<Fg3dMesh> &          meshes = g_gg.getRef(meshesN);
    const FgVertss &            vertss = g_gg.getVal(allVertssN);
    FgOpt<FgMeshesIntersect>    vpt = fgMeshesIntersect(winSize,pos,toOics,meshes,vertss);
    if (vpt.valid()) {
        FgMeshesIntersect       isct = vpt.val();
        size_t                  dstSurfIdx = g_gg.getVal(surfIdx);
        Fg3dMesh &              mesh = meshes[isct.meshIdx];
        if ((isct.surfIdx != dstSurfIdx) && (dstSurfIdx < mesh.surfaces.size())) {
            Fg3dSurface &           srcSurf = mesh.surfaces[isct.surfIdx];
            Fg3dSurface &           dstSurf = mesh.surfaces[dstSurfIdx];
            if (srcSurf.isTri(isct.surfPnt.triEquivIdx)) {
                // Copy surface points on selected tri to destination surface:
                for (FgSurfPoint sp : srcSurf.surfPoints) {
                    if (sp.triEquivIdx == isct.surfPnt.triEquivIdx) {
                        sp.triEquivIdx = uint(dstSurf.tris.size());
                        dstSurf.surfPoints.push_back(sp);
                    }
                }
                // Move selected tri to destination surface:
                size_t          idx = isct.surfPnt.triEquivIdx;
                if (dstSurf.tris.hasUvs())
                    dstSurf.tris.uvInds.push_back(srcSurf.tris.uvInds[idx]);
                dstSurf.tris.vertInds.push_back(srcSurf.tris.vertInds[idx]);
                srcSurf.removeTri(isct.surfPnt.triEquivIdx);
            }
            else {                      // It's a quad
                // Copy surface points on selected quad to destination surface:
                size_t                  quadIdx = (isct.surfPnt.triEquivIdx - srcSurf.tris.size())/2;
                for (FgSurfPoint sp : srcSurf.surfPoints) {
                    size_t              spQuadIdx2 = sp.triEquivIdx - srcSurf.tris.size(),
                                        spQuadIdx = spQuadIdx2 / 2,
                                        spQuadMod = spQuadIdx2 % 2;
                    if (spQuadIdx == quadIdx) {
                        size_t          idx = dstSurf.tris.size() + 2*dstSurf.quads.size() + spQuadMod;
                        sp.triEquivIdx = uint(idx);
                        dstSurf.surfPoints.push_back(sp);
                    }
                }
                // Move selected quad to destination surface:
                if (dstSurf.quads.hasUvs())
                    dstSurf.quads.uvInds.push_back(srcSurf.quads.uvInds[quadIdx]);
                dstSurf.quads.vertInds.push_back(srcSurf.quads.vertInds[quadIdx]);
                srcSurf.removeQuad(quadIdx);
            }
        }
    }
}

// Currently only works on tri facets:
static
void
assignPaint(
    FgDgn<Fg3dMeshes> meshesN,FgDgn<FgVertss> allVertssN,FgDgn<size_t> surfIdx,
    FgVect2UI winSize,FgVect2I pos,FgMat44F toOics)
{
    if (g_gg.dg.sinkNode(meshesN))      // feature disabled if node not modifiable
        return;
    vector<Fg3dMesh> &          meshes = g_gg.getRef(meshesN);
    const FgVertss &            vertss = g_gg.getVal(allVertssN);
    FgOpt<FgMeshesIntersect>    vpt = fgMeshesIntersect(winSize,pos,toOics,meshes,vertss);
    if (vpt.valid()) {
        FgMeshesIntersect       isct = vpt.val();
        size_t                  dstSurfIdx = g_gg.getVal(surfIdx);
        Fg3dMesh &              mesh = meshes[isct.meshIdx];
        if ((isct.surfIdx != dstSurfIdx) && (dstSurfIdx < mesh.surfaces.size())) {
            FgVect3UI           tri = mesh.surfaces[isct.surfIdx].getTriEquiv(isct.surfPnt.triEquivIdx);
            Fg3dSurfaces        surfs = fgSplitSurface(mesh.surfaces[isct.surfIdx]);
            for (size_t ss=0; ss<surfs.size(); ++ss) {
                if (fgContains(surfs[ss].tris.vertInds,tri)) {
                    mesh.surfaces[dstSurfIdx].merge(surfs[ss]);
                    surfs.erase(surfs.begin()+ss);
                    mesh.surfaces[isct.surfIdx] = fgMergeSurfaces(surfs);
                    return;
                }
            }
        }
    }
}

// Modifies 'api' to add it's hooks:
static
FgGuiPtr
fg3dGuiEdit(FgGuiApi3d & api,const FgString & uid,FgDgn<Fg3dMeshes> meshesN,FgDgn<FgVertss> allVertssN)
{
    if (g_gg.dg.sinkNode(meshesN) || g_gg.dg.sinkNode(allVertssN))
        return fgGuiSpacer(0,0);
    vector<FgGuiTab>    tabs;
    FgGuiRadio          vertSelModeRadio = fgGuiRadio(fgSvec<FgString>("Single","Edge Seam","Fold Seam"),uid+"_vertMarkMode");
    api.vertMarkModeN = vertSelModeRadio.idxN;
    api.pointLabel = g_gg.addNode(FgString());
    api.colorBySurface = g_gg.addInput(false,uid+"_facet_color");
    FgGuiPtr            points = 
        fgGuiSplit(false,fgSvec(
            fgGuiText(
                "Mark Surface Point: ctrl-shift-left-click on surface.\n"
                "A point with an identical non-null name will be overwritten"
            ),
            fgGuiSplit(true,
                fgGuiText("Name:"),
                fgGuiTextEdit(api.pointLabel)),
            fgGuiButtonTr("Clear all surface points",std::bind(clearAllSurfPts,meshesN))
        ));
    FgGuiPtr            verts =
        fgGuiSplit(false,fgSvec(
            fgGuiText("Mark Vertex: ctrl-shift-right-click on surface near vertex"),
            fgGuiGroupbox("Vertex Selection Mode",vertSelModeRadio.win),
            fgGuiButtonTr("Mark all seam vertices",std::bind(markAllSeams,meshesN)),
            fgGuiButtonTr("Clear all marked vertices",std::bind(clearAllMarkedVerts,meshesN))
        ));
    Fg3dSurfaces &      surfs = g_gg.getRef(meshesN)[0].surfaces;
    FgStrings    surfNames;
    for (size_t ss=0; ss<surfs.size(); ++ss) {
        if (surfs[ss].name.empty())             // Generally not the case since default surf names assigned on load
            surfs[ss].name = fgToStr(ss);
        surfNames.push_back(surfs[ss].name);
    }
    FgGuiPtr            facets;
    if (surfNames.empty())
        facets = fgGuiText("There are no surfaces to edit");
    else {
        FgGuiRadio          surfChoice = fgGuiRadio(surfNames);
        api.shiftRightDragAction = std::bind(assignTri,meshesN,allVertssN,surfChoice.idxN,_1,_2,_3);
        api.ctrlShiftMiddleClickAction = std::bind(assignPaint,meshesN,allVertssN,surfChoice.idxN,_1,_2,_3);
        facets = fgGuiSplit(false,
            fgGuiText("Assign tri to surface selection: shift-right-drag\n"
                "Assign connected tris (not quads) to surface: shift-middle-click"),
            fgGuiGroupbox("Surface Selection",surfChoice.win),
            fgGuiCheckbox("Color by surface",api.colorBySurface));
    }
    tabs.push_back(FgGuiTab("Points",true,points));
    tabs.push_back(FgGuiTab("Verts",true,verts));
    tabs.push_back(FgGuiTab("Facets",true,facets));
    FgGuiPtr            saveButtons = fgGuiSplit(true,
        fgGuiButtonTr("TRI",std::bind(saveMesh,string("tri"),meshesN,allVertssN)),
        fgGuiButtonTr("FGMESH",std::bind(saveMesh,string("fgmesh"),meshesN,allVertssN)));
    return fgGuiSplit(false,fgGuiText("Save pre-morphed to file:"),saveButtons,fgGuiTabs(tabs));
}

FgGui3dCtls
fgGui3dCtls(
    FgDgn<vector<Fg3dMesh> >    meshesN,
    FgDgn<FgVertss >            allVertssN,
    FgDgn<vector<FgImgs> >      texssN,
    FgDgn<FgMat32D>             viewBoundsN,
    FgRenderCtrls               renderCtrls,
    FgDgn<FgLighting>           lightingN,
    std::function<void(bool,FgVect2I)>    bothButtonsDrag,
    uint                        simple,
    bool                        textEditBoxes)
{
    FgGui3dCtls                 ret;
    FgDgn<FgPoses>              posesN = g_gg.addNode(FgPoses(),"morphNames");
    g_gg.addLink(lnkPoses,meshesN,posesN);
    FgDgn<vector<double> >      morphValsN = g_gg.addInput(vector<double>(),"morphVals");
    ret.morphedVertssN = g_gg.addNode(FgVertss(),"morphedVertss");
    g_gg.addLink(lnkPoseShape,fgUints(meshesN,allVertssN,posesN,morphValsN),ret.morphedVertssN);
    ret.morphCtls = fgGuiSplitScroll(g_gg.addUpdateFlag(posesN),
        std::bind(getPanes,posesN,morphValsN,textEditBoxes),3);
    vector<Fg3dMesh>            meshes = g_gg.getVal(meshesN);
    string                      uid = "3d";
    FgStrings            panTiltOptions = fgSvec<FgString>("Pan / Tilt","Unconstrained");
    FgGuiRadio                  panTiltRadio;
    if (simple == 0)
        panTiltRadio = fgGuiRadio(panTiltOptions,panTiltOptions,uid+"_PanTiltMode",panTiltOptions[1]);
    else
        panTiltRadio = fgGuiRadio(panTiltOptions,uid+"_PanTiltMode");
    FgGuiApi3d      api;
    api.meshesN = meshesN;
    api.vertssN = ret.morphedVertssN;
    api.normssN = g_gg.addNode(vector<Fg3dNormals>(),"normss");
    api.texssN = texssN;
    api.viewBounds = viewBoundsN;
    api.panTiltMode = panTiltRadio.idxN;
    api.light = lightingN;
    api.xform = g_gg.addNode(Fg3dCamera(),"xform");
    api.renderOptions = renderCtrls.optsN;
    if (simple == 2)
        api.panTiltLimits = true;
    api.panDegrees = g_gg.addInput(0.0,uid+"camPan");
    api.tiltDegrees = g_gg.addInput(0.0,uid+"camTilt");
    api.pose = g_gg.addInput(FgQuaternionD(),uid+"Pose");
    api.trans = g_gg.addInput(FgVect2D(0),uid+"Trans");
    api.logRelSize = g_gg.addInput(-0.3,uid+"LogRelSize");
    // Give non-zero viewport initial val to avoid NaNs in initial dependency calc for rendering:
    api.viewportDims = g_gg.addNode(FgVect2UI(1),"viewportDims");
    api.bgImg = renderCtrls.bgImgApi;
    api.bothButtonsDragAction = bothButtonsDrag;
    g_gg.addLink(linkNorms,fgUints(meshesN,ret.morphedVertssN),api.normssN);
    Fg3dCameraParams    defaultCps;     // Use default for lensFovDeg:
    FgDgn<double>       lensFovDeg = g_gg.addInput(defaultCps.fovMaxDeg,uid+"LensFovDeg");
    FgVectD2            logScaleRange(log(1.0/5.0),log(5.0));
    g_gg.addLink(linkXform,
        fgUints(viewBoundsN,api.panTiltMode,api.panDegrees,api.tiltDegrees,api.pose,api.trans,
                api.logRelSize,lensFovDeg,api.viewportDims),
        api.xform);
    ret.editCtls = fg3dGuiEdit(api,uid+"_edit",meshesN,allVertssN);
    ret.viewport = fgsp(api);
    FgGuiPtr        viewCtlText =
        fgGuiText(
            "  Rotate: left-click-drag (or touch-drag)\n"
            "  Scale: right-click-drag up/down (or pinch-to-zoom)\n"
            "  Move: shift-left-click-drag (or middle-click-drag)");
    FgVectD2        panTiltClip(-360.0,360.0);      // Values are wrapped by viewport impl
    FgGuiPtr        viewCtlPanTiltText =
        fgGuiSplit(false,
            fgGuiSplit(true,fgGuiTextEditFixed(api.panDegrees,panTiltClip),fgGuiTextEditFixed(api.tiltDegrees,panTiltClip)),
            fgGuiSpacer(0,30));
    if (textEditBoxes)
        panTiltRadio.win = fgGuiSplit(true,panTiltRadio.win,viewCtlPanTiltText);
    FgGuiPtr        viewCtlRotation = fgGuiGroupboxTr("Object rotation",panTiltRadio.win);
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
                30,     // Lots of room required for "Orthographic" at end of slider
                textEditBoxes
            )
        );
    // Required to disambiguate float / double overloads:
    double (*pexpd)(double) = &std::exp;
    double (*plogd)(double) = &std::log;
    FgGuiPtr        viewCtlScaleTe = fgGuiTextEditFloat(api.logRelSize,logScaleRange,6,pexpd,plogd);
    FgGuiPtr        viewCtlScale =
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
                );
    if (textEditBoxes)
        viewCtlScale = fgGuiSplit(true,viewCtlScale,viewCtlScaleTe);
    viewCtlScale = fgGuiGroupboxTr("Object relative scale",viewCtlScale);
    FgGuiPtr        viewCtlReset = fgGuiButtonTr("Reset Camera",std::bind(
        resetView,lensFovDeg,api.panDegrees,api.tiltDegrees,api.pose,api.trans,api.logRelSize));
    if (simple == 2)
        ret.cameraGui = fgGuiSplit(false,viewCtlText,viewCtlScale,viewCtlReset);
    else
        ret.cameraGui = fgGuiSplit(false,fgSvec(
            viewCtlText,viewCtlRotation,viewCtlFov,viewCtlScale,viewCtlReset));
    return ret;
}

static
FGLINK(linkSelect)
{
    FGLINKARGS(2,1);
    const vector<Fg3dMesh> &    meshes = inputs[0]->valueRef();
    size_t                      sel = inputs[1]->valueRef();
    FGASSERT(sel < meshes.size());
    vector<bool>                sels(meshes.size(),false);
    sels[sel] = true;
    outputs[0]->set(sels);
}

static
FGLINK(linkParts)
{
    FGLINKARGS(2,3);
    const vector<Fg3dMesh> &    meshesIn = inputs[0]->valueRef();
    const vector<bool> &        selections = inputs[1]->valueRef();
    FGASSERT(meshesIn.size() == selections.size());
    vector<Fg3dMesh>            meshes;
    FgVertss             vertss;
    vector<FgImgs>              imgss;
    for (size_t ii=0; ii<selections.size(); ++ii) {
        if (selections[ii]) {
            const Fg3dMesh &    mesh = meshesIn[ii];
            meshes.push_back(mesh);
            vertss.push_back(mesh.allVerts());
            imgss.push_back(mesh.albedoMapsOld());
        }
    }
    outputs[0]->set(meshes);
    outputs[1]->set(vertss);
    outputs[2]->set(imgss);
}

Fg3dMesh
FgViewMeshes(
    const vector<Fg3dMesh> &    meshes,
    bool                        compare)
{
    FGASSERT(meshes.size() > 0);
    FgString                    store = fgDirUserAppDataLocalFaceGen("SDK","FgViewMeshes");
    g_gg = FgGuiGraph(store);
    FgDgn<Fg3dMeshes>           meshesN = g_gg.addNode(Fg3dMeshes(),"meshes");
    FgDgn<FgVertss>             allVertsN = g_gg.addNode(FgVertss(),"allVerts");
    FgDgn<FgImgss>              texssN = g_gg.addNode(FgImgss(),"texss");
    FgGuiTab                    meshSelect;
    if (meshes.size() == 1) {   // No selection pane in this case so edit controls will work
        g_gg.setVal(meshesN,meshes);
        FgVertss                vertss;
        vector<FgImgs>          texss;
        vertss.push_back(meshes[0].allVerts());
        texss.push_back(meshes[0].albedoMapsOld());
        g_gg.setVal(allVertsN,vertss);
        g_gg.setVal(texssN,texss);
    }
    else {
        FgDgn<vector<Fg3dMesh> >    meshesSrcN = g_gg.addNode(meshes,"meshesSrc");
        vector<bool>                sels(meshes.size(),true);
        FgDgn<vector<bool> >        selsN = g_gg.addNode(sels,"sels");
        g_gg.addLink(linkParts,fgUints(meshesSrcN,selsN),fgUints(meshesN,allVertsN,texssN));
        FgStrings            meshNames;
        for (size_t ii=0; ii<meshes.size(); ++ii) {
            FgString        meshName = meshes[ii].name;
            // Names must be unique for radio selection buttons to work:
            if (meshName.empty())
                meshName = "Mesh " + fgToStr(ii);
            if (fgContains(meshNames,meshName)) {
                size_t      idx = 1;
                while (fgContains(meshNames,meshName+"_"+fgToStr(idx)))
                    ++idx;
                meshName += "_"+fgToStr(idx);
            }
            meshNames.push_back(meshName);
        }
        if (compare) {
            FgGuiRadio          selMeshRadio = fgGuiRadio(meshNames);
            g_gg.addLink(linkSelect,fgUints(meshesSrcN,selMeshRadio.idxN),selsN);
            meshSelect = fgGuiTab("Select",true,selMeshRadio.win);
        }
        else {
            meshSelect = fgGuiTab("Select",true,fgGuiCheckboxes(meshNames,sels,selsN));
        }
    }
    FgMat32F                viewBounds = fgBounds(meshes);
    FgDgn<FgMat32D>         viewBoundsN = g_gg.addNode(fgF2D(viewBounds),"viewBounds");
    FgDgn<FgString>         meshStatsN = g_gg.addNode(FgString(),"meshStats");
    g_gg.addLink(linkMeshStats2,meshesN,meshStatsN);
    FgRenderCtrls           renderOpts = fgRenderCtrls(0);
    FgGuiLighting           lighting = fgGuiLighting();
    FgGui3dCtls             ga3 = fgGui3dCtls(meshesN,allVertsN,texssN,viewBoundsN,renderOpts,lighting.outN,nullptr,0,true);
    vector<FgGuiTab>        viewTabs = fgSvec(
        fgGuiTab("Camera",true,ga3.cameraGui),
        fgGuiTab("Render",true,renderOpts.win),
        fgGuiTab("Lighting",true,lighting.win));
    vector<FgGuiTab>        mainTabs = fgSvec(
        fgGuiTab("View",false,fgGuiTabs(viewTabs)),
        fgGuiTab("Morphs",true,ga3.morphCtls)//,
        //fgGuiTab("Info",true,fgGuiSplitScroll(fgSvec(fgGuiText(meshStatsN))))
    );
    if (meshes.size() == 1)
        mainTabs.push_back(fgGuiTab("Edit",true,ga3.editCtls));
    else
        mainTabs.push_back(meshSelect);
    mainTabs.push_back(fgGuiTab("Info",true,fgGuiSplitScroll(fgSvec(fgGuiText(fgToStr(meshes))))));
    fgGuiImplStart(
        FgString("FaceGen SDK FgViewMeshes"),
        //fgGuiSplitAdj(true,ga3.viewport,ga3.morphCtls),
        fgGuiSplitAdj(true,ga3.viewport,fgGuiTabs(mainTabs)),
        store);
    if (meshes.size() == 1)
        return g_gg.getVal(meshesN)[0];
    else
        return Fg3dMesh();
}

static
void
simple(const FgArgs &)
{
    FgString    dir = fgDataDir() + "base/";
    Fg3dMesh    mesh = fgLoadTri(dir+"JaneLoresFace.tri",dir+"JaneLoresFace.jpg");
    fgViewMesh(mesh);
}

static
void
surfs(const FgArgs &)
{
    Fg3dMesh        mesh;
    mesh.verts.push_back(FgVect3F(0,0,0));
    mesh.verts.push_back(FgVect3F(1,0,0));
    mesh.verts.push_back(FgVect3F(1,1,0));
    mesh.verts.push_back(FgVect3F(0,1,0));
    mesh.verts.push_back(FgVect3F(0,0,-1));
    mesh.verts.push_back(FgVect3F(0,1,-1));
    Fg3dSurface     surf;
    surf.name = "1";
    surf.tris.vertInds.push_back(FgVect3UI(0,1,2));
    surf.tris.vertInds.push_back(FgVect3UI(0,5,4));
    mesh.surfaces.push_back(surf);
    surf.name = "2";
    surf.tris.vertInds[0] = FgVect3UI(0,2,3);
    surf.tris.vertInds[1] = FgVect3UI(0,3,5);
    mesh.surfaces.push_back(surf);
    fgViewMesh(mesh);
}

void
fgTestmGuiMesh(const FgArgs & args)
{
    vector<FgCmd>       cmds;
    cmds.push_back(FgCmd(simple,"simple","Jane face cutout"));
    cmds.push_back(FgCmd(surfs,"surfs","Multiple surface mesh"));
    fgMenu(args,cmds);
}

// */
