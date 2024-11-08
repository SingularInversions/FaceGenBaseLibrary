//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGDIRECT3D_HPP
#define FGDIRECT3D_HPP

#include "FgSerial.hpp"
#include "FgWindows.hpp"
#include "FgThrowWindows.hpp"
#include "FgGuiApi3d.hpp"

namespace Fg {

// Empty if fails:
String8s            getGpusDescription();

// The error DXGI_ERROR_DEVICE_REMOVED (0x887A0005) can be returned from at least 5 different D3D calls and
// has happened to many customers. Docs say:
// "The GPU device instance has been suspended. Use GetDeviceRemovedReason to determine the appropriate action."
// Possible reasons include: GPU driver hangs, GPU changes power-saving states, GPU driver updated or changed (eg. when switching monitors)
// Presumably most cases are due to the first two reasons. We need to catch this and re-create the D3D instance.
// MS Example code also handles DXGI_ERROR_DEVICE_RESET (0x887A0007) in same way, but this error does not occur anywhere
// in our error logs so we don't.
// The result of catching and re-creating the device and swap chain on one customer test computer
// (MacBook Pro 2019 w/ Bootcamp, Win10, AMD Radeon Pro 5500M, Intel UHD Graphics 630) was 3-5 seconds lag
// in updates ... presumably due to repeatedly recreating the device and swap chain :(
struct      ExceptD3dDeviceRemoved : public FgException
{
    ExceptD3dDeviceRemoved() : FgException {"D3D Device Removed",""} {}
};

struct      D3dMap
{
    WinPtr<ID3D11Texture2D>          map;        // Can be empty
    WinPtr<ID3D11ShaderResourceView> view;       // Empty iff above is empty

    bool                valid() const {return bool(view); }
    void                reset() {view.reset(); map.reset(); }
};

struct      D3dSurf
{
    WinPtr<ID3D11Buffer>        triVerts;       // 3 Verts for each tri. Null if no facets for this surf.
    WinPtr<ID3D11Buffer>        lineVerts;      // 2 Verts for each edge. Null if no facets or computation delayed.
    D3dMap                      albedoMap,          // Can be empty
                                modulationMap,      // Can be empty
                                specularMap;        // "
    // we need private flags to be sure of updates in case maps have other dependencies:
    DfFPtr                     albedoMapFlag,      // null if there is no map
                                modulationMapFlag,  // "
                                specularMapFlag;    // "
    D3dSurf(NPT<ImgRgba8> const & a,NPT<ImgRgba8> const & m,NPT<ImgRgba8> const & s)
    {
        if (a.ptr)
            albedoMapFlag = cUpdateFlagT(a);
        if (m.ptr)
            modulationMapFlag = cUpdateFlagT(m);
        if (s.ptr)
            specularMapFlag = cUpdateFlagT(s);
    }
};

struct      D3dMesh
{
    // Currently also flags changes in original mesh (ie. surf points & marked verts) since it's in the
    // same mesh node as base verts:
    DfFPtr                     vertsFlag;      // Verts changed ? Has same lifetime as objects below
    WinPtr<ID3D11Buffer>        surfPoints;     // 3 verts/tri x 20 tris/icosahedron x numSurfPoints. Null if none.
    WinPtr<ID3D11Buffer>        markedPoints;
    WinPtr<ID3D11Buffer>        allVerts;
};

struct      PipelineState
{
    ComPtr<ID3D11VertexShader>      m_pVertexShader;
    ComPtr<ID3D11PixelShader>       m_pPixelShader;
    ComPtr<ID3D11InputLayout>       m_pInputLayout;

    void                attachVertexShader(ComPtr<ID3D11Device> pDevice);
    void                attachVertexShader2(ComPtr<ID3D11Device> pDevice);
    void                attachPixelShader(ComPtr<ID3D11Device> pDevice,std::string const & fname);
    void                apply(ComPtr<ID3D11DeviceContext> pContext);
};

struct      D3d
{
    D3d(HWND                hwnd,
        NPT<RendMeshes>     rms,
        NPT<double>         logRelSizeN,            // For scaling the surface point markers
        bool                try_11_1);              // try to get D3D 11.1 before reverting to 11.0

    ~D3d();

    void                    renderBackBuffer(
        BackgroundImage const &     bgi,
        Lighting                    lighting,
        Mat44F                      worldToD3vs,    // modelview
        Mat44F                      d3vsToD3ps,     // projection
        Vec2UI                      viewportSize,
        RendOptions const &         rendOpts,
        float                       texModStrength,
        bool                        backgroundTransparent=false);   // For screen grab option

    void                    setBgImage(BackgroundImage const & bgi);
    void                    resize(Vec2UI windowSize);
    void                    showBackBuffer();
    ImgRgba8                capture(Vec2UI viewportSize);

private:
    uint                            maxMapSize = 4096;  // Play it safe
    bool                            supports11_1;       // Does GPU support D3D 11.1 ? Otherwise use 11.0
    bool                            supportsFlip;       // Supports DXGI_SWAP_EFFECT_FLIP_DISCARD ?
    // Created in constructor:
    NPT<RendMeshes>                 rendMeshesN;
    NPTF<double>                    logRelSize;         // Camera control parameter for object relative size
    OPT<Vec3F>                      origMeshesDimsN;    // Bounding box size of original meshes
    DfFPtr                         origMeshChangeFlag; // If any of the original meshes are changed
    ComPtr<ID3D11Device>            pDevice;            // Handle to driver instance for GPU or emulator
    ComPtr<ID3D11DeviceContext>     pContext;
    ComPtr<IDXGISwapChain>          pSwapChain;         // Created by DXGI factor from 'pDevice'
    ComPtr<ID3D11RenderTargetView>  pRTV;               // View into Render Target - back buffer of 'pSwapchain'
    ComPtr<ID3D11DepthStencilView>  pDSV;
    D3dMap                          greyMap;            // For surfaces without an albedo map
    D3dMap                          blackMap;           // For surfaces without a specular map
    D3dMap                          whiteMap;           // 'shiny' rendering option specular map
    struct  TintMap
    {
        D3dMap          light;      // light tint (close to white)
        D3dMap          trans;      // half-transparent light tint
        D3dMap          strong;     // strongly tinted for point/line coloring
    };
    Svec<TintMap>                   tintMaps;           // for color-coded part displays
    D3dMap                          noModulationMap;    // Modulate all with value 1
    ComPtr<ID3D11DepthStencilState> pDepthStencilStateDefault;
    ComPtr<ID3D11DepthStencilState> pDepthStencilStateDisable;
    ComPtr<ID3D11DepthStencilState> pDepthStencilStateWriteDisable;
    ComPtr<ID3D11DepthStencilState> pDepthStencilStateWrite;
    ComPtr<ID3D11BlendState>        pBlendStateDefault;
    ComPtr<ID3D11BlendState>        pBlendStateColorWriteDisable;       
    ComPtr<ID3D11BlendState>        pBlendStateLerp;
    TriSurf                         icosahedron;            // For rendering points. CW winding.
    // Set in renderBackBuffer:
    D3dMap                          bgImg;
    bool                            flatShaded = false;     // Are D3dSurf::triVerts buffers currently flat shaded ?
    ComPtr<ID3D11UnorderedAccessView> pUAVTextureHeadOIT;
    ComPtr<ID3D11UnorderedAccessView> pUAVBufferLinkedListOIT;
    PipelineState                   opaquePassPSO;
    PipelineState                   transparentFirstPassPSO;
    PipelineState                   transparentSecondPassPSO;

    void                    initializeRenderTexture(Vec2UI windowSize);

    // All member sizes must be multiples of 16 bytes (presumably for alignment).
    // HLSL uses column-major matrices so we need to transpose:
    struct Scene
    {
        Mat44F              mvm;
        Mat44F              projection;
        Vec4F               ambient;            // RGBA [0,1]
        Vec4F               lightDir[2];        // Normalized direction TO light in FCCS
        Vec4F               lightColor[2];      // RGBA [0,1]
        Vec4F               detTexMod;          // strength of detail texture modulation (if present). Only first element is used.
    };

    // Since D3D does not allow multiple indexing (one for vert pos, one for vert uv) we use the
    // simplest alternative; expand every vertex of every triangle in the list into its full values.
    // If optimization is needed in future we can pre-process meshes upon loading to give triangle
    // strips:
    struct      Vert
    {
        Vec3F               pos;
        Vec3F               norm;
        Vec2F               uv;
    };

    typedef Svec<Vert>    Verts;

    Verts               makeVertList(
        RendMesh const &        rendMesh,
        Mesh const &            origMesh,
        size_t                  surfNum,
        bool                    shadeFlat);

    WinPtr<ID3D11Buffer>        makeConstBuff(const Scene & scene);
    WinPtr<ID3D11Buffer>        makeVertBuff(const Verts & verts);
    // Returns null pointer if no surf points:
    WinPtr<ID3D11Buffer>        makeSurfPoints(RendMesh const & rendMesh,Mesh const & origMesh);
    // Returns null pointer if no marked verts:
    WinPtr<ID3D11Buffer>        makeMarkedVerts(RendMesh const & rendMesh,Mesh const & origMesh);
    WinPtr<ID3D11Buffer>        makeAllVerts(Vec3Fs const & verts);
    Verts                       makeLineVerts(RendMesh const & rendMesh,Mesh const & origMesh,size_t surfNum);
    Verts                       makeFlagVerts(RendMesh const & rendMesh,Mesh const & origMesh,size_t surfNum);
    WinPtr<ID3D11Texture2D>     loadMap(ImgRgba8 const & map);
    WinPtr<ID3D11ShaderResourceView> makeMapView(ID3D11Texture2D* mapPtr);
    D3dMap                      makeMap(ImgRgba8 const & map);
    WinPtr<ID3D11SamplerState>  makeSamplerState();
    WinPtr<ID3D11Buffer>        setScene(Scene const & scene);
    WinPtr<ID3D11Buffer>        makeScene(Lighting lighting,Mat44F worldToD3vs,Mat44F d3vsToD3ps,float detModStrength=1.0f);
    // Ambient-only version:
    WinPtr<ID3D11Buffer>        makeScene(Vec3F ambient,Mat44F worldToD3vs,Mat44F d3vsToD3ps,float detModStrength=1.0f);
    void                        renderBgImg(BackgroundImage const & bgi,Vec2UI viewportSize,bool transparentPass);
    D3dMesh &                   getD3dMesh(RendMesh const & rm) const;
    D3dSurf &                   getD3dSurf(RendSurf const & rs) const;
    void                        updateMap_(DfFPtr const & flag,NPT<ImgRgba8> const & in,D3dMap & out);
    void                        setVertexBuffer(ID3D11Buffer * vertBuff);
    size_t                      selectTintMap(size_t idx,size_t num) const;
    void                        renderTris(RendMeshes const & rendMeshes,RendOptions const & rendOpts,bool transparentPass);
    void                        handleHResult(char const * fpath,uint lineNum,HRESULT hr);
};

}

#endif

// */
