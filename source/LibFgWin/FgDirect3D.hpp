//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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

Ustrings        // Empty if fails
getGpusDescription();

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
    D3dMap                      albedoMap;      // Can be empty
    D3dMap                      modulationMap;  // Can be empty
    D3dMap                      specularMap;    // "
};

struct D3dMesh
{
    WinPtr<ID3D11Buffer>        surfPoints;     // 3 verts/tri x 20 tris/icosahedron x numSurfPoints. Null if none.
    WinPtr<ID3D11Buffer>        markedPoints;
    WinPtr<ID3D11Buffer>        allVerts;
};


struct MSAAResolver {
    auto apply(ComPtr<ID3D11DeviceContext> pDeviceContext, ComPtr<ID3D11RenderTargetView> pRTVSrc, ComPtr<ID3D11RenderTargetView> pRTVDsv, DXGI_FORMAT format) const -> void {
        ComPtr<ID3D11Resource> pTexture;
        pRTVDsv->GetResource(pTexture.GetAddressOf());

        ComPtr<ID3D11Resource> pTexture_MSAA;
        pRTVSrc->GetResource(pTexture_MSAA.GetAddressOf());

        pDeviceContext->ResolveSubresource(pTexture.Get(), 0, pTexture_MSAA.Get(), 0, format);
    }

    auto apply(ComPtr<ID3D11DeviceContext> pDeviceContext, ComPtr<ID3D11DepthStencilView> pDSVSrc, ComPtr<ID3D11DepthStencilView> pDSVDsv, DXGI_FORMAT format) const-> void {
        static_assert(true, "No implementation");
    }
};

struct GraphicsPSO {
    auto apply(ComPtr<ID3D11DeviceContext> pDeviceContext) const -> void {
        pDeviceContext->IASetPrimitiveTopology(PrimitiveTopology);
        pDeviceContext->IASetInputLayout(pInputLayout.Get());
        pDeviceContext->VSSetShader(pVS.Get(), nullptr, 0);
        pDeviceContext->PSSetShader(pPS.Get(), nullptr, 0);
        pDeviceContext->RSSetState(pRasterState.Get());
        pDeviceContext->OMSetDepthStencilState(pDepthStencilState.Get(), 0);
        pDeviceContext->OMSetBlendState(pBlendState.Get(), nullptr, BlendMask);
    }

    ComPtr<ID3D11InputLayout>       pInputLayout = nullptr;
    ComPtr<ID3D11VertexShader>      pVS = nullptr;
    ComPtr<ID3D11PixelShader>       pPS = nullptr;
    ComPtr<ID3D11RasterizerState>   pRasterState = nullptr;
    ComPtr<ID3D11DepthStencilState> pDepthStencilState = nullptr;
    ComPtr<ID3D11BlendState>        pBlendState = nullptr;
    uint32_t                        BlendMask = 0xFFFFFFFF;
    D3D11_PRIMITIVE_TOPOLOGY        PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

struct ComputePSO {
    auto apply(ComPtr<ID3D11DeviceContext> pDeviceContext) const -> void {
        pDeviceContext->CSSetShader(pCS.Get(), nullptr, 0);
    }
    ComPtr<ID3D11ComputeShader> pCS = nullptr;
};



struct      D3d
{
    D3d(HWND                hwnd,
        NPT<RendMeshes>     rms,
        NPT<double>         logRelSizeN);       // For scaling the surface point markers

    ~D3d();

    void
    renderBackBuffer(
        BackgroundImage const &     bgi,
        Lighting                    lighting,
        Mat44F                      worldToD3vs,    // modelview
        Mat44F                      d3vsToD3ps,     // projection
        Vec2UI                      viewportSize,
        RendOptions const &         rendOpts,
        bool                        backgroundTransparent=false);   // For screen grab option

    void                            setBgImage(BackgroundImage const & bgi);
    void                            resize(Vec2UI windowSize);
    void                            showBackBuffer();
    ImgC4UC                         capture(Vec2UI viewportSize);

private:
    uint                            sampleCount_MSAA  = 4;
    uint                            colorQuality_MSAA = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
    uint                            depthQuality_MSAA = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
    uint32_t                        maxAverageLayers  = 16;

    DXGI_FORMAT                     colorBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT                     depthBufferFormat = DXGI_FORMAT_D32_FLOAT; 

    uint                            maxMapSize = 4096;  // Play it safe
    bool                            supports11_1;       // Does GPU support D3D 11.1 ? Otherwise use 11.0
    bool                            supportsFlip;       // Supports DXGI_SWAP_EFFECT_FLIP_DISCARD ?
    // Created in constructor:
    NPT<RendMeshes>                   rendMeshesN;
    NPTF<double>                      logRelSize;         // Camera control parameter for object relative size
    OPT<Vec3F>                        origMeshesDimsN;    // Bounding box size of original meshes
    DfgFPtr                           origMeshChangeFlag; // If any of the original meshes are changed
    ComPtr<ID3D11Device>              pDevice;            // Handle to driver instance for GPU or emulator
    ComPtr<ID3D11DeviceContext>       pContext;
    ComPtr<IDXGISwapChain>            pSwapChain;         // Created by DXGI factor from 'pDevice'
    ComPtr<ID3D11RenderTargetView>    pRTVSwapChain;               // View into Render Target - back buffer of 'pSwapchain'
    ComPtr<ID3D11UnorderedAccessView> pUAVSwapChain;

    ComPtr<ID3D11RenderTargetView>    pRTV_MSAA;               // View into Render Target - back buffer of 'pSwapchain'
    ComPtr<ID3D11DepthStencilView>    pDSV_MSAA;

    ComPtr<ID3D11UnorderedAccessView> pUAVTextureHeadOIT;
    ComPtr<ID3D11ShaderResourceView>  pSRVTextureHeadOIT;
    ComPtr<ID3D11UnorderedAccessView> pUAVBufferLinkedListOIT;
    ComPtr<ID3D11ShaderResourceView>  pSRVBufferLinkedListOIT;

    ComPtr<ID3D11SamplerState>        pSamplerWrapLinear;
    ComPtr<ID3D11Buffer>              pConstBuffer;

    D3dMap                          greyMap;            // For surfaces without an albedo map
    D3dMap                          blackMap;           // For surfaces without a specular map
    D3dMap                          whiteMap;           // 'shiny' rendering option specular map
    Arr<D3dMap,4>                   tintMaps;           // For surface coloring display option
    Arr<D3dMap,4>                   tintTransMaps;      // With transparency for mesh coloring display option
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

    
    std::unique_ptr<MSAAResolver>   pMSAAResolver;
    std::unique_ptr<GraphicsPSO>    pPSOWireFrame;
    std::unique_ptr<GraphicsPSO>    pPSOImage;
    std::unique_ptr<GraphicsPSO>    pPSOPoints;
    std::unique_ptr<GraphicsPSO>    pPSOGeometryOpaqueCullBack;
    std::unique_ptr<GraphicsPSO>    pPSOGeometryOpaqueCullNone;
    std::unique_ptr<GraphicsPSO>    pPSOGeometryTransparent;
    std::unique_ptr<ComputePSO>     pPSOGeometryResolve;


    void
    initializeRenderTexture(Vec2UI windowSize);

    // All member sizes must be multiples of 8 bytes (presumably for alignment).
    // HLSL uses column-major matrices so we need to transpose:
    struct Scene
    {
        Mat44F      mvm;
        Mat44F      projection;
        Vec4F       ambient;            // RGBA [0,1]
        Vec4F       lightDir[2];        // Normalized direction TO light in FCCS
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

    typedef Svec<Vert>  Verts;

    template<typename T>
    ComPtr<ID3D11Buffer> makeConstBuff();
    
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

    ComPtr<ID3D11SamplerState> makeLinearWrapSampler();

    void updateScene(ComPtr<ID3D11Buffer> pBuffer, Scene& scene);

    void updateScene(ComPtr<ID3D11Buffer> pBuffer, Lighting lighting,Mat44F worldToD3vs,Mat44F d3vsToD3ps);

    void updateScene(ComPtr<ID3D11Buffer> pBuffer, Vec3F ambient, Mat44F worldToD3vs, Mat44F d3vsToD3ps);

    void renderBgImg(BackgroundImage const & bgi, Vec2UI viewportSize, bool  transparentPass);

    D3dMesh &
    getD3dMesh(RendMesh const & rm) const;

    D3dSurf &
    getD3dSurf(RendSurf const & rs) const;

    void
    updateMap_(NPTF<ImgC4UC> const & in,D3dMap & out);

    void
    setVertexBuffer(ID3D11Buffer * vertBuff);

    void
    renderTris(RendMeshes const & rendMeshes, RendOptions const & rendOpts, bool transparentPass);
};

}

#endif

// */
