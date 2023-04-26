//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPI3D_HPP
#define FGGUIAPI3D_HPP

#include "FgGuiApi.hpp"
#include "FgSerial.hpp"
#include "Fg3dCamera.hpp"
#include "FgLighting.hpp"
#include "Fg3dMesh.hpp"
#include "FgAny.hpp"
#include "FgMarr.hpp"

namespace Fg {

enum struct     AlbedoMode {
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

    DfgNPtrs                deps() const
    {
        return Svec<DfgNPtr>{imgN.ptr,offset.ptr,lnScale.ptr,foregroundTransparency.ptr};
    }
};

struct      RendSurf
{
    NPT<ImgRgba8>           smoothMapN;                 // albedo without detail texture. Image can be empty
    NPT<bool>               albedoHasTransparencyN;     // true only if smoothMap exists and has transparency
    NPT<ImgRgba8>           modulationMapN;             // modulates smoothMap. Image can be empty
    NPT<ImgRgba8>           specularMapN;               // Image can be empty
    // Note that on 3D window shutdown this data is destructed only after the GPU context is destructed,
    // so the GPU object must handle doing this in advance if desired (D3D is easy since you just call ClearState()).
    Sptr<Any>               gpuData = std::make_shared<Any>();
};
typedef Svec<RendSurf>  RendSurfs;

struct      RendMesh
{
    // The original mesh will be empty if this mesh is not currently selected:
    NPT<Mesh>               origMeshN;          // Should point to an Input if client wants mesh editing
    NPT<Vec3Fs>             posedVertsN;
    NPT<MeshNormals>        normalsN;
    Sptr<Any>               gpuData = std::make_shared<Any>();
    RendSurfs               rendSurfs;
};
typedef Svec<RendMesh>      RendMeshes;

Opt<MeshesIntersect> intersectMeshes(
    Vec2UI                  winSize,
    Vec2I                   pos,
    Mat44F                  worldToD3ps,
    RendMeshes const &      meshes);

// bool: is shift key down as well ? Vec2I: drag delta in pixels
typedef Sfun<void(bool,Vec2I)>          BothButtonsDragAction;
// Vec2UI: viewport size in pixels (X,Y)
// Vec2I: viewport position in raster coordinates (RCS) (at end of click or end of drag resp.)
// Mat44F: transform verts from world to D3PS
typedef Sfun<void(Vec2UI,Vec2I,Mat44F)> MouseAction;

AffineEw2F          cD3psToRcs(Vec2UI viewportSize);    // for handling MouseAction and MouseAction params

// This function must be defined in the corresponding OS-specific implementation:
struct      Gui3d;
GuiImplPtr          guiGetOsImpl(Gui3d const & guiApi);

struct      Gui3d : GuiBase
{
    // Unmodified sources. Must be defined unless labelled [opt]:
    NPT<RendMeshes>             rendMeshesN;
    NPT<size_t>                 panTiltMode;    // 0 - pan/tilt, 1 - unconstrained
    RPT<Lighting>               light;
    NPT<Camera>                 xform;
    RPT<RendOptions>            renderOptions;
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
    // bool: is shift key down as well ?
    Sfun<void(bool,VertIdx,Vec3F)>  ctlDragAction;          // Can be empty
    BothButtonsDragAction           bothButtonsDragAction;
    MouseAction                     shiftRightDragAction,
                                    shiftCtrlMiddleDragAction;
    // clickActions are called when the mouse button is released (otherwise they may yet be drag actions):
    Marr3<MouseAction,3,2,2>        clickActions;           // by button (LMR), shift (no/yes), ctrl (no/yes)
    // TODO: refactor to use this (problem is lastCtlClick also needs to track intersect from first click down):
    // by L button down (no/yes), M button (no/yes), R button (no/yes), shift (no/yes), ctrl (no/yes):
    // Marr5<MouseAction,2,2,2,2,2>    dragActions;

    // current design is to call 'makeViewControls' to complete setup of this object.
    // TODO: break that up into calls which are made first and results can be passed to a constructor:
    Gui3d(NPT<RendMeshes> rmN,bool tft=true) : rendMeshesN{rmN}, tryForTransparency{tft} {}

    virtual GuiImplPtr      getInstance() {return guiGetOsImpl(*this); }
    // Implementation:
    void                    panTilt(Vec2I delta);
    // Used by two-finger rotate gesture. Does nothing in pan-tilt mode:
    void                    roll(int delta);
    void                    scale(int delta);
    void                    translate(Vec2I delta);
    void                    ctlClick(Vec2UI winSize,Vec2I pos,Mat44F worldToD3ps);
    void                    ctlDrag(bool left, Vec2UI winSize,Vec2I delta,Mat44F worldToD3ps);
    void                    translateBgImage(Vec2UI winSize,Vec2I delta);
    void                    scaleBgImage(Vec2UI winSize,Vec2I delta);

private:
    VertIdx                 lastCtlClick;
};

// if 'pos' intersects a surface, it overwrites any surface point on that surface with the same
// name as 'pointLabelN', otherwise creates a new one:
void                markSurfacePoint(
    NPT<RendMeshes> const & rendMeshesN,
    NPT<String8> const &    pointLabelN,
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
