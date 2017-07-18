//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Dec 18, 2009
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

struct  FgGuiLighting
{
    FgGuiPtr                win;
    FgDgn<FgLighting>       outN;
    FgMatrixC<FgDgn<double>,3,1>    dirAxisNs[2];   // Directional lighting axis inputs
};

FgGuiLighting
fgGuiLighting();

struct  FgBgImageCtrls
{
    FgGuiPtr                    win;
    uint                        changeFlag;
    FgGuiApiBgImage             api;
};

FgBgImageCtrls
fgBgImageCrls();

struct  FgRenderCtrls
{
    FgGuiPtr                    win;
    FgDgn<Fg3dRenderOptions>    optsN;
    FgGuiApiBgImage             bgImgApi;
};

FgRenderCtrls
fgRenderCtrls(
    // 0 - all controls and default marked points & verts to visible.
    // 1 - only color/shiny/flat
    // 2 - only color/shiny
    // 3 - only shiny/flat/wireframe
    uint    simple);

struct FgGui3dCtls
{
    FgGuiPtr            viewport;
    FgGuiPtr            cameraGui;
    FgGuiPtr            selectCtls;
    FgGuiPtr            morphCtls;
    FgGuiPtr            editCtls;
    FgGuiPtr            infoText;
    FgDgn<FgVertss>     morphedVertssN;     // Output
};

FgGui3dCtls
fgGui3dCtls(
    FgDgn<vector<Fg3dMesh> >    meshesN,            // Input
    FgDgn<FgVertss>             allVertssN,         // Input
    FgDgn<vector<FgImgs> >      texssN,             // Input
    FgDgn<FgMat32D>             viewBoundsN,        // Input
    FgRenderCtrls               renderCtrls,        // Inputs
    FgDgn<FgLighting>           lightingN,          // Input
    boost::function<void(bool,FgVect2I)>    bothButtonsDrag,
    // 0 - all render ctls, default marked points viewable, default unconstrained rotation
    // 1 - only color/shiny/flat/wireframe,
    // 2 - only color/shiny, and limit pan/tilt,
    // 3 - only shiny/flat/wireframe:
    uint                        simple=0,
    bool                        textEditBoxes=false);       // Display for sliders

// If only one mesh is provided then edit controls will also be available and the resulting mesh
// will be returned. Otherwise an empty mesh is returned:
Fg3dMesh
FgViewMeshes(
    const vector<Fg3dMesh> &    meshesOecs,
    bool                        compare=false);     // Radio button selects between meshes (if more than one)

inline
Fg3dMesh
fgViewMesh(const Fg3dMesh & mesh)
{return FgViewMeshes(fgSvec(mesh)); }

#endif
