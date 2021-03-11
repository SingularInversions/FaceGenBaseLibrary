//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dDisplay.hpp"
#include "FgGuiApi.hpp"
#include "FgFileSystem.hpp"
#include "FgTestUtils.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dTopology.hpp"
#include "FgCommand.hpp"
#include "FgStdSet.hpp"

using namespace std;
using namespace std::placeholders;

namespace Fg {

namespace {

Arr<GuiPtr,3>
makeSliderBank3(
    IPT<Vec3F>              valN,
    Arr<String8,3> const &  labels,
    VecD2                   range,
    double                  tickSpacing)
{
    Arr<GuiPtr,3>  ret;
    for (size_t ss=0; ss<3; ++ss) {
        GuiSlider        s;
        s.updateFlag = makeUpdateFlag(valN);
        s.getInput = [valN,ss](){return valN.cref()[ss]; };
        s.setOutput = [valN,ss](double v){valN.ref()[ss] = v; };
        s.label = labels[ss];
        s.range = range;
        s.tickSpacing = tickSpacing;
        ret[ss] = guiMakePtr(s);
    }
    return ret;
}

// Output value has range [0,1]:
GuiVal<Vec3F>
makeColorSliders(String8 const & store,double init,double tickSpacing)
{
    IPT<Vec3F>          valN = makeSavedIPT(Vec3F(init),store);
    array<GuiPtr,3>     sliders = makeSliderBank3(valN,array<String8,3>(),VecD2(0,1),tickSpacing);
    array<String8,3>    labels {{"Red","Green","Blue"}};
    GuiPtrs             wins;
    for (uint ii=0; ii<3; ++ii) {
        wins.push_back(guiSplitH({
            guiText(labels[ii],45),       // Fixed width larger than all colors to align sliders
            sliders[ii]}));
    }
    return GuiVal<Vec3F>(valN,guiSplit(false,wins));
}

GuiVal<Vec3F>
makeDirectionSliders(array<IPT<double>,3> inputNs,String8 const & store)
{
    Vec3D               defaultVal(0,0,1);
    VecD2               bounds(-100000,100000);
    array<string,3>     labels {{"Right:","Up:","Backward:"}};
    GuiPtrs             ctls;
    for (uint xx=0; xx<3; ++xx) {
        inputNs[xx].initSaved(defaultVal[xx],store+toStr(xx));
        ctls.push_back(guiText(labels[xx]));
        ctls.push_back(guiTextEditFloat(inputNs[xx],bounds,6));
    }
    GuiVal<Vec3F> ret;
    ret.valN = link3<Vec3F,double,double,double>(inputNs[0],inputNs[1],inputNs[2],
        [](double v0,double v1,double v2){return Vec3F(v0,v1,v2); });
    ret.win = guiSplitV({
        guiText("Direction vector to light (from viewer's point of view):"),
        guiSplit(true,ctls)});
    return ret;
}

struct  LightCtrls
{
    GuiPtr                clrSliders;
    GuiPtr                dirSliders;
    OPT<Light>            light;
};

LightCtrls
makeLightCtrls(array<IPT<double>,3> inputNs,double defaultBrightness,String8 const & store)
{
    LightCtrls              ret;
    GuiVal<Vec3F>     color = makeColorSliders(store+"Color",defaultBrightness,0.1);
    GuiVal<Vec3F>     dir = makeDirectionSliders(inputNs,store+"Direction");
    ret.light = link2<Light,Vec3F,Vec3F>(color.valN,dir.valN,[](Vec3F c,Vec3F d){return Light(c,d); });
    ret.clrSliders = color.win;
    ret.dirSliders = dir.win;
    return ret;
}

void
lightDirectionDrag(
    bool                    shiftKey,
    Vec2I                   pixels,
    array<IPT<double>,3>    light1,
    array<IPT<double>,3>    light2)
{
    auto const &            lgt = shiftKey ? light2 : light1;
    Vec3D                   dir;
    for (uint ii=0; ii<3; ++ii)
        dir[ii] = lgt[ii].val();
    dir = normalize(dir);
    Vec3D                   axis(pixels[1],pixels[0],0);
    double                  axisLen = axis.len();
    if (axisLen > 0.0) {
        Mat33D              rot = matRotateAxis(axisLen/500.0,axis/axisLen);
        dir = rot * dir;
        for (uint ii=0; ii<3; ++ii)
            lgt[ii].set(dir[ii]);
    }
}

}

GuiPtr
makeLightingCtrls(
    RPT<Lighting>           lightingR,
    BothButtonsDragAction & bothButtonsDragAction,
    String8 const &         store)
{
    GuiVal<Vec3F>     ambient = makeColorSliders(store+"Ambient",0.4,0.1);
    String8                sp = store + "Diffuse";
    array<IPT<double>,3>    dir1Ns,
                            dir2Ns;
    LightCtrls              light1 = makeLightCtrls(dir1Ns,0.6,sp+"1"),
                            light2 = makeLightCtrls(dir2Ns,0.0,sp+"2");
    NPT<Lighting>         lightingN =
        link3<Lighting,Vec3F,Light,Light>(ambient.valN,light1.light,light2.light,
            [](Vec3F a,Light l1,Light l2)
            {return Lighting(a,svec(l1,l2)); }
        );
    connect(lightingR,lightingN);
    bothButtonsDragAction = std::bind(lightDirectionDrag,_1,_2,dir1Ns,dir2Ns);
    auto                    resetLighting = [lightingN,dir1Ns,dir2Ns]()
        {
            DfgNPtrs       nodes;
            nodes.push_back(lightingN.ptr);
            for (const IPT<double> & i : dir1Ns)
                nodes.push_back(i.ptr);
            for (const IPT<double> & i : dir2Ns)
                nodes.push_back(i.ptr);
            setInputsToDefault(nodes);
        };
    return guiSplitScroll(svec(
        guiGroupbox("Ambient",ambient.win),
        guiGroupbox("Light 1",guiSplitV({light1.clrSliders,light1.dirSliders})),
        guiGroupbox("Light 2",guiSplitV({light2.clrSliders,light2.dirSliders})),
        guiText("\nInteractively adjust light direction:\n"
            "Light 1: hold down left and right mouse buttons while dragging\n"
            "Light 2: As above but also hold down shift key"),
        guiButton("Reset Lighting",resetLighting)
    ));
}

namespace {

void
bgImageLoad2(BackgroundImage bgi)
{
    Opt<String8>     fname = guiDialogFileLoad(imgFileExtensionsDescription(),imgFileExtensions());
    if (!fname.valid())
        return;
    if (fname.val().empty())
        return;
    ImgC4UC &       img = bgi.imgN.ref();
    try {
        loadImage_(fname.val(),img);
    }
    catch (const FgException & e) {
        guiDialogMessage("Unable to load background image",e.no_tr_message());
        return;
    }
    bgi.origDimsN.set(img.dims());
    bgi.offset.set(Vec2F(0.0));
    bgi.lnScale.set(0.0);
}

GuiPtr
backgroundCtrls(BackgroundImage bgImg,String8 const & store)
{
    bgImg.imgN.initSaved(ImgC4UC(),store+"Image",true);
    bgImg.origDimsN.initSaved(Vec2UI(0),store+"OrigDims");
    bgImg.offset.initSaved(Vec2F(0),store+"Offset");
    bgImg.lnScale.initSaved(0.0,store+"LnScale");
    bgImg.foregroundTransparency.initSaved(0.0,store+"ForegroundTransparency");
    GuiPtr      bgImageSlider = guiSlider(bgImg.foregroundTransparency,"Foreground transparency",VecD2(0,1),0.0),
                loadButton = guiButton("Load Image",bind(bgImageLoad2,bgImg)),
                clearButton = guiButton("Clear Image",[bgImg](){bgImg.imgN.ref().clear();}),
                bgImageButtons = guiSplitH({loadButton,clearButton}),
                text = guiText("Move: Ctrl-Shift-Left-Drag\nScale: Ctrl-Shift-Right-Drag");
    return guiGroupbox("Background Image",guiSplit(false,{bgImageButtons,bgImageSlider,text}));
}

struct OptInit
{
    bool        defVal;
    string      store;
    string      desc;
};

}

GuiPtr
makeRendCtrls(
    RPT<RendOptions>    rendOptionsR,
    BackgroundImage     bgImg,
    String8 const &     store,
    bool                structureOptions,
    bool                twoSidedOption,
    bool                pointOptions)
{
    vector<OptInit>         opts = {
        {false,"Shiny","Shiny"},                            // 0
        {false,"Wireframe","Wireframe"},                    // 1
        {false,"FlatShaded","Flat Shaded"},                 // 2
        {true,"Facets","Facets"},                           // 3
        {pointOptions,"SurfacePoints","Surface Points"},    // 4
        {pointOptions,"MarkedVerts","Marked Vertices"},     // 5
        {false,"AllVertices","All Vertices"},               // 6
        {true,"TwoSided","Two Sided"},                      // 7
        {false,"ShowAxes","Show origin and axes (red:X green:Y blue:Z)"} // 8
    };
    vector<IPT<bool> >      optNs;
    GuiPtrs                 optWs;
    for (const OptInit & opt : opts) {
        optNs.push_back(makeSavedIPT(opt.defVal,store+opt.store));
        optWs.push_back(guiCheckbox(opt.desc,optNs.back()));
    }
    NPT<Bools>              optsN = linkCollate<bool>(optNs);
    String8s                albedoModeLabels = cAlbedoModeLabels();
    IPT<size_t>             optRcN = makeSavedIPTEub<size_t>(0,store+"albedoMode",albedoModeLabels.size());
    GuiVal<Vec3F>           bgColor = makeColorSliders(store+"BgColor",0.0,0.1);
    NPT<RendOptions>        rendOptionsN = link3<RendOptions,size_t,Bools,Vec3F>(optRcN,optsN,bgColor.valN,
        [](size_t const & albedoMode,Bools const & opts,Vec3F const & bgColor)    // Elements [0,1]
    {
        RendOptions       r;
        r.albedoMode = AlbedoMode(albedoMode);
        r.shiny = opts[0];
        r.wireframe = opts[1];
        r.flatShaded = opts[2];
        r.facets = opts[3];
        r.surfPoints = opts[4];
        r.markedVerts = opts[5];
        r.allVerts = opts[6];
        r.twoSided = opts[7];
        r.showAxes = opts[8];
        r.backgroundColor = bgColor;
        return r;
    });
    connect(rendOptionsR,rendOptionsN);
    size_t                  numColorOptions = 2;
    if (pointOptions)
        numColorOptions = 4;
    else if (structureOptions)
        numColorOptions = 3;
    GuiPtrs                 enabledOptWs {
        optWs[0]
    };
    if (structureOptions) {
        enabledOptWs.push_back(optWs[1]);
        enabledOptWs.push_back(optWs[2]);
        enabledOptWs.push_back(optWs[3]);
        enabledOptWs.push_back(optWs[6]);
    }
    if (twoSidedOption)
        enabledOptWs.push_back(optWs[7]);
    if (pointOptions) {
        enabledOptWs.push_back(optWs[4]);
        enabledOptWs.push_back(optWs[5]);
    }
    size_t                  chunkSz = cMax(size_t(2),(enabledOptWs.size() + 1) / 2);
    GuiPtrs                 colWs {guiSplit(false,cHead(enabledOptWs,chunkSz))};
    if (enabledOptWs.size() > chunkSz)
        colWs.push_back(guiSplit(false,cRest(enabledOptWs,chunkSz)));
    GuiPtr                  checkboxesW = guiSplit(true,colWs),
                            checkboxesSpacedW = guiSplitV({guiSpacer(0,15),checkboxesW}),
                            colsW = guiSplit(true,{
                                guiGroupbox("Color",guiRadio(cHead(albedoModeLabels,numColorOptions),optRcN)),
                                guiSpacer(10,0),
                                checkboxesSpacedW});
    GuiPtr                  renderCtls;
    if (pointOptions)
        renderCtls = guiSplitV({colsW,optWs[8]});
    else
        renderCtls = colsW;
    GuiPtr          viewCtlRender = guiGroupboxTr("Render",renderCtls),
                    viewCtlColor = guiGroupboxTr("Background Color",bgColor.win),
                    bgImageCtrls = backgroundCtrls(bgImg,store+"BgImage");
    return guiSplit(false,{viewCtlRender,viewCtlColor,bgImageCtrls});
}

namespace {

void
assignTri2(      // Re-assign a tri (or quad) to a different surface if intersected
    NPT<RendMeshes>         rendMeshesN,
    NPT<size_t>             surfIdx,
    Vec2UI               winSize,
    Vec2I                pos,
    Mat44F                toOics)
{
    RendMeshes const &          rms = rendMeshesN.cref();
    Opt<MeshesIntersect>    vpt = intersectMeshes(winSize,pos,toOics,rms);
    if (vpt.valid()) {
        MeshesIntersect       isct = vpt.val();
        size_t                  dstSurfIdx = surfIdx.val();
        RendMesh const &       rm = rms[isct.meshIdx];
        Mesh *              meshPtr = rm.origMeshN.valPtr();
        if (meshPtr) {
            if ((isct.surfIdx != dstSurfIdx) && (dstSurfIdx < meshPtr->surfaces.size())) {
                Surf &           srcSurf = meshPtr->surfaces[isct.surfIdx];
                Surf &           dstSurf = meshPtr->surfaces[dstSurfIdx];
                if (srcSurf.isTri(isct.surfPnt.triEquivIdx)) {
                    // Copy surface points on selected tri to destination surface:
                    for (SurfPoint sp : srcSurf.surfPoints) {
                        if (sp.triEquivIdx == isct.surfPnt.triEquivIdx) {
                            sp.triEquivIdx = uint(dstSurf.tris.size());
                            dstSurf.surfPoints.push_back(sp);
                        }
                    }
                    // Move selected tri to destination surface:
                    size_t          idx = isct.surfPnt.triEquivIdx;
                    if (srcSurf.tris.hasUvs() && dstSurf.tris.hasUvs())
                        dstSurf.tris.uvInds.push_back(srcSurf.tris.uvInds[idx]);
                    dstSurf.tris.posInds.push_back(srcSurf.tris.posInds[idx]);
                    srcSurf.removeTri(isct.surfPnt.triEquivIdx);
                }
                else {                      // It's a quad
                    // Copy surface points on selected quad to destination surface:
                    size_t                  quadIdx = (isct.surfPnt.triEquivIdx - srcSurf.tris.size())/2;
                    for (SurfPoint sp : srcSurf.surfPoints) {
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
                    if (srcSurf.quads.hasUvs() && dstSurf.quads.hasUvs())
                        dstSurf.quads.uvInds.push_back(srcSurf.quads.uvInds[quadIdx]);
                    dstSurf.quads.posInds.push_back(srcSurf.quads.posInds[quadIdx]);
                    srcSurf.removeQuad(quadIdx);
                }
            }
        }
    }
}

// Currently only works on tri facets:
void
assignPaint2(
    NPT<RendMeshes>     rendMeshesN,
    NPT<size_t>         surfIdx,
    Vec2UI              winSize,
    Vec2I               pos,
    Mat44F              toOics)
{
    RendMeshes const &      rms = rendMeshesN.cref();
    Opt<MeshesIntersect>    vpt = intersectMeshes(winSize,pos,toOics,rms);
    if (vpt.valid()) {
        MeshesIntersect         isct = vpt.val();
        size_t                  dstSurfIdx = surfIdx.val();
        RendMesh const &        rm = rms[isct.meshIdx];
        Mesh *                  meshPtr = rm.origMeshN.valPtr();
        if (meshPtr) {
            if ((isct.surfIdx != dstSurfIdx) && (dstSurfIdx < meshPtr->surfaces.size())) {
                Vec3UI          tri = meshPtr->surfaces[isct.surfIdx].getTriEquivPosInds(isct.surfPnt.triEquivIdx);
                Surfs           surfs = splitByContiguous(meshPtr->surfaces[isct.surfIdx]);
                for (size_t ss=0; ss<surfs.size(); ++ss) {
                    if (contains(surfs[ss].tris.posInds,tri)) {
                        meshPtr->surfaces[dstSurfIdx].merge(surfs[ss]);
                        surfs.erase(surfs.begin()+ss);
                        meshPtr->surfaces[isct.surfIdx] = mergeSurfaces(surfs);
                        return;
                    }
                }
            }
        }
    }
}

void
saveMesh2(string format,NPT<RendMeshes> rendMeshesN)
{
    RendMeshes const &      rms = rendMeshesN.cref();
    if (rms.empty())
        return;
    Opt<String8>            fname = guiDialogFileSave("FaceGen",format);
    if (fname.valid()) {
        Meshes              meshes;
        for (RendMesh const & rm : rms)
            meshes.push_back(rm.origMeshN.cref());
        saveMesh(meshes,fname.val());
    }
}

GuiPtr
makeEditFacetsPane(NPT<RendMeshes> rendMeshesN,IPT<size_t> surfChoiceN)
{
    RendMeshes const &      rms = rendMeshesN.cref();
    FGASSERT(!rms.empty());
    RendMesh const &        rm = rms[0];
    Mesh *                  meshPtr = rm.origMeshN.valPtr();
    FGASSERT(meshPtr);
    String8s                surfNames;
    for (size_t ss=0; ss<meshPtr->surfaces.size(); ++ss) {
        Surf &       surf = meshPtr->surfaces[ss];
        if (surf.name.empty())      // Generally not the case since default surf names assigned on load
            surf.name = toStr(ss);
        surfNames.push_back(surf.name);
    }
    GuiPtr                  facets;
    if (surfNames.empty())
        facets = guiText("There are no surfaces to edit");
    else {
        GuiPtr              surfChoiceW = guiRadio(surfNames,surfChoiceN);
        facets = guiSplitV({
            guiText("Assign tri to surface selection: shift-right-drag\n"
                "Assign connected tris (not quads) to surface: shift-middle-click"),
            guiGroupbox("Surface Selection",surfChoiceW)});
    }
    return facets;
}

struct  MeshSurfsName
{
    String8        meshName;
    String8s       surfNames;
};
typedef vector<MeshSurfsName>   MeshSurfsNames;

// Modifies 'api' to add it's hooks:
GuiPtr
makeEditPane(OPT<MeshSurfsNames> const & meshSurfsNamesN,Gui3d & api)
{
    String8s            vsmrLabels {"Single","Edge Seam","Fold Seam","Fill"};
    IPT<size_t>         vertSelModeN(0);
    GuiPtr              vertSelModeW = guiRadio(vsmrLabels,vertSelModeN);
    IPT<String8>        pointLabelN = makeIPT(String8());
    NPT<RendMeshes>     rendMeshesN = api.rendMeshesN;
    api.clickActions.at(0,1,1) = bind(markSurfacePoint,rendMeshesN,pointLabelN,_1,_2,_3);
    api.clickActions.at(2,1,1) = bind(markMeshVertex,rendMeshesN,vertSelModeN,_1,_2,_3);
    GuiPtr              pointsW = guiSplit(false,{
        guiText(
            "Mark Surface Point: ctrl-shift-left-click on surface.\n"
            "A point with an identical non-null name will be overwritten"
        ),
        guiSplitH({guiText("Name:"),guiTextEdit(pointLabelN)}),
        guiButtonTr("Clear all surface points",
            [rendMeshesN]()
            {
                RendMeshes const &  rms = rendMeshesN.cref();
                for (RendMesh const & rm : rms) {
                    Mesh *          meshPtr = rm.origMeshN.valPtr();
                    if (meshPtr) {
                        for (Surf & surf : meshPtr->surfaces)
                            surf.surfPoints.clear();
                    }
                }
            })
    });
    GuiPtr            vertsW = guiSplit(false,{
        guiText("Mark Vertex: ctrl-shift-right-click on surface near vertex"),
        guiGroupbox("Vertex Selection Mode",vertSelModeW),
        guiButtonTr("Mark all seam vertices",
            [rendMeshesN]()
            {
                RendMeshes const &      rms = rendMeshesN.cref();
                for (RendMesh const & rm : rms) {
                    Mesh *          meshPtr = rm.origMeshN.valPtr();
                    if (meshPtr) {
                        MeshTopology        topo(meshPtr->verts.size(),meshPtr->getTriEquivs().posInds);
                        vector<set<uint> >  seams = topo.seams();
                        for (size_t ss=0; ss<seams.size(); ++ss) {
                            const set<uint> &   seam = seams[ss];
                            for (set<uint>::const_iterator it=seam.begin(); it != seam.end(); ++it)
                                meshPtr->markedVerts.push_back(MarkedVert(*it));
                        }
                    }
                }
            }),
        guiButtonTr("Clear all marked vertices",
            [rendMeshesN]()
            {
                RendMeshes const & rms = rendMeshesN.cref();
                for (RendMesh const & rm : rms) {
                    Mesh *          meshPtr = rm.origMeshN.valPtr();
                    if (meshPtr)
                        meshPtr->markedVerts.clear();
                }
            })
    });
    IPT<size_t>         surfChoiceN(0);
    api.shiftRightDragAction = bind(assignTri2,rendMeshesN,surfChoiceN,_1,_2,_3);
    api.clickActions.at(1,1,0) = bind(assignPaint2,rendMeshesN,surfChoiceN,_1,_2,_3);
    GuiPtr              facetsW = guiDynamic(bind(makeEditFacetsPane,rendMeshesN,surfChoiceN),makeUpdateFlag(meshSurfsNamesN));
    GuiTabDefs          tabs {
        {"Points",true,pointsW},
        {"Verts",true,vertsW},
        {"Facets",true,facetsW},
    };
    GuiPtr              saveButtonsW = guiSplitH({
        guiButtonTr("TRI",bind(saveMesh2,string("tri"),rendMeshesN)),
        guiButtonTr("FGMESH",bind(saveMesh2,string("fgmesh"),rendMeshesN))});
    return guiSplit(false,{guiText("Save pre-morphed to file:"),saveButtonsW,guiTabs(tabs)});
}

// Unique names will be left unchanged.
// Empty names will be assigned the baseName plus a number.
// Non-unique names will have a number added.
String8s
makeUniqueNames(String8s const & names,String8 const & baseName)
{
    String8s           ret;
    ret.reserve(names.size());
    for (size_t nn=0; nn<names.size(); ++nn) {
        String8            name = names[nn];
        if (name.empty())
            name = baseName + "_" + toStr(nn);
        if (contains(ret,name)) {
            size_t      idx = 1;
            while (contains(ret,name+"_"+toStr(idx)))
                ++idx;
            name += "_"+toStr(idx);
        }
        ret.push_back(name);
    }
    return ret;
}

}

GuiPtr
makeCameraCtrls(
    Gui3d &                 api,
    NPT<Mat32D>             viewBoundsN,
    String8 const &         store,
    uint                    simple,
    bool                    textEditBoxes)
{
    String8s                panTiltOptions = {"Pan / Tilt","Unconstrained"};
    size_t                  panTiltDefault = (simple == 0) ? 1 : 0;
    IPT<size_t>             panTiltRadioN = makeSavedIPTEub(panTiltDefault,store+"PanTiltMode",panTiltOptions.size());
    GuiPtr                  panTiltRadioW = guiRadio(panTiltOptions,panTiltRadioN);
    api.panTiltMode = panTiltRadioN;
    if (simple == 2)
        api.panTiltLimits = true;
    api.panDegrees.initSaved(0.0,store+"Pan");
    api.tiltDegrees.initSaved(0.0,store+"Tilt");
    api.pose.initSaved(QuaternionD(),store+"Pose");
    api.trans.initSaved(Vec2D(0),store+"Trans");
    api.logRelSize.initSaved(-0.3,store+"LogRelSize");
    // Give non-zero viewport initial val to avoid NaNs in initial dependency calc for rendering:
    api.viewportDims.init(Vec2UI(1));
    CameraParams            defaultCps;     // Use default for lensFovDeg:
    IPT<double>             lensFovDeg = makeSavedIPT(defaultCps.fovMaxDeg,store+"LensFovDeg");
    VecD2                   logScaleRange(log(1.0/5.0),log(5.0));
    OPT<CameraParams>       camParamsN =
        link5<CameraParams,Mat32D,QuaternionD,Vec2D,double,double>(
            viewBoundsN,api.pose,api.trans,api.logRelSize,lensFovDeg,
            [](const Mat32D & vb,QuaternionD const & p,const Vec2D & t,double const & lrs,double const & lfd)
            {
                CameraParams    r;
                r.modelBounds = vb;
                r.pose = p;
                r.relTrans = t;
                r.logRelScale = lrs;
                r.fovMaxDeg = lfd;
                return r;
            });
    OPT<Vec2D>              panTiltN = link2<Vec2D,double,double>(api.panDegrees,api.tiltDegrees,
        [](double const & p,double const & t){return Vec2D(p,t); });
    api.xform = link4<Camera,CameraParams,size_t,Vec2D,Vec2UI>(
        camParamsN,api.panTiltMode,panTiltN,api.viewportDims,
        [](const CameraParams & cp,const size_t & m,const Vec2D & pt,const Vec2UI & v)
        {
            CameraParams        cps = cp;
            if (m == 0) {
                QuaternionD     pan(degToRad(pt[0]),1),
                                tilt(degToRad(pt[1]),0);
                cps.pose = pan*tilt;
            }
            return cps.camera(v);
        });
    GuiPtr                  viewCtlText =
        guiText(
            "  Rotate: left-click-drag (or touch-drag)\n"
            "  Scale: right-click-drag up/down (or pinch-to-zoom)\n"
            "  Move: shift-left-click-drag (or middle-click-drag)");
    VecD2        panTiltClip(-360.0,360.0);      // Values are wrapped by viewport impl
    GuiPtr        viewCtlPanTiltText =
        guiSplitV({
            guiSplitH({
                guiTextEditFixed(api.panDegrees,panTiltClip),
                guiTextEditFixed(api.tiltDegrees,panTiltClip)}),
            guiSpacer(0,30)});
    if (textEditBoxes)
        panTiltRadioW = guiSplitH({panTiltRadioW,viewCtlPanTiltText});
    GuiPtr                  viewCtlRotation = guiGroupboxTr("Object rotation",panTiltRadioW);
    GuiPtr                  viewCtlFov =
        guiGroupboxTr(
            "Lens field of view (degrees)",
            guiSlider(
                lensFovDeg,
                String8(),
                VecD2(0.0,60.0),
                15.0,
                guiTickLabels(VecD2(0.0,60.0),15.0,0.0),
                svec(
                    GuiTickLabel(0.0,fgTr("Orthographic")),
                    GuiTickLabel(30.0,fgTr("Normal")),
                    GuiTickLabel(60.0,fgTr("Wide"))),
                30,     // Lots of room required for "Orthographic" at end of slider
                textEditBoxes
            )
        );
    IPT<double>             lnRelSizeN = api.logRelSize;
    auto                    getVal = [=](){return std::exp(lnRelSizeN.val()); };
    auto                    setVal = [=](double val){lnRelSizeN.set(std::log(val)); };
    GuiPtr                  viewCtlScaleTe = guiTextEditFloat(
        makeUpdateFlag(lnRelSizeN),getVal,setVal,VecD2{exp(-20.0),exp(20.0)},6);
    GuiPtr                  viewCtlScale = guiSlider(
                        api.logRelSize,
                        String8(),
                        logScaleRange,
                        (logScaleRange[1]-logScaleRange[0])/4.0,
                        svec(
                            GuiTickLabel(logScaleRange[0],"0.2"),
                            GuiTickLabel(0.0,"1.0"),
                            GuiTickLabel(logScaleRange[1],"5.0")),
                        vector<GuiTickLabel>(),
                        30      // Match width of slider above
                    );
    if (textEditBoxes)
        viewCtlScale = guiSplitH({viewCtlScale,viewCtlScaleTe});
    viewCtlScale = guiGroupboxTr("Object relative scale",viewCtlScale);
    DfgNPtrs                resetDeps;
    resetDeps.push_back(api.panDegrees.ptr);
    resetDeps.push_back(api.tiltDegrees.ptr);
    resetDeps.push_back(api.pose.ptr);
    resetDeps.push_back(api.trans.ptr);
    resetDeps.push_back(api.logRelSize.ptr);
    resetDeps.push_back(lensFovDeg.ptr);
    GuiPtr              viewCtlReset = guiButtonTr("Reset Camera",[resetDeps](){setInputsToDefault(resetDeps);});
    if (simple == 2)
        return guiSplit(false,{viewCtlText,viewCtlFov,viewCtlScale,viewCtlReset});
    else
        return guiSplit(false,{viewCtlText,viewCtlRotation,viewCtlFov,viewCtlScale,viewCtlReset});
}

OPT<Vec3Fs>
linkAllVerts(NPT<Mesh> meshN)
{return link1<Mesh,Vec3Fs>(meshN,[](Mesh const & mesh){return mesh.allVerts();}); }

OPT<MeshNormals>
linkNormals(const NPT<Mesh> & meshN,const NPT<Vec3Fs> & posedVertsN)
{
    return link2<MeshNormals,Mesh,Vec3Fs>(meshN,posedVertsN,
        [](Mesh const & mesh,Vec3Fs const & verts)
        {return cNormals(mesh.surfaces,verts); });
}

OPT<Mesh>
linkLoadMesh(NPT<String8> pathBaseN)
{
    return link1<String8,Mesh>(pathBaseN,
        [](String8 const & pathBase)
        {
            if (pathBase.empty())
                return Mesh();                      // Deselected
            else
                return loadTri(pathBase+".tri");
            //else if (pathExists(pathBase+".fgMesh"))
            //    loadFgmesh(pathBase+".fgmesh",mesh);
        });
}

OPT<ImgC4UC>
linkLoadImage(NPT<String8> filenameN)
{
    return link1_<String8,ImgC4UC>(filenameN,
        [](String8 const & fn,ImgC4UC & img)
        {
            img.clear();        // Ensure cleared in case load fails.
            if (!fn.empty())
                loadImage_(fn,img);
        });
}

GuiPosedMeshes::GuiPosedMeshes() :
    poseDefsN(linkN<Mesh,PoseDefs>(vector<NPT<Mesh> >(),[](Meshes const & m){return cPoseDefs(m);})),
    // Don't save pose state as it is confusing, even to experienced users for instance when only a small
    // morph is left from a previous session so it appears to be mesh/identity issue:
    poseValsN(makeIPT(PoseVals{}))
{}

void
GuiPosedMeshes::addMesh(
        NPT<Mesh>           meshN,
        NPT<Vec3Fs>         allVertsN,
        ImgNs               smoothNs,
        ImgNs               specularNs,
        ImgNs               modulationNs)
{
    if (specularNs.empty())
        specularNs.resize(smoothNs.size(),makeIPT(ImgC4UC{}));
    if (modulationNs.empty())
        modulationNs.resize(smoothNs.size(),makeIPT(ImgC4UC{}));
    // We can't check against mesh surfaces at this time since it may not be selected:
    FGASSERT(smoothNs.size() == modulationNs.size());
    FGASSERT(smoothNs.size() == specularNs.size());
    addLink(meshN.ptr,poseDefsN.ptr);
    RendMesh            rm;
    rm.origMeshN = meshN;
    size_t              S = smoothNs.size();
    for (size_t ss=0; ss<S; ++ss) {
        RendSurf            rs;
        rs.smoothMapN = smoothNs[ss];
        rs.albedoHasTransparencyN = link1<ImgC4UC,bool>(rs.smoothMapN,
            [](ImgC4UC const & img){return usesAlpha(img);});     // Returns false if image empty
        rs.modulationMapN = modulationNs[ss];
        rs.specularMapN = specularNs[ss];
        rm.rendSurfs.push_back(rs);
    }
    rm.posedVertsN = link3_<Vec3Fs,Mesh,Vec3Fs,PoseVals>(meshN,allVertsN,poseValsN,
        [=](Mesh const & mesh,Vec3Fs const & allVerts,PoseVals const & poseVals,Vec3Fs & verts)
        {
            verts = mesh.poseShape(allVerts,poseVals);
        }
    );
    rm.normalsN = linkNormals(meshN,rm.posedVertsN);
    rendMeshes.push_back(rm);
}

void
GuiPosedMeshes::addMesh(NPT<Mesh> meshN,NPT<Vec3Fs> vertsN,size_t numSurfs)
{
    ImgNs       albedoNs = ImgNs(numSurfs,makeIPT(ImgC4UC{}));
    addMesh(meshN,vertsN,albedoNs);
}

// Called by GuiSplitScroll when 'labelsN' changes:
GuiPtr
cPoseControlW(NPT<PoseDefs> poseDefsN,IPT<PoseVals> poseValsN,bool textEditBoxes)
{
    PoseDefs const &        poseDefs = poseDefsN.cref();
    PoseVals &              poseVals = poseValsN.ref();
    poseVals.clear();
    if (poseDefs.empty())
        return {guiText("\nThe currently selected models contain no expression morphs.")};
    for (PoseDef const & poseDef : poseDefs)
        poseVals[poseDef.name] = poseDef.neutral;
    GuiPtrs             buttonWs {
        guiButton("Set All Zero",[poseValsN]()
        {
            // Do not alter labels, just values:
            for (auto & pv : poseValsN.ref())
                pv.second = 0.0f;
        }),
        guiButton("Load File",[poseValsN]()
        {
            Opt<String8>                fname = guiDialogFileLoad("FaceGen XML expression file",{"xml"});
            if (fname.valid()) {
                Svec<pair<String8,float> >  data;       // Legacy format
                loadBsaXml(fname.val(),data);
                PoseVals &                  pvs = poseValsN.ref();
                // Do not alter the labels, only update values where labels exist:
                for (pair<String8,float> const & d : data) {
                    auto                    it = pvs.find(d.first);
                    if (it != pvs.end())
                        it->second = d.second;
                }
            }
        }),
        guiButton("Save File",[poseValsN]()
        {
            Opt<String8>                fname = guiDialogFileSave("FaceGen XML expression file",{"xml"});
            if (fname.valid()) {
                PoseVals const &            pvs = poseValsN.cref();
                Svec<pair<String8,float> >  data(pvs.begin(),pvs.end());    // Legacy format
                saveBsaXml(fname.val(),data);
            }
        }),
    };
    GuiPtr              buttonsW = guiSplitV({guiSplit(true,buttonWs),guiSpacer(0,7)});
    GuiPtrs             sliderFacsWs,
                        sliderCompWs;
    for (PoseDef const & pose : poseDefs) {
        String8 const &     name = pose.name;
        auto                getFn = [=]()
        {
            PoseVals const &    poseVals = poseValsN.cref();
            auto                it = poseValsN.cref().find(name);
            FGASSERT(it != poseVals.end());
            return double(it->second);
        };
        auto                setFn = [=](double v){poseValsN.ref()[name] = float(v); };
        GuiSlider           slider;
        slider.updateFlag = makeUpdateFlag(poseValsN);
        slider.getInput = getFn;
        slider.setOutput = setFn;
        slider.label = pose.name;
        slider.range[0] = pose.bounds[0];
        slider.range[1] = pose.bounds[1];
        slider.tickSpacing = 0.1;
        GuiPtr              sliderW;
        if (textEditBoxes) {
            auto            getVal = getFn;
            auto            setVal = setFn;
            GuiPtr          teW = guiTextEditFixed(makeUpdateFlag(poseValsN),getVal,setVal,VecD2{-1,2},2);
            sliderW = guiSplitH({guiMakePtr(slider),teW});
        }
        else
            sliderW = guiMakePtr(slider);
        if (beginsWith(pose.name.m_str,"AU"))
            sliderFacsWs.push_back(sliderW);
        else
            sliderCompWs.push_back(sliderW);
    }
    if (sliderFacsWs.empty())
        return guiSplitV({buttonsW,guiSplitScroll(sliderCompWs)});
    else if (sliderCompWs.empty())
        return guiSplitV({buttonsW,guiSplitScroll(sliderFacsWs)});
    // Two tabs of expressions:
    GuiTabDefs      sliderTs {
        guiTab("FACS",true,guiSplitScroll(sliderFacsWs)),
        guiTab("Composite",true,guiSplitScroll(sliderCompWs)),
    };
    return guiSplitV({buttonsW,guiTabs(sliderTs)});
}

GuiPtr
GuiPosedMeshes::makePoseCtrls(bool editBoxes) const
{
    auto        makeSliders =  bind(cPoseControlW,poseDefsN,poseValsN,editBoxes);
    return guiDynamic(makeSliders,makeUpdateFlag(poseDefsN));
}

OPT<Mat32D>
linkBounds(RendMeshes const & rms)
{
    vector<NPT<Mat32F> >      boundsNs;
    for (RendMesh const & rm : rms) {
        boundsNs.push_back(link1<Mesh,Mat32F>(rm.origMeshN,
            [](Mesh const & m){return cBounds(m.verts);}));
    }
    return linkN<Mat32F,Mat32D>(boundsNs,[](vector<Mat32F> const & bs)
    {
        Mat32F    bounds = cBoundsUnion(bs);
        if (bounds[1] < bounds[0])      // No meshes selected
            bounds = Mat32F(0,1,0,1,0,1);
        return Mat32D(bounds);
    });
}

OPT<String8>
linkMeshStats(RendMeshes const & rms)
{
    Svec<NPT<String8> >     statsNs;
    auto                    statsFn = [](Mesh const & m)
    {
        ostringstream       oss;
        if (!m.verts.empty())
            oss << m;
        return String8{oss.str()};
    };
    for (RendMesh const & rm : rms)
        statsNs.push_back(link1<Mesh,String8>(rm.origMeshN,statsFn));
    return linkN<String8,String8>(statsNs,[](String8s const & strs){return cat(strs,"");});
}

GuiPtr
makeViewCtrls(Gui3d & gui3d,NPT<Mat32D> viewBoundsN,String8 const & store)
{
    GuiPtr          cameraW = makeCameraCtrls(gui3d,viewBoundsN,store+"Camera",0,true),
                    renderOptsW = makeRendCtrls(gui3d.renderOptions,gui3d.bgImg,store+"RendOpts",true,true,true),
                    lightingW = makeLightingCtrls(gui3d.light,gui3d.bothButtonsDragAction,store+"Light");
    GuiTabDefs      viewTabs = {
                        guiTab("Camera",true,cameraW),
                        guiTab("Render",true,renderOptsW),
                        guiTab("Lighting",true,lightingW)
                    };
    return guiTabs(viewTabs);
}

GuiPtr
guiCaptureSaveImage(NPT<Vec2UI> viewportDims,Sptr<Gui3d::Capture> const & capture,String8 const & store)
{
    IPT<double>         widN = makeSavedIPT(1024.0,store+"Width"),
                        hgtN = makeSavedIPT(1024.0,store+"Height");
    IPT<size_t>         szSelN = makeSavedIPTEub<size_t>(0,store+"Manual",2);
    IPT<bool>           bgTransN = makeSavedIPT<bool>(false,store+"Transparent");
    NPT<String8>        vpDimsN = link1<Vec2UI,String8>(viewportDims,[](Vec2UI dims)
        {return "(" + toStr(dims[0]) + "," + toStr(dims[1]) + ")"; });
    GuiPtr              widW = guiTextEditFixed(widN,VecD2(256,4096),0),
                        hgtW = guiTextEditFixed(hgtN,VecD2(256,4096),0),
                        dimsLW = guiRadio({"Current render size","Custom"},szSelN),
                        dimsRUW = guiText(vpDimsN),
                        dimsRLW = guiSplit(true,{guiText("Width"),widW,guiText("Height"),hgtW}),
                        dimsRW = guiSplit(false,{guiSpacer(1,3),dimsRUW,guiSpacer(1,3),dimsRLW}),
                        dims3W = guiSplit(true,{dimsLW,dimsRW}),
                        dimsW = guiGroupbox("Pixel size",dims3W),
                        bgTransW = guiCheckbox("Transparent Background (PNG or TARGA only)",bgTransN);
    GuiVal<string>      capImgFormat = guiImageFormat("Image Format",false,store+"Format");
    NPT<string>         formatN = capImgFormat.valN;
    auto                save = [=]()
    {
        if (capture->func) {
            Vec2UI          dims = szSelN.val() ? Vec2UI(widN.val(),hgtN.val()) : viewportDims.val();
            Opt<String8>    fname = guiDialogFileSave("",formatN.cref());
            if (fname.valid())
                saveImage(fname.val(),capture->func(dims,bgTransN.val()));
        }
    };
    GuiPtr              saveImageW = guiSplit(false,{
        guiText("Save the current render image to a file"),
        dimsW,
        bgTransW,
        capImgFormat.win,
        guiButton("Save",save)
    });
    return saveImageW;
}

Mesh
viewMesh(Meshes const & meshes,bool compare)
{
    FGASSERT(meshes.size() > 0);
    String8                 store = getDirUserAppDataLocalFaceGen("SDK","viewMesh");
    Mat32F                  viewBounds = cBounds(meshes);
    if (!isFinite(viewBounds))
        fgThrow("viewMesh: Mesh vertices contain invalid floating point values");
    IPT<Mat32D>             viewBoundsN = makeIPT(fgF2D(cBounds(meshes)));
    GuiPosedMeshes          gpms;
    vector<IPT<Mesh> >      meshNs;
    vector<IPT<MeshSurfsName> >  meshSurfsNameNs;
    // Ensure we have valid, unique names:
    String8s                meshNames = makeUniqueNames(sliceMember(meshes,&Mesh::name),"Mesh");
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        Mesh const &        mesh = meshes[mm];
        mesh.checkValidity();       // TODO: need to report actual problem rather than throw internal error
        IPT<Mesh>           meshN = makeIPT(mesh);
        IPT<Vec3Fs>         allVertsN = makeIPT(mesh.allVerts());
        ImgNs               albedoNs,
                            specularNs;
        IPT<ImgC4UC>        emptyImageN = makeIPT(ImgC4UC());
        for (Surf const & surf : mesh.surfaces) {
            if (surf.material.albedoMap)
                albedoNs.push_back(makeIPT(*surf.material.albedoMap));
            else
                albedoNs.push_back(emptyImageN);
            if (surf.material.specularMap)
                specularNs.push_back(makeIPT(*surf.material.specularMap));
            else
                specularNs.push_back(emptyImageN);
        }
        gpms.addMesh(meshN,allVertsN,albedoNs,specularNs);
        meshNs.push_back(meshN);
        {
            MeshSurfsName       msn;
            msn.meshName = meshNames[mm];
            msn.surfNames = makeUniqueNames(sliceMember(mesh.surfaces,&Surf::name),"Surf");
            meshSurfsNameNs.push_back(makeIPT(msn));
        }
    }
    OPT<MeshSurfsNames>     meshSurfsNamesN = linkCollate<MeshSurfsName>(meshSurfsNameNs);
    GuiPtr                  meshSelect,
                            meshInfo;
    NPT<Bools>              selsN;
    if (compare) {
        IPT<String8s>           meshNamesN = makeIPT(meshNames);
        IPT<String8>            meshSelectN = makeIPT(meshNames[0]);
        IPT<size_t>             selMeshN(0);
        GuiPtr                  selMeshRadio = guiRadio(meshNames,selMeshN);
        size_t                  sz = meshNames.size();
        selsN = link1<size_t,Bools>(selMeshN,[sz](size_t const & idx)
            {
                Bools           ret(sz,false);
                if (idx < sz)
                    ret[idx] = true;
                return ret;
            });
        meshSelect = guiSplitScroll({selMeshRadio});
    }
    else {
        GuiVal<Bools>       cbs = guiCheckboxes(meshNames,Bools(meshes.size(),true));
        selsN = cbs.valN;
        meshSelect = cbs.win;
    }
    IPT<RendMeshes>     rmsN {gpms.rendMeshes};
    OPT<RendMeshes>     rendMeshesN = link2<RendMeshes,RendMeshes,Bools>(rmsN,selsN,[](const RendMeshes & m,Bools const & s)
    {
        return cFilter(m,s);
    });
    Gui3d               gui3d {rendMeshesN};
    NPT<String8>        statsTextN = link1<RendMeshes,String8>(gui3d.rendMeshesN,[](RendMeshes const & rms)
    {
        size_t              cnt = 0;
        ostringstream       oss;
        for (RendMesh const & rm : rms) {
            Mesh const &        m = rm.origMeshN.cref();
            oss << fgnl << "Mesh " << cnt++ << fgpush << m << fgpop;
        }
        return oss.str();
    });
    GuiPtr              statsTextW = guiText(statsTextN);
    GuiTabDefs          mainTabs = {
        guiTab("View",false,makeViewCtrls(gui3d,viewBoundsN,store+"View")),
        guiTab("Morphs",true,gpms.makePoseCtrls(true)),
        guiTab("Select",true,meshSelect),
        guiTab("Edit",true,makeEditPane(meshSurfsNamesN,gui3d)),
        guiTab("Info",true,guiSplitScroll({statsTextW})),
        guiTab("System",true,guiTextLines(gui3d.gpuInfo,16,true))
    };
    guiStartImpl(
        makeIPT<String8>("FaceGen SDK viewMesh"),
        guiSplitAdj(true,make_shared<Gui3d>(gui3d),guiTabs(mainTabs)),
        store);
    return meshNs[0].cref();
}

bool
guiSelectFids(Mesh & mesh,Strings const & fidLabels,uint mode)
{
    FGASSERT(!mesh.surfaces.empty());
    Surf &              surf = mesh.surfaces[0];
    Sptr<ImgC4UC>       imgOrig = surf.material.albedoMap;
    if (imgOrig) {
        surf.material.albedoMap = make_shared<ImgC4UC>(*imgOrig);
        // Reduce contrast so underlying surface shape is easily visible in combination:
        //ImgC4UC &   img = *surf.material.albedoMap;
        //for (size_t pp=0; pp<img.numPixels(); ++pp)
        //    img[pp] = img[pp] / 2 + RgbaUC(127,127,127,127);
    }
    SurfPoints &        sps = surf.surfPoints;
    if (mode == 1) {
        fgout << fgnl << "Place the following surface points in order then close the window:";
        for (size_t ii=0; ii<fidLabels.size(); ++ii)
            if (!contains(sps,fidLabels[ii]))
                fgout << fgnl << fidLabels[ii];
        Mesh                editMesh = viewMesh(mesh);
        SurfPoints const  & editSps = editMesh.surfaces[0].surfPoints;
        if (editSps.size() == fidLabels.size()) {
            sps.clear();
            sps.resize(fidLabels.size());
            for (size_t ii=0; ii<editSps.size(); ++ii) {
                SurfPoint       sp(editSps[ii]);
                if (sp.label.empty()) {
                    size_t          idx = findFirstIdx(sps,"");
                    FGASSERT(idx < sps.size());
                    sp.label = fidLabels[idx];
                    sps[idx] = (sp);
                }
                else {
                    size_t          idx = findFirstIdx(fidLabels,sp.label);
                    FGASSERT(idx < fidLabels.size());
                    sps[idx] = sp;
                }
            }
        }
        else {
            surf.material.albedoMap = imgOrig;
            return false;
        }
    }
    else {
        for (size_t ii=0; ii<fidLabels.size(); ++ii) {
            if (!contains(sps,fidLabels[ii])) {
                fgout
                    << fgnl << "Place the following surface point (ctrl-shift-left-click) then close the window."
                    << fgnl << "Only your last selection will be used:"
                    << fgnl << fidLabels[ii];
                Mesh                editMesh = viewMesh(mesh);
                if (editMesh.surfaces[0].surfPoints.size() > sps.size()) {
                    SurfPoint         sp(editMesh.surfaces[0].surfPoints.back());
                    sp.label = fidLabels[ii];
                    sps.push_back(sp);
                }
                else {
                    surf.material.albedoMap = imgOrig;
                    return false;
                }
            }
            else if (mode == 2) {
                fgout
                    << fgnl << "Modify the following surface point (ctrl-shift-left-click) if desired."
                    << fgnl << "Only your last selection will be used:"
                    << fgnl << fidLabels[ii];
                Mesh                editMesh = viewMesh(mesh);
                if (editMesh.surfaces[0].surfPoints.size() > sps.size()) {
                    size_t              idx = findFirstIdx(sps,fidLabels[ii],true);
                    SurfPoint         sp(editMesh.surfaces[0].surfPoints.back());
                    sp.label = fidLabels[ii];
                    sps[idx] = sp;
                }
            }
        }
    }
    surf.material.albedoMap = imgOrig;
    return true;
}

static void
simple(CLArgs const &)
{
    String8         dir = dataDir() + "base/";
    Mesh            mesh = loadTri(dir+"JaneLoresFace.tri",dir+"JaneLoresFace.jpg");
    viewMesh(mesh);
}

static void
surfs(CLArgs const &)
{
    Mesh            mesh;
    mesh.verts.push_back(Vec3F(0,0,0));
    mesh.verts.push_back(Vec3F(1,0,0));
    mesh.verts.push_back(Vec3F(1,1,0));
    mesh.verts.push_back(Vec3F(0,1,0));
    mesh.verts.push_back(Vec3F(0,0,-1));
    mesh.verts.push_back(Vec3F(0,1,-1));
    Surf            surf;
    surf.name = "1";
    surf.tris.posInds.push_back(Vec3UI(0,1,2));
    surf.tris.posInds.push_back(Vec3UI(2,3,0));
    //mesh.surfaces.push_back(surf);
    //surf.name = "2";
    surf.tris.posInds.push_back(Vec3UI(0,3,5));
    surf.tris.posInds.push_back(Vec3UI(5,4,0));
    mesh.surfaces.push_back(surf);
    viewMesh(mesh);
}

void
fgTestmGuiMesh(CLArgs const & args)
{
    Cmds       cmds;
    cmds.push_back(Cmd(simple,"simple","Jane face cutout"));
    cmds.push_back(Cmd(surfs,"surfs","Multiple surface mesh"));
    doMenu(args,cmds);
}

}

// */
