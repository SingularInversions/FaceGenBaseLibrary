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
    g_gg.setVal(lensFovDeg,defaultCps.fovMaxDeg);
    g_gg.setVal(panTilt,FgVect2D(0));
    g_gg.setVal(pose,FgQuaternionD());
    g_gg.setVal(translate,FgVect2D(0));
    g_gg.setVal(logRelSize,defaultCps.logRelScale);
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
FGLINK(linkMorphNames)
{
    FGLINKARGS(1,1);
    const vector<Fg3dMesh> &    meshes = inputs[0]->valueRef();
    vector<FgString> &          morphNames = outputs[0]->valueRef();
    set<FgString>               ns;
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        const Fg3dMesh &        mesh = meshes[ii];
        for (size_t jj=0; jj<mesh.numMorphs(); ++jj)
            ns.insert(mesh.morphName(jj));
    }
    morphNames.clear();
    for (set<FgString>::const_iterator it=ns.begin(); it != ns.end(); ++it)
        morphNames.push_back(*it);
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
    // TODO: Keep morphs vals as a label:val dictionary.
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
    FgValidVal<FgString>        fname = fgGuiDialogFileSave("FaceGen TRI","tri");
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
    const vector<double> &  vals = g_gg.getVal(n);
    return vals[idx];
}

static
void
setOutput(FgDgn<vector<double> > n,size_t idx,double val)
{
    vector<double> &        vals = g_gg.dg.nodeValRef(n);
    vals[idx] = val;
}

static
void
setAllZero(FgDgn<vector<double> > n)
{
    vector<double> &        vals = g_gg.dg.nodeValRef(n);
    for (size_t ii=0; ii<vals.size(); ++ii)
        vals[ii] = 0.0;
}

static 
FgGuiPtrs
getPanes(FgDgn<vector<FgString> > in,FgDgn<vector<double> > out)
{
    vector<FgString>    labels = g_gg.getVal(in);
    vector<double> &    output = g_gg.dg.nodeValRef(out);
    if (output.size() != labels.size()) {   // past value is stale
        output.clear();
        output.resize(labels.size(),0.0);
    }
    FgGuiPtrs        sliders;
    for (size_t ii=0; ii<labels.size(); ++ii) {
        FgGuiApiSlider      slider;
        slider.updateFlagIdx = g_gg.addUpdateFlag(out);
        slider.getInput = boost::bind(getInput,out,ii);
        slider.setOutput = boost::bind(setOutput,out,ii,_1);
        slider.label = labels[ii];
        slider.range = FgVectD2(0,1);
        slider.tickSpacing = 0.5;
        sliders.push_back(fgGuiPtr(slider));
    }
    sliders.push_back(fgGuiButton("Set All To Zero",boost::bind(setAllZero,out)));
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
        vector<vector<uint> >   seams = topo.seams();
        for (size_t ss=0; ss<seams.size(); ++ss) {
            for (size_t vv=0; vv<seams[ss].size(); ++vv)
                mesh.markedVerts.push_back(FgMarkedVert(seams[ss][vv]));
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
FGLINK(linkColSel)
{
    FGLINKARGS(3,1);
    double      red = inputs[0]->valueRef();
    double      green = inputs[1]->valueRef();
    double      blue = inputs[2]->valueRef();
    FgVect3F &  col = outputs[0]->valueRef();
    col = FgVect3F(red,green,blue);
}

FgGui3dCtls
fgGui3dCtls(
    FgDgn<vector<Fg3dMesh> >    meshesN,
    FgDgn<vector<FgVerts> >     allVertssN,
    FgDgn<vector<FgImgs> >      texssN,
    FgDgn<FgMat32D>          viewBoundsN,
    uint                        simple)
{
    FgGui3dCtls                 ret;
    FgDgn<vector<FgString> >    morphNamesN(g_gg,"morphNames");
    g_gg.addLink(linkMorphNames,fgUints(meshesN),fgUints(morphNamesN));
    FgDgn<vector<double> >      morphValsN = g_gg.addInput(vector<double>(),"morphVals");
    ret.morphedVertssN = g_gg.addNode(vector<FgVerts>(),"morphedVertss");
    g_gg.addLink(linkMorphs,fgUints(meshesN,allVertssN,morphNamesN,morphValsN),fgUints(ret.morphedVertssN));
    ret.morphCtls =
        fgGuiSplitScroll(g_gg.addUpdateFlag(morphNamesN.idx()),boost::bind(getPanes,morphNamesN,morphValsN));
    vector<Fg3dMesh>            meshes = g_gg.getVal(meshesN);

    string                  uid = "3d";
    FgGuiApi3d              api;
    api.meshesN = meshesN;
    api.vertssN = ret.morphedVertssN;
    api.normssN = g_gg.addNode(vector<Fg3dNormals>(),"normss");
    api.texssN = texssN;
    api.viewBounds = viewBoundsN;
    api.panTiltMode = g_gg.addInput(size_t(0),uid+"PanTiltMode");
    api.light = g_gg.addNode(FgLighting(),"light");
    api.xform = g_gg.addNode(Fg3dCamera(),"xform");
    api.renderOptions = g_gg.addNode(Fg3dRenderOptions(),"renderOptions");
    api.vertMarkModeN = g_gg.addNode(size_t(0));
    api.pointLabel = g_gg.addNode(FgString());
    if (simple == 2)
        api.panTiltLimits = true;
    api.panTiltDegrees = g_gg.addInput(FgVect2D(0),uid+"PanTilt");
    api.pose = g_gg.addInput(FgQuaternionD(),uid+"Pose");
    api.trans = g_gg.addInput(FgVect2D(0),uid+"Trans");
    Fg3dCameraParams    defaultCps;
    api.logRelSize = g_gg.addInput(defaultCps.logRelScale,uid+"LogRelSize");
    api.viewportDims = g_gg.addNode(FgVect2UI(0),"viewportDims");
    // Including api.viewBounds, api.panTiltDegrees and api.logRelSize in the below caused a feedback
    // issue that broke updates of the other sliders:
    vector<uint>    apiDeps =
        fgUints(api.meshesN,api.vertssN,api.normssN,api.texssN,api.light,api.xform,api.renderOptions);
    api.updateFlagIdx = g_gg.addUpdateFlag(apiDeps);
    api.updateTexFlagIdx = g_gg.addUpdateFlag(api.texssN);

    g_gg.addLink(linkNorms,fgUints(meshesN,ret.morphedVertssN),fgUints(api.normssN));
    FgDgn<bool>         useTexture = g_gg.addInput((simple != 3),uid+"UseTexture"),
                        shiny = g_gg.addInput(false,uid+"Shiny"),
                        wireframe = g_gg.addInput(false,uid+"Wireframe"),
                        flatShaded = g_gg.addInput(false,uid+"FlatShaded"),
                        facets = g_gg.addInput(true,uid+"Facets"),
                        surfPoints = g_gg.addInput(false,uid+"SurfPoints"),
                        markedVerts = g_gg.addInput(false,uid+"MarkedVerts"),
                        allVerts = g_gg.addInput(false,uid+"AllVerts"),
                        twoSidedN = g_gg.addInput(true,uid+"TwoSided");
    FgDgn<FgVect3F>     bgColorN(g_gg,"bgColor");
    g_gg.addLink(linkRenderOpts,
        fgSvec<uint>(facets,useTexture,shiny,wireframe,flatShaded,surfPoints,markedVerts,allVerts,twoSidedN,bgColorN),
        fgUints(api.renderOptions));
    FgDgn<double>       lensFovDeg = g_gg.addInput(defaultCps.fovMaxDeg,uid+"LensFovDeg");
    FgVectD2            logScaleRange(log(1.0/5.0),log(5.0));
    FgGuiPtr            rcColor = fgGuiCheckboxTr("Color maps",useTexture),
                        rcShiny = fgGuiCheckboxTr("Shiny",shiny),
                        rcWire = fgGuiCheckboxTr("Wireframe",wireframe),
                        rcFlat = fgGuiCheckboxTr("Flat shaded",flatShaded),
                        rcFacet = fgGuiCheckboxTr("Facets",facets),
                        rcSurf = fgGuiCheckboxTr("Surface points",surfPoints),
                        rcMark = fgGuiCheckboxTr("Marked vertices",markedVerts),
                        rcAll = fgGuiCheckboxTr("All vertices",allVerts),
                        rcTwoSided = fgGuiCheckboxTr("Two sided",twoSidedN);
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
    g_gg.addLink(linkXform,
        fgUints(viewBoundsN,api.panTiltMode,api.panTiltDegrees,api.pose,api.trans,
                api.logRelSize,lensFovDeg,api.viewportDims),
        fgUints(api.xform));
    FgDgn<double>       redN = g_gg.addInput(0.3,"bgColR"),
                        greenN = g_gg.addInput(0.3,"bgColG"),
                        blueN = g_gg.addInput(0.3,"bgColB");
    g_gg.addLink(linkColSel,fgUints(redN,greenN,blueN),fgUints(bgColorN));
    ret.viewport = fgsp(api);

    FgGuiPtr        viewCtlText =
        fgGuiTextRich(
            "                   Rotate: left-click-drag on head\n"
            "                   Scale: right-click-drag up/down on head\n"
            "                   Move: shift-left-click-drag on head");
    FgGuiPtr        viewCtlRender = fgGuiGroupboxTr("Render",renderCtls);
    FgGuiPtr        viewCtlRotation =
        fgGuiGroupboxTr("Object rotation",
            fgGuiRadio(api.panTiltMode,fgSvec<FgString>("Pan / tilt","Unconstrained")));
    FgGuiPtr        viewCtlColor =
        fgGuiGroupboxTr("Background Color",
            fgGuiSplit(false,
                fgGuiSlider(redN,fgTr("Red"),FgVectD2(0,1),0.5),
                fgGuiSlider(greenN,fgTr("Green"),FgVectD2(0,1),0.5),
                fgGuiSlider(blueN,fgTr("Blue"),FgVectD2(0,1),0.5)));
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
                    FgGuiApiTickLabel(60.0,fgTr("Wide")))
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
                    FgGuiApiTickLabel(logScaleRange[1],"5.0"))));
    FgGuiPtr        viewCtlReset = fgGuiButtonTr("Reset Camera",boost::bind(
        resetView,lensFovDeg,api.panTiltDegrees,api.pose,api.trans,api.logRelSize));
    if (simple == 2)
        ret.viewCtls = fgGuiSplit(false,viewCtlText,viewCtlRender,viewCtlColor,viewCtlScale,viewCtlReset);
    else
        ret.viewCtls = fgGuiSplit(false,fgSvec(
            viewCtlText,viewCtlRender,viewCtlRotation,viewCtlColor,viewCtlFov,viewCtlScale,viewCtlReset));
    ret.editCtls =
        fgGuiSplit(false,fgSvec(
            fgGuiTextRich("Mark Surface Point: ctrl-shift-left-click on surface\n"),
            fgGuiSplit(true,
                fgGuiTextRich("Name:"),
                fgGuiTextEdit(api.pointLabel)),
            fgGuiTextRich("Mark Vertex: ctrl-shift-right-click on surface near vertex"),
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
            g_gg.addLink(linkSelect,fgUints(meshesSrcN,selN),fgUints(selsN));
            meshSelect = fgGuiTab("Select",true,fgGuiRadio(selN,meshNames));
        }
        else {
            meshSelect = fgGuiTab("Select",true,fgGuiTextRich("TODO: part selections"));
        }
    }
    FgMat32F                 viewBounds = fgBounds(meshes[0].verts);
    for (size_t ii=1; ii<meshes.size(); ++ii)
        viewBounds = fgBounds(viewBounds,fgBounds(meshes[ii].verts));
    FgDgn<FgMat32D>          viewBoundsN = g_gg.addNode(fgF2D(viewBounds),"viewBounds");
    FgDgn<FgString>             meshStatsN(g_gg,"meshStats");
    g_gg.addLink(linkMeshStats2,fgUints(meshesN),fgUints(meshStatsN));
    FgGui3dCtls         ga3 = fgGui3dCtls(meshesN,allVertsN,texssN,viewBoundsN);
    vector<FgGuiTab>    tabDefs = fgSvec(
        fgGuiTab("View",true,ga3.viewCtls),
        fgGuiTab("Morphs",true,ga3.morphCtls)//,
        //fgGuiTab("Info",true,fgGuiSplitScroll(fgSvec(fgGuiTextRich(meshStatsN))))
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
    FgDgn<FgMat32D>          viewBoundsN = g_gg.addNode(fgF2D(fgBounds(mesh.verts)),"viewBounds");
    FgDgn<FgString>             meshStatsN(g_gg,"meshStats");
    g_gg.addLink(linkMeshStats2,fgUints(meshesN),fgUints(meshStatsN));
    FgGui3dCtls         ga3 = fgGui3dCtls(meshesN,allVertsN,texssN,viewBoundsN);
    vector<FgGuiTab>    tabDefs = fgSvec(
        fgGuiTab("View",true,ga3.viewCtls),
        fgGuiTab("Morphs",true,ga3.morphCtls),
        fgGuiTab("Info",true,fgGuiTextRich(meshStatsN)));
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
