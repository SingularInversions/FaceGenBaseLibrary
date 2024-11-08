//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPI3D_HPP
#define FGGUIAPI3D_HPP

#include "FgGuiApi.hpp"
#include "FgSerial.hpp"
#include "FgCamera.hpp"
#include "Fg3dMesh.hpp"
#include "FgAny.hpp"
#include "FgMarr.hpp"

namespace Fg {

enum struct         AlbedoMode {
    map    = 0,
    none   = 1,
    byMesh = 2,
    bySurf = 3,
};
String8s            cAlbedoModeLabels();

struct      RendOptions
{
    bool            facets;
    AlbedoMode      albedoMode = AlbedoMode::map;
    bool            shiny;
    bool            wireframe;
    bool            flatShaded;
    bool            surfPoints;
    bool            markedVerts;
    bool            allVerts;
    bool            twoSided;
    Vec3F           backgroundColor;    // Elements in [0,1]
    bool            showAxes=false;
};

struct      MeshesIntersect
{
    size_t              meshIdx;
    size_t              surfIdx;
    SurfPoint           surfPnt;
};
typedef Svec<MeshesIntersect>   MeshesIntersects;

struct      BackgroundImage
{
    // User-selected background image. Empty means none selected and ignore all values below. Should have alpha=1:
    IPT<ImgRgba8>           imgN;
    // Original image dimensions must be known to get correct aspect ratio for display:
    IPT<Vec2UI>             origDimsN;
    // These inputs are needed for mouse+keyboard controls.
    // Offset units are relative to BG image max dimension before scaling:
    IPT<Vec2F>              offset;
    IPT<double>             lnScale;    // Log units relative to filling render window
    IPT<double>             foregroundTransparency;     // 0 - foreground opaque, 1 - foreground invisible

    DfNPtrs                deps() const
    {
        return Svec<DfNPtr>{imgN.ptr,offset.ptr,lnScale.ptr,foregroundTransparency.ptr};
    }
};

// the renderer assumes no map if either the NPT is empty, or it it contains an empty image:
struct      RendSurf
{
    NPT<ImgRgba8>           smoothMapN;                 // albedo without detail texture
    NPT<bool>               albedoHasTransparencyN;     // true only if smoothMap exists and has transparency
    NPT<ImgRgba8>           modulationMapN;             // modulates smoothMap
    NPT<ImgRgba8>           specularMapN;
    // Note that on 3D window shutdown this data is destructed only after the GPU context is destructed,
    // so the GPU object must handle doing this in advance if desired (D3D is easy since you just call ClearState()).
    Sptr<Any>               gpuData = std::make_shared<Any>();

    // ctors set up 'hasTransparency' and make zero-size images for any not provided:
    RendSurf() {}
    RendSurf(NPT<ImgRgba8> const & s);
    RendSurf(NPT<ImgRgba8> const & s,NPT<ImgRgba8> const & m,NPT<ImgRgba8> const & p);
};
typedef Svec<RendSurf>  RendSurfs;

struct      RendMesh
{
    // The original mesh will be empty if this mesh is not currently selected:
    NPT<Mesh>               origMeshN;          // Should point to an Input if client wants mesh editing
    NPT<Vec3Fs>             shapeVertsN;        // after application of morphs etc.
    NPT<MeshNormals>        normalsN;
    RendSurfs               rendSurfs;          // must be 1-1 with origMesh.surfaces since GPU needs to store surf data there
    Sptr<Any>               gpuData = std::make_shared<Any>();
};
typedef Svec<RendMesh>      RendMeshes;

struct      MeshesIsectPoint
{
    MeshesIntersect         isect;
    Vec3F                   pos;    // world CS. Need to capture this at click time since shape can change
};

struct      LastClick
{
    // no need to store viewport size since that can't change without invalidating last click info
    Vec2I                   pos;            // cursor position in viewport (IRCS) at click
    Mat44F                  xform;          // transform from world coordinates to D3PS
    // while the following is derived from the above it needs to be cached to avoid an expensive
    // recalculation while dragging, and also because the object shape may be changing (eg. ctrl-drag):
    Opt<MeshesIsectPoint>   isect;          // surface point intersected by click (if any)
};

Opt<MeshesIsectPoint> intersectMeshesPoint(Vec2UI winSize,Vec2I pos,Mat44F worldToD3ps,RendMeshes const & meshes);
Opt<MeshesIntersect> intersectMeshes(Vec2UI winSize,Vec2I pos,Mat44F worldToD3ps,RendMeshes const & meshes);

typedef Sfun<void(
    bool,           // is the shift key down as well ?
    Vec2I)>         // viewport drag delta in RCS
    BothButtonsDragAction;

typedef Sfun<void(
    Vec2UI,         // viewport size in pixels (X,Y)
    Vec2I,          // viewport position of click in raster coordinates (RCS)
    Mat44F)>        // transform verts from world to D3PS
    ClickAction;

typedef Sfun<void(
    Vec2UI,         // viewport size in pixels (X,Y)
    Vec2I,          // viewport drag delta in RCS
    Mat44F,         // transform verts from world to D3PS
    LastClick)>     // information about the mouse down event before this drag (the first in LMR order if more than one)
    DragAction;

AxAffine2F          cD3psToRcs(Vec2UI viewportSize);    // for handling ClickAction and ClickAction params

// This function must be defined in the corresponding OS-specific implementation:
struct      Gui3d;
GuiImplPtr          guiGetOsImpl(Gui3d const & guiApi);

struct      Gui3d : GuiBase
{
    // Unmodified sources. Must be defined unless labelled [opt]:
    NPT<RendMeshes>             rendMeshesN;
    NPT<size_t>                 panTiltMode;    // 0 - pan/tilt, 1 - unconstrained
    RPT<Lighting>               lightingN;
    NPT<Camera>                 cameraN;
    RPT<RendOptions>            renderOptions;
    NPT<double>                 texModStrengthN = makeIPT(1.0);     // defaults to an input
    bool                        panTiltLimits = false;  // Limit pan and tilt to +/- 90 degrees (default false)
    BackgroundImage             bgImg;
    struct      Capture {       // Put in struct for easier syntax to call a std::function
        // First argument is desired pixel size, second is background transparency option:
        Sfun<ImgRgba8(Vec2UI,bool)>   func;
    };
    // Will contain the functions for capturing current render once the GPU is set up:
    Sptr<Capture> const         capture = std::make_shared<Capture>();
    // If defined, will be called with the filename when the user drops it on the 3D window:
    Sfun<void(String8 const &)> fileDragDrop;
    bool                        tryForTransparency = true;  // set to false to force D3D 11.0

    // Modified (by mouse/keyboard commands processed by viewport):
    IPT<double>                 panDegrees;
    IPT<double>                 tiltDegrees;
    IPT<QuaternionD>            pose;
    IPT<Vec2D>                  trans;
    IPT<double>                 logRelSize;
    IPT<Vec2UI>                 viewportDims;
    IPT<String8>                gpuInfo;

    // Mouse-Keyboard Actions:
    struct      VertIdx
    {
        bool        valid=false;
        size_t      meshIdx;
        size_t      vertIdx;
    };
    // bool: is shift key down as well ? Vec3F: delta in HCS
    Sfun<void(bool,VertIdx,Vec3F)>  ctlDragAction;          // Can be empty
    BothButtonsDragAction           bothButtonsDragAction;
    ClickAction                     shiftRightDragAction,
                                    shiftCtrlMiddleDragAction;
    // clickActions are called when the mouse button is released (otherwise they may yet be drag actions):
    D3Arr<ClickAction,3,2,2>        clickActions;           // by button (LMR), shift (no/yes), ctrl (no/yes)
    // track information about the last mouse button down messages in case needed for subsequent drag motions.
    // Reset to invalid when the mouse button goes back up, so we can ignore button-down mouse moves in which
    // the button was pushed down outside the client area:
    Arr<Opt<LastClick>,3>           lastButtonDown;     // left, middle, right
    // by L button down (no/yes), M button (no/yes), R button (no/yes), shift (no/yes), ctrl (no/yes):
    D5Arr<DragAction,2,2,2,2,2>     dragActions;

    // current design is to call 'makeViewControls' to complete setup of this object.
    // TODO: break that up into calls which are made first and results can be passed to a constructor.
    // Sets up some defaults for simple use cases:
    Gui3d(NPT<RendMeshes> rmN,bool tft=true);

    virtual GuiImplPtr      getInstance() {return guiGetOsImpl(*this); }
    // Implementation:
    void                    panTilt(Vec2I delta);
    // Used by two-finger rotate gesture. Does nothing in pan-tilt mode:
    void                    roll(int delta);
    void                    scale(int delta);
    void                    translate(Vec2I delta);
    void                    buttonDown(size_t buttonIdx,Vec2UI winSz,Vec2I pos,Mat44F worldToD3ps);
    void                    ctlDrag(bool left, Vec2UI winSize,Vec2I delta,Mat44F worldToD3ps);
    void                    translateBgImage(Vec2UI winSize,Vec2I delta);
    void                    scaleBgImage(Vec2UI winSize,Vec2I delta);
};

// if 'pos' intersects a surface, it overwrites any surface point on that surface with the same
// name as 'pointLabelN', otherwise creates a new one:
void                markSurfacePoint(
    NPT<RendMeshes> const & rendMeshesN,
    NPT<String8> const &    pointLabelN,    // if empty string, no surface point will be marked
    Vec2UI                  winSize,
    Vec2I                   pos,
    Mat44F                  worldToD3ps);

void                markMeshVertex(
    NPT<RendMeshes> const & rendMeshesN,
    NPT<size_t> const &     vertMarkModeN,
    Vec2UI                  winSize,
    Vec2I                   pos,
    Mat44F                  worldToD3ps);

}

#endif
