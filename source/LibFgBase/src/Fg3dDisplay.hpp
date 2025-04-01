//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Typical assignment of mouse controls in FaceGen:
//
//                      Lbutton         Rbutton         Mbutton         LRButtons
// --------------------------------------------------------------------------------
// click                showSrfPnt                                      ---
// drag                 rotate          scale           translate       rot_light
//
// shift-click                                          asgnPaintTris   ---
// shift-drag           translate       asgnTri
//
// ctrl-click                                                           ---
// ctrl-drag            deformS         deformA
//
// ctrl-shift-click     surf_pnt        sel_vert                        ---
// ctrl-shift-drag      bg_trans        bg_scale        deleteVerts
//

#ifndef FG3DDISPLAY_HPP
#define FG3DDISPLAY_HPP

#include "Fg3dMesh.hpp"
#include "FgGuiApi3d.hpp"

namespace Fg {

GuiPtr              makeCameraCtrls(
    Gui3d &                 gui3d,
    NPT<Mat32D>             viewBoundsN,
    String8 const &         storePath,
    // 0 - all render ctls, default marked points viewable, default unconstrained rotation
    // 1 - only color/shiny/flat/wireframe,
    // 2 - only color/shiny, and limit pan/tilt,
    // 3 - only shiny/flat/wireframe:
    uint                    simple=0,
    bool                    textEditBoxes=false);       // Display for sliders

GuiPtr              makeRendCtrls(
    RPT<RendOptions>    rendOptionsR,
    BackgroundImage     bgImg,
    String8 const &     store,
    bool                structureOptions,           // color by mesh, wire, flat, allverts, facets
    bool                twoSidedOption,
    bool                pointOptions,               // surf points, marked verts
    IPT<bool> *         supportTransparencyNPtr=nullptr);

GuiPtr              makeLightingCtrls(
    RPT<Lighting>           lightingR,                  // Assigned
    BothButtonsDragAction & bothButtonsDragAction,      // Assigned
    String8 const &         storePath);

// View controls with all option selectors enabled:
GuiPtr              makeViewCtrls(Gui3d & gui3d,NPT<Mat32D> viewBoundsN,String8 const & storePath);
// Returns sub-wins of image save dialog window to save from the given render capture function:
GuiPtrs             cGuiFileImageWs(NPT<Vec2UI> viewportDims,Sptr<Gui3d::Capture> const & capture,String8 const & store);

OPT<Vec3Fs>         linkAllVerts(NPT<Mesh>);
// Load from pathBase + '.tri'. TODO: Support '.fgmesh'.
OPT<Mesh>           linkLoadMesh(NPT<String8> pathBaseN);   // Empty filename -> empty mesh
OPT<Mat32D>         linkMeshBounds(NPT<Mesh> const &);      // returns [max,lowest] if mesh has no verts (for composition)
OPT<SurfNormals>    linkMeshNormals(const NPT<Mesh> & meshN,const NPT<Vec3Fs> & shapeVertsN);  // mesh can be empty

typedef Svec<NPT<ImgRgba8>>  ImgNs;

typedef std::map<String8,float> MorphValMap;

RendMesh            cRendMesh(
    NPT<Mesh> const &       meshN,
    NPT<Vec3Fs> const &     allVertsN,
    NPT<MorphValMap> const & morphValMapN,
    RendSurfs const &       rss);

struct      GuiMorphMeshes
{
    RendMeshes          rendMeshes;
    // Aggregates all unique pose labels. Changes in this node trigger re-creation of the expression tab:
    OPT<MorphCtrls>     morphCtrlsN;
    // Only contains entries for currently instantiated pose sliders; others are forgotten.
    IPT<MorphValMap>    morphValMapN;

    GuiMorphMeshes();

    void                addMesh(
        NPT<Mesh>           meshN,          // Name, verts, maps not used. Editing controls if IPT.
        NPT<Vec3Fs>         allVertsN,
        // Albedo before any texture modulation. Must be 1-1 with meshN surfaces (num surfaces not yet dynamic:
        ImgNs               smoothNs,
        // Specular maps must be 1-1 if non-empty. Images can also be empty:
        ImgNs               specularNs=ImgNs{},
        // Optional texture modulation images must be 1-1 if non-empty. Images can also be empty:
        ImgNs               modulationNs=ImgNs{});

    // Convenience to add mesh with no maps:
    void                addMesh(
        NPT<Mesh>           meshN,          // As above
        NPT<Vec3Fs>         allVertsN,
        size_t              numSurfs);      // Not yet dynamic so must be specified for static construction

    GuiPtr              makePoseCtrls(bool editBoxes) const;
};

OPT<Mat32D>         linkBounds(RendMeshes const &);
// Text of the (non-empty) mesh statstics:
OPT<String8>        linkMeshStats(RendMeshes const &);
// If only one mesh is provided then edit controls will also be available and the resulting mesh
// will be returned. Otherwise an empty mesh is returned:
Mesh                viewMesh(
    Meshes const &      meshesOecs,         // Assign the 'name' field if desired for mesh selection
    // use radio button to select mesh (if more than 1) rather than checkboxes:
    bool                compare=false,
    String8 const &     saveName={});       // if given, offer option to save under this filename in edit
inline Mesh         viewMesh(Mesh const & mesh) {return viewMesh({mesh},false); }
// GUI window guides user through placement of a surface 0 point for each of 'labels', regardless of whether
// one already exists. Mesh is modified and returns true if user completes task, false if user cancels before
// then. Currently not set up to handle multi-surface meshes.
bool                guiPlaceSurfPoints_(Strings const & labels,Mesh & mesh);

}

#endif
