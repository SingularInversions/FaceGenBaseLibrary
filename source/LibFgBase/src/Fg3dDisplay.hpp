//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Dec 18, 2009
//

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

struct  FgRenderCtrls
{
    FgGuiPtr                    win;
    FgDgn<Fg3dRenderOptions>    optsN;
    FgDgn<FgImgRgbaUb>          bgImgN;
    FgDgn<FgVect2UI>            bgImgDimsN;     // Dimensions of original BG image.
};

FgRenderCtrls
fgRenderCtrls(
    // 1 - only color/shiny/flat
    // 2 - only color/shiny
    // 3 - only shiny/flat/wireframe
    uint    simple);

struct FgGui3dCtls
{
    FgGuiPtr        viewport;
    FgGuiPtr        cameraGui;
    FgGuiPtr        selectCtls;
    FgGuiPtr        morphCtls;
    FgGuiPtr        editCtls;
    FgGuiPtr        infoText;
    FgDgn<vector<FgVerts> >     morphedVertssN;     // Output
};

FgGui3dCtls
fgGui3dCtls(
    FgDgn<vector<Fg3dMesh> >    meshesN,            // Input
    FgDgn<vector<FgVerts> >     allVertssN,         // Input
    FgDgn<vector<FgImgs> >      texssN,             // Input
    FgDgn<FgMat32D>             viewBoundsN,        // Input
    FgRenderCtrls               renderCtrls,        // Inputs
    FgDgn<FgLighting>           lightingN,          // Input
    boost::function<void(bool,FgVect2I)>    bothButtonsDrag,
    // 1 - only color/shiny/flat/wireframe, 2 - only color/shiny, and limit pan/tilt,
    // 3 - only shiny/flat/wireframe:
    uint                        simple=0);

// The compare option gives radio buttons for mesh selection rather than checkboxes
void
fgDisplayMeshes(const vector<Fg3dMesh> & meshesOecs,bool compare=false);

inline
void
fgDisplayMesh(const Fg3dMesh & mesh)
{fgDisplayMeshes(fgSvec(mesh)); }

Fg3dMesh
fgDisplayForEdit(const Fg3dMesh & mesh);

#endif
