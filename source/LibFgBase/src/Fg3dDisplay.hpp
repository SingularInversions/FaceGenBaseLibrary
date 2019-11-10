//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Typical assignment of mouse controls in FaceGen:
//
//                  Lbutton     Rbutton     Mbutton     LRbuttons
// --------------------------------------------------------------
// click
// drag             rotate      scale       translate   rot_light
// Shift-click
// Shift-drag       translate   sel_tri
// Ctl-click
// Ctl-drag         deformS     deformA
// Shift-Ctl-click  surf_pnt    sel_vert
// Shift-Ctl-drag   bg_trans    bg_scale

#ifndef FG3DDISPLAY_HPP
#define FG3DDISPLAY_HPP

#include "Fg3dMesh.hpp"
#include "FgGuiApi3d.hpp"

namespace Fg {

GuiPtr
makeCameraCtrls(
    Gui3d &                 gui3d,
    NPT<Mat32D>    viewBoundsN,
    const Ustring &        storePath,
    // 0 - all render ctls, default marked points viewable, default unconstrained rotation
    // 1 - only color/shiny/flat/wireframe,
    // 2 - only color/shiny, and limit pan/tilt,
    // 3 - only shiny/flat/wireframe:
    uint                        simple=0,
    bool                        textEditBoxes=false);       // Display for sliders

GuiPtr
makeRendCtrls(
    RPT<RendOptions>          rendOptionsR,
    // 0 - all controls and default marked points & verts to visible.
    // 1 - only color/shiny/flat
    // 2 - only color/shiny
    // 3 - only shiny/flat/wireframe
    uint    simple,
    const Ustring & storePath);

GuiPtr
makeLightingCtrls(
    RPT<FgLighting>         lightingR,                              // Assigned
    IPT<BothButtonsDragAction> bothButtonsDragActionI,    // "
    const Ustring &        storePath);

GuiPtr
makeViewCtrls(
    Gui3d &                 gui3d,
    NPT<Mat32D>    viewBoundsN,
    const Ustring &        storePath,
    // 0 - all render ctls, default marked points viewable, default unconstrained rotation
    // 1 - only color/shiny/flat/wireframe,
    // 2 - only color/shiny, and limit pan/tilt,
    // 3 - only shiny/flat/wireframe:
    uint                        simple=0,
    bool                        textEditBoxes=false);       // Display for sliders

// Returns an image save dialog window to save from the given render capture function:
GuiPtr
guiCaptureSaveImage(Sptr<Gui3d::Capture> const & capture);

OPT<Vec3Fs>
linkAllVerts(NPT<Mesh>);

OPT<Vec3Fs>
linkPosedVerts(
    NPT<Mesh>           meshN,          // input
    NPT<Vec3Fs>         allVertsN,      // input
    OPT<PoseVals>       posesN,         // output as an additional source
    NPT<Doubles>        morphValsN);    // input as an additional sink

// Load from pathBase + '.tri'. TODO: Support '.fgmesh'.
OPT<Mesh>
linkLoadMesh(NPT<Ustring> pathBaseN);          // Empty filename -> empty mesh

OPT<Normals>
linkNormals(const NPT<Mesh> & meshN,const NPT<Vec3Fs> & posedVertsN);  // mesh can be empty

OPT<ImgC4UC>
linkLoadImage(NPT<Ustring> filenameN);         // Empty filename -> empty image

typedef Svec<NPT<ImgC4UC> >   ImgNs;

struct  GuiPosedMeshes
{
    RendMeshes          rendMeshes;
    OPT<PoseVals>       poseLabelsN;        // Aggregates all unique pose labels
    IPT<Doubles>        poseValsN;          // Use to set pose coefficients

    GuiPosedMeshes();

    void
    addMesh(
        NPT<Mesh>       meshN,          // Name, verts, maps not used. Editing controls if IPT.
        NPT<Vec3Fs>     allVertsN,
        ImgNs           albedoNs,       // Must be 1-1 with meshN surfaces but can be empty (and the same)
        ImgNs           specularNs);    // "

    // As above with no maps:
    void
    addMesh(NPT<Mesh>,NPT<Vec3Fs>);

    GuiPtr
    makePoseCtrls(bool editBoxes) const;
};

OPT<Mat32D>
linkBounds(RendMeshes const &);

// Text of the (non-empty) mesh statstics:
OPT<Ustring>
linkMeshStats(RendMeshes const &);

// If only one mesh is provided then edit controls will also be available and the resulting mesh
// will be returned. Otherwise an empty mesh is returned:
Mesh
meshView(
    const Meshes &      meshesOecs,         // Assign the 'name' field if desired for mesh selection
    bool                compare=false);     // Radio button selects between meshes (if more than one)

inline
Mesh
meshView(const Mesh & mesh)
{return meshView(fgSvec(mesh)); }

}

#endif
