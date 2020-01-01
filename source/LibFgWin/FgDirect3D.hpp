//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGDIRECT3D_HPP
#define FGDIRECT3D_HPP

#include "FgStdLibs.hpp"
#include "FgBoostLibs.hpp"
#include "FgWindows.hpp"
#include "FgThrowWindows.hpp"
#include "FgGuiApi3d.hpp"

namespace Fg {

struct  D3dMap
{
    WinPtr<ID3D11Texture2D>          map;        // Can be empty
    WinPtr<ID3D11ShaderResourceView> view;       // Empty iff above is empty

    bool valid() const
    {return bool(view); }

    void reset()
    {
        view.reset();
        map.reset();
    }
};

struct  D3dSurf
{
    WinPtr<ID3D11Buffer>        triVerts;       // 3 Verts for each tri. Null if no facets for this surf.
    WinPtr<ID3D11Buffer>        lineVerts;      // 2 Verts for each edge. Null if no facets or computation delayed.
    D3dMap                         albedoMap;      // Can be empty
    D3dMap                         specularMap;    // "

    void reset() {triVerts.reset(); lineVerts.reset(); albedoMap.reset(); specularMap.reset(); }
};

struct D3dMesh
{
    WinPtr<ID3D11Buffer>        surfPoints;
    WinPtr<ID3D11Buffer>        markedPoints;
    WinPtr<ID3D11Buffer>        allVerts;

    void reset() {surfPoints.reset(); markedPoints.reset(); }
};

ComPtr<ID3DBlob>
compileShader(std::string const & file,std::string const & entrypoint,LPCSTR target);

struct  PipelineState
{
    ComPtr<ID3D11VertexShader>      m_pVertexShader;
    ComPtr<ID3D11PixelShader>       m_pPixelShader;
    ComPtr<ID3D11InputLayout>       m_pInputLayout;

    void
    attachVertexShader(ComPtr<ID3D11Device> pDevice);
    void
    attachVertexShader2(ComPtr<ID3D11Device> pDevice);

    void
    attachPixelShader(ComPtr<ID3D11Device> pDevice,std::string const & fname);

    void
    apply(ComPtr<ID3D11DeviceContext> pContext);
};

Ustring
getGpusDescription();

struct D3d {
    uint                            maxMapSize = 4096;  // Play it safe
    bool                            supports11_1;       // Does GPU support D3D 11.1 ? Otherwise use 11.0
    // Created in constructor:
    ComPtr<ID3D11Device>            pDevice;            // Handle to driver instance for GPU or emulator
    ComPtr<ID3D11DeviceContext>     pContext;
    ComPtr<IDXGISwapChain>          pSwapChain;         // Created by DXGI factor from 'pDevice'
    ComPtr<ID3D11RenderTargetView>  pRTV;               // View into Render Target - back buffer of 'pSwapchain'
    ComPtr<ID3D11DepthStencilView>  pDSV;
    D3dMap                          greyMap;            // For surfaces without an albedo map
    D3dMap                          blackMap;           // For surfaces without a specular map
    D3dMap                          whiteMap;           // 'shiny' rendering option specular map
    ComPtr<ID3D11DepthStencilState> pDepthStencilStateDefault;
    ComPtr<ID3D11DepthStencilState> pDepthStencilStateDisable;
    ComPtr<ID3D11DepthStencilState> pDepthStencilStateWriteDisable;
    ComPtr<ID3D11DepthStencilState> pDepthStencilStateWrite;
    ComPtr<ID3D11BlendState>        pBlendStateDefault;
    ComPtr<ID3D11BlendState>        pBlendStateColorWriteDisable;       
    ComPtr<ID3D11BlendState>        pBlendStateLerp;
    // Created in render:
    D3dMap                          bgImg;
    // TODO: this needs to live with D3dMesh struct:
    bool                            flatShaded = false;     // Are the tri lists currently flat shaded ?
    ComPtr<ID3D11UnorderedAccessView> pUAVTextureHeadOIT;
    ComPtr<ID3D11UnorderedAccessView> pUAVBufferLinkedListOIT;
    PipelineState                   opaquePassPSO;
    PipelineState                   transparentFirstPassPSO;
    PipelineState                   transparentSecondPassPSO;

    D3d(HWND hwnd);

    void
    initializeRenderTexture(Vec2UI windowSize);

    void
    resize(Vec2UI windowSize);

    // All member sizes must be multiples of 8 bytes (presumably for alignment).
    // HLSL uses column-major matrices so we need to transpose:
    struct Scene
    {
        Mat44F      mvm;
        Mat44F      projection;
        Vec4F       ambient;            // RGBA [0,1]
        Vec4F       lightDir[2];        // Normalized direction TO light in CCS
        Vec4F       lightColor[2];      // RGBA [0,1]
    };

    // Since D3D does not allow multiple indexing (one for vert pos, one for vert uv) we use the
    // simplest alternative; expand every vertex of every triangle in the list into its full values.
    // If optimization is needed in future we can pre-process meshes upon loading to give triangle
    // strips:
    struct  Vert
    {
        Vec3F    pos;
        Vec3F    norm;
        Vec2F    uv;
    };

    typedef Svec<Vert>    Verts;

    WinPtr<ID3D11Buffer>
    makeConstBuff(const Scene & scene);

    WinPtr<ID3D11Buffer>
    makeVertBuff(const Verts & verts);

    // Returns null pointer if no surf points:
    WinPtr<ID3D11Buffer>
    makeSurfPoints(RendMesh const & rendMesh,Mesh const & origMesh);

    // Returns null pointer if no marked verts:
    WinPtr<ID3D11Buffer>
    makeMarkedVerts(RendMesh const & rendMesh,Mesh const & origMesh);

    WinPtr<ID3D11Buffer>
    makeAllVerts(Vec3Fs const & verts);

    Verts makeVertList(
        RendMesh const &        rendMesh,
        Mesh const &            origMesh,
        size_t                  surfNum,
        bool                    shadeFlat);

    Verts makeLineVerts(RendMesh const & rendMesh,Mesh const & origMesh,size_t surfNum);

    WinPtr<ID3D11Texture2D>
    loadMap(ImgC4UC const & map);

    WinPtr<ID3D11ShaderResourceView>
    makeMapView(ID3D11Texture2D* mapPtr);

    D3dMap
    makeMap(ImgC4UC const & map);

    WinPtr<ID3D11SamplerState>
    makeSamplerState();

    WinPtr<ID3D11Buffer>
    setScene(Scene const & scene);

    WinPtr<ID3D11Buffer>
    makeScene(Lighting lighting,Mat44F worldToD3vs,Mat44F d3vsToD3ps);

    // Ambient-only version:
    WinPtr<ID3D11Buffer>
    makeScene(Vec3F ambient,Mat44F worldToD3vs,Mat44F d3vsToD3ps);

    void
    setBgImage(BackgroundImage const & bgi);

    void renderBgImg(BackgroundImage const & bgi, Vec2UI viewportSize, bool  transparentPass);

    D3dMesh &
    getD3dMesh(RendMesh const & rm) const;

    D3dSurf &
    getD3dSurf(RendSurf const & rs) const;

    void
    renderBackBuffer(
        BackgroundImage const &     bgi,
        RendMeshes const &          rendMeshes,
        Lighting                    lighting,
        Mat44F                      worldToD3vs,    // modelview
        Mat44F                      d3vsToD3ps,     // projection
        Vec2UI                      viewportSize,
        const RendOptions &         rendOpts);

    void
    showBackBuffer();

    ImgC4UC
    capture(Vec2UI viewportSize);

    void
    setVertexBuffer(ID3D11Buffer * vertBuff);

    void
    renderTris(RendMeshes const & rendMeshes, RendOptions const & rendOpts, bool transparentPass);

    void
    reset();
};

}

#endif

// */
