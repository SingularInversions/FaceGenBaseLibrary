//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// * Requires D3D 11.1 runtime / DXGI 11.1 (guaranteed on Win7 SP1 and higher, released 12.08.
//   Direct3D 11 subset of DirectX 11.
// * Requires GPU hardware support for 11.0 (no OIT) or 11.1 (OIT)
// * D3D 11.2 is NOT supported on Win7 but we don't need its features anyway.
// * D3D 11.0 is the earliest version to use shader model 5.0. As of 19.08 you can't buy a standalone
//      GPU that doesn't support 11.1, and only the very lowest Intel CPU onboard GPU doesn't.
// * D3D Uses LEFT-handed coordinate systems with CLOCKWISE winding.
// * D3VS - Direct3D View Space / Camera Space (LEFT handed coordinate system)
//      Origin at principal point, no units specified
//      X - camera's right
//      Y - camera's up
//      Z - camera direction
// * D3PS - Direct3D Projection Space (LEFT handed coordinate system)
//      X - image right, clip [-1,1]
//      Y - image up, clip [-1,1]
//      Z - depth (near to far) [0,1]
// * D3SS - Direct3D Screen Space
//      Top left pixel (0,0) bottom right pixel (X-1,Y-1)
// * D3TC = DirectD Texture Coordinates
//      X - image right [0,1]
//      Y - image down [0,1]
// * Indexed rendering was not possible as D3D does not support multiple indices per
//   vertex so that would require some complex logic to determine which vertes to duplicate
//   for uv seams. Expanding all per-vertex data is ~6x memory so if this becomes an issue then
//   add triangle stripping (via indices created at file load time).
// * Quad rendering is not built-in (to any modern GPU) and complicated to do manually, so create
//   the edges in CPU and use line rendering on GPU.
//
// TODO:
//
// * Handle DXGI_ERROR_DEVICE_REMOVED (0x887A0005). This has happened many users (at CreateBuffer & other calls)
//      The GPU device instance has been suspended. Use GetDeviceRemovedReason to determine the appropriate action.
//      MS lists 4 possible reasons: GPU stops responding. GPU changes power-saving states. GPU driver updated. GPU literally removed.
//

#include "stdafx.h"

// The corresponding DLLs appear to always be included with Win 7SP1,8,10:
#pragma comment (lib,"dxgi.lib")
#pragma comment (lib,"d3d11.lib")
// However the following was NOT present on some windows machines so we don't use the compilation API:
// d3dcompiler.lib -> d3dcompiler_47.dll

#include "FgDirect3D.hpp"
#include "FgGuiWin.hpp"
#include "FgCoordSystem.hpp"
#include "FgFileSystem.hpp"
#include "FgHex.hpp"
#include "Fg3dMeshOps.hpp"

using namespace std;

namespace Fg {

Ustrings
getGpusDescription()
{
    Ustrings                ret;
    IDXGIAdapter1 *         pAdapter; 
    IDXGIFactory1*          pFactory = nullptr; 
    if(FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory) ,(void**)&pFactory)))
        return ret;
    for (uint ii=0;pFactory->EnumAdapters1(ii,&pAdapter) != DXGI_ERROR_NOT_FOUND;++ii) {
        DXGI_ADAPTER_DESC1      desc;
        pAdapter->GetDesc1(&desc);
        ret.push_back("Adapter"+toStr(ii)+": "+Ustring(wstring(desc.Description)));
    }
    if(pFactory)
        pFactory->Release();
    return ret;
}

Ustring
getDefaultGpuDescription()
{
    Ustrings        gpus = getGpusDescription();
    if (gpus.empty())
        return "No GPUs detected";
    else
        return gpus[0];
}


static string loadShader(string const& nameFile) {
    return loadRawString(dataDir() + nameFile);
}

#define DEPTH_BIAS_D32_FLOAT(d) (d/(1/pow(2,23)))


D3d::D3d(HWND hwnd,NPT<RendMeshes> rmsN,NPT<double> lrsN) : rendMeshesN{rmsN}, logRelSize{lrsN}
{
    origMeshesDimsN = link1<RendMeshes,Vec3F>(rendMeshesN,[](RendMeshes const & rms)
    {
        Mat32F      bounds {floatMax,-floatMax, floatMax,-floatMax, floatMax,-floatMax};
        for (RendMesh const & rm : rms) {
            Mat32F      bnds = cBounds(rm.origMeshN.cref().verts);
            bounds = cBoundsUnion(bounds,bnds);
        }
        return bounds.colVec(1) - bounds.colVec(0);
    });
    HRESULT             hr = 0;
    {
        // Use dummy size; window size not yet determined:
        uint const      width = 8,
                        height = 8;
        //Create device and swapchain
        DXGI_SWAP_CHAIN_DESC desc = {};
        desc.BufferCount = 2;           // Front and back buffer
        desc.BufferDesc.Width = width;
        desc.BufferDesc.Height = height;
        desc.BufferDesc.Format = colorBufferFormat;
        desc.BufferDesc.RefreshRate.Numerator = 60;
        desc.BufferDesc.RefreshRate.Denominator = 1;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
        desc.OutputWindow = hwnd;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Windowed = true;
        uint32_t createFlag = 0;
#ifdef _DEBUG
        createFlag |= D3D11_CREATE_DEVICE_DEBUG;
        createFlag |= D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT;
#endif
        // We must attempt multiple setup calls until we find the best available driver.
        // Software driver for 11.1 with OIT didn't work (Win7SP1) so don't go there.
        Svec<tuple<D3D_FEATURE_LEVEL,D3D_DRIVER_TYPE,DXGI_SWAP_EFFECT> >  configs = {
            // FLIP_DISCARD only supported by Win 10 but must be used in that case as some systems
            // with fail to consistently display buffer without it:
            {D3D_FEATURE_LEVEL_11_1,D3D_DRIVER_TYPE_HARDWARE,DXGI_SWAP_EFFECT_FLIP_DISCARD},
            {D3D_FEATURE_LEVEL_11_1,D3D_DRIVER_TYPE_HARDWARE,DXGI_SWAP_EFFECT_DISCARD},
            {D3D_FEATURE_LEVEL_11_0,D3D_DRIVER_TYPE_HARDWARE,DXGI_SWAP_EFFECT_DISCARD},
            // Warp driver is very fast:
            {D3D_FEATURE_LEVEL_11_0,D3D_DRIVER_TYPE_WARP,DXGI_SWAP_EFFECT_DISCARD},
            {D3D_FEATURE_LEVEL_11_0,D3D_DRIVER_TYPE_REFERENCE,DXGI_SWAP_EFFECT_DISCARD}
        };
        string      failString;

        for (auto config : configs) {
            supports11_1 = (get<0>(config) == D3D_FEATURE_LEVEL_11_1);
            supportsFlip = (get<2>(config) == DXGI_SWAP_EFFECT_FLIP_DISCARD);
            desc.SwapEffect = get<2>(config);
            // If a system doesn't support 11.1 this returns E_INVALIDARG (0x80070057)
            // If a system doesn't support 11.0 this returns DXGI_ERROR_UNSUPPORTED (0x887A0004)
            // E_FAIL (0x80004005) "debug layer enabled but not installed" happens quite a bit ...
            // DXGI_ERROR_SDK_COMPONENT_MISSING (0x887A002D) SDK needed for debug layer ? only seen once
            hr = D3D11CreateDeviceAndSwapChain(
                nullptr,                // Use first video adapter (card/driver) if more than one of this type
                get<1>(config),         // Driver type
                nullptr,                // No software rasterizer DLL handle
                createFlag,
                &get<0>(config),1,      // Feature level
                D3D11_SDK_VERSION,
                &desc,
                pSwapChain.GetAddressOf(),  // Returned
                pDevice.GetAddressOf(),     // Returned
                nullptr,nullptr);
            if (SUCCEEDED(hr))
                break;
            failString += "HR="+toHexString(hr)+" ";
        }
        if (FAILED(hr))
            throwWindows("No Direct3D 11.0 support",failString);


        //There may be artifacts 
        FG_ASSERT_D3D(pDevice->CheckMultisampleQualityLevels(colorBufferFormat, sampleCount_MSAA, &colorQuality_MSAA));
        FG_ASSERT_D3D(pDevice->CheckMultisampleQualityLevels(depthBufferFormat, sampleCount_MSAA, &depthQuality_MSAA)); 
       

        this->initializeRenderTexture(Vec2UI(width,height));
        pDevice->GetImmediateContext(pContext.GetAddressOf());
    }

    pMSAAResolver = std::make_unique<MSAAResolver>();
    pPSOWireFrame = std::make_unique<GraphicsPSO>();
    pPSOPoints    = std::make_unique<GraphicsPSO>();
    pPSOImage = std::make_unique<GraphicsPSO>();
    pPSOGeometryOpaqueCullNone = std::make_unique<GraphicsPSO>();
    pPSOGeometryOpaqueCullBack = std::make_unique<GraphicsPSO>();
    pPSOGeometryTransparent = std::make_unique<GraphicsPSO>();
    pPSOGeometryResolve = std::make_unique<ComputePSO>();
    pSamplerWrapLinear = this->makeLinearWrapSampler();
    pConstBuffer = this->makeConstBuff<Scene>();

    //Create PSO opaque
    {
        ComPtr<ID3D11InputLayout>  pInputLayout;
        ComPtr<ID3D11VertexShader> pVS;
        ComPtr<ID3D11PixelShader>  pPS;
        ComPtr<ID3D11RasterizerState> pRasterStateFillCullBack;
        ComPtr<ID3D11RasterizerState> pRasterStateFillCullNone;
        ComPtr<ID3D11DepthStencilState> pDepthStencilState;
        ComPtr<ID3D11BlendState> pBlendState;

        auto const pBlobVS = loadShader("base/shaders/dx11_shared_VS.cso");
        auto const pBlobPS = loadShader("base/shaders/dx11_opaque_PS.cso");

        FG_ASSERT_D3D(pDevice->CreateVertexShader(pBlobVS.data(), pBlobVS.size(), nullptr, pVS.ReleaseAndGetAddressOf()));
        FG_ASSERT_D3D(pDevice->CreatePixelShader(pBlobPS.data(), pBlobPS.size(), nullptr, pPS.ReleaseAndGetAddressOf()));

        {
            D3D11_INPUT_ELEMENT_DESC layout[] = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
            
            };
            FG_ASSERT_D3D(pDevice->CreateInputLayout(layout, _countof(layout), pBlobVS.data(), pBlobVS.size(), pInputLayout.ReleaseAndGetAddressOf()));
        }

        {
            D3D11_RASTERIZER_DESC desc = {};
            desc.FillMode = D3D11_FILL_SOLID;
            desc.CullMode = D3D11_CULL_BACK;
            desc.FrontCounterClockwise = false;
            desc.DepthClipEnable = true;
            desc.DepthBias = 1000;
            desc.SlopeScaledDepthBias = 0.5f;
            desc.DepthBiasClamp = 0.001f;
            FG_ASSERT_D3D(pDevice->CreateRasterizerState(&desc, pRasterStateFillCullBack.GetAddressOf()));
        }

        {
            D3D11_RASTERIZER_DESC desc = {};
            desc.FillMode = D3D11_FILL_SOLID;
            desc.CullMode = D3D11_CULL_NONE;
            desc.FrontCounterClockwise = false;
            desc.DepthClipEnable = true;
            desc.DepthBias = 1000;
            desc.SlopeScaledDepthBias = 0.5f;
            desc.DepthBiasClamp = 0.001f;
            FG_ASSERT_D3D(pDevice->CreateRasterizerState(&desc, pRasterStateFillCullNone.GetAddressOf()));
        }

        {
            D3D11_DEPTH_STENCIL_DESC desc = {};
            desc.DepthEnable = true;
            desc.StencilEnable = false;
            desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            desc.DepthFunc = D3D11_COMPARISON_LESS;         
            FG_ASSERT_D3D(pDevice->CreateDepthStencilState(&desc, pDepthStencilState.GetAddressOf()));
        }

        {
            D3D11_BLEND_DESC desc = {};
            desc.AlphaToCoverageEnable = false;
            desc.IndependentBlendEnable = false;
            desc.RenderTarget[0].BlendEnable = false;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            FG_ASSERT_D3D(pDevice->CreateBlendState(&desc, pBlendState.GetAddressOf()));
        }

        pPSOGeometryOpaqueCullBack->pInputLayout = pInputLayout;
        pPSOGeometryOpaqueCullBack->pVS = pVS;
        pPSOGeometryOpaqueCullBack->pPS = pPS;
        pPSOGeometryOpaqueCullBack->pRasterState = pRasterStateFillCullBack;
        pPSOGeometryOpaqueCullBack->pDepthStencilState = pDepthStencilState;
        pPSOGeometryOpaqueCullBack->pBlendState = pBlendState;


        pPSOGeometryOpaqueCullNone->pInputLayout = pInputLayout;
        pPSOGeometryOpaqueCullNone->pVS = pVS;
        pPSOGeometryOpaqueCullNone->pPS = pPS;
        pPSOGeometryOpaqueCullNone->pRasterState = pRasterStateFillCullNone;
        pPSOGeometryOpaqueCullNone->pDepthStencilState = pDepthStencilState;
        pPSOGeometryOpaqueCullNone->pBlendState = pBlendState;

    }

    //Create PSO transparent 
    {
        ComPtr<ID3D11InputLayout>  pInputLayout;
        ComPtr<ID3D11VertexShader> pVS;
        ComPtr<ID3D11PixelShader>  pPS;
        ComPtr<ID3D11RasterizerState> pRasterState;
        ComPtr<ID3D11DepthStencilState> pDepthStencilState;
        ComPtr<ID3D11BlendState> pBlendState;

        auto const pBlobVS = loadShader("base/shaders/dx11_shared_VS.cso");
        auto const pBlobPS = loadShader("base/shaders/dx11_transparent_PS.cso");

        FG_ASSERT_D3D(pDevice->CreateVertexShader(pBlobVS.data(), pBlobVS.size(), nullptr, pVS.ReleaseAndGetAddressOf()));
        FG_ASSERT_D3D(pDevice->CreatePixelShader(pBlobPS.data(), pBlobPS.size(), nullptr, pPS.ReleaseAndGetAddressOf()));

        {
            D3D11_INPUT_ELEMENT_DESC layout[] = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }

            };
            FG_ASSERT_D3D(pDevice->CreateInputLayout(layout, _countof(layout), pBlobVS.data(), pBlobVS.size(), pInputLayout.ReleaseAndGetAddressOf()));
        }

        {
            D3D11_RASTERIZER_DESC desc = {};
            desc.FillMode = D3D11_FILL_SOLID;
            desc.CullMode = D3D11_CULL_NONE;
            desc.FrontCounterClockwise = false;
            desc.DepthClipEnable = true;
            desc.DepthBias = 1000;
            desc.SlopeScaledDepthBias = 0.5f;
            desc.DepthBiasClamp = 0.001f;
            FG_ASSERT_D3D(pDevice->CreateRasterizerState(&desc, pRasterState.GetAddressOf()));
        }

        {
            D3D11_DEPTH_STENCIL_DESC desc = {};
            desc.DepthEnable = true;
            desc.StencilEnable = false;
            desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
            desc.DepthFunc = D3D11_COMPARISON_LESS;
            FG_ASSERT_D3D(pDevice->CreateDepthStencilState(&desc, pDepthStencilState.GetAddressOf()));
        }

        {
            D3D11_BLEND_DESC desc = {};
            desc.AlphaToCoverageEnable = false;
            desc.IndependentBlendEnable = false;
            desc.RenderTarget[0].BlendEnable = false;
            desc.RenderTarget[0].RenderTargetWriteMask = 0;
            FG_ASSERT_D3D(pDevice->CreateBlendState(&desc, pBlendState.GetAddressOf()));
        }

        pPSOGeometryTransparent->pInputLayout = pInputLayout;
        pPSOGeometryTransparent->pVS = pVS;
        pPSOGeometryTransparent->pPS = pPS;
        pPSOGeometryTransparent->pRasterState = pRasterState;
        pPSOGeometryTransparent->pDepthStencilState = pDepthStencilState;
        pPSOGeometryTransparent->pBlendState = pBlendState;
    }

    //Create PSO WireFrame
    {

        ComPtr<ID3D11InputLayout>  pInputLayout = pPSOGeometryTransparent->pInputLayout; 
        ComPtr<ID3D11VertexShader> pVS = pPSOGeometryTransparent->pVS;
        ComPtr<ID3D11PixelShader>  pPS = pPSOGeometryTransparent->pPS;
        ComPtr<ID3D11RasterizerState>   pRasterState = pPSOGeometryTransparent->pRasterState;
        ComPtr<ID3D11DepthStencilState> pDepthStencilState = pPSOGeometryTransparent->pDepthStencilState;
        ComPtr<ID3D11BlendState> pBlendState = pPSOGeometryTransparent->pBlendState;

        pPSOWireFrame->pInputLayout = pInputLayout;
        pPSOWireFrame->pVS = pVS;
        pPSOWireFrame->pPS = pPS;
        pPSOWireFrame->pRasterState = pRasterState;
        pPSOWireFrame->pDepthStencilState = pDepthStencilState;
        pPSOWireFrame->pBlendState = pBlendState;
        pPSOWireFrame->PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;

    }

    //Create PSO Image
    {
        ComPtr<ID3D11InputLayout>  pInputLayout = pPSOGeometryOpaqueCullNone->pInputLayout;
        ComPtr<ID3D11VertexShader> pVS = pPSOGeometryOpaqueCullNone->pVS;
        ComPtr<ID3D11PixelShader>  pPS = pPSOGeometryOpaqueCullNone->pPS;
        ComPtr<ID3D11RasterizerState>   pRasterState = pPSOGeometryOpaqueCullNone->pRasterState;
        ComPtr<ID3D11DepthStencilState> pDepthStencilState;
        ComPtr<ID3D11BlendState> pBlendState;


        {
            D3D11_DEPTH_STENCIL_DESC desc = {};
            desc.DepthEnable = false;
            desc.StencilEnable = false;
            desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
            FG_ASSERT_D3D(pDevice->CreateDepthStencilState(&desc, pDepthStencilState.GetAddressOf()));
        }

        {
            D3D11_BLEND_DESC desc = {};
            desc.AlphaToCoverageEnable = false;
            desc.IndependentBlendEnable = false;
            desc.RenderTarget[0].BlendEnable = true;
            desc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_SRC_ALPHA;
            desc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_ALPHA;
            desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
            desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
            desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            FG_ASSERT_D3D(pDevice->CreateBlendState(&desc, pBlendState.GetAddressOf()));
        }

        pPSOImage->pInputLayout = pInputLayout;
        pPSOImage->pVS = pVS;
        pPSOImage->pPS = pPS;
        pPSOImage->pRasterState = pRasterState;
        pPSOImage->pDepthStencilState = pDepthStencilState;
        pPSOImage->pBlendState = pBlendState;
        pPSOImage->PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    }

    //Create PSO Points
    {
        ComPtr<ID3D11InputLayout>  pInputLayout = pPSOGeometryTransparent->pInputLayout;
        ComPtr<ID3D11VertexShader> pVS = pPSOGeometryTransparent->pVS;
        ComPtr<ID3D11PixelShader>  pPS = pPSOGeometryTransparent->pPS;
        ComPtr<ID3D11RasterizerState>   pRasterState = pPSOGeometryTransparent->pRasterState;
        ComPtr<ID3D11DepthStencilState> pDepthStencilState = pPSOGeometryTransparent->pDepthStencilState;
        ComPtr<ID3D11BlendState> pBlendState = pPSOGeometryTransparent->pBlendState;
   
        pPSOPoints->pInputLayout = pInputLayout;
        pPSOPoints->pVS = pVS;
        pPSOPoints->pPS = pPS;
        pPSOPoints->pRasterState = pRasterState;
        pPSOPoints->pDepthStencilState = pDepthStencilState;
        pPSOPoints->pBlendState = pBlendState;
        pPSOPoints->PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    }

    //Create PSO resolve transparent and opaque
    {
        ComPtr<ID3D11ComputeShader> pCS;
        auto const pBlobCS = loadShader("base/shaders/dx11_resolve_CS.cso");
        FG_ASSERT_D3D(pDevice->CreateComputeShader(pBlobCS.data(), pBlobCS.size(), nullptr, pCS.ReleaseAndGetAddressOf()));
        pPSOGeometryResolve->pCS = pCS;
    }


    greyMap = makeMap(ImgC4UC(Vec2UI(2,2),RgbaUC(200,200,200,255)));
    blackMap = makeMap(ImgC4UC(Vec2UI(2,2),RgbaUC(0,0,0,255)));
    whiteMap = makeMap(ImgC4UC(Vec2UI(2,2),RgbaUC(255,255,255,255)));
    {
        Arr<Vec3UC,4>           colors {{{200,200,200},{255,200,200},{200,255,200},{200,200,255}}};
        for (size_t ii=0; ii<colors.size(); ++ii) {
            Vec3UC              c = colors[ii];
            tintMaps[ii] = makeMap(ImgC4UC(Vec2UI{2,2},RgbaUC{c[0],c[1],c[2],255}));
            tintTransMaps[ii] = makeMap(ImgC4UC(Vec2UI{2,2},RgbaUC{c[0],c[1],c[2],150}));
        }
    }
    noModulationMap = makeMap(ImgC4UC(Vec2UI(2,2),RgbaUC(64,64,64,255)));
    icosahedron = reverseWinding(cIcosahedron());                               // CC to CW
}

D3d::~D3d()
{
    // Release any GPU data pointers held outside this object:
    RendMeshes const &      rms = rendMeshesN.cref();
    for (RendMesh const & rm : rms) {
        rm.gpuData->reset();
        for (RendSurf const & rs : rm.rendSurfs)
            rs.gpuData->reset();
    }
    if (pContext != nullptr) {
        pContext->ClearState();
    }
}

void
D3d::initializeRenderTexture(Vec2UI windowSize)
{
    {   // Create RTV from BackBuffer.
        // There is no need to hold the pointer to the buffer since it's held by the view and
        // will be automatically released when the view is released:
        ComPtr<ID3D11Texture2D> pBackBuffer;  
        FG_ASSERT_D3D(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(pBackBuffer.GetAddressOf())));
        FG_ASSERT_D3D(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, pRTVSwapChain.ReleaseAndGetAddressOf()));
        FG_ASSERT_D3D(pDevice->CreateUnorderedAccessView(pBackBuffer.Get(), nullptr, pUAVSwapChain.ReleaseAndGetAddressOf()));
    }

    {     
        ComPtr<ID3D11Texture2D> pBackBufferMSAA;
        {
            D3D11_TEXTURE2D_DESC desc = {};
            desc.ArraySize = 1;
            desc.MipLevels = 1;
            desc.Width = windowSize[0];
            desc.Height = windowSize[1];
            desc.Format = colorBufferFormat;
            desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;;
            desc.SampleDesc.Count = sampleCount_MSAA;
            desc.SampleDesc.Quality = desc.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;            
            desc.Usage = D3D11_USAGE_DEFAULT;
            FG_ASSERT_D3D(pDevice->CreateTexture2D(&desc, nullptr, pBackBufferMSAA.GetAddressOf()));
            FG_ASSERT_D3D(pDevice->CreateRenderTargetView(pBackBufferMSAA.Get(), nullptr, pRTV_MSAA.ReleaseAndGetAddressOf()));
        }

        ComPtr<ID3D11Texture2D> pDepthBufferMSAA;
        {
            D3D11_TEXTURE2D_DESC desc = {};
            desc.ArraySize = 1;
            desc.MipLevels = 1;
            desc.Width = windowSize[0];
            desc.Height = windowSize[1];
            desc.Format = depthBufferFormat;
            desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            desc.SampleDesc.Count = sampleCount_MSAA;
            desc.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
            desc.Usage = D3D11_USAGE_DEFAULT;
            FG_ASSERT_D3D(pDevice->CreateTexture2D(&desc, nullptr, pDepthBufferMSAA.GetAddressOf()));
            FG_ASSERT_D3D(pDevice->CreateDepthStencilView(pDepthBufferMSAA.Get(), nullptr, pDSV_MSAA.ReleaseAndGetAddressOf()));
        }

    }

    {   //Create Head texture for OIT
        ComPtr<ID3D11Texture2D> pTextureOIT;
        D3D11_TEXTURE2D_DESC desc = {};
        desc.ArraySize = 1;
        desc.Width = windowSize[0];
        desc.Height = windowSize[1];
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        desc.Format = DXGI_FORMAT_R32_UINT;
       
        FG_ASSERT_D3D(pDevice->CreateTexture2D(&desc, nullptr, pTextureOIT.GetAddressOf()));
        FG_ASSERT_D3D(pDevice->CreateUnorderedAccessView(pTextureOIT.Get(), nullptr, pUAVTextureHeadOIT.ReleaseAndGetAddressOf()));
        FG_ASSERT_D3D(pDevice->CreateShaderResourceView(pTextureOIT.Get(), nullptr,  pSRVTextureHeadOIT.ReleaseAndGetAddressOf()));
    }

    {   //Create LinkedList with atomic counter for OIT 

        struct ListNode {
            uint32_t  Next;
            uint32_t  Color;
            uint32_t  Depth;
            uint32_t  Coverage;
        };

        ComPtr<ID3D11Buffer> pBufferOIT;
        {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(ListNode) * windowSize[0] * windowSize[1] * maxAverageLayers;
            desc.CPUAccessFlags = 0;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
            desc.StructureByteStride = sizeof(ListNode);
            FG_ASSERT_D3D(pDevice->CreateBuffer(&desc, nullptr, pBufferOIT.GetAddressOf()));
        }

        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
            desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
            desc.Buffer.FirstElement = 0;
            desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
            desc.Buffer.NumElements = windowSize[0] * windowSize[1] * maxAverageLayers;
            FG_ASSERT_D3D(pDevice->CreateUnorderedAccessView(pBufferOIT.Get(), &desc, pUAVBufferLinkedListOIT.ReleaseAndGetAddressOf()));
        }

        {
            D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
            desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            desc.Buffer.FirstElement = 0;
            desc.Buffer.NumElements = windowSize[0] * windowSize[1] * maxAverageLayers;
            FG_ASSERT_D3D(pDevice->CreateShaderResourceView(pBufferOIT.Get(), &desc, pSRVBufferLinkedListOIT.ReleaseAndGetAddressOf()));
        }   
    }
}

void
D3d::resize(Vec2UI windowSize)
{
    if (pContext == nullptr)
        return;
    pContext->OMSetRenderTargets(0,nullptr,nullptr);
    pRTVSwapChain.Reset();
    pUAVSwapChain.Reset();

    FG_ASSERT_D3D(pSwapChain->ResizeBuffers(2, windowSize[0], windowSize[1], DXGI_FORMAT_UNKNOWN, 0));
    this->initializeRenderTexture(windowSize);
 
}

template<typename T>
ComPtr<ID3D11Buffer> D3d::makeConstBuff() {
    D3D11_BUFFER_DESC  desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;   
    desc.ByteWidth = sizeof(T);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ComPtr<ID3D11Buffer> pBuffer;

    FG_ASSERT_D3D(pDevice->CreateBuffer(&desc, nullptr, pBuffer.GetAddressOf()));
    return pBuffer;
}

WinPtr<ID3D11Buffer>
D3d::makeVertBuff(const Verts & verts)
{
    if (verts.empty())
        return WinPtr<ID3D11Buffer>();  // D3D gives error for zero size buffer creation
    ID3D11Buffer*           ptr;
    D3D11_BUFFER_DESC       bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;             // read/write by GPU only
    bd.ByteWidth = sizeof(Vert) * uint(verts.size());
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA  initData = {};
    initData.pSysMem = verts.data();
    HRESULT                 hr = pDevice->CreateBuffer(&bd,&initData,&ptr);
    FG_ASSERT_D3D(hr);
    return WinPtr<ID3D11Buffer>(ptr);
}

// Returns null pointer if no surf points:
WinPtr<ID3D11Buffer>
D3d::makeSurfPoints(RendMesh const & rendMesh,Mesh const & origMesh)
{
    // This isn't quite right since it will recalc when anything in rendMeshesN changes ...
    // but it's not clear what the perfect solution would look like:
    float                   sz = cMaxElem(origMeshesDimsN.val()) * (1.0f / 256.0f),
                            relSize = float(exp(logRelSize.node.val())),
                            scale = cMax(1.0f,relSize);
    sz /= scale;            // Shrink points as they are zoomed in for greater visual accuracy
    Verts                   verts;
    Vec3Fs const &          rendVerts = rendMesh.posedVertsN.cref();
    for (Surf const & origSurf : origMesh.surfaces) {
        for (SurfPoint const & sp : origSurf.surfPoints) {
            Vec3F               pos = cSurfPointPos(sp,origSurf.tris,origSurf.quads,rendVerts);
            for (Vec3UI tri : icosahedron.tris) {
                for (uint idx : tri.m) {
                    Vert            v;
                    v.pos = pos + icosahedron.verts[idx] * sz;
                    v.norm = icosahedron.verts[idx];            // icosahedron verts are unit distance from origin
                    v.uv = Vec2F{0};
                    verts.push_back(v);
                }
            }
        }
    }
    return makeVertBuff(verts);
}

// Returns null pointer if no marked verts:
WinPtr<ID3D11Buffer>
D3d::makeMarkedVerts(RendMesh const & rendMesh,Mesh const & origMesh)
{
    Verts               verts;
    Vec3Fs const &      rendVerts = rendMesh.posedVertsN.cref();
    for (MarkedVert const & mv : origMesh.markedVerts) {
        Vert            v;
        v.pos = rendVerts[mv.idx];
        v.norm = Vec3F{0,0,1};      // Random non-zero value can be normalized by shader
        v.uv = Vec2F{0};
        verts.push_back(v);
    }
    return makeVertBuff(verts);
}

WinPtr<ID3D11Buffer>
D3d::makeAllVerts(Vec3Fs const & verts)
{
    Verts               avs;
    for (Vec3F pos : verts) {
        Vert            v;
        v.pos = pos;
        v.norm = Vec3F(0,0,1);   // Random non-zero value can be normalized by shader
        v.uv = Vec2F(0);
        avs.push_back(v);
    }
    return makeVertBuff(avs);
}

D3d::Verts
D3d::makeVertList(
    RendMesh const &        rendMesh,
    Mesh const &            origMesh,
    size_t                  surfNum,
    bool                    shadeFlat)
{
    D3d::Verts              vertList;
    Surf const &            origSurf = origMesh.surfaces[surfNum];
    MeshNormals const &         normals = rendMesh.normalsN.cref();
    Vec3Fs const &          norms = normals.vert;
    FacetNormals const &    normFlats = normals.facet[surfNum];
    Vec3Fs const &          verts = rendMesh.posedVertsN.cref();
    vertList.reserve(3*size_t(origSurf.numTriEquivs()));
    for (size_t tt=0; tt<origSurf.numTriEquivs(); ++tt) {
        TriUv                   tri = origSurf.getTriEquiv(tt);
        for (uint ii=2; ii<3; --ii) {   // Reverse order due to D3D LEFT-handed coordinate system
            Vert                v;
            size_t              posIdx = tri.posInds[ii];
            v.pos = verts[posIdx];
            if (shadeFlat)
                v.norm = normFlats.triEquiv(tt);
            else
                v.norm = norms[posIdx];
            if (!origMesh.uvs.empty()) {
                v.uv = origMesh.uvs[tri.uvInds[ii]];
                v.uv[1] = 1.0f - v.uv[1];   // Convert from OTCS to D3TCS
            }
            vertList.push_back(v);
        }
    }
    return vertList;
}

D3d::Verts
D3d::makeLineVerts(RendMesh const & rendMesh,Mesh const & origMesh,size_t surfNum) {
    D3d::Verts              ret;
    Vec3Fs const &          verts = rendMesh.posedVertsN.cref();
    Surf const &            origSurf = origMesh.surfaces[surfNum];
    Vec3UIs const &         tris = origSurf.tris.posInds;
    Vec4UIs const &         quads = origSurf.quads.posInds;
    ret.reserve(8*quads.size()+6*tris.size());
    Vert                        v;
    v.norm = Vec3F(0,0,1);
    v.uv = Vec2F(0);
    for (Vec3UI tri : tris) {
        for (uint ii=0; ii<3; ++ii) {
            v.pos = verts[tri[ii]];
            ret.push_back(v);
            v.pos = verts[tri[(ii+1)%3]];
            ret.push_back(v);
        }
    }
    for (Vec4UI quad : quads) {
        for (uint ii=0; ii<4; ++ii) {
            v.pos = verts[quad[ii]];
            ret.push_back(v);
            v.pos = verts[quad[(ii+1)%4]];
            ret.push_back(v);
        }
    }
    return ret;
}

WinPtr<ID3D11Texture2D>
D3d::loadMap(ImgC4UC const & map) {
    ImgC4UCs                mip = cMipMap(map); // TODO need to speed up the remapping to pow2 in here for big images
    uint                    numMips = cMin(uint(mip.size()),8);
    D3D11_SUBRESOURCE_DATA  initData[8] = {};
    for (size_t mm=0; mm<numMips; ++mm) {
        initData[mm].pSysMem = mip[mm].data();
        initData[mm].SysMemPitch = mip[mm].width()*4;
    }
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = map.width();
    desc.Height = map.height();
    // MS Docs useless, but it appears 1 means no mipmaps provided and 0 means all
    // mipmaps provided and 'initData' above must an array of mipmap pointers:
    desc.MipLevels = numMips;
    // One image in this array. Arr images must all be the same size and format
    // and can be used for cube maps.
    desc.ArraySize = 1;
    // The 'NORM' part here means that it is mapped to a float by dividing by 255:
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;      // 1 sample per pixel (no FSAA)
    desc.SampleDesc.Quality = 0;    // For MSAA there are higher quality levels (which are slower)
    desc.Usage = D3D11_USAGE_DEFAULT;
    // D3D11_BIND_RENDER_TARGET was added to have GPU generate mipmaps but didn't work:
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;        // CPU does not need to acess
    // Also required for gpu mipmap generation that didn't work:
    //desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    ID3D11Texture2D*    ptr;
    HRESULT             hr = pDevice->CreateTexture2D(&desc,initData,&ptr);
    FG_ASSERT_D3D(hr);
    return WinPtr<ID3D11Texture2D>(ptr);
}

WinPtr<ID3D11ShaderResourceView>
D3d::makeMapView(ID3D11Texture2D* mapPtr)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC     SRVDesc = {};
    SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MipLevels = uint(-1);   // Use all available mipmap levels
    ID3D11ShaderResourceView*           ptr;
    HRESULT         hr = pDevice->CreateShaderResourceView(mapPtr,&SRVDesc,&ptr);
    FG_ASSERT_D3D(hr);
    // Couldn't get this to work. mipmaps currently generated in CPU:
    //pContext->GenerateMips(ptr);
    return WinPtr<ID3D11ShaderResourceView>(ptr);
}

D3dMap
D3d::makeMap(ImgC4UC const & map)
{
    D3dMap             ret;
    ret.map = loadMap(map);
    ret.view = makeMapView(ret.map.get());
    return ret;
}

ComPtr<ID3D11SamplerState>
D3d::makeLinearWrapSampler()
{
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;      // Always use trilinear
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;                    // Make use of all mipmap levels available
    ComPtr<ID3D11SamplerState> pSampler;
 
    FG_ASSERT_D3D(pDevice->CreateSamplerState(&sampDesc, pSampler.GetAddressOf()));
    return pSampler;
}

void D3d::updateScene(ComPtr<ID3D11Buffer> pBuffer, Scene& scene)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource = {};
    pContext->Map(pBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, &scene, sizeof(Scene));
    pContext->Unmap(pBuffer.Get(), 0);
}


void D3d::updateScene(ComPtr<ID3D11Buffer> pBuffer, Lighting lighting, Mat44F worldToD3vs, Mat44F d3vsToD3ps)
{
    lighting.lights.resize(2);                          // Ensure we have 2
    Scene                   scene;
    scene.mvm = worldToD3vs.transpose();                // D3D is column-major (of course)
    scene.projection = d3vsToD3ps.transpose();          // "
    scene.ambient = asHomogVec(lighting.ambient);
    for (uint ii=0; ii<2; ++ii) {
        Vec3F               d = lighting.lights[ii].direction;
        scene.lightDir[ii] = asHomogVec(Vec3F{d[0],d[1],-d[2]});    // OECS to D3VS
        scene.lightColor[ii] = asHomogVec(lighting.lights[ii].colour);
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource = {};

    pContext->Map(pBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, &scene, sizeof(Scene));
    pContext->Unmap(pBuffer.Get(), 0);
}


void D3d::updateScene(ComPtr<ID3D11Buffer> pBuffer, Vec3F ambient, Mat44F worldToD3vs, Mat44F d3vsToD3ps)
{
    Light             diffuseBlack(Vec3F(0), Vec3F(0, 0, 1));
    Lighting          lighting(ambient, svec(diffuseBlack, diffuseBlack));
    return updateScene(pBuffer, lighting, worldToD3vs, d3vsToD3ps);
}



void
D3d::setBgImage(BackgroundImage const & bgi)
{
    bgImg.reset();
    ImgC4UC const &     img = bgi.imgN.cref();
    if (img.empty())
        return;
    Vec2UI              p2dims = clampHi(pow2Ceil(img.dims()),maxMapSize);
    ImgC4UC             map(p2dims);
    imgResize(img,map);
    bgImg = makeMap(map);
}

void
D3d::renderBgImg(BackgroundImage const & bgi, Vec2UI viewportSize, bool  transparentPass) {

    WinPtr<ID3D11Buffer>            bgImageVerts;
    {   // Background image polys:
        Vec2F        im = Vec2F(bgi.origDimsN.val()),
                        xr(im[0]*viewportSize[1],im[1]*viewportSize[0]);
        Vec2F        sz = Vec2F(xr) / cMaxElem(xr) * exp(bgi.lnScale.val()),
                        off = bgi.offset.val();
        float           xo = off[0] * 2.0f,
                        yo = -off[1] * 2.0f,
                        xh = sz[0] + xo,
                        xl = -sz[0] + xo,
                        yh = sz[1] + yo,
                        yl = -sz[1] + yo;
        Verts           bgiVerts;
        Vert            v;
        v.norm = Vec3F(0,0,1);           // Normalized, direction irrelevant.
        v.pos = Vec3F(xl,yh,1);
        v.uv = Vec2F(0,0);
        bgiVerts.push_back(v);              // Top left
        v.pos = Vec3F(xh,yh,1);
        v.uv = Vec2F(1,0);
        bgiVerts.push_back(v);              // Top right
        v.pos = Vec3F(xl,yl,1);
        v.uv = Vec2F(0,1);
        bgiVerts.push_back(v);              // Bottom left

        v.pos = Vec3F(xh,yh,1);
        v.uv = Vec2F(1,0);
        bgiVerts.push_back(v);              // Top right
        v.pos = Vec3F(xh,yl,1);
        v.uv = Vec2F(1,1);
        bgiVerts.push_back(v);              // Bottom right
        v.pos = Vec3F(xl,yl,1);
        v.uv = Vec2F(0,1);
        bgiVerts.push_back(v);              // Bottom left
        bgImageVerts = makeVertBuff(bgiVerts);
    }
    setVertexBuffer(bgImageVerts.get());
    ID3D11ShaderResourceView*       mapViews[3];    // Albedo, specular resp.
    mapViews[0] = bgImg.view.get();
    mapViews[1] = blackMap.view.get();
    mapViews[2] = noModulationMap.view.get();
    pContext->PSSetShaderResources(0,3,mapViews);
    Scene                       scene;
    scene.mvm = Mat44F::identity();
    scene.projection = Mat44F::identity();
    scene.ambient = Vec4F(1);
    if (transparentPass) {
        double                      ft = clampBounds(bgi.foregroundTransparency.val(),0.0,1.0);
        scene.ambient[3] = static_cast<float>(ft);
    }

    
    this->updateScene(pConstBuffer, scene);
    pContext->Draw(6,0);
}

D3dMesh &
D3d::getD3dMesh(RendMesh const & rm) const
{
    Any &  gpuMesh = *rm.gpuData;
    if (gpuMesh.empty())
        gpuMesh.reset(D3dMesh());
    return gpuMesh.ref<D3dMesh>();
}

D3dSurf &
D3d::getD3dSurf(RendSurf const & rs) const
{
    Any &  gpuSurf = *rs.gpuData;
    if (gpuSurf.empty())
        gpuSurf.reset(D3dSurf());
    return gpuSurf.ref<D3dSurf>();
}

void
D3d::updateMap_(NPTF<ImgC4UC> const & in,D3dMap & out)
{
    if (in.checkUpdate()) {
        out.reset();
        ImgC4UC const &     img = in.cref();
        if (!img.empty())
            out = makeMap(img);
    }
}

void
D3d::renderBackBuffer(
    BackgroundImage const &     bgi,
    Lighting                    lighting,       // OECS
    Mat44F                      worldToD3vs,    // modelview
    Mat44F                      d3vsToD3ps,     // projection
    Vec2UI                      viewportSize,
    RendOptions const &         rendOpts,
    bool                        backgroundTransparent)
{
    // No render view during create - created by first resize:
    if (pRTVSwapChain == nullptr)
        return;
    // Update 'd3dMeshes' (mesh and map data) if required:
    RendMeshes const &      rendMeshes = rendMeshesN.cref();
    for (RendMesh const & rendMesh : rendMeshes) {
        Mesh const &        origMesh = rendMesh.origMeshN.cref();
        D3dMesh &           d3dMesh = getD3dMesh(rendMesh);
        bool                vertsChanged = rendMesh.surfVertsFlag->checkUpdate();
        bool                trisChanged = vertsChanged || (flatShaded != rendOpts.flatShaded);                          
        bool                valid = !origMesh.verts.empty();
        if (rendOpts.allVerts && rendMesh.allVertsFlag->checkUpdate()) {
            d3dMesh.allVerts.reset();
            if (valid)
                d3dMesh.allVerts = makeAllVerts(rendMesh.posedVertsN.cref());
        }
        if (vertsChanged || logRelSize.checkUpdate()) {
            d3dMesh.surfPoints.reset();
            if (valid)
                d3dMesh.surfPoints = makeSurfPoints(rendMesh,origMesh);
        }
        if (vertsChanged) {
            d3dMesh.markedPoints.reset();
            if (valid)
                d3dMesh.markedPoints = makeMarkedVerts(rendMesh,origMesh);
        }
        if (valid)
            FGASSERT(rendMesh.rendSurfs.size() == origMesh.surfaces.size());
        for (size_t ss=0; ss<rendMesh.rendSurfs.size(); ++ss) {     // TODO: don't duplicate shared maps ! (eg. multisurface mesh)
            RendSurf const &        rs = rendMesh.rendSurfs[ss];
            D3dSurf &               d3dSurf = getD3dSurf(rs);
            updateMap_(rs.smoothMapN,d3dSurf.albedoMap);
            updateMap_(rs.modulationMapN,d3dSurf.modulationMap);
            updateMap_(rs.specularMapN,d3dSurf.specularMap);
            if (trisChanged) {
                d3dSurf.lineVerts.reset();  // Release but delay computation in case not needed
                d3dSurf.triVerts.reset();
                if (valid) {
                    Verts           verts = makeVertList(rendMesh,origMesh,ss,rendOpts.flatShaded);
                    if (!verts.empty())
                        d3dSurf.triVerts = makeVertBuff(verts);
                }
            }
        }
    }

    flatShaded = rendOpts.flatShaded;

    auto const clearColor = { rendOpts.backgroundColor[0], rendOpts.backgroundColor[1], rendOpts.backgroundColor[2], 1.0f };
    auto const viewport = CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(viewportSize[0]), static_cast<float>(viewportSize[1]));
    auto const scissor = CD3D11_RECT(0, 0, viewportSize[0], viewportSize[1]);
    auto const threadGroupsX = static_cast<uint32_t>(std::ceil(viewportSize[0] / 8.0f));
    auto const threadGroupsY = static_cast<uint32_t>(std::ceil(viewportSize[1] / 8.0f));


    pContext->ClearRenderTargetView(pRTV_MSAA.Get(), std::data(clearColor));
    pContext->ClearDepthStencilView(pDSV_MSAA.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    pContext->ClearUnorderedAccessViewUint(pUAVTextureHeadOIT.Get(), std::data({ 0xFFFFFFFF }));

    pContext->RSSetViewports(1, &viewport);
    pContext->RSSetScissorRects(1, &scissor);

    pContext->VSSetConstantBuffers(0, 1, pConstBuffer.GetAddressOf());
    pContext->PSSetConstantBuffers(0, 1, pConstBuffer.GetAddressOf());
    pContext->PSSetSamplers(0, 1, pSamplerWrapLinear.GetAddressOf());

    ID3D11RenderTargetView* ppRTVClear[] = { nullptr };
    ID3D11ShaderResourceView* ppSRVClear[] = { nullptr, nullptr, nullptr };
    ID3D11UnorderedAccessView* ppUAVClear[] = { nullptr, nullptr };

    //Opaque Render Pass
    pContext->OMSetRenderTargets(1, pRTV_MSAA.GetAddressOf(), pDSV_MSAA.Get());

    if (bgImg.valid()) {   
        pPSOImage->apply(pContext);
        renderBgImg(bgi, viewportSize, true);
        pContext->PSSetShaderResources(0, _countof(ppSRVClear), ppSRVClear);
    }


    if (rendOpts.facets) {
        this->updateScene(pConstBuffer, lighting, worldToD3vs, d3vsToD3ps);

        rendOpts.twoSided ? pPSOGeometryOpaqueCullNone->apply(pContext) : pPSOGeometryOpaqueCullBack->apply(pContext);
        renderTris(rendMeshes, rendOpts, false);
        pContext->PSSetShaderResources(0, _countof(ppSRVClear), ppSRVClear);
       
    }

    if (rendOpts.surfPoints) {   
        this->updateScene(pConstBuffer, Lighting{ Light{Vec3F{1,0,0}} }, worldToD3vs, d3vsToD3ps);
    
        ID3D11ShaderResourceView* mapViews[] = { whiteMap.view.get(),  blackMap.view.get(), noModulationMap.view.get() }; 
        pPSOGeometryOpaqueCullBack->apply(pContext);    
        pContext->PSSetShaderResources(0, _countof(mapViews), mapViews);
     
        for (RendMesh const& rendMesh : rendMeshes) {
            Mesh const& origMesh = rendMesh.origMeshN.cref();
            if (origMesh.surfPointNum() > 0) {
                D3dMesh& d3dMesh = getD3dMesh(rendMesh);
                if (d3dMesh.surfPoints) {   // Can be null if no surf points
                    setVertexBuffer(d3dMesh.surfPoints.get());
                    pContext->Draw(uint(origMesh.surfPointNum() * 60), 0);
                }
            }
        }
        pContext->PSSetShaderResources(0, _countof(ppSRVClear), ppSRVClear);    
    }

    // Transparen Render Pass
    pContext->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, pDSV_MSAA.Get(), 0, 2, std::data({ pUAVTextureHeadOIT.Get(), pUAVBufferLinkedListOIT.Get() }), std::data({ 0x0u, 0x0u }));

    if (rendOpts.wireframe) {
   
        this->updateScene(pConstBuffer, rendOpts.facets ? Vec3F(0, 0, 1) : Vec3F(1, 1, 1), worldToD3vs, d3vsToD3ps);
    
        pPSOWireFrame->apply(pContext);  
   
        for (auto const& rendMesh : rendMeshes) {
            auto const& origMesh = rendMesh.origMeshN.cref();
            // Must loop through origMesh surfaces in case the mesh has been emptied (and the redsurfs remain):
            for (size_t ss = 0; ss < origMesh.surfaces.size(); ++ss) {
                D3dSurf& d3dSurf = getD3dSurf(rendMesh.rendSurfs[ss]);
                Surf const& origSurf = origMesh.surfaces[ss];
                if (origSurf.empty())
                    continue;
                if (!d3dSurf.lineVerts)     // GPU needs updating
                    d3dSurf.lineVerts = makeVertBuff(makeLineVerts(rendMesh, origMesh, ss));
                setVertexBuffer(d3dSurf.lineVerts.get());
                ID3D11ShaderResourceView* mapViews[3];    // Albedo, specular resp.
                mapViews[0] = greyMap.view.get();
                if (rendOpts.shiny)
                    mapViews[1] = whiteMap.view.get();
                else if (d3dSurf.specularMap.valid())
                    mapViews[1] = d3dSurf.specularMap.view.get();
                else
                    mapViews[1] = blackMap.view.get();
                mapViews[2] = noModulationMap.view.get();
                pContext->PSSetShaderResources(0, 3, mapViews);
                pContext->Draw(uint(origSurf.numTris() * 6 + origSurf.numQuads() * 8), 0);
            }
        }
        pContext->PSSetShaderResources(0, _countof(ppSRVClear), ppSRVClear);
    }

    if (rendOpts.allVerts) {
        this->updateScene(pConstBuffer, Vec3F{ 0,1,0 }, worldToD3vs, d3vsToD3ps);
    
        ID3D11ShaderResourceView* mapViews[] = { whiteMap.view.get(),  blackMap.view.get(),  noModulationMap.view.get() };
        pPSOPoints->apply(pContext);
        pContext->PSSetShaderResources(0, _countof(mapViews), mapViews);
 
        for (RendMesh const& rendMesh : rendMeshes) {
            D3dMesh const& d3dMesh = getD3dMesh(rendMesh);
            if (d3dMesh.allVerts) {
                Mesh const& origMesh = rendMesh.origMeshN.cref();
                setVertexBuffer(d3dMesh.allVerts.get());
                pContext->Draw(uint(origMesh.verts.size()), 0);
            }
        }
     
    }
    if (rendOpts.markedVerts) {
        this->updateScene(pConstBuffer, Vec3F{ 1,1,0 }, worldToD3vs, d3vsToD3ps);
    
        ID3D11ShaderResourceView* mapViews[] = { whiteMap.view.get(),  blackMap.view.get(),  noModulationMap.view.get() };
        pPSOPoints->apply(pContext);
        pContext->PSSetShaderResources(0, _countof(mapViews), mapViews);
    
        for (RendMesh const& rendMesh : rendMeshes) {
            Mesh const& origMesh = rendMesh.origMeshN.cref();
            if (!origMesh.markedVerts.empty()) {
                D3dMesh& d3dMesh = getD3dMesh(rendMesh);
                if (d3dMesh.markedPoints) {   // Can be null if no marked verts
                    setVertexBuffer(d3dMesh.markedPoints.get());
                    pContext->Draw(uint(origMesh.markedVerts.size()), 0);
                }
            }
        }
    
        pContext->PSSetShaderResources(0, _countof(ppSRVClear), ppSRVClear);      
    }

    if (rendOpts.facets) {
        this->updateScene(pConstBuffer, lighting, worldToD3vs, d3vsToD3ps);
   
        pPSOGeometryTransparent->apply(pContext);     
        renderTris(rendMeshes, rendOpts, true);     
        pContext->PSSetShaderResources(0, _countof(ppSRVClear), ppSRVClear);      
    }

    pContext->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, _countof(ppUAVClear), ppUAVClear, nullptr);
    pMSAAResolver->apply(pContext, pRTV_MSAA, pRTVSwapChain, colorBufferFormat);
  

    pPSOGeometryResolve->apply(pContext);
    pContext->CSSetShaderResources(0, 2, std::data({ pSRVTextureHeadOIT.Get(), pSRVBufferLinkedListOIT.Get() }));
    pContext->CSSetUnorderedAccessViews(0, 1, pUAVSwapChain.GetAddressOf(), nullptr);
    pContext->Dispatch(threadGroupsX, threadGroupsY, 1);

    pContext->CSSetShaderResources(0, _countof(ppSRVClear), ppSRVClear);
    pContext->CSSetUnorderedAccessViews(0, _countof(ppUAVClear), ppUAVClear, nullptr);
        
}

void
D3d::showBackBuffer()
{
    // No render view during create - created by first resize:
    if (pRTVSwapChain == nullptr)
        return;
    FG_ASSERT_D3D(pSwapChain->Present(0, 0));
}

ImgC4UC
D3d::capture(Vec2UI viewportSize)
{
    HRESULT                     hr;
    WinPtr<ID3D11Texture2D>     pBuffer;
    {
        ID3D11Texture2D*        ptr;
        hr = pSwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),(LPVOID*)&ptr);
        FG_ASSERT_D3D(hr);
        pBuffer.reset(ptr);
    }
    D3D11_TEXTURE2D_DESC        desc;
    pBuffer->GetDesc(&desc);
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    desc.Usage = D3D11_USAGE_STAGING;
    WinPtr<ID3D11Texture2D>     pTexture;
    {
        ID3D11Texture2D*        ptr;
        hr = pDevice->CreateTexture2D(&desc,NULL,&ptr);
        FG_ASSERT_D3D(hr);
        pTexture.reset(ptr);
    }
    pContext->CopyResource(pTexture.get(),pBuffer.get());
    D3D11_MAPPED_SUBRESOURCE    resource;
    unsigned int                subresource = D3D11CalcSubresource(0,0,0);
    hr = pContext->Map(pTexture.get(),subresource,D3D11_MAP_READ_WRITE,0,&resource);
    FG_ASSERT_D3D(hr);
    ImgC4UC                     ret(viewportSize);
    uchar*                      dst = (uchar*)resource.pData;
    for (size_t rr=0; rr<viewportSize[1]; ++rr) {
        memcpy(&ret.xy(0,rr),dst,4ULL*viewportSize[0]);
        dst += size_t(resource.RowPitch);
    }
    return ret;
}

void
D3d::setVertexBuffer(ID3D11Buffer * vertBuff)
{
    UINT                stride[] = { sizeof(Vert) };
    UINT                offset[] = { 0 };
    ID3D11Buffer*       buffers[] = { vertBuff };
    // The way these buffers are interpreted by the HLSL is determined by 'IASetInputLayout' above:
    pContext->IASetVertexBuffers(0,1,buffers,stride,offset);
}

void
D3d::renderTris(RendMeshes const & rendMeshes,RendOptions const & rendOpts,bool transparentPass)
{
    size_t              mm {0};
    for (RendMesh const & rendMesh : rendMeshes) {
        if (rendMesh.posedVertsN.cref().empty())    // Not selected
            continue;
        Mesh const &        origMesh = rendMesh.origMeshN.cref();
        for (size_t ss=0; ss<origMesh.surfaces.size(); ++ss) {
            Surf const &        origSurf = origMesh.surfaces[ss];
            if (origSurf.empty())
                continue;
            RendSurf const &    rendSurf = rendMesh.rendSurfs[ss];
            bool                transparency =
                (rendSurf.albedoHasTransparencyN.val() || (rendOpts.albedoMode==AlbedoMode::byMesh));
            if (transparentPass == transparency) {
                D3dSurf &               d3dSurf = getD3dSurf(rendSurf);
                FGASSERT(d3dSurf.triVerts);
                setVertexBuffer(d3dSurf.triVerts.get());
                ID3D11ShaderResourceView*       mapViews[3];    // Albedo, Specular, Modulation resp.
                mapViews[2] = noModulationMap.view.get();
                if (rendOpts.albedoMode==AlbedoMode::map) {
                    if (d3dSurf.albedoMap.valid())
                        mapViews[0] = d3dSurf.albedoMap.view.get();
                    else
                        mapViews[0] = greyMap.view.get();
                    if (d3dSurf.modulationMap.valid())
                        mapViews[2] = d3dSurf.modulationMap.view.get();
                }
                else if (rendOpts.albedoMode==AlbedoMode::bySurf)
                    mapViews[0] = tintMaps[ss%tintMaps.size()].view.get();
                else if (rendOpts.albedoMode==AlbedoMode::byMesh)
                    mapViews[0] = tintTransMaps[mm%tintTransMaps.size()].view.get();
                else
                    mapViews[0] = greyMap.view.get();
                if (rendOpts.shiny)
                    mapViews[1] = whiteMap.view.get();
                else if (d3dSurf.specularMap.valid())
                    mapViews[1] = d3dSurf.specularMap.view.get();
                else
                    mapViews[1] = blackMap.view.get();
                pContext->PSSetShaderResources(0,3,mapViews);
                pContext->Draw(uint(origSurf.numTriEquivs())*3,0);
            }
        }
        ++mm;
    }


}

}

// */
