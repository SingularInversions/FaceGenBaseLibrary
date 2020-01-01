//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
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

// Output value has range [0,1]:
static GuiVal<Vec3F>
makeColorSliders(Ustring const & store,double init,double tickSpacing)
{
    IPT<Vec3F>          valN = makeSavedIPT(Vec3F(init),store);
    array<GuiPtr,3>     sliders = guiSliders(valN,array<Ustring,3>(),VecD2(0,1),tickSpacing);
    array<Ustring,3>    labels {{"Red","Green","Blue"}};
    GuiPtrs             wins;
    for (uint ii=0; ii<3; ++ii) {
        wins.push_back(guiSplit(true,
            guiText(labels[ii],45),       // Fixed width larger than all colors to align sliders
            sliders[ii]));
    }
    return GuiVal<Vec3F>(valN,guiSplit(false,wins[0],wins[1],wins[2]));
}

GuiVal<Vec3F>
makeDirectionSliders(array<IPT<double>,3> inputNs,Ustring const & store)
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
    ret.win = guiSplit(false,
        guiText("Direction vector to light (from viewer's point of view):"),
        guiSplit(true,ctls));
    return ret;
}

struct  LightCtrls
{
    GuiPtr                clrSliders;
    GuiPtr                dirSliders;
    OPT<Light>            light;
};

LightCtrls
makeLightCtrls(array<IPT<double>,3> inputNs,double defaultBrightness,Ustring const & store)
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
    const array<IPT<double>,3> &    lgt = shiftKey ? light2 : light1;
    Vec3D                        dir;
    for (uint ii=0; ii<3; ++ii)
        dir[ii] = lgt[ii].val();
    dir = normalize(dir);
    Vec3D                    axis(pixels[1],pixels[0],0);
    double                      axisLen = axis.len();
    if (axisLen > 0.0) {
        Mat33D                rot = matRotateAxis(axisLen/500.0,axis/axisLen);
        dir = rot * dir;
        for (uint ii=0; ii<3; ++ii)
            lgt[ii].set(dir[ii]);
    }
}

}

GuiPtr
makeLightingCtrls(
    RPT<Lighting>         lightingR,
    IPT<BothButtonsDragAction> bothButtonsDragActionI,
    Ustring const &        store)
{
    GuiVal<Vec3F>     ambient = makeColorSliders(store+"Ambient",0.4,0.1);
    Ustring                sp = store + "Diffuse";
    array<IPT<double>,3>    dir1Ns,
                            dir2Ns;
    LightCtrls              light1 = makeLightCtrls(dir1Ns,0.6,sp+"1"),
                            light2 = makeLightCtrls(dir2Ns,0.0,sp+"2");
    NPT<Lighting>         lightingN =
        link3<Lighting,Vec3F,Light,Light>(ambient.valN,light1.light,light2.light,
            [](Vec3F a,Light l1,Light l2)
            {return Lighting(a,fgSvec(l1,l2)); }
        );
    connect(lightingR,lightingN);
    bothButtonsDragActionI.init(std::bind(lightDirectionDrag,_1,_2,dir1Ns,dir2Ns));
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
    return guiSplitScroll(fgSvec(
        guiGroupbox("Ambient",ambient.win),
        guiGroupbox("Light 1",guiSplit(false,light1.clrSliders,light1.dirSliders)),
        guiGroupbox("Light 2",guiSplit(false,light2.clrSliders,light2.dirSliders)),
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
    Opt<Ustring>     fname = guiDialogFileLoad(imgFileExtensionsDescription(),imgFileExtensions());
    if (!fname.valid())
        return;
    if (fname.val().empty())
        return;
    ImgC4UC &       img = bgi.imgN.ref();
    try {
        imgLoadAnyFormat(fname.val(),img);
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
backgroundCtrls(BackgroundImage bgImg,Ustring const & store)
{
    bgImg.imgN.initSaved(ImgC4UC(),store+"Image",true);
    bgImg.origDimsN.initSaved(Vec2UI(0),store+"OrigDims");
    bgImg.offset.initSaved(Vec2F(0),store+"Offset");
    bgImg.lnScale.initSaved(0.0,store+"LnScale");
    bgImg.foregroundTransparency.initSaved(0.0,store+"ForegroundTransparency");
    GuiPtr    bgImageSlider = guiSlider(bgImg.foregroundTransparency,"Foreground transparency",VecD2(0,1),0.0),
                loadButton = guiButton("Load Image",bind(bgImageLoad2,bgImg)),
                clearButton = guiButton("Clear Image",[bgImg](){bgImg.imgN.ref().clear();}),
                bgImageButtons = guiSplit(true,loadButton,clearButton),
                text = guiText("Move: Ctrl-Shift-Left-Drag\nScale: Ctrl-Shift-Right-Drag");
    return guiGroupbox("Background Image",guiSplit(false,bgImageButtons,bgImageSlider,text));
}

static RendOptions
linkRenderOpts2(
    Bools const &       opts,
    Vec3F const &       bgColor)    // Elements [0,1]
{
    RendOptions       r;
    r.useTexture = opts[0];
    r.shiny = opts[1];
    r.wireframe = opts[2];
    r.flatShaded = opts[3];
    r.facets = opts[4];
    r.surfPoints = opts[5];
    r.markedVerts = opts[6];
    r.allVerts = opts[7];
    r.twoSided = opts[8];
    r.showAxes = opts[9];
    r.backgroundColor = bgColor;
    return r;
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
    uint                simple,
    Ustring const &    store)
{
    vector<OptInit>         opts = {
        {(simple != 3),"ColorMaps","Color Maps"},           // 0
        {false,"Shiny","Shiny"},                            // 1
        {false,"Wireframe","Wireframe"},                    // 2
        {false,"FlatShaded","Flat Shaded"},                 // 3
        {true,"Facets","Facets"},                           // 4
        {(simple==0),"SurfacePoints","Surface Points"},     // 5
        {(simple==0),"MarkedVerts","Marked Vertices"},      // 6
        {false,"AllVertices","All Vertices"},               // 7
        {true,"TwoSided","Two Sided"},                      // 8
        {false,"ShowAxes","Show origin and axes (red:X green:Y blue:Z)"} // 9
    };
    vector<IPT<bool> >      optNs;
    GuiPtrs               optWs;
    for (const OptInit & opt : opts) {
        optNs.push_back(makeSavedIPT(opt.defVal,store+opt.store));
        optWs.push_back(guiCheckbox(opt.desc,optNs.back()));
    }
    NPT<Bools>              optsN = linkCollate<bool>(optNs);
    GuiVal<Vec3F>           bgColor = makeColorSliders(store+"BgColor",0.0,0.1);
    NPT<RendOptions>        rendOptionsN = link2<RendOptions,Bools,Vec3F>(optsN,bgColor.valN,linkRenderOpts2);
    connect(rendOptionsR,rendOptionsN);
    GuiPtr                  renderCtls;
    if (simple == 1)
        renderCtls = guiSplit(true,
            guiSplit(false,optWs[0],optWs[1],optWs[2]),
            guiSplit(false,optWs[3],optWs[4]),
            guiSplit(false,optWs[7],optWs[8]));
    else if (simple == 2)
        renderCtls = guiSplit(false,optWs[0],optWs[1]);
    else if (simple == 3)
        renderCtls = guiSplit(false,optWs[1],optWs[2],optWs[3]);
    else    // simple == 0 or default to all controls:
        renderCtls = guiSplit(false,guiSplit(true,
            guiSplit(false,optWs[0],optWs[1],optWs[2]),
            guiSplit(false,optWs[3],optWs[4],optWs[5]),
            guiSplit(false,optWs[6],optWs[7],optWs[8])),optWs[9]);
    GuiPtr        viewCtlRender = guiGroupboxTr("Render",renderCtls),
                    viewCtlColor = guiGroupboxTr("Background Color",bgColor.win),
                    bgImageCtrls = backgroundCtrls(bgImg,store+"BgImage");
    return guiSplit(false,viewCtlRender,viewCtlColor,bgImageCtrls);
}

namespace {

void
expr_load2(NPT<PoseVals> posesN,IPT<Doubles> valsN)
{
    Opt<Ustring> fname = guiDialogFileLoad("FaceGen XML expression file",fgSvec<string>("xml"));
    if (fname.valid()) {
        const PoseVals &         poses = posesN.cref();
        Doubles &                vals = valsN.ref();
        FGASSERT(poses.size() == vals.size());
        vector<pair<Ustring,double> >  lvs;
        fgLoadXml(fname.val(),lvs);
        for (size_t ii=0; ii<lvs.size(); ++ii) {
            size_t      idx = findFirstIdx(poses,lvs[ii].first);
            if (idx < poses.size())
                vals[idx] = lvs[ii].second;
        }
    }
}

void
expr_save2(NPT<PoseVals> posesN,NPT<Doubles > valsN)
{
    Opt<Ustring> fname = guiDialogFileSave("FaceGen XML expression file","xml");
    if (fname.valid()) {
        const PoseVals &             poses = posesN.cref();
        const Doubles &              vals = valsN.cref();
        FGASSERT(poses.size() == vals.size());
        vector<pair<Ustring,double> >  lvs;
        lvs.reserve(poses.size());
        for (size_t ii=0; ii<poses.size(); ++ii)
            lvs.push_back(make_pair(poses[ii].name,vals[ii]));
        fgSaveXml(fname.val(),lvs);
    }
}

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
                    dstSurf.tris.vertInds.push_back(srcSurf.tris.vertInds[idx]);
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
                    dstSurf.quads.vertInds.push_back(srcSurf.quads.vertInds[quadIdx]);
                    srcSurf.removeQuad(quadIdx);
                }
            }
        }
    }
}

// Currently only works on tri facets:
void
assignPaint2(
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
                Vec3UI           tri = meshPtr->surfaces[isct.surfIdx].getTriEquivPosInds(isct.surfPnt.triEquivIdx);
                Surfs        surfs = fgSplitSurface(meshPtr->surfaces[isct.surfIdx]);
                for (size_t ss=0; ss<surfs.size(); ++ss) {
                    if (contains(surfs[ss].tris.vertInds,tri)) {
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
saveMesh2(
    string                  format,
    NPT<RendMeshes>         rendMeshesN)
{
    RendMeshes const &          rms = rendMeshesN.cref();
    if (rms.empty())
        return;
    Opt<Ustring>             fname = guiDialogFileSave("FaceGen",format);
    if (fname.valid()) {
        Meshes              meshes;
        for (RendMesh const & rm : rms)
            meshes.push_back(rm.origMeshN.cref());
        meshSaveAnyFormat(meshes,fname.val());
    }
}

GuiPtr
makeEditFacetsPane(NPT<RendMeshes> rendMeshesN,Gui3d & viewport)
{
    RendMeshes const &      rms = rendMeshesN.cref();
    FGASSERT(!rms.empty());
    RendMesh const &       rm = rms[0];
    Mesh *              meshPtr = rm.origMeshN.valPtr();
    FGASSERT(meshPtr);
    Ustrings               surfNames;
    for (size_t ss=0; ss<meshPtr->surfaces.size(); ++ss) {
        Surf &       surf = meshPtr->surfaces[ss];
        if (surf.name.empty())      // Generally not the case since default surf names assigned on load
            surf.name = toStr(ss);
        surfNames.push_back(surf.name);
    }
    GuiPtr                facets;
    if (surfNames.empty())
        facets = guiText("There are no surfaces to edit");
    else {
        IPT<size_t>     surfChoiceN(0);
        GuiPtr        surfChoiceW = guiRadio(surfNames,surfChoiceN);
        viewport.shiftRightDragActionI.init(bind(assignTri2,rendMeshesN,surfChoiceN,_1,_2,_3));
        viewport.shiftMiddleClickActionI.init(bind(assignPaint2,rendMeshesN,surfChoiceN,_1,_2,_3));
        facets = guiSplit(false,
            guiText("Assign tri to surface selection: shift-right-drag\n"
                "Assign connected tris (not quads) to surface: shift-middle-click"),
            guiGroupbox("Surface Selection",surfChoiceW),
            guiCheckbox("Color by surface",viewport.colorBySurface));
    }
    return facets;
}

struct  MeshSurfsName
{
    Ustring        meshName;
    Ustrings       surfNames;
};
typedef vector<MeshSurfsName>   MeshSurfsNames;

// Modifies 'api' to add it's hooks:
GuiPtr
makeEditPane(const OPT<MeshSurfsNames> & meshSurfsNamesN,Gui3d & api)
{
    vector<GuiTabDef>    tabs;
    Ustrings           vsmrLabels = {"Single","Edge Seam","Fold Seam"};
    IPT<size_t>         vertSelModeN(0);
    GuiPtr            vertSelModeW = guiRadio(vsmrLabels,vertSelModeN);
    api.vertMarkModeN = vertSelModeN;
    IPT<Ustring>       pointLabelN = makeIPT(Ustring());
    api.pointLabel = pointLabelN;
    NPT<RendMeshes>     rendMeshesN = api.rendMeshesN;
    GuiPtr            points = 
        guiSplit(false,fgSvec(
            guiText(
                "Mark Surface Point: ctrl-shift-left-click on surface.\n"
                "A point with an identical non-null name will be overwritten"
            ),
            guiSplit(true,
                guiText("Name:"),
                guiTextEdit(pointLabelN)),
            guiButtonTr("Clear all surface points",
                [rendMeshesN]()
                {
                    RendMeshes const &      rms = rendMeshesN.cref();
                    for (RendMesh const & rm : rms) {
                        Mesh *          meshPtr = rm.origMeshN.valPtr();
                        if (meshPtr) {
                            for (Surf & surf : meshPtr->surfaces)
                                surf.surfPoints.clear();
                        }
                    }
                })
        ));
    GuiPtr            verts =
        guiSplit(false,fgSvec(
            guiText("Mark Vertex: ctrl-shift-right-click on surface near vertex"),
            guiGroupbox("Vertex Selection Mode",vertSelModeW),
            guiButtonTr("Mark all seam vertices",
                [rendMeshesN]()
                {
                    RendMeshes const &      rms = rendMeshesN.cref();
                    for (RendMesh const & rm : rms) {
                        Mesh *          meshPtr = rm.origMeshN.valPtr();
                        if (meshPtr) {
                            Fg3dTopology        topo(meshPtr->verts,meshPtr->getTriEquivs().vertInds);
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
        ));
    GuiPtr    facets = guiDynamic(bind(makeEditFacetsPane,rendMeshesN,api),makeUpdateFlag(meshSurfsNamesN));
    tabs.push_back(GuiTabDef("Points",true,points));
    tabs.push_back(GuiTabDef("Verts",true,verts));
    tabs.push_back(GuiTabDef("Facets",true,facets));
    GuiPtr            saveButtons = guiSplit(true,
        guiButtonTr("TRI",bind(saveMesh2,string("tri"),rendMeshesN)),
        guiButtonTr("FGMESH",bind(saveMesh2,string("fgmesh"),rendMeshesN)));
    return guiSplit(false,guiText("Save pre-morphed to file:"),saveButtons,guiTabs(tabs));
}

// Unique names will be left unchanged.
// Empty names will be assigned the baseName plus a number.
// Non-unique names will have a number added.
Ustrings
makeUniqueNames(const Ustrings & names,Ustring const & baseName)
{
    Ustrings           ret;
    ret.reserve(names.size());
    for (size_t nn=0; nn<names.size(); ++nn) {
        Ustring            name = names[nn];
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
    NPT<Mat32D>           viewBoundsN,
    Ustring const &        store,
    uint                    simple,
    bool                    textEditBoxes)
{
    Ustrings               panTiltOptions = {"Pan / Tilt","Unconstrained"};
    size_t                  panTiltDefault = (simple == 0) ? 1 : 0;
    IPT<size_t>             panTiltRadioN = makeSavedIPT<size_t>(panTiltDefault,store+"PanTiltMode");
    GuiPtr                panTiltRadioW = guiRadio(panTiltOptions,panTiltRadioN);
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
    CameraParams    defaultCps;     // Use default for lensFovDeg:
    IPT<double>         lensFovDeg = makeSavedIPT(defaultCps.fovMaxDeg,store+"LensFovDeg");
    VecD2            logScaleRange(log(1.0/5.0),log(5.0));
    OPT<CameraParams>   camParamsN =
        link5<CameraParams,Mat32D,QuaternionD,Vec2D,double,double>(
            viewBoundsN,api.pose,api.trans,api.logRelSize,lensFovDeg,
            [](const Mat32D & vb,const QuaternionD & p,const Vec2D & t,const double & lrs,const double & lfd)
            {
                CameraParams    r;
                r.modelBounds = vb;
                r.pose = p;
                r.relTrans = t;
                r.logRelScale = lrs;
                r.fovMaxDeg = lfd;
                return r;
            });
    OPT<Vec2D>   panTiltN = link2<Vec2D,double,double>(api.panDegrees,api.tiltDegrees,
        [](const double & p,const double & t){return Vec2D(p,t); });
    api.xform = link4<Camera,CameraParams,size_t,Vec2D,Vec2UI>(
        camParamsN,api.panTiltMode,panTiltN,api.viewportDims,
        [](const CameraParams & cp,const size_t & m,const Vec2D & pt,const Vec2UI & v)
        {
            CameraParams    cps = cp;
            if (m == 0) {
                QuaternionD   pan(degToRad(pt[0]),1),
                                tilt(degToRad(pt[1]),0);
                cps.pose = pan*tilt;
            }
            return cps.camera(v);
        });
    GuiPtr        viewCtlText =
        guiText(
            "  Rotate: left-click-drag (or touch-drag)\n"
            "  Scale: right-click-drag up/down (or pinch-to-zoom)\n"
            "  Move: shift-left-click-drag (or middle-click-drag)");
    VecD2        panTiltClip(-360.0,360.0);      // Values are wrapped by viewport impl
    GuiPtr        viewCtlPanTiltText =
        guiSplit(false,
            guiSplit(true,
                guiTextEditFixed(api.panDegrees,panTiltClip),
                guiTextEditFixed(api.tiltDegrees,panTiltClip)),
            guiSpacer(0,30));
    if (textEditBoxes)
        panTiltRadioW = guiSplit(true,panTiltRadioW,viewCtlPanTiltText);
    GuiPtr        viewCtlRotation = guiGroupboxTr("Object rotation",panTiltRadioW);
    GuiPtr        viewCtlFov =
        guiGroupboxTr(
            "Lens field of view (degrees)",
            guiSlider(
                lensFovDeg,
                Ustring(),
                VecD2(0.0,60.0),
                15.0,
                guiTickLabels(VecD2(0.0,60.0),15.0,0.0),
                fgSvec(
                    GuiTickLabel(0.0,fgTr("Orthographic")),
                    GuiTickLabel(30.0,fgTr("Normal")),
                    GuiTickLabel(60.0,fgTr("Wide"))),
                30,     // Lots of room required for "Orthographic" at end of slider
                textEditBoxes
            )
        );
    IPT<double>     lnRelSizeN = api.logRelSize;
    auto            getVal = [=](){return std::exp(lnRelSizeN.val()); };
    auto            setVal = [=](double val){lnRelSizeN.set(std::log(val)); };
    GuiPtr          viewCtlScaleTe = guiTextEditFloat(
        makeUpdateFlag(lnRelSizeN),getVal,setVal,VecD2{exp(-20.0),exp(20.0)},6);
    GuiPtr          viewCtlScale = guiSlider(
                        api.logRelSize,
                        Ustring(),
                        logScaleRange,
                        (logScaleRange[1]-logScaleRange[0])/4.0,
                        fgSvec(
                            GuiTickLabel(logScaleRange[0],"0.2"),
                            GuiTickLabel(0.0,"1.0"),
                            GuiTickLabel(logScaleRange[1],"5.0")),
                        vector<GuiTickLabel>(),
                        30      // Match width of slider above
                    );
    if (textEditBoxes)
        viewCtlScale = guiSplit(true,viewCtlScale,viewCtlScaleTe);
    viewCtlScale = guiGroupboxTr("Object relative scale",viewCtlScale);
    DfgNPtrs           resetDeps;
    resetDeps.push_back(api.panDegrees.ptr);
    resetDeps.push_back(api.tiltDegrees.ptr);
    resetDeps.push_back(api.pose.ptr);
    resetDeps.push_back(api.trans.ptr);
    resetDeps.push_back(api.logRelSize.ptr);
    resetDeps.push_back(lensFovDeg.ptr);
    GuiPtr        viewCtlReset = guiButtonTr("Reset Camera",[resetDeps](){setInputsToDefault(resetDeps);});
    if (simple == 2)
        return guiSplit(false,viewCtlText,viewCtlScale,viewCtlReset);
    else
        return guiSplit(false,fgSvec(
            viewCtlText,viewCtlRotation,viewCtlFov,viewCtlScale,viewCtlReset));
}

OPT<Vec3Fs>
linkAllVerts(NPT<Mesh> meshN)
{return link1<Mesh,Vec3Fs>(meshN,[](Mesh const & mesh){return mesh.allVerts();}); }

OPT<Vec3Fs>
linkPosedVerts(NPT<Mesh> meshN,NPT<Vec3Fs> allVertsN,OPT<PoseVals> posesN,NPT<Doubles> morphValsN)
{
    return link4_<Vec3Fs,Mesh,Vec3Fs,PoseVals,Doubles>(meshN,allVertsN,posesN,morphValsN,[=](
            Mesh const &            mesh,
            Vec3Fs const &             allVerts,
            PoseVals const &             poses,
            Doubles const &              poseVals,
            Vec3Fs &                   verts)
        {
            // 'poseVals' may not be initialized yet since it's set by GUI so fake it until then:
            Floats              vals = scast<float>(poseVals);
            if (vals.size() != poses.size())
                vals.resize(poses.size(),0);
            map<Ustring,float>         poseMap;
            for (size_t ii=0; ii<poses.size(); ++ii)
                poseMap[poses[ii].name] = vals[ii];
            verts = mesh.poseShape(allVerts,poseMap);
        }
    );
}

OPT<Normals>
linkNormals(const NPT<Mesh> & meshN,const NPT<Vec3Fs> & posedVertsN)
{
    return link2_<Normals,Mesh,Vec3Fs>(meshN,posedVertsN,
        [](const Mesh & mesh,const Vec3Fs & verts,Normals & norms)
        {cNormals_(mesh.surfaces,verts,norms); });
}

OPT<Mesh>
linkLoadMesh(NPT<Ustring> pathBaseN)
{
    return link1<Ustring,Mesh>(pathBaseN,
        [](Ustring const & pathBase)
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
linkLoadImage(NPT<Ustring> filenameN)
{
    return link1_<Ustring,ImgC4UC>(filenameN,
        [](Ustring const & fn,ImgC4UC & img)
        {
            img.clear();        // Ensure cleared in case load fails.
            if (!fn.empty())
                imgLoadAnyFormat(fn,img);
        });
}

GuiPosedMeshes::GuiPosedMeshes() :
    poseLabelsN(
        linkN<Mesh,PoseVals>(vector<NPT<Mesh> >(),[](Meshes const & m){return fgPoses(m);})
    ),
    poseValsN(makeIPT(Doubles()))
{}

void
GuiPosedMeshes::addMesh(
        NPT<Mesh>           meshN,
        NPT<Vec3Fs>         allVertsN,
        ImgNs               albedoNs,
        ImgNs               specularNs)
{
    // We can't check against mesh surfaces at this time since it may not be selected:
    FGASSERT(albedoNs.size() == specularNs.size());
    addLink(meshN.ptr,poseLabelsN.ptr);
    RendMesh            rm;
    rm.origMeshN = meshN;
    size_t              S = albedoNs.size();
    for (size_t ss=0; ss<S; ++ss) {
        RendSurf            rs;
        rs.albedoMap = albedoNs[ss];
        rs.albedoMapFlag = makeUpdateFlag(albedoNs[ss]);
        rs.albedoHasTransparencyN = link1<ImgC4UC,bool>(rs.albedoMap,
            [](ImgC4UC const & img){return fgUsesAlpha(img);});     // Returns false if image empty
        rs.specularMap = specularNs[ss];
        rs.specularMapFlag = makeUpdateFlag(specularNs[ss]);
        rm.rendSurfs.push_back(rs);
    }
    rm.posedVertsN = linkPosedVerts(meshN,allVertsN,poseLabelsN,poseValsN);
    rm.normalsN = linkNormals(meshN,rm.posedVertsN);
    rm.surfVertsFlag = makeUpdateFlag(rm.posedVertsN);
    rm.allVertsFlag = makeUpdateFlag(rm.posedVertsN);
    rendMeshes.push_back(rm);
}

void
GuiPosedMeshes::addMesh(NPT<Mesh> meshN,NPT<Vec3Fs> vertsN)
{
    ImgNs       albedoNs;
    albedoNs.push_back(makeIPT(ImgC4UC()));
    ImgNs       specularNs;
    specularNs.push_back(makeIPT(ImgC4UC()));
    addMesh(meshN,vertsN,albedoNs,specularNs);
}


// Called by GuiSplitScroll when 'labelsN' changes:
GuiPtrs
makePoseCtrlSliders(NPT<PoseVals> posesN,IPT<Doubles> valsN,bool textEditBoxes)
{
    PoseVals const &         poses = posesN.cref();
    Doubles &                output = valsN.ref();
    if (output.size() != poses.size()) {   // past value is stale
        output.clear();
        output.resize(poses.size());
        for (size_t ii=0; ii<output.size(); ++ii)
            output[ii] = poses[ii].neutral;
    }
    GuiPtrs               buttons;
    buttons.push_back(guiButton("Set All Zero",[valsN](){fgFill(valsN.ref(),0.0);}));
    buttons.push_back(guiButton("Load File",bind(expr_load2,posesN,valsN)));
    buttons.push_back(guiButton("Save File",bind(expr_save2,posesN,valsN)));
    GuiPtrs               sliders;
    sliders.push_back(guiSplit(true,buttons));
    sliders.push_back(guiSpacer(0,7));
    for (size_t ii=0; ii<poses.size(); ++ii) {
        GuiSlider      slider;
        slider.updateFlag = makeUpdateFlag(valsN);
        slider.getInput = [valsN,ii](){return valsN.cref()[ii];};
        slider.setOutput = [valsN,ii](double v){valsN.ref()[ii] = v;};
        slider.label = poses[ii].name;
        slider.range[0] = poses[ii].bounds[0];
        slider.range[1] = poses[ii].bounds[1];
        slider.tickSpacing = 0.1;
        if (textEditBoxes) {
            auto            getVal = [=](){return valsN.cref()[ii]; };
            auto            setVal = [=](double val){valsN.ref()[ii] = val; };
            GuiPtr          teW = guiTextEditFixed(makeUpdateFlag(valsN),getVal,setVal,VecD2{-1,2},2);
            sliders.push_back(guiSplit(true,guiMakePtr(slider),teW));
        }
        else
            sliders.push_back(guiMakePtr(slider));
    }
    if (poses.empty())
        sliders.push_back(guiText("The currently selected models contain no expression morphs."));
    return sliders;
}

GuiPtr
GuiPosedMeshes::makePoseCtrls(bool editBoxes) const
{
    auto        makeSliders =  bind(makePoseCtrlSliders,poseLabelsN,poseValsN,editBoxes);
    return guiSplitScroll(makeUpdateFlag(poseLabelsN),makeSliders,3);
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
        Mat32F    bounds = boundsUnion(bs);
        if (bounds[1] < bounds[0])      // No meshes selected
            bounds = Mat32F(0,1,0,1,0,1);
        return Mat32D(bounds);
    });
}

OPT<Ustring>
linkMeshStats(RendMeshes const & rms)
{
    Svec<NPT<Ustring> >    statsNs;
    for (RendMesh const & rm : rms)
        statsNs.push_back(link1<Mesh,Ustring>(rm.origMeshN,[](Mesh const & m)
            {
                if (m.verts.empty())
                    return Ustring();
                stringstream    ss;
                ss << m;
                return Ustring(ss.str());
            }));
    return linkN<Ustring,Ustring>(statsNs,[](Ustrings const & strs){return cat(strs,"");});
}

GuiPtr
makeViewCtrls(
    Gui3d &             gui3d,
    NPT<Mat32D>       viewBoundsN,
    Ustring const &    store,
    uint                simple,
    bool                textEditBoxes)
{
    GuiPtr          cameraW = makeCameraCtrls(gui3d,viewBoundsN,store+"Camera",simple,textEditBoxes),
                    renderOptsW = makeRendCtrls(gui3d.renderOptions,gui3d.bgImg,simple,store+"RendOpts"),
                    lightingW = makeLightingCtrls(gui3d.light,gui3d.bothButtonsDragActionI,store+"Light");
    GuiTabDefs      viewTabs = {
                        guiTab("Camera",true,cameraW),
                        guiTab("Render",true,renderOptsW),
                        guiTab("Lighting",true,lightingW)
                    };
    return guiTabs(viewTabs);
}

GuiPtr
guiCaptureSaveImage(NPT<Vec2UI> viewportDims,Sptr<Gui3d::Capture> const & capture,Ustring const & store)
{
    IPT<double>         widN = makeSavedIPT(1024.0,store+"Width"),
                        hgtN = makeSavedIPT(1024.0,store+"Height");
    IPT<size_t>         szSelN = makeSavedIPT<size_t>(0,store+"Manual");
    NPT<Ustring>        vpDimsN = link1<Vec2UI,Ustring>(viewportDims,[](Vec2UI dims)
        {return "(" + toStr(dims[0]) + "," + toStr(dims[1]) + ")"; });
    GuiPtr              widW = guiTextEditFixed(widN,VecD2(256,4096),0),
                        hgtW = guiTextEditFixed(hgtN,VecD2(256,4096),0),
                        dimsLW = guiRadio({"Current render size","Custom"},szSelN),
                        dimsRUW = guiText(vpDimsN),
                        dimsRLW = guiSplit(true,{guiText("Width"),widW,guiText("Height"),hgtW}),
                        dimsRW = guiSplit(false,guiSpacer(1,3),dimsRUW,guiSpacer(1,3),dimsRLW),
                        dims3W = guiSplit(true,{dimsLW,dimsRW}),
                        dimsW = guiGroupbox("Pixel size",dims3W);
    GuiVal<string>      capImgFormat = guiImageFormat("Image Format",false,store+"Format");
    NPT<string>         formatN = capImgFormat.valN;
    auto                save = [=]()
        {
            if (capture->getImg) {
                Vec2UI          dims = szSelN.val() ? Vec2UI(widN.val(),hgtN.val()) : viewportDims.val();
                Opt<Ustring>    fname = guiDialogFileSave("",formatN.cref());
                if (fname.valid()) {
                    imgSaveAnyFormat(fname.val(),capture->getImg(dims));
                }
            }
        };
    GuiPtr              saveImage = guiSplit(false,
        guiText("Save the current render image to a file"),
        guiSplit(false,dimsW,capImgFormat.win),
        guiButton("Save",save));
    return saveImage;
}

Mesh
meshView(const Meshes & meshes,bool compare)
{
    FGASSERT(meshes.size() > 0);
    Ustring                 store = fgDirUserAppDataLocalFaceGen("SDK","meshView");
    IPT<Mat32D>             viewBoundsN = makeIPT(fgF2D(cBounds(meshes)));
    GuiPosedMeshes          mrms;
    vector<IPT<Mesh> >      meshNs;
    vector<IPT<MeshSurfsName> >  meshSurfsNameNs;
    // Ensure we have valid, unique names:
    Ustrings                meshNames = makeUniqueNames(sliceMember(meshes,&Mesh::name),"Mesh");
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        const Mesh &        mesh = meshes[mm];
        IPT<Mesh>           meshN = makeIPT(mesh);
        IPT<Vec3Fs>         allVertsN = makeIPT(mesh.allVerts());
        ImgNs               albedoNs,
                            specularNs;
        IPT<ImgC4UC>        emptyImageN = makeIPT(ImgC4UC());
        for (const Surf & surf : mesh.surfaces) {
            if (surf.material.albedoMap)
                albedoNs.push_back(makeIPT(*surf.material.albedoMap));
            else
                albedoNs.push_back(emptyImageN);
            if (surf.material.specularMap)
                specularNs.push_back(makeIPT(*surf.material.specularMap));
            else
                specularNs.push_back(emptyImageN);
        }
        mrms.addMesh(meshN,allVertsN,albedoNs,specularNs);
        meshNs.push_back(meshN);
        {
            MeshSurfsName       msn;
            msn.meshName = meshNames[mm];
            msn.surfNames = makeUniqueNames(sliceMember(mesh.surfaces,&Surf::name),"Surf");
            meshSurfsNameNs.push_back(makeIPT(msn));
        }
    }
    OPT<MeshSurfsNames>     meshSurfsNamesN = linkCollate<MeshSurfsName>(meshSurfsNameNs);
    GuiPtr                  meshSelect;
    NPT<Bools>              selsN;
    if (compare) {
        IPT<Ustrings>           meshNamesN = makeIPT(meshNames);
        IPT<Ustring>            meshSelectN = makeIPT(meshNames[0]);
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
        meshSelect = selMeshRadio;
    }
    else {
        GuiVal<Bools>       cbs = guiCheckboxes(meshNames,Bools(meshes.size(),true));
        selsN = cbs.valN;
        meshSelect = cbs.win;
    }
    IPT<RendMeshes>     rmsN {mrms.rendMeshes};
    Gui3d               gui3d;
    gui3d.rendMeshesN = link2<RendMeshes,RendMeshes,Bools>(rmsN,selsN,
        [](const RendMeshes & m,const Bools & s){return fgFilter(m,s); });
    GuiTabDefs          mainTabs = {
        guiTab("View",false,makeViewCtrls(gui3d,viewBoundsN,store+"View",0,true)),
        guiTab("Morphs",true,mrms.makePoseCtrls(true)),
        guiTab("Select",true,meshSelect),
        guiTab("Edit",true,makeEditPane(meshSurfsNamesN,gui3d)),
        guiTab("Info",true,guiSplitScroll(fgSvec(guiText(linkMeshStats(mrms.rendMeshes))))),
        guiTab("System",true,guiTextLines(gui3d.gpuInfo,16,true))
    };
    guiStartImpl(
        Ustring("FaceGen SDK meshView"),
        guiSplitAdj(true,std::make_shared<Gui3d>(gui3d),guiTabs(mainTabs)),
        store);
    return meshNs[0].cref();
}

namespace {

void
simple(CLArgs const &)
{
    Ustring    dir = dataDir() + "base/";
    Mesh    mesh = loadTri(dir+"JaneLoresFace.tri",dir+"JaneLoresFace.jpg");
    meshView(fgSvec(mesh),false);
}

void
surfs(CLArgs const &)
{
    Mesh        mesh;
    mesh.verts.push_back(Vec3F(0,0,0));
    mesh.verts.push_back(Vec3F(1,0,0));
    mesh.verts.push_back(Vec3F(1,1,0));
    mesh.verts.push_back(Vec3F(0,1,0));
    mesh.verts.push_back(Vec3F(0,0,-1));
    mesh.verts.push_back(Vec3F(0,1,-1));
    Surf     surf;
    surf.name = "1";
    surf.tris.vertInds.push_back(Vec3UI(0,1,2));
    surf.tris.vertInds.push_back(Vec3UI(2,3,0));
    //mesh.surfaces.push_back(surf);
    //surf.name = "2";
    surf.tris.vertInds.push_back(Vec3UI(0,3,5));
    surf.tris.vertInds.push_back(Vec3UI(5,4,0));
    mesh.surfaces.push_back(surf);
    meshView(fgSvec(mesh),false);
}

}

void
fgTestmGuiMesh(CLArgs const & args)
{
    vector<Cmd>       cmds;
    cmds.push_back(Cmd(simple,"simple","Jane face cutout"));
    cmds.push_back(Cmd(surfs,"surfs","Multiple surface mesh"));
    doMenu(args,cmds);
}

}

// */
