//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPI3D_HPP
#define FGGUIAPI3D_HPP

#include "FgGuiApiBase.hpp"
#include "FgStdExtensions.hpp"
#include "Fg3dCamera.hpp"
#include "FgLighting.hpp"
#include "Fg3dNormals.hpp"
#include "FgAny.hpp"

namespace Fg {

struct  RendOptions
{
    bool            facets;
    bool            useTexture;
    bool            shiny;
    bool            wireframe;
    bool            flatShaded;
    bool            surfPoints;
    bool            markedVerts;
    bool            allVerts;
    bool            twoSided;
    Vec3F           backgroundColor;    // Elements in [0,1]
    // Set by render impl not by client:
    bool            colorBySurface=false;
    bool            showAxes=false;
};

struct  MeshesIntersect
{
    size_t              meshIdx;
    size_t              surfIdx;
    SurfPoint           surfPnt;
};

struct  BackgroundImage
{
    // User-selected background image. Empty means none selected and ignore all values below. Should have alpha=1:
    IPT<ImgC4UC>            imgN;
    // Original image dimensions must be known to get correct aspect ratio for display:
    IPT<Vec2UI>             origDimsN;
    // These inputs are needed for mouse+keyboard controls.
    // Offset units are relative to BG image max dimension before scaling:
    IPT<Vec2F>              offset;
    IPT<double>             lnScale;    // Log units relative to filling render window
    IPT<double>             foregroundTransparency;     // 0 - foreground opaque, 1 - foreground invisible

    DfgNPtrs
    deps() const
    {return svec<DfgNPtr>(imgN.ptr,offset.ptr,lnScale.ptr,foregroundTransparency.ptr); }
};

struct  RendSurf
{
    NPT<ImgC4UC>            albedoMap;                  // Image can be empty
    DfgFPtr                 albedoMapFlag;
    NPT<bool>               albedoHasTransparencyN;     // False if image is empty
    NPT<ImgC4UC>            specularMap;                // Image can be empty
    DfgFPtr                 specularMapFlag;
    // Can't use std::any here since 'gpuData' uses unique_ptr which can't be copy-constructed.
    // The shared_ptr used for Any's reference semantics is overkill (we only ever want one copy)
    // so one could create a no-copy-constructor version of std::any and use that instead:
    Sptr<Any>               gpuData = std::make_shared<Any>();
};
typedef Svec<RendSurf>  RendSurfs;

struct  RendMesh
{
    // The original mesh will be empty if this mesh is not currently selected:
    NPT<Mesh>               origMeshN;          // Should point to an Input if client wants mesh editing
    NPT<Vec3Fs>             posedVertsN;
    DfgFPtr                 surfVertsFlag;      // Must point to 'posedVertsN'
    DfgFPtr                 allVertsFlag;       // "
    NPT<Normals>            normalsN;
    Sptr<Any>               gpuData = std::make_shared<Any>();
    RendSurfs               rendSurfs;
};
typedef Svec<RendMesh>      RendMeshes;

Opt<MeshesIntersect>
intersectMeshes(
    Vec2UI                  winSize,
    Vec2I                   pos,
    Mat44F                  worldToD3ps,
    RendMeshes const &      meshes);

// bool: is shift key down as well ? Vec2I: drag delta in pixels
typedef std::function<void(bool,Vec2I)>                  BothButtonsDragAction;
// Vec2UI: viewport size
// Vec2I: final position
// Mat44F: transform verts to OICS
typedef std::function<void(Vec2UI,Vec2I,Mat44F)>    ShiftRightDragAction;
// Vec2UI: viewport size
// Vec2I: click position
// Mat44F: transform verts to OICS
typedef std::function<void(Vec2UI,Vec2I,Mat44F)>    CtrlShiftMiddleClickAction;

// This function must be defined in the corresponding OS-specific implementation:
struct  Gui3d;
GuiImplPtr guiGetOsImpl(Gui3d const & guiApi);

struct  Gui3d : GuiBase
{
    // Unmodified sources. Must be defined unless labelled [opt]:
    NPT<RendMeshes>             rendMeshesN;
    NPT<size_t>                 panTiltMode;    // 0 - pan/tilt, 1 - unconstrained
    RPT<Lighting>               light;
    NPT<Camera>                 xform;
    RPT<RendOptions>            renderOptions;
    IPT<bool>                   colorBySurface {false}; // Render each surface within a mesh a different color
    NPT<size_t>                 vertMarkModeN;  // 0 - single, 1 - edge seam, 2 - fold seam
    NPT<Ustring>                pointLabel;     // Label for surface point creation
    bool                        panTiltLimits = false;  // Limit pan and tilt to +/- 90 degrees (default false)
    BackgroundImage             bgImg;
    struct  Capture {
        Sfun<ImgC4UC(Vec2UI)>   getImg;
    };
    // Will contain the functions for capturing current render once the GPU is set up:
    Sptr<Capture> const         capture = std::make_shared<Capture>();

    // Modified (by mouse/keyboard commands processed by viewport):
    IPT<double>                 panDegrees;
    IPT<double>                 tiltDegrees;
    IPT<QuaternionD>            pose;
    IPT<Vec2D>                  trans;
    IPT<double>                 logRelSize;
    IPT<Vec2UI>                 viewportDims;
    IPT<Ustring>                gpuInfo;

    // Actions:
    struct  VertIdx
    {
        bool        valid=false;
        size_t      meshIdx;
        size_t      vertIdx;
    };
    // WARNING: drag actions are subject to click actions before the drag since click actions
    // currently take effect on button down, not up:
    // bool: is shift key down as well ?
    Sfun<void(bool,VertIdx,Vec3F)>  ctlDragAction;          // Can be empty
    IPT<BothButtonsDragAction>      bothButtonsDragActionI;
    IPT<ShiftRightDragAction>       shiftRightDragActionI;
    IPT<CtrlShiftMiddleClickAction> shiftMiddleClickActionI;

    Gui3d() {}

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }

    // Implementation:
    void
    panTilt(Vec2I delta);

    // Used by two-finger rotate gesture. Does nothing in pan-tilt mode:
    void
    roll(int delta);

    void
    scale(int delta);

    void
    translate(Vec2I delta);

    void
    markSurfPoint(Vec2UI winSize,Vec2I pos,Mat44F worldToD3ps);

    void
    markVertex(Vec2UI winSize,Vec2I pos,Mat44F worldToD3ps);

    void
    ctlClick(Vec2UI winSize,Vec2I pos,Mat44F worldToD3ps);

    void
    ctlDrag(bool left, Vec2UI winSize,Vec2I delta,Mat44F worldToD3ps);

    void
    ctrlShiftLeftDrag(Vec2UI winSize,Vec2I delta);

    void
    ctrlShiftRightDrag(Vec2UI winSize,Vec2I delta);

private:
    VertIdx             lastCtlClick;
};

}

#endif
