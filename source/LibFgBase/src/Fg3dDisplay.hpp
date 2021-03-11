//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Typical assignment of mouse controls in FaceGen:
//
//                      Lbutton         Rbutton         Mbutton         LRButtons
// --------------------------------------------------------------------------------
// click
// drag                 rotate          scale           translate       rot_light
//
// shift-click                                          asgnPaintTris
// shift-drag           translate       asgnTri
//
// ctrl-click
// ctrl-drag            deformS         deformA
//
// ctrl-shift-click     surf_pnt        sel_vert
// ctrl-shift-drag      bg_trans        bg_scale
//

#ifndef FG3DDISPLAY_HPP
#define FG3DDISPLAY_HPP

#include "Fg3dMesh.hpp"
#include "FgGuiApi3d.hpp"

namespace Fg {

GuiPtr
makeCameraCtrls(
    Gui3d &                 gui3d,
    NPT<Mat32D>    viewBoundsN,
    String8 const &        storePath,
    // 0 - all render ctls, default marked points viewable, default unconstrained rotation
    // 1 - only color/shiny/flat/wireframe,
    // 2 - only color/shiny, and limit pan/tilt,
    // 3 - only shiny/flat/wireframe:
    uint                        simple=0,
    bool                        textEditBoxes=false);       // Display for sliders

GuiPtr
makeRendCtrls(
    RPT<RendOptions>    rendOptionsR,
    BackgroundImage     bgImg,
    String8 const &     store,
    bool                structureOptions,       // color by mesh, wire, flat, allverts, facets
    bool                twoSidedOption,
    bool                pointOptions);          // surf points, marked verts

GuiPtr
makeLightingCtrls(
    RPT<Lighting>         lightingR,                    // Assigned
    BothButtonsDragAction & bothButtonsDragAction,      // Assigned
    String8 const &        storePath);

// View controls with all option selectors enabled:
GuiPtr
makeViewCtrls(Gui3d & gui3d,NPT<Mat32D> viewBoundsN,String8 const & storePath);

// Returns an image save dialog window to save from the given render capture function:
GuiPtr
guiCaptureSaveImage(
    NPT<Vec2UI>                     viewportDims,
    Sptr<Gui3d::Capture> const &    capture,
    String8 const &                 store);

OPT<Vec3Fs>
linkAllVerts(NPT<Mesh>);

OPT<Vec3Fs>
linkPosedVerts(
    NPT<Mesh>           meshN,          // input
    NPT<Vec3Fs>         allVertsN,      // input
    OPT<PoseDefs>       posesN,         // output as an additional source
    NPT<Doubles>        morphValsN);    // input as an additional sink

// Load from pathBase + '.tri'. TODO: Support '.fgmesh'.
OPT<Mesh>
linkLoadMesh(NPT<String8> pathBaseN);          // Empty filename -> empty mesh

OPT<MeshNormals>
linkNormals(const NPT<Mesh> & meshN,const NPT<Vec3Fs> & posedVertsN);  // mesh can be empty

OPT<ImgC4UC>
linkLoadImage(NPT<String8> filenameN);         // Empty filename -> empty image

typedef Svec<NPT<ImgC4UC> >   ImgNs;

struct  PoseVal
{
    String8             name;
    float               val;
};

typedef std::map<String8,float>     PoseVals;

struct  GuiPosedMeshes
{
    RendMeshes          rendMeshes;
    OPT<PoseDefs>       poseDefsN;          // Aggregates all unique pose labels
    IPT<PoseVals>       poseValsN;          // Only contains entries for currently instantiated pose sliders; others are forgotten.

    GuiPosedMeshes();

    void
    addMesh(
        NPT<Mesh>       meshN,          // Name, verts, maps not used. Editing controls if IPT.
        NPT<Vec3Fs>     allVertsN,
        // Albedo before any texture modulation. Must be 1-1 with meshN surfaces (num surfaces not yet dynamic:
        ImgNs           smoothNs,
        // Specular maps must be 1-1 if non-empty. Images can also be empty:
        ImgNs           specularNs=ImgNs{},
        // Optional texture modulation images must be 1-1 if non-empty. Images can also be empty:
        ImgNs           modulationNs=ImgNs{});

    // Add mesh with no maps. Number of surfaces is not yet dynamic so must be specified for static construction:
    void
    addMesh(NPT<Mesh>,NPT<Vec3Fs>,size_t numSurfs);

    GuiPtr
    makePoseCtrls(bool editBoxes) const;
};

OPT<Mat32D>
linkBounds(RendMeshes const &);

// Text of the (non-empty) mesh statstics:
OPT<String8>
linkMeshStats(RendMeshes const &);

// If only one mesh is provided then edit controls will also be available and the resulting mesh
// will be returned. Otherwise an empty mesh is returned:
Mesh
viewMesh(
    Meshes const &      meshesOecs,         // Assign the 'name' field if desired for mesh selection
    bool                compare=false);     // Radio button selects between meshes (if more than one)

inline
Mesh
viewMesh(Mesh const & mesh)
{return viewMesh(Svec<Mesh>(1,mesh)); }

// GUI queries user for any given fids not already labelled surface 0 points.
// Returns true if user enters all fids, false if user closes before entering all fids:
bool
guiSelectFids(
    Mesh &              mesh,
    Strings const &     fidLabels,
    // 0 - simple, 1 - expert, 2 - edit.
    // Simple prompts for each fiducial separately and allows multiple tries per fiducial.
    // Expert prompts only once and expects all fiducials to be placed without repeats.
    // Edit allows changing existing fiducials.
    uint                mode=0);

}

#endif
