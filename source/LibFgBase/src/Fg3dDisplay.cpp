//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dDisplay.hpp"
#include "FgGuiApi.hpp"
#include "FgFileSystem.hpp"
#include "FgTestUtils.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgTopology.hpp"
#include "FgCommand.hpp"
#include "FgBestN.hpp"


using namespace std;
using namespace std::placeholders;

namespace Fg {

namespace {

Arr<GuiPtr,3>       makeSliderBank3(
    IPT<Vec3F>              valN,
    Arr<String8,3> const &  labels,
    VecD2                   range,
    double                  tickSpacing)
{
    Arr<GuiPtr,3>  ret;
    for (size_t ss=0; ss<3; ++ss) {
        GuiSlider        s;
        s.updateFlag = cUpdateFlagT(valN);
        s.getValFn = [valN,ss](){return valN.val()[ss]; };
        s.setValFn = [valN,ss](double v){valN.ref()[ss] = v; };
        s.label = labels[ss];
        s.range = range;
        s.tickSpacing = tickSpacing;
        ret[ss] = guiPtr(s);
    }
    return ret;
}

// Output value has range [0,1]:
GuiVal<Vec3F>       makeColorSliders(String8 const & store,double init,double tickSpacing)
{
    IPT<Vec3F>          valN = makeSavedIPT(Vec3F(init),store);
    Arr<GuiPtr,3>       sliders = makeSliderBank3(valN,Arr<String8,3>{},VecD2(0,1),tickSpacing);
    Arr<String8,3>      labels {"Red","Green","Blue"};
    Img<GuiPtr>         wins {2,3};
    for (uint ii=0; ii<3; ++ii) {
        wins.xy(0,ii) = guiText(labels[ii]);
        wins.xy(1,ii) = sliders[ii];
    }
    return GuiVal<Vec3F>(valN,guiSplit(wins));
}

GuiVal<Vec3F>       makeDirectionSliders(Arr<IPT<double>,3> inputNs,String8 const & store)
{
    Vec3D               defaultVal(0,0,1);
    VecD2               bounds(-100000,100000);
    Arr<String,3>       labels {"Right:","Up:","Backward:"};
    GuiPtrs             ctls;
    for (uint xx=0; xx<3; ++xx) {
        inputNs[xx].initSaved(defaultVal[xx],store+toStr(xx));
        ctls.push_back(guiText(labels[xx]));
        ctls.push_back(guiTextEditFloat(inputNs[xx],bounds,6));
    }
    GuiVal<Vec3F> ret;
    ret.valN = link3(inputNs[0],inputNs[1],inputNs[2],[](double v0,double v1,double v2){return Vec3F(v0,v1,v2); });
    ret.win = guiSplitV({
        guiText("Direction vector to light (from viewer's point of view):"),
        guiSplitH(ctls)});
    return ret;
}

struct      LightCtrls
{
    GuiPtr                clrSliders;
    GuiPtr                dirSliders;
    OPT<Light>            light;
};

LightCtrls          makeLightCtrls(Arr<IPT<double>,3> inputNs,double defaultBrightness,String8 const & store)
{
    LightCtrls              ret;
    GuiVal<Vec3F>     color = makeColorSliders(store+"Color",defaultBrightness,0.1);
    GuiVal<Vec3F>     dir = makeDirectionSliders(inputNs,store+"Direction");
    ret.light = link2(color.valN,dir.valN,[](Vec3F c,Vec3F d){return Light(c,d); });
    ret.clrSliders = color.win;
    ret.dirSliders = dir.win;
    return ret;
}

void                lightDirectionDrag(
    bool                    shiftKey,
    Vec2I                   pixels,
    Arr<IPT<double>,3>    light1,
    Arr<IPT<double>,3>    light2)
{
    auto const &            lgt = shiftKey ? light2 : light1;
    Vec3D                   dir;
    for (uint ii=0; ii<3; ++ii)
        dir[ii] = lgt[ii].val();
    dir = normalize(dir);
    Vec3D                   axis(pixels[1],pixels[0],0);
    double                  axisLen = cLenD(axis);
    if (axisLen > 0.0) {
        Mat33D              rot = matRotateAxis(axisLen/500.0,axis/axisLen);
        dir = rot * dir;
        for (uint ii=0; ii<3; ++ii)
            lgt[ii].set(dir[ii]);
    }
}

}

GuiPtr              makeLightingCtrls(
    RPT<Lighting>           lightingR,
    BothButtonsDragAction & bothButtonsDragAction,
    String8 const &         store)
{
    GuiVal<Vec3F>     ambient = makeColorSliders(store+"Ambient",0.4,0.1);
    String8                sp = store + "Diffuse";
    Arr<IPT<double>,3>      dir1Ns,
                            dir2Ns;
    LightCtrls              light1 = makeLightCtrls(dir1Ns,0.6,sp+"1"),
                            light2 = makeLightCtrls(dir2Ns,0.0,sp+"2");
    NPT<Lighting>         lightingN =
        link3(ambient.valN,light1.light,light2.light,[](Vec3F a,Light l1,Light l2){return Lighting(a,{l1,l2}); });
    connect(lightingR,lightingN);
    bothButtonsDragAction = std::bind(lightDirectionDrag,_1,_2,dir1Ns,dir2Ns);
    auto                    resetLighting = [lightingN,dir1Ns,dir2Ns]()
        {
            DfNPtrs       nodes;
            nodes.push_back(lightingN.ptr);
            for (const IPT<double> & i : dir1Ns)
                nodes.push_back(i.ptr);
            for (const IPT<double> & i : dir2Ns)
                nodes.push_back(i.ptr);
            setInputsToDefault(nodes);
        };
    return guiSplitScroll({
        guiGroupbox("Ambient",ambient.win),
        guiGroupbox("Light 1",guiSplitV({light1.clrSliders,light1.dirSliders})),
        guiGroupbox("Light 2",guiSplitV({light2.clrSliders,light2.dirSliders})),
        guiText("\nInteractively adjust light direction:\n"
            "Light 1: hold down left and right mouse buttons while dragging\n"
            "Light 2: As above but also hold down shift key"),
        guiButton("Reset Lighting",resetLighting)
    });
}

namespace {

void                bgImageLoad2(BackgroundImage bgi)
{
    Opt<String8>     fname = guiDialogFileLoad(clOptionsStr(getImgExts()),getImgExts());
    if (!fname.has_value())
        return;
    if (fname.value().empty())
        return;
    ImgRgba8 &       img = bgi.imgN.ref();
    try {
        loadImage_(fname.value(),img);
    }
    catch (const FgException & e) {
        guiDialogMessage("Unable to load background image",e.nativeMessage());
        return;
    }
    bgi.origDimsN.set(img.dims());
    bgi.offset.set(Vec2F(0.0));
    bgi.lnScale.set(0.0);
}

GuiPtr              backgroundCtrls(BackgroundImage bgImg,String8 const & store)
{
    bgImg.imgN.initSaved(ImgRgba8(),store+"Image",true);
    bgImg.origDimsN.initSaved(Vec2UI(0),store+"OrigDims");
    bgImg.offset.initSaved(Vec2F(0),store+"Offset");
    bgImg.lnScale.initSaved(0.0,store+"LnScale");
    bgImg.foregroundTransparency.initSaved(0.0,store+"ForegroundTransparency");
    GuiPtr      bgImageSlider = guiSlider(bgImg.foregroundTransparency,"Foreground transparency",VecD2(0,1),0.0),
                loadButton = guiButton("Load Image",bind(bgImageLoad2,bgImg)),
                clearButton = guiButton("Clear Image",[bgImg](){bgImg.imgN.ref().clear();}),
                bgImageButtons = guiSplitH({loadButton,clearButton}),
                text = guiText("Move: Ctrl-Shift-Left-Drag\nScale: Ctrl-Shift-Right-Drag");
    return guiGroupbox("Background Image",guiSplitV({bgImageButtons,bgImageSlider,text}));
}

struct      OptInit
{
    bool        defVal;
    string      store;
    string      desc;
};

}

GuiPtr              makeRendCtrls(
    RPT<RendOptions>    rendOptionsR,
    BackgroundImage     bgImg,
    String8 const &     store,
    bool                structureOptions,
    bool                twoSidedOption,
    bool                pointOptions,
    IPT<bool> *         supportTransparencyNPtr)
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
    Svec<IPT<bool>>         optNs;
    GuiPtrs                 optWs;
    for (const OptInit & opt : opts) {
        optNs.push_back(makeSavedIPT(opt.defVal,store+opt.store));
        optWs.push_back(guiCheckbox(opt.desc,optNs.back()));
    }
    NPT<Bools>              optsN = linkCollate(optNs);
    String8s                albedoModeLabels = cAlbedoModeLabels();
    IPT<size_t>             optRcN = makeSavedIPTEub<size_t>(0,store+"albedoMode",albedoModeLabels.size());
    GuiVal<Vec3F>           bgColor = makeColorSliders(store+"BgColor",0.0,0.1);
    NPT<RendOptions>        rendOptionsN = link3(optRcN,optsN,bgColor.valN,
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
        enabledOptWs.insert(enabledOptWs.end(),{optWs[1],optWs[2],optWs[3],optWs[6],});
    }
    if (twoSidedOption)
        enabledOptWs.push_back(optWs[7]);
    if (pointOptions) {
        enabledOptWs.push_back(optWs[4]);
        enabledOptWs.push_back(optWs[5]);
    }
    size_t                  chunkSz = cMax(size_t(2),(enabledOptWs.size() + 1) / 2);
    GuiPtrs                 colWs {guiSplitV(cHead(enabledOptWs,chunkSz))};
    if (enabledOptWs.size() > chunkSz)
        colWs.push_back(guiSplitV(cRest(enabledOptWs,chunkSz)));
    GuiPtr                  checkboxesW = guiSplitH(colWs),
                            checkboxesSpacedW = guiSplitV({guiSpacer(0,15),checkboxesW}),
                            colsW = guiSplitH({
                                guiGroupbox("Color",guiRadio(cHead(albedoModeLabels,numColorOptions),optRcN)),
                                guiSpacer(10,0),
                                checkboxesSpacedW});
    GuiPtr                  renderCtls;
    if (pointOptions)
        renderCtls = guiSplitV({colsW,optWs[8]});
    else
        renderCtls = colsW;
    GuiPtrs         renderGroups {
        guiGroupboxTr("Render",renderCtls),
        guiGroupboxTr("Background Color",bgColor.win),
        backgroundCtrls(bgImg,store+"BgImage"),
    };
    if (supportTransparencyNPtr!=nullptr) {
        IPT<bool>       stN = *supportTransparencyNPtr;     // lambda captures must store this, not the pointer to it
        GuiRadio        radio;
        radio.labels = {
            "Direct3D 11.1 (transparency; only works if supported)",
            "Direct3D 11.0 (no transparency; use if hair is not rendering)"
        };
        radio.getFn = [=](){return stN.val() ? 0U : 1U; };
        radio.setFn = [=](uint sel){stN.set(sel==0); };
        renderGroups.push_back(guiGroupbox("GPU Feature Level - restart program after selection",make_shared<GuiRadio>(radio)));
    }
    return guiSplitV(renderGroups);
}

namespace {

void                assignTri(      // Re-assign a tri (or quad) to a different surface if intersected
    NPT<RendMeshes>     rendMeshesN,
    NPT<size_t>         surfIdx,
    Vec2UI              winSize,
    Vec2I               pos,
    Mat44F              toOics)
{
    RendMeshes const &      rms = rendMeshesN.val();
    Opt<MeshesIntersect>    vpt = intersectMeshes(winSize,pos,toOics,rms);
    if (vpt.has_value()) {
        MeshesIntersect         isct = vpt.value();
        size_t                  dstSurfIdx = surfIdx.val();
        RendMesh const &        rm = rms[isct.meshIdx];
        Mesh *                  meshPtr = rm.origMeshN.valPtr();
        if (meshPtr) {
            if ((isct.surfIdx != dstSurfIdx) && (dstSurfIdx < meshPtr->surfaces.size())) {
                Surf &           srcSurf = meshPtr->surfaces[isct.surfIdx];
                Surf &           dstSurf = meshPtr->surfaces[dstSurfIdx];
                if (isct.surfPnt.triEquivIdx < srcSurf.tris.size()) {   // it's a tri
                    // Copy surface points on selected tri to destination surface:
                    for (SurfPointName sp : srcSurf.surfPoints) {
                        if (sp.point.triEquivIdx == isct.surfPnt.triEquivIdx) {
                            sp.point.triEquivIdx = uint(dstSurf.tris.size());
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
                else {                                                  // It's a quad
                    // Copy surface points on selected quad to destination surface:
                    size_t                  quadIdx = (isct.surfPnt.triEquivIdx - srcSurf.tris.size())/2;
                    for (SurfPointName sp : srcSurf.surfPoints) {
                        size_t              spQuadIdx2 = sp.point.triEquivIdx - srcSurf.tris.size(),
                                            spQuadIdx = spQuadIdx2 / 2,
                                            spQuadMod = spQuadIdx2 % 2;
                        if (spQuadIdx == quadIdx) {
                            size_t          idx = dstSurf.tris.size() + 2*dstSurf.quads.size() + spQuadMod;
                            sp.point.triEquivIdx = uint(idx);
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
void                assignPaint(
    NPT<RendMeshes>     rendMeshesN,
    NPT<size_t>         surfIdx,
    Vec2UI              winSize,
    Vec2I               pos,
    Mat44F              toD3ps)
{
    RendMeshes const &      rms = rendMeshesN.val();
    Opt<MeshesIntersect>    vpt = intersectMeshes(winSize,pos,toD3ps,rms);
    if (vpt.has_value()) {
        MeshesIntersect         isct = vpt.value();
        size_t                  dstSurfIdx = surfIdx.val();
        RendMesh const &        rm = rms[isct.meshIdx];
        Mesh *                  meshPtr = rm.origMeshN.valPtr();
        if (meshPtr) {
            if ((isct.surfIdx != dstSurfIdx) && (dstSurfIdx < meshPtr->surfaces.size())) {
                Arr3UI          tri = meshPtr->surfaces[isct.surfIdx].getTriEquivVertInds(isct.surfPnt.triEquivIdx);
                Surfs           surfs = splitByContiguous(meshPtr->surfaces[isct.surfIdx]);
                for (size_t ss=0; ss<surfs.size(); ++ss) {
                    if (contains(surfs[ss].tris.vertInds,tri)) {
                        merge_(meshPtr->surfaces[dstSurfIdx],surfs[ss]);
                        surfs.erase(surfs.begin()+ss);
                        Surf                tmp = merge(surfs);
                        // Copy the merged structure and surf point list, preserving name and material:
                        Surf &              surf = meshPtr->surfaces[isct.surfIdx];
                        surf.quads = tmp.quads;
                        surf.tris = tmp.tris;
                        surf.surfPoints = tmp.surfPoints;
                        return;
                    }
                }
            }
        }
    }
}

GuiPtr              cMeshEditPointsW(Gui3d & api)
{
    NPT<RendMeshes>     rendMeshesN = api.rendMeshesN;
    IPT<String8>        pointLabelN {};
    api.clickActions.at(0,1,1) = bind(markSurfacePoint,rendMeshesN,pointLabelN,_1,_2,_3);
    // left-click no modifiers:
    api.clickActions.at(0,0,0) = [rendMeshesN,pointLabelN](Vec2UI winSize,Vec2I screenPos,Mat44F worldToD3ps)
    {
        RendMeshes const &      rms = rendMeshesN.val();
        Opt<MeshesIntersect>    vpt = intersectMeshes(winSize,screenPos,worldToD3ps,rms);
        if (vpt.has_value()) {
            // Perform all calculations in the original mesh coordinates:
            MeshesIntersect const & pt = vpt.value();
            SurfPoint const &       sp = pt.surfPnt;
            FGASSERT(pt.meshIdx < rms.size());
            RendMesh const &        rm = rms[pt.meshIdx];
            Mesh const &            origMesh = rm.origMeshN.val();
            FGASSERT(pt.surfIdx < origMesh.surfaces.size());
            Surf const &            origSurf = origMesh.surfaces[pt.surfIdx];
            Vec3F                   origPos = origSurf.surfPointPos(origMesh.verts,sp);
            // Find the closest surface point name
            Min<double,string>      closest;
            for (Surf const & surf : origMesh.surfaces) {
                for (size_t ss=0; ss<surf.surfPoints.size(); ++ss) {
                    Vec3F               pos = surf.surfPointPos(origMesh.verts,ss);
                    double              dmag = cMagD(pos-origPos);
                    closest.update(dmag,surf.surfPoints[ss].label);
                }
            }
            if (closest.valid())
                pointLabelN.set(closest.val());
        }
    };
    auto                clearPointsFn = [rendMeshesN]()
    {
        RendMeshes const &  rms = rendMeshesN.val();
        for (RendMesh const & rm : rms) {
            Mesh *          meshPtr = rm.origMeshN.valPtr();
            if (meshPtr) {
                for (Surf & surf : meshPtr->surfaces)
                    surf.surfPoints.clear();
            }
        }
    };
    auto                makeListFn = [rendMeshesN]()
    {
        RendMeshes const &      rms = rendMeshesN.val();
        String8s                labels;
        for (RendMesh const & rm : rms) {
            Mesh const &            mesh = rm.origMeshN.val();
            for (Surf const & surf : mesh.surfaces)
                for (SurfPointName const & sp : surf.surfPoints)
                    labels.push_back(sp.label);
        }
        if (labels.empty())
            return guiText("No surface points");
        else
            return guiText(cat(labels,"\n"));
    };
    GuiPtr              textW = guiText(
        "View surface point name: left-click on point.\n"
        "Mark surface point: ctrl-shift-left-click on surface.\n"
        "A point with an identical non-null name will be overwritten"),
                        listW = guiDynamic(makeListFn,cUpdateFlagT(rendMeshesN)),
                        listWS = guiSplitScroll(Img<GuiPtr>{{1,1},{listW}});    // there can be many surface points
    GuiPtrs             subWs = {
        textW,
        guiSplitH({guiText("Name:"),guiTextEdit(pointLabelN)}),
        guiButton("Clear all surface points",clearPointsFn),
        guiGroupbox("Surface Points",listWS),
    };
    return guiSplitV(subWs);
}

GuiPtr              cMeshEditFacetsW(NPT<RendMeshes> rendMeshesN,IPT<size_t> surfChoiceN)
{
    RendMeshes const &      rms = rendMeshesN.val();
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

struct      MeshSurfsName
{
    String8        meshName;
    String8s       surfNames;
};
typedef Svec<MeshSurfsName>     MeshSurfsNames;

GuiPtr              cMeshEditVertsMarkW(NPT<RendMeshes> const & rendMeshesN,Gui3d & api)
{
    IPT<size_t>         vertSelModeN(0);
    api.clickActions.at(2,1,1) = bind(markMeshVertex,rendMeshesN,vertSelModeN,_1,_2,_3);
    auto                markAllFn = [rendMeshesN]()
    {
        RendMeshes const &      rms = rendMeshesN.val();
        for (RendMesh const & rm : rms) {
            Mesh *          meshPtr = rm.origMeshN.valPtr();
            if (meshPtr) {
                SurfTopo        topo {meshPtr->verts.size(),meshPtr->getTriEquivs().vertInds};
                auto                boundaries = topo.boundaries();
                for (auto const & boundary : boundaries)
                    for (auto const & be : boundary)
                        if (!contains(meshPtr->markedVerts,be.vertIdx))
                            meshPtr->markedVerts.emplace_back(be.vertIdx);
            }
        }
    };
    auto                clearAllFn = [rendMeshesN]()
    {
        RendMeshes const & rms = rendMeshesN.val();
        for (RendMesh const & rm : rms) {
            Mesh *          meshPtr = rm.origMeshN.valPtr();
            if (meshPtr)
                meshPtr->markedVerts.clear();
        }
    };
    String8s            vsmrLabels {"Single","Edge Seam","Fold Seam","Fill"};
    GuiPtr              vertSelModeW = guiRadio(vsmrLabels,vertSelModeN);
    GuiPtrs             subWs {
        guiText("Mark Vertex: ctrl-shift-right-click on surface near vertex"),
        guiGroupbox("Mode",vertSelModeW),
        guiButton("Mark all seam vertices",markAllFn),
        guiButton("Clear all marked vertices",clearAllFn),
    };
    return guiSplitV(subWs);
}

GuiPtr              cMeshEditVertsDelW(
    NPT<RendMeshes> const & rendMeshesN,
    Gui3d &                 api,
    String8 const &         store)
{
    IPT<bool>           delBackfacingN {true};
    GuiPtr              delBackfacingW = guiCheckbox("Delete back-facing polys",delBackfacingN);
    IPT<double>         delLnRadiusN = makeSavedIPT(-4.0,store+"delLnRadius");
    GuiPtr              delRadiusW = guiSlider(delLnRadiusN,"delete radius",{-6,-2},1);
    auto                delVertsFn = [rendMeshesN,delLnRadiusN,delBackfacingN]
                        (Vec2UI viewportSize,Vec2I posRcs,Mat44F toD3ps)
    {
        RendMeshes const &  rms = rendMeshesN.val();
        if (rms.size() == 1) {      // only support single mesh for now
            double              radius = cMaxElem(viewportSize) * exp(delLnRadiusN.val());
            AxAffine2F          d3psToRcs = cD3psToRcs(viewportSize);
            Vec2F               userRcs {posRcs};
            Uints               toRemove;
            RendMesh const &    rm = rms[0];
            Vec3Fs const &      verts = rm.shapeVertsN.val();
            MeshNormals const & norms = rm.normalsN.val();
            for (size_t vv=0; vv<verts.size(); ++vv) {
                Vec3F const &       vert = verts[vv];
                Vec4F               d3psH = toD3ps * append(vert,1.0f);
                if (d3psH[3] != 0.0f) {
                    Vec3F           d3ps = projectHomog(d3psH);
                    Vec2F           rcs = d3psToRcs * Vec2F{d3ps[0],d3ps[1]};
                    if (cLenD(rcs-userRcs) < radius) {
                        Vec4F           normH = toD3ps * append(norms.vert[vv],0.0f);
                        // A negative Z component is forward-facing.
                        // Including zero lets us catch verts not part of any surface (zero norm vec):
                        if ((normH[2] <= 0) || delBackfacingN.val())
                            toRemove.push_back(scast<uint>(vv));
                    }
                }
            }
            if (!toRemove.empty()) {
                Mesh *              origMeshPtr = rm.origMeshN.valPtr();
                if (origMeshPtr) {                      // if editing is possible
                    Mesh                origMesh = *origMeshPtr;
                    *origMeshPtr = removeVerts(origMesh,toRemove);
                }
            }
        }
    };
    api.shiftCtrlMiddleDragAction = delVertsFn;
    GuiPtr              delTextW =
        guiText("Delete Vertices: ctrl-shift-middle-click and drag on surface near vertices");
    return guiSplitV({delTextW,delBackfacingW,delRadiusW});
}

// Modifies 'api' to add it's hooks:
GuiPtr              cMeshEditW(Gui3d & api,String8 const & saveName,String8 const & store)
{
    NPT<RendMeshes>     rendMeshesN = api.rendMeshesN;
    GuiPtr              facetsW;
    {
        IPT<size_t>         surfChoiceN(0);
        api.shiftRightDragAction = bind(assignTri,rendMeshesN,surfChoiceN,_1,_2,_3);
        api.clickActions.at(1,1,0) = bind(assignPaint,rendMeshesN,surfChoiceN,_1,_2,_3);
        facetsW = guiDynamic(bind(cMeshEditFacetsW,rendMeshesN,surfChoiceN),cUpdateFlagT(api.rendMeshesN));
    }
    GuiTabDefs          editVertsTabs {
        {"Mark",cMeshEditVertsMarkW(rendMeshesN,api),true},
        {"Delete",cMeshEditVertsDelW(rendMeshesN,api,store+"VertsDel"),true},
    };
    GuiTabDefs          editTabs {
        {"Points",cMeshEditPointsW(api),true},
        {"Verts",guiTabs(editVertsTabs),true},
        {"Facets",facetsW,true},
    };
    GuiPtr              saveAsW;
    {
        auto                saveFn = [=]()
        {
            RendMeshes const &      rms = rendMeshesN.val();
            if (rms.empty())
                return;
            Meshes              meshes;
            for (RendMesh const & rm : rms)
                meshes.push_back(rm.origMeshN.val());
            saveMergeMesh(meshes,saveName);
        };
        auto                saveAsFn = [=](string format)
        {
            RendMeshes const &      rms = rendMeshesN.val();
            if (rms.empty())
                return;
            Opt<String8>            fname = guiDialogFileSave("FaceGen",format);
            if (fname.has_value()) {
                Meshes              meshes;
                for (RendMesh const & rm : rms)
                    meshes.push_back(rm.origMeshN.val());
                saveMergeMesh(meshes,fname.value());
            }
        };
        auto                reloadFn = [=]()
        {
            RendMeshes const &      rms = rendMeshesN.val();
            if (rms.empty())
                return;
            Mesh                    *ptr = rms[0].origMeshN.valPtr();
            if (ptr)
                *ptr = loadMesh(saveName);
        };
        GuiPtrs             buttons {
            guiText("Save pre-morphed as:"),
            guiButton("TRI",bind(saveAsFn,string("tri"))),
            guiButton("FGMESH",bind(saveAsFn,string("fgmesh"))),
        };
        if (!saveName.empty()) {
            buttons.insert(buttons.begin(),guiButton("reload original mesh",reloadFn));
            buttons.push_back(guiButton(saveName,saveFn,true));
        }
        saveAsW = guiSplitV(buttons);
    }
    return guiSplitV({guiTabs(editTabs),saveAsW});
}

}

GuiPtr              makeCameraCtrls(
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
    auto                    camPrmFn = [](Mat32D const & vb,QuaternionD const & p,Vec2D t,double lrs,double lfd)
    {
        CameraParams        r;
        r.modelBounds = vb;
        r.pose = p;
        r.relTrans = t;
        r.logRelScale = lrs;
        r.fovMaxDeg = lfd;
        return r;
    };
    OPT<CameraParams>       camParamsN = link5(viewBoundsN,api.pose,api.trans,api.logRelSize,lensFovDeg,camPrmFn);
    auto                    cameraFn = [](CameraParams const & cp,size_t m,double panDeg,double tiltDeg,Vec2UI v)
    {
        CameraParams        cps = cp;
        if (m == 0) {
            QuaternionD     pan {degToRad(panDeg),1},
                            tilt {degToRad(tiltDeg),0};
            cps.pose = pan*tilt;
        }
        return cps.camera(v);
    };
    api.cameraN = link5(camParamsN,api.panTiltMode,api.panDegrees,api.tiltDegrees,api.viewportDims,cameraFn);
    GuiPtr                  viewCtlText =
        guiText(
            "  Rotate: left-click-drag (or touch-drag)\n"
            "  Scale: right-click-drag up/down (or pinch-to-zoom)\n"
            "  Move: shift-left-click-drag (or middle-click-drag)");
    VecD2                   panTiltClip(-360.0,360.0);      // Values are wrapped by viewport impl
    GuiPtr                  viewCtlPanTiltText =
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
                {
                    GuiTickLabel(0.0,fgTr("Orthographic")),
                    GuiTickLabel(30.0,fgTr("Normal")),
                    GuiTickLabel(60.0,fgTr("Wide")),
                },
                30,     // Lots of room required for "Orthographic" at end of slider
                textEditBoxes
            )
        );
    IPT<double>             lnRelSizeN = api.logRelSize;
    auto                    getVal = [=](){return std::exp(lnRelSizeN.val()); };
    auto                    setVal = [=](double val){lnRelSizeN.set(std::log(val)); };
    GuiPtr                  viewCtlScaleTe = guiTextEditFloat(
        cUpdateFlagT(lnRelSizeN),getVal,setVal,VecD2{exp(-20.0),exp(20.0)},6);
    GuiPtr                  viewCtlScale = guiSlider(
                        api.logRelSize,
                        String8(),
                        logScaleRange,
                        (logScaleRange[1]-logScaleRange[0])/4.0,
                        {
                            GuiTickLabel(logScaleRange[0],"0.2"),
                            GuiTickLabel(0.0,"1.0"),
                            GuiTickLabel(logScaleRange[1],"5.0"),
                        },
                        vector<GuiTickLabel>(),
                        30      // Match width of slider above
                    );
    if (textEditBoxes)
        viewCtlScale = guiSplitH({viewCtlScale,viewCtlScaleTe});
    viewCtlScale = guiGroupboxTr("Object relative scale",viewCtlScale);
    DfNPtrs                resetDeps {
        {api.panDegrees.ptr},
        {api.tiltDegrees.ptr},
        {api.pose.ptr},
        {api.trans.ptr},
        {api.logRelSize.ptr},
        {lensFovDeg.ptr},
    };
    GuiPtr              viewCtlReset = guiButton("Reset Camera",[resetDeps](){setInputsToDefault(resetDeps);});
    if (simple == 2)
        return guiSplitV({viewCtlText,viewCtlFov,viewCtlScale,viewCtlReset});
    else
        return guiSplitV({viewCtlText,viewCtlRotation,viewCtlFov,viewCtlScale,viewCtlReset});
}

OPT<Vec3Fs>         linkAllVerts(NPT<Mesh> meshN)
{
    return link1(meshN,[](Mesh const & mesh){return mesh.allVerts();});
}

OPT<Mat32D>         linkMeshBounds(NPT<Mesh> const & meshN)
{
    auto                fn = [](Mesh const & mesh){return Mat32D{cBounds(mesh.verts)}; };
    return link1(meshN,fn);
}

OPT<MeshNormals>    linkMeshNormals(NPT<Mesh> const & meshN,NPT<Vec3Fs> const & shapeVertsN)
{
    auto            fn = [](Mesh const & mesh,Vec3Fs const & verts) {return cNormals(mesh.surfaces,verts); };
    return link2(meshN,shapeVertsN,fn);
}

OPT<Mesh>           linkLoadMesh(NPT<String8> pathBaseN)
{
    auto                loadFn = [](String8 const & pathBase)
    {
        if (pathBase.empty())
            return Mesh();                          // Deselected
        else if (pathExists(pathBase+".fgMesh"))
            return loadFgmesh(pathBase+".fgmesh");
        else
            return loadTri(pathBase+".tri");
    };
    return link1(pathBaseN,loadFn);
}

GuiMorphMeshes::GuiMorphMeshes() :
    morphCtrlsN {linkN(Svec<NPT<Mesh>>(),[](Meshes const & m){return cPoseDefs(m);})},
    // Don't save pose state as it is confusing, even to experienced users for instance when only a small
    // morph is left from a previous session so it appears to be mesh/identity issue:
    morphValMapN {}
{}

RendMesh            cRendMesh(
    NPT<Mesh> const &       meshN,
    NPT<Vec3Fs> const &     allVertsN,
    NPT<MorphValMap> const & morphValMapN,
    RendSurfs const &       rss)
{
    RendMesh            ret;
    ret.origMeshN = meshN;
    ret.shapeVertsN = link3(meshN,allVertsN,morphValMapN,applyMorphs);
    ret.normalsN = linkMeshNormals(meshN,ret.shapeVertsN);
    ret.rendSurfs = rss;
    return ret;
}

void                GuiMorphMeshes::addMesh(
        NPT<Mesh>           meshN,
        NPT<Vec3Fs>         allVertsN,
        ImgNs               smoothNs,
        ImgNs               specularNs,
        ImgNs               modulationNs)
{
    if (specularNs.empty())
        specularNs.resize(smoothNs.size(),makeIPT(ImgRgba8{}));
    if (modulationNs.empty())
        modulationNs.resize(smoothNs.size(),makeIPT(ImgRgba8{}));
    // We can't check against mesh surfaces at this time since it may not be selected:
    FGASSERT(smoothNs.size() == modulationNs.size());
    FGASSERT(smoothNs.size() == specularNs.size());
    addLink(meshN.ptr,morphCtrlsN.ptr);
    RendSurfs           rendSurfs;
    size_t              S = smoothNs.size();
    for (size_t ss=0; ss<S; ++ss)
        rendSurfs.emplace_back(smoothNs[ss],modulationNs[ss],specularNs[ss]);
    rendMeshes.push_back(cRendMesh(meshN,allVertsN,morphValMapN,rendSurfs));
}

void                GuiMorphMeshes::addMesh(NPT<Mesh> meshN,NPT<Vec3Fs> vertsN,size_t numSurfs)
{
    ImgNs       albedoNs = ImgNs(numSurfs,makeIPT(ImgRgba8{}));
    addMesh(meshN,vertsN,albedoNs);
}

// Called by GuiSplitScroll when 'labelsN' changes:
GuiPtr              cPoseControlW(NPT<MorphCtrls> morphCtrlsN,IPT<MorphValMap> morphValMapN,bool textEditBoxes)
{
    MorphCtrls const &        poseDefs = morphCtrlsN.val();
    MorphValMap &              morphVals = morphValMapN.ref();
    morphVals.clear();
    if (poseDefs.empty())
        return {guiText("\nThe currently selected models contain no expression morphs.")};
    for (MorphCtrl const & poseDef : poseDefs)
        morphVals[poseDef.name] = poseDef.neutral;
    GuiPtrs             buttonWs {
        guiButton("Set All Zero",[morphValMapN]()
        {
            // Do not alter labels, just values:
            for (auto & pv : morphValMapN.ref())
                pv.second = 0.0f;
        }),
        guiButton("Load File",[morphValMapN]()
        {
            Opt<String8>                fname = guiDialogFileLoad("TXT expression settings file",{"txt"});
            if (fname.has_value()) {
                Svec<MorphVal>               data = dsrlzText<Svec<MorphVal>>(loadRawString(fname.value()));
                MorphValMap &                  pvs = morphValMapN.ref();
                // Do not alter the labels, only update values where labels exist:
                for (MorphVal const & d : data) {
                    auto                    it = pvs.find(d.name);
                    if (it != pvs.end())
                        it->second = d.val;
                }
            }
        }),
        guiButton("Save File",[morphValMapN]()
        {
            Opt<String8>                fname = guiDialogFileSave("TXT expression settings file",{"txt"});
            if (fname.has_value()) {
                MorphValMap const &            pvs = morphValMapN.val();
                Svec<MorphVal>               data;
                for (auto const & pv : pvs)
                    data.push_back(MorphVal{pv.first,pv.second});
                saveRaw(srlzText(data),fname.value());
            }
        }),
    };
    GuiPtr              buttonsW = guiSplitV({guiSplitH(buttonWs),guiSpacer(0,7)});
    GuiPtrs             sliderFacsWs,
                        sliderCompWs;
    for (MorphCtrl const & pose : poseDefs) {
        String8 const &     name = pose.name;
        auto                getFn = [=]()
        {
            MorphValMap const &    morphVals = morphValMapN.val();
            auto                it = morphValMapN.val().find(name);
            FGASSERT(it != morphVals.end());
            return double(it->second);
        };
        auto                setFn = [=](double v){morphValMapN.ref()[name] = float(v); };
        GuiSlider           slider;
        slider.updateFlag = cUpdateFlagT(morphValMapN);
        slider.getValFn = getFn;
        slider.setValFn = setFn;
        slider.label = pose.name;
        slider.range[0] = pose.bounds[0];
        slider.range[1] = pose.bounds[1];
        slider.tickSpacing = 0.1;
        GuiPtr              sliderW;
        if (textEditBoxes) {
            auto            getVal = getFn;
            auto            setVal = setFn;
            GuiPtr          teW = guiTextEditFixed(cUpdateFlagT(morphValMapN),getVal,setVal,VecD2{-1,2},2);
            sliderW = guiSplitH({guiPtr(slider),teW});
        }
        else
            sliderW = guiPtr(slider);
        if (beginsWith(pose.name.m_str,"AU"))
            sliderFacsWs.push_back(sliderW);
        else
            sliderCompWs.push_back(sliderW);
    }
    if (sliderFacsWs.empty())
        return guiSplitV({buttonsW,guiSplitScroll(sliderCompWs)});
    if (sliderCompWs.empty())
        return guiSplitV({buttonsW,guiSplitScroll(sliderFacsWs)});
    // Two tabs of expressions:
    GuiTabDefs          sliderTs {
        {"FACS",guiSplitScroll(sliderFacsWs),true},
        {"Composite",guiSplitScroll(sliderCompWs),true},
    };
    return guiSplitV({buttonsW,guiTabs(sliderTs)});
}

GuiPtr              GuiMorphMeshes::makePoseCtrls(bool editBoxes) const
{
    auto                makeSliders =  bind(cPoseControlW,morphCtrlsN,morphValMapN,editBoxes);
    return guiDynamic(makeSliders,cUpdateFlagT(morphCtrlsN));
}

OPT<Mat32D>         linkBounds(RendMeshes const & rms)
{
    Svec<NPT<Mat32F>>       boundsNs;
    for (RendMesh const & rm : rms) {
        boundsNs.push_back(link1(rm.origMeshN,[](Mesh const & m){return cBounds(m.verts);}));
    }
    return linkN(boundsNs,[](vector<Mat32F> const & bs)
    {
        Mat32F    bounds = cBoundsUnion(bs);
        if (bounds[1] < bounds[0])      // No meshes selected
            bounds = Mat32F(0,1,0,1,0,1);
        return Mat32D(bounds);
    });
}

OPT<String8>        linkMeshStats(RendMeshes const & rms)
{
    Svec<NPT<String8>>      statsNs;
    auto                    statsFn = [](Mesh const & m)
    {
        ostringstream       oss;
        if (!m.verts.empty())
            oss << m;
        return String8{oss.str()};
    };
    for (RendMesh const & rm : rms)
        statsNs.push_back(link1(rm.origMeshN,statsFn));
    return linkN(statsNs,[](String8s const & strs){return cat(strs,"");});
}

GuiPtr              makeViewCtrls(Gui3d & gui3d,NPT<Mat32D> viewBoundsN,String8 const & store)
{
    GuiPtr          cameraW = makeCameraCtrls(gui3d,viewBoundsN,store+"Camera",0,true),
                    renderOptsW = makeRendCtrls(gui3d.renderOptions,gui3d.bgImg,store+"RendOpts",true,true,true),
                    lightingW = makeLightingCtrls(gui3d.lightingN,gui3d.bothButtonsDragAction,store+"Light");
    GuiTabDefs      viewTabs = {
        {"Camera",cameraW,true},
        {"Render",renderOptsW,true},
        {"Lighting",lightingW,true},
    };
    return guiTabs(viewTabs);
}

GuiPtrs             cGuiFileImageWs(NPT<Vec2UI> viewportDims,Sptr<Gui3d::Capture> const & capture,String8 const & store)
{
    IPT<double>         widN = makeSavedIPT(1024.0,store+"Width"),
                        hgtN = makeSavedIPT(1024.0,store+"Height");
    IPT<size_t>         szSelN = makeSavedIPTEub<size_t>(0,store+"Manual",2);
    IPT<bool>           bgTransN = makeSavedIPT<bool>(false,store+"Transparent");
    auto                fn = [](Vec2UI dims){return String8{"(" + toStr(dims[0]) + "," + toStr(dims[1]) + ")"}; };
    NPT<String8>        vpDimsN = link1(viewportDims,fn);
    GuiPtr              widW = guiTextEditFixed(widN,VecD2(256,4096),0),
                        hgtW = guiTextEditFixed(hgtN,VecD2(256,4096),0),
                        dimsLW = guiRadio({"Current render size","Custom"},szSelN),
                        dimsRUW = guiText(vpDimsN),
                        dimsRLW = guiSplitH({guiText("Width"),widW,guiText("Height"),hgtW}),
                        dimsRW = guiSplitV({guiSpacer(1,3),dimsRUW,guiSpacer(1,3),dimsRLW}),
                        dims3W = guiSplitH({dimsLW,dimsRW});
    ImgFormats          imgFormats = mapMember(getImgFormatsInfo(),&ImgFormatInfo::format);   // support all fmts
    GuiVal<ImgFormat>   imgFormat = guiImgFormatSelector(imgFormats,store+"ImgFormat");
    NPT<ImgFormat>      imgFormatN = imgFormat.valN;
    auto                save = [=]()
    {
        if (capture->func) {
            Vec2UI          dims = szSelN.val() ? Vec2UI(widN.val(),hgtN.val()) : viewportDims.val();
            String          imgExt = getImgFormatInfo(imgFormatN.val()).extensions.at(0);
            Opt<String8>    fname = guiDialogFileSave("",imgExt);
            if (fname.has_value())
                saveImage(capture->func(dims,bgTransN.val()),fname.value());
        }
    };
    return {
        guiGroupbox("Pixel size",dims3W),
        guiCheckbox("Transparent Background (PNG or TGA only)",bgTransN),
        guiGroupbox("Image Format",imgFormat.win),
        guiButton("Save render image to file",save),
    };
}

Mesh                viewMesh(Meshes const & meshes,bool compare,String8 const & saveName)
{
    FGASSERT(meshes.size() > 0);
    String8                 store = getDirUserAppDataLocalFaceGen({"SDK","viewMesh"});
    Mat32F                  viewBounds = cBounds(meshes);
    if (!isFinite(viewBounds))
        fgThrow("viewMesh: Mesh vertices contain invalid floating point values");
    IPT<Mat32D>             viewBoundsN = makeIPT(Mat32D{cBounds(meshes)});
    GuiMorphMeshes          gpms;
    Svec<IPT<Mesh>>         meshNs;
    String8s                meshNames = mapMember(meshes,&Mesh::name);
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        if (meshNames[mm].empty())                  // in case client didn't provide names (cmdViewMesh does)
            meshNames[mm] = toStrDigits(mm,2);
        Mesh const &        mesh = meshes[mm];
        try {mesh.validate(); }
        catch (FgException & e) {
            e.contexts.emplace_back("in mesh",mesh.name.m_str);
            guiDialogMessage("Warning",e.nativeMessage());
        }
        IPT<Mesh>           meshN = makeIPT(mesh);
        OPT<Vec3Fs>         allVertsN = link1(meshN,[](Mesh const & m){return m.allVerts(); });
        ImgNs               albedoNs,
                            specularNs;
        IPT<ImgRgba8>       emptyImageN = makeIPT(ImgRgba8());
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
    }
    IPT<RendMeshes>         rmsN {gpms.rendMeshes};
    NPT<RendMeshes>         rendMeshesN;
    GuiPtr                  meshSelect,
                            meshInfo;
    if (compare) {
        NPT<Bools>              selsN;
        IPT<String8s>           meshNamesN = makeIPT(meshNames);
        IPT<String8>            meshSelectN = makeIPT(meshNames[0]);
        IPT<size_t>             selMeshN(0);
        GuiPtr                  selMeshRadio = guiRadio(meshNames,selMeshN);
        size_t                  sz = meshNames.size();
        auto                    selFn = [sz](size_t const & idx)
        {
            Bools           ret(sz,false);
            if (idx < sz)
                ret[idx] = true;
            return ret;
        };
        selsN = link1(selMeshN,selFn);
        meshSelect = guiSplitScroll({selMeshRadio});
        auto                filterFn = [](RendMeshes const & rms,Bools const & sels)
        {
            return selectIf(rms,sels);
        };
        rendMeshesN = link2(rmsN,selsN,filterFn);
    }
    else {
        NPT<Bools>              selsN;
        Svec<IPT<bool>>         selectNs = genSvec<IPT<bool>>(meshNames.size(),[](size_t){return makeIPT<bool>(true); });
        auto                    flipFn = [selectNs]()
        {
            for (IPT<bool> const & selN : selectNs) {
                bool &          v = selN.ref();
                v = !v;
            }
        };
        meshSelect = guiSplitV({guiCheckboxes(meshNames,selectNs),guiButton("Flip Selections",flipFn)});
        selsN = linkCollate(selectNs);
        auto                filterFn = [](RendMeshes const & rms,Bools const & sels)
        {
            return selectIf(rms,sels);
        };
        rendMeshesN = link2(rmsN,selsN,filterFn);
    }
    Gui3d               gui3d {rendMeshesN};
    NPT<String8>        statsTextN = link1(gui3d.rendMeshesN,[](RendMeshes const & rms)
    {
        size_t              cnt = 0;
        ostringstream       oss;
        for (RendMesh const & rm : rms) {
            Mesh const &        m = rm.origMeshN.val();
            oss << fgnl << "Mesh " << cnt++ << fgpush << m << fgpop;
        }
        return String8{oss.str()};
    });
    GuiPtr              statsTextW = guiText(statsTextN);
    GuiTabDefs          mainTabs = {
        {"View",makeViewCtrls(gui3d,viewBoundsN,store+"View")},
        {"Morphs",gpms.makePoseCtrls(true),true},
        {"Select",meshSelect,true},
        {"Edit",cMeshEditW(gui3d,saveName,store+"Edit"),true},
        {"Info",guiSplitScroll({statsTextW}),true},
        {"System",guiTextLines(gui3d.gpuInfo,40,40,{true,true}),true},
    };
    guiStartImpl(
        makeIPT<String8>("FaceGen SDK viewMesh"),
        guiSplitAdj(true,make_shared<Gui3d>(gui3d),guiTabs(mainTabs)),
        store);
    return meshNs[0].val();
}

bool                guiPlaceSurfPoints_(Strings const & toPlace,Mesh & mesh)
{
    String8             store = getDirUserAppDataLocalFaceGen({"PlaceSurfPoints"});
    // the first 'toPlace.size()' steps are for placement, the last is review:
    IPT<Mesh>           meshN = makeIPT(mesh);
    IPT<MeshesIntersects> placedN;
    OPT<size_t>         stepN = link1(placedN,[](MeshesIntersects const & mis){return mis.size(); });
    GuiPtr              promptW,viewport;
    {
        auto                backFn = [meshN,placedN]()
        {
            MeshesIntersects &  mis = placedN.ref();
            if (!mis.empty()) {
                meshN.ref().surfaces[mis.back().surfIdx].surfPoints.pop_back();
                mis.pop_back();
            }
        };
        GuiPtrs             promptWs;
        String              txt0 = "Click on the ",
                            txt1 = "\n\n"
            "mark: left-click\n"
            "rotate: left-click-drag\n"
            "translate: middle-click-drag (or shift-left-drag)\n"
            "scale: right-click-drag";
        promptWs.push_back(guiText(txt0 + toPlace[0] + txt1));
        for (size_t ii=1; ii<toPlace.size(); ++ii) {
            GuiPtr              textW = guiText(txt0 + toPlace[ii] + txt1),
                                buttonW = guiButton("back",backFn);
            promptWs.push_back(guiSplitH({textW,buttonW}));
        }
        GuiPtr              textW = guiText("Review your points, close to finish"),
                            buttonW = guiButton("back",backFn);
        promptWs.push_back(guiSplitH({textW,buttonW}));
        promptW = guiSelect(stepN,promptWs);
    }
    {
        OPT<Mat32D>         boundsN = link1(meshN,[](Mesh const & mesh){return Mat32D(cBounds(mesh.verts)); });
        OPT<Vec3Fs>         vertsN = link1(meshN,[](Mesh const & mesh){return mesh.verts; });
        GuiMorphMeshes      gpms;
        auto                colorPtr = mesh.surfaces[0].material.albedoMap;
        if (colorPtr) {
            IPT<ImgRgba8>       colorN {*colorPtr};
            gpms.addMesh(meshN,vertsN,{colorN});
        }
        else
            gpms.addMesh(meshN,vertsN,1);
        Gui3d               gui3d {makeIPT(gpms.rendMeshes)};
        makeViewCtrls(gui3d,boundsN,store+"View");      // discard GUI but we need the setup for 'gui3d'
        NPT<RendMeshes>     rendMeshesN = gui3d.rendMeshesN;
        auto                clickFn = [meshN,placedN,toPlace,rendMeshesN](Vec2UI winSize,Vec2I pos,Mat44F worldToD3ps)
        {
            MeshesIntersects &  mis = placedN.ref();
            if (mis.size() < toPlace.size()) {
                RendMeshes const &      rms = rendMeshesN.val();
                Opt<MeshesIntersect>    vpt = intersectMeshes(winSize,pos,worldToD3ps,rms);
                if (vpt.has_value()) {
                    MeshesIntersect         pt = vpt.value();
                    FGASSERT(pt.meshIdx < rms.size());
                    RendMesh const &        rm = rms[pt.meshIdx];
                    Mesh *                  origMeshPtr = rm.origMeshN.valPtr();
                    FGASSERT(pt.surfIdx < origMeshPtr->surfaces.size());
                    Surf &                  surf = origMeshPtr->surfaces[pt.surfIdx];
                    surf.surfPoints.emplace_back(pt.surfPnt,toPlace[mis.size()]);
                    mis.push_back(pt);
                }
            }
        };
        gui3d.clickActions.at(0,0,0) = clickFn;     // place points by left-click
        viewport = make_shared<Gui3d>(gui3d);
    }
    IPT<String8>            titleN {"FaceGen Place Named Surface Points"};
    GuiPtr                  mainW = guiSplitV({promptW,viewport});
    guiStartImpl(titleN,mainW,store);
    if (stepN.val() == toPlace.size()) {
        mesh = meshN.val();
        return true;
    }
    return false;
}


}

// */
