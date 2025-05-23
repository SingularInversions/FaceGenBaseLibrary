//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
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
//      X - image left to right, clip [-1,1]
//      Y - image bottom to top, clip [-1,1]
//      Z - inverse depth (near to far) [0,1]
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

#include "stdafx.h"

// The corresponding DLLs appear to always be included with Win 7SP1,8,10:
#pragma comment (lib,"dxgi.lib")
#pragma comment (lib,"d3d11.lib")
// However the following was NOT present on some windows machines so we don't use the compilation API:
// d3dcompiler.lib -> d3dcompiler_47.dll

#include "FgDirect3D.hpp"
#include "FgGuiWin.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dMesh.hpp"

#define FG_ASSERT_D3D(hr) handleHResult(__FILE__,__LINE__,hr)

using namespace std;

namespace Fg {

String8s            getGpusDescription()
{
    String8s                ret;
    IDXGIAdapter1 *         pAdapter; 
    IDXGIFactory1*          pFactory = nullptr; 
    if(FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory) ,(void**)&pFactory)))
        return ret;
    for (uint ii=0;pFactory->EnumAdapters1(ii,&pAdapter) != DXGI_ERROR_NOT_FOUND;++ii) {
        DXGI_ADAPTER_DESC1      desc;
        pAdapter->GetDesc1(&desc);
        ret.push_back("Adapter"+toStr(ii)+": "+String8(wstring(desc.Description)));
    }
    if(pFactory)
        pFactory->Release();
    return ret;
}

String8             getDefaultGpuDescription()
{
    String8s        gpus = getGpusDescription();
    if (gpus.empty())
        return "No GPUs detected";
    else
        return gpus[0];
}

void                PipelineState::attachVertexShader(ComPtr<ID3D11Device> pDevice)
{
    string              shaderIR = loadRawString(dataDir()+"base/shaders/dx11_shared_VS.cso");
    HRESULT             hr;
    hr = pDevice->CreateVertexShader(shaderIR.data(),shaderIR.size(),nullptr,m_pVertexShader.GetAddressOf());
    FG_ASSERT_HR(hr);
    Arr<pair<char const *,DXGI_FORMAT>,3>  semantics {
        {"POSITION",DXGI_FORMAT_R32G32B32_FLOAT},
        {"NORMAL",  DXGI_FORMAT_R32G32B32_FLOAT},
        {"TEXCOORD",DXGI_FORMAT_R32G32_FLOAT}
    };
    vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDescs;
    D3D11_INPUT_ELEMENT_DESC        elementDesc = {};
    for (auto sem : semantics) {
        elementDesc.SemanticName = sem.first;
        elementDesc.SemanticIndex = 0;
        elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
        elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        elementDesc.Format = sem.second;
        inputLayoutDescs.push_back(elementDesc);
    }
    hr = pDevice->CreateInputLayout(
        inputLayoutDescs.data(),static_cast<uint32_t>(inputLayoutDescs.size()),
        shaderIR.data(),shaderIR.size(),
        m_pInputLayout.ReleaseAndGetAddressOf());
    FG_ASSERT_HR(hr);
}

void                PipelineState::attachVertexShader2(ComPtr<ID3D11Device> pDevice)
{
    string              shaderIR = loadRawString(dataDir()+"base/shaders/dx11_transparent_VS.cso");
    HRESULT             hr;
    hr = pDevice->CreateVertexShader(shaderIR.data(),shaderIR.size(),nullptr,m_pVertexShader.GetAddressOf());
    FG_ASSERT_HR(hr);
    vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDescs;
    D3D11_INPUT_ELEMENT_DESC        elementDesc = {};
    elementDesc.SemanticName = "SV_VertexID";
    elementDesc.SemanticIndex = 0;
    elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    elementDesc.Format = DXGI_FORMAT_R32_UINT;
    inputLayoutDescs.push_back(elementDesc);
    hr = pDevice->CreateInputLayout(
        inputLayoutDescs.data(),static_cast<uint32_t>(inputLayoutDescs.size()),
        shaderIR.data(),shaderIR.size(),
        m_pInputLayout.ReleaseAndGetAddressOf());
    FG_ASSERT_HR(hr);
}

void                PipelineState::attachPixelShader(ComPtr<ID3D11Device> pDevice,string const & fname)
{
    string              shaderIR = loadRawString(dataDir()+"base/shaders/"+fname);
    HRESULT             hr = pDevice->CreatePixelShader(shaderIR.data(),shaderIR.size(),
        nullptr, m_pPixelShader.GetAddressOf());
    FG_ASSERT_HR(hr);
}

void                PipelineState::apply(ComPtr<ID3D11DeviceContext> pContext)
{
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pContext->IASetInputLayout(m_pInputLayout.Get());
    pContext->VSSetShader(m_pVertexShader.Get(),nullptr,0);
    pContext->PSSetShader(m_pPixelShader.Get(),nullptr,0);
}

D3d::D3d(HWND hwnd,NPT<RendMeshes> rmsN,NPT<double> lrsN,bool try_11_1) : rendMeshesN{rmsN}, logRelSize{lrsN}
{
    origMeshesDimsN = link1(rendMeshesN,[](RendMeshes const & rms)
    {
        Arr<Vec3F,2>        bounds {Vec3F{lims<float>::max()},Vec3F{lims<float>::lowest()}};
        for (RendMesh const & rm : rms)
            bounds = updateBounds(rm.origMeshN.val().verts,bounds);
        return multAcc(bounds,Arr2F{-1,1});
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
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferDesc.RefreshRate.Numerator = 60;
        desc.BufferDesc.RefreshRate.Denominator = 1;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.OutputWindow = hwnd;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Windowed = true;
        uint32_t            createFlag = 0;
#ifdef _DEBUG
        createFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        struct      Config {
            D3D_FEATURE_LEVEL   featureLevel;
            D3D_DRIVER_TYPE     driverType;
            DXGI_SWAP_EFFECT    swapEffect;
        };
        // We must attempt multiple setup calls until we find the best available driver.
        // Software driver for 11.1 with OIT didn't work (Win7SP1) so don't go there.
        Svec<Config>        configs;
        if (try_11_1) {
            // FLIP_DISCARD only supported by Win 10 but must be used in that case as some systems
            // with fail to consistently display buffer without it:
            configs.push_back({D3D_FEATURE_LEVEL_11_1,D3D_DRIVER_TYPE_HARDWARE,DXGI_SWAP_EFFECT_FLIP_DISCARD});
            configs.push_back({D3D_FEATURE_LEVEL_11_1,D3D_DRIVER_TYPE_HARDWARE,DXGI_SWAP_EFFECT_DISCARD});
        }
        configs.push_back({D3D_FEATURE_LEVEL_11_0,D3D_DRIVER_TYPE_HARDWARE,DXGI_SWAP_EFFECT_DISCARD});
            // Warp driver is very fast:
        configs.push_back({D3D_FEATURE_LEVEL_11_0,D3D_DRIVER_TYPE_WARP,DXGI_SWAP_EFFECT_DISCARD});
        configs.push_back({D3D_FEATURE_LEVEL_11_0,D3D_DRIVER_TYPE_REFERENCE,DXGI_SWAP_EFFECT_DISCARD});
        string              failString;
        for (auto config : configs) {
            supports11_1 = (config.featureLevel == D3D_FEATURE_LEVEL_11_1);
            supportsFlip = (config.swapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD);
            desc.SwapEffect = config.swapEffect;
            // If a system doesn't support 11.1 this returns E_INVALIDARG (0x80070057)
            // If a system doesn't support 11.0 this returns DXGI_ERROR_UNSUPPORTED (0x887A0004)
            // E_FAIL (0x80004005) "debug layer enabled but not installed" happens quite a bit ...
            // DXGI_ERROR_SDK_COMPONENT_MISSING (0x887A002D) SDK needed for debug layer ? only seen once
            hr = D3D11CreateDeviceAndSwapChain(     // Creates an immediate mode rendering context
                nullptr,                // Use first video adapter (card/driver) if more than one of this type
                config.driverType,
                nullptr,                // No software rasterizer DLL handle
                createFlag,
                &config.featureLevel,1,
                D3D11_SDK_VERSION,
                &desc,
                pSwapChain.GetAddressOf(),  // Returned
                pDevice.GetAddressOf(),     // Returned
                nullptr,nullptr);
            if (SUCCEEDED(hr))
                break;
            failString += "HR="+toHexString(scast<uint32>(hr))+" ";
        }
        if (FAILED(hr))
            throwWindows("No Direct3D 11.0 support",failString);
        this->initializeRenderTexture(Vec2UI(width,height));
        pDevice->GetImmediateContext(pContext.GetAddressOf());
    }
    opaquePassPSO.attachVertexShader(pDevice);
    opaquePassPSO.attachPixelShader(pDevice,"dx11_opaque_PS.cso");
    if (supports11_1) {
        transparentFirstPassPSO.attachVertexShader(pDevice);
        transparentFirstPassPSO.attachPixelShader(pDevice,"dx11_transparent_PS1.cso");
        transparentSecondPassPSO.attachVertexShader2(pDevice);
        transparentSecondPassPSO.attachPixelShader(pDevice,"dx11_transparent_PS2.cso");
    }
    {
        auto desc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
        desc.RenderTarget[0].RenderTargetWriteMask = 0;
        hr = pDevice->CreateBlendState(&desc,pBlendStateColorWriteDisable.GetAddressOf());
        FG_ASSERT_D3D(hr);
    }
    {
        auto desc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());    
        hr = pDevice->CreateBlendState(&desc,pBlendStateDefault.GetAddressOf());
        FG_ASSERT_D3D(hr);
    }
    {
        auto desc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
        desc.RenderTarget[0].BlendEnable = true;
        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;      
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;   
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;           
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
        desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;   
        hr = pDevice->CreateBlendState(&desc,pBlendStateLerp.GetAddressOf());
        FG_ASSERT_D3D(hr);
    }
    {
        auto desc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
        desc.DepthEnable = false;        
        hr = pDevice->CreateDepthStencilState(&desc,pDepthStencilStateDisable.GetAddressOf());
        FG_ASSERT_D3D(hr);
    }
    {
        auto desc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        hr = pDevice->CreateDepthStencilState(&desc,pDepthStencilStateWrite.GetAddressOf());
        FG_ASSERT_D3D(hr);
    }
    {
        auto desc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        hr = pDevice->CreateDepthStencilState(&desc,pDepthStencilStateWriteDisable.GetAddressOf());
        FG_ASSERT_D3D(hr);
    }
    {
        auto desc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
        hr = pDevice->CreateDepthStencilState(&desc,pDepthStencilStateDefault.GetAddressOf());
        FG_ASSERT_D3D(hr);
    }
    // Just in case 1x1 image has memory alignment and two-sided interpolation edge-case issues:
    greyMap = makeMap(ImgRgba8(Vec2UI(2,2),Rgba8(200,200,200,255)));
    blackMap = makeMap(ImgRgba8(Vec2UI(2,2),Rgba8(0,0,0,255)));
    whiteMap = makeMap(ImgRgba8(Vec2UI(2,2),Rgba8(255,255,255,255)));
    {
        // Need floating point for subdivision to work:
        auto                tintFn = [](float b,float t) -> Svec<Vec4UC>
        {
            float constexpr     c = 255;
            Svec<Vec4F>         init {{c,b,b,t},{c,c,b,t},{b,c,b,t},{b,c,c,t},{b,b,c,t},{c,b,c,t},};
            return mapCast<Vec4UC>(subdivideSimpleClosed(subdivideSimpleClosed(init)));
        };
        Svec<Vec4UC>            colsL = tintFn(200,255),
                                colsT = tintFn(200,127),
                                colsS = tintFn(100,255);
        size_t                  N = colsL.size();       // 6 * 2 * 2 = 24 colors
        tintMaps.reserve(N);
        for (size_t ii=0; ii<N; ++ii) {
            tintMaps.push_back(TintMap{
                makeMap(ImgRgba8{Vec2UI{2},Rgba8{colsL[ii].m}}),
                makeMap(ImgRgba8{Vec2UI{2},Rgba8{colsT[ii].m}}),
                makeMap(ImgRgba8{Vec2UI{2},Rgba8{colsS[ii].m}}),
            });
        }
    }
    noModulationMap = makeMap(ImgRgba8(Vec2UI(2,2),Rgba8(64,64,64,255)));
    icosahedron = reverseWinding(cIcosahedron());                               // CC to CW
}

D3d::~D3d()
{
    // Release any GPU data pointers held outside this object:
    RendMeshes const &      rms = rendMeshesN.val();
    for (RendMesh const & rm : rms) {
        rm.gpuData->reset();
        for (RendSurf const & rs : rm.rendSurfs)
            rs.gpuData->reset();
    }
    if (pContext != nullptr) {
        pContext->ClearState();
    }
}

void                D3d::initializeRenderTexture(Vec2UI windowSize)
{
    HRESULT         hr;
    {   // Create RTV from BackBuffer.
        // There is no need to hold the pointer to the buffer since it's held by the view and
        // will be automatically released when the view is released:
        ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = pSwapChain->GetBuffer(
            0,__uuidof(ID3D11Texture2D),reinterpret_cast<LPVOID*>(pBackBuffer.GetAddressOf()));
        FG_ASSERT_D3D(hr);
        hr = pDevice->CreateRenderTargetView(pBackBuffer.Get(),nullptr,pRTV.ReleaseAndGetAddressOf());
        FG_ASSERT_D3D(hr);
    }
    {   //Create DSV from Depth Buffer
        ComPtr<ID3D11Texture2D> pDepthBuffer;
        D3D11_TEXTURE2D_DESC desc = {};
        desc.ArraySize = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.Width = windowSize[0];
        desc.Height = windowSize[1];
        desc.MipLevels = 1;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        hr = pDevice->CreateTexture2D(&desc,nullptr,pDepthBuffer.GetAddressOf());
        FG_ASSERT_D3D(hr);
        hr = pDevice->CreateDepthStencilView(pDepthBuffer.Get(),nullptr,pDSV.ReleaseAndGetAddressOf());
        FG_ASSERT_D3D(hr);
    }
    if (!supports11_1)
        return;
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
        hr = pDevice->CreateTexture2D(&desc,nullptr,pTextureOIT.GetAddressOf());
        FG_ASSERT_D3D(hr);
        hr = pDevice->CreateUnorderedAccessView(
            pTextureOIT.Get(),nullptr,pUAVTextureHeadOIT.ReleaseAndGetAddressOf());
        FG_ASSERT_D3D(hr);
    }
    {   //Create LinkedList with atomic counter for OIT 
        struct ListNode
        {
            uint32_t  Next;
            uint32_t  Color;
            float     Depth;
        };
        uint32_t constexpr      maxAverageLayers = 16;
        ComPtr<ID3D11Buffer>    pBufferOIT;
        D3D11_BUFFER_DESC       obd = {};
        obd.ByteWidth = sizeof(ListNode) * windowSize[0] * windowSize[1] * maxAverageLayers;
        obd.CPUAccessFlags = 0;
        obd.BindFlags = (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
        obd.Usage = D3D11_USAGE_DEFAULT;
        obd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        obd.StructureByteStride = sizeof(ListNode);
        hr = pDevice->CreateBuffer(&obd,nullptr,pBufferOIT.GetAddressOf());
        FG_ASSERT_D3D(hr);
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavd = {};
        uavd.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavd.Buffer.FirstElement = 0;
        uavd.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
        uavd.Buffer.NumElements = windowSize[0] * windowSize[1] * maxAverageLayers;
        hr = pDevice->CreateUnorderedAccessView(
            pBufferOIT.Get(),&uavd,pUAVBufferLinkedListOIT.ReleaseAndGetAddressOf());
        FG_ASSERT_D3D(hr);
    }
}

void                D3d::resize(Vec2UI windowSize)
{
    if (pContext == nullptr)
        return;
    pContext->OMSetRenderTargets(0,nullptr,nullptr);
    pRTV.Reset();
    HRESULT             hr = pSwapChain->ResizeBuffers(2,windowSize[0],windowSize[1],DXGI_FORMAT_UNKNOWN,0);
    FG_ASSERT_D3D(hr);
    this->initializeRenderTexture(windowSize);
    CD3D11_VIEWPORT     viewport {0.0f,0.0f,scast<float>(windowSize[0]),scast<float>(windowSize[1])};
    pContext->RSSetViewports(1,&viewport);
}

WinPtr<ID3D11Buffer> D3d::makeConstBuff(const Scene & scene)
{
    D3D11_BUFFER_DESC       bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;             // read/write by GPU only
    bd.ByteWidth = sizeof(Scene);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    ID3D11Buffer*           ptr;
    HRESULT                 hr = pDevice->CreateBuffer(&bd,nullptr,&ptr);
    FG_ASSERT_D3D(hr);
    pContext->UpdateSubresource(ptr,0,nullptr,&scene,0,0);
    return WinPtr<ID3D11Buffer>(ptr);
}

WinPtr<ID3D11Buffer> D3d::makeVertBuff(const Verts & verts)
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
WinPtr<ID3D11Buffer> D3d::makeSurfPoints(RendMesh const & rendMesh,Mesh const & origMesh)
{
    // This isn't quite right since it will recalc when anything in rendMeshesN changes ...
    // but it's not clear what the perfect solution would look like.
    // Use the median dimension because we don't want use of a full body mesh to make the
    // markers too small to see, and use of a very flat face cutout to make them too big.
    float                   sz = cMedian(origMeshesDimsN.val().m) * (1.0f / 160.0f),
                            relSize = float(exp(logRelSize.node.val())),
                            scale = cMax(1.0f,relSize*2);
    sz /= scale;            // Shrink points as they are zoomed in for greater visual accuracy
    Verts                   verts;
    Vec3Fs const &          rendVerts = rendMesh.shapeVertsN.val();
    for (Surf const & origSurf : origMesh.surfaces) {
        for (SurfPointName const & sp : origSurf.surfPoints) {
            Vec3F               pos = origSurf.surfPointPos(rendVerts,sp.point);
            for (Arr3UI tri : icosahedron.tris) {
                for (uint idx : tri) {
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
WinPtr<ID3D11Buffer> D3d::makeMarkedVerts(RendMesh const & rendMesh,Mesh const & origMesh)
{
    Verts               verts;
    Vec3Fs const &      rendVerts = rendMesh.shapeVertsN.val();
    for (MarkedVert const & mv : origMesh.markedVerts) {
        Vert            v;
        v.pos = rendVerts[mv.idx];
        v.norm = Vec3F{0,0,1};      // Random non-zero value can be normalized by shader
        v.uv = Vec2F{0};
        verts.push_back(v);
    }
    return makeVertBuff(verts);
}

WinPtr<ID3D11Buffer> D3d::makeAllVerts(Vec3Fs const & verts)
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

D3d::Verts          D3d::makeVertList(
    RendMesh const &        rendMesh,
    Mesh const &            origMesh,
    size_t                  surfNum,
    bool                    shadeFlat)
{
    D3d::Verts              vertList;
    Surf const &            origSurf = origMesh.surfaces[surfNum];
    SurfNormals const &     normals = rendMesh.normalsN.val();
    Vec3Fs const &          norms = normals.vert;
    FacetNormals const &    normFlats = normals.facet[surfNum];
    Vec3Fs const &          verts = rendMesh.shapeVertsN.val();
    vertList.reserve(3*size_t(origSurf.numTriEquivs()));
    bool                    hasUVs = origSurf.hasUvIndices();
    for (size_t tt=0; tt<origSurf.numTriEquivs(); ++tt) {
        Arr3UI                  vertInds = origSurf.getTriEquivVertInds(tt),
                                uvInds = hasUVs ? origSurf.getTriEquivUvInds(tt) : Arr3UI{0};
        for (uint ii=2; ii<3; --ii) {   // Reverse order due to D3D LEFT-handed coordinate system
            Vert                v;
            size_t              vertIdx = vertInds[ii];
            v.pos = verts[vertIdx];
            if (shadeFlat)
                v.norm = normFlats.triEquiv(tt);
            else
                v.norm = norms[vertIdx];
            if (hasUVs) {
                v.uv = origMesh.uvs[uvInds[ii]];
                v.uv[1] = 1.0f - v.uv[1];               // Convert from OTCS to D3TCS
            }
            vertList.push_back(v);
        }
    }
    return vertList;
}

D3d::Verts          D3d::makeLineVerts(RendMesh const & rendMesh,Mesh const & origMesh,size_t surfNum) {
    D3d::Verts              ret;
    Vec3Fs const &          verts = rendMesh.shapeVertsN.val();
    Surf const &            origSurf = origMesh.surfaces[surfNum];
    Arr3UIs const &         tris = origSurf.tris.vertInds;
    Arr4UIs const &         quads = origSurf.quads.vertInds;
    ret.reserve(8*quads.size()+6*tris.size());
    for (Arr3UI tri : tris) {
        for (uint ii=0; ii<3; ++ii) {
            ret.push_back(Vert{verts[tri[ii]],{0,0,1},{0,0}});
            ret.push_back(Vert{verts[tri[(ii+1)%3]],{0,0,1},{0,0}});
        }
    }
    for (Arr4UI quad : quads) {
        for (uint ii=0; ii<4; ++ii) {
            ret.push_back(Vert{verts[quad[ii]],{0,0,1},{0,0}});
            ret.push_back(Vert{verts[quad[(ii+1)%4]],{0,0,1},{0,0}});
        }
    }
    return ret;
}

D3d::Verts          D3d::makeFlagVerts(RendMesh const & rendMesh,Mesh const & origMesh,size_t surfNum) {
    D3d::Verts              ret;
    Vec3Fs const &          verts = rendMesh.shapeVertsN.val();
    Surf const &            origSurf = origMesh.surfaces[surfNum];
    Arr3UIs const &         tris = origSurf.tris.vertInds;
    size_t                  T = origSurf.tris.size();
    FGASSERT(origSurf.edgeFlags.size() == T);
    for (size_t tt=0; tt<T; ++tt) {
        for (uint ee=0; ee<3; ++ee) {
            if (origSurf.edgeFlags[tt][ee]) {
                Arr2UI              edge = getEdge(tris[tt],ee);
                for (uint idx : edge)
                    ret.push_back(Vert{verts[idx],{0,0,1},{0,0}});
            }
        }
    }
    return ret;
}

WinPtr<ID3D11Texture2D> D3d::loadMap(ImgRgba8 const & map) {
    FGASSERT(!map.empty());
    ImgRgba8s               mipmap;
    if (isPow2(map.dims()))
        mipmap = cMipmap(map);
    else {                  // power of 2 images are strongly preferred by GPUs so make it so:
        // TODO: resampling is a slow way to do this and gives noticable lag for large images.
        ImgRgba8            p2 = scaleResample(map,mapPow2Ceil(map.dims()));
        mipmap = cMipmap(p2);
    }
    uint                    numMips = cMin(scast<uint>(mipmap.size()),8U);
    D3D11_SUBRESOURCE_DATA  initData[8] = {};
    for (size_t mm=0; mm<numMips; ++mm) {
        initData[mm].pSysMem = mipmap[mm].dataPtr();
        initData[mm].SysMemPitch = mipmap[mm].width()*4;
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

WinPtr<ID3D11ShaderResourceView> D3d::makeMapView(ID3D11Texture2D* mapPtr)
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

D3dMap              D3d::makeMap(ImgRgba8 const & map)
{
    D3dMap             ret;
    ret.map = loadMap(map);
    ret.view = makeMapView(ret.map.get());
    return ret;
}

WinPtr<ID3D11SamplerState> D3d::makeSamplerState()
{
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;      // Always use trilinear
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;                    // Make use of all mipmap levels available
    ID3D11SamplerState* ptr;
    HRESULT             hr = pDevice->CreateSamplerState(&sampDesc,&ptr);
    FG_ASSERT_D3D(hr);
    return WinPtr<ID3D11SamplerState>(ptr);
}

WinPtr<ID3D11Buffer> D3d::setScene(Scene const & scene)
{
    WinPtr<ID3D11Buffer>        sceneBuff = makeConstBuff(scene);
    {
        ID3D11Buffer*           sbs[] = { sceneBuff.get() };
        pContext->VSSetConstantBuffers(0,1,sbs);
        pContext->PSSetConstantBuffers(0,1,sbs);
    }
    return sceneBuff;
}

WinPtr<ID3D11Buffer> D3d::makeScene(Lighting lighting,Mat44F worldToD3vs,Mat44F d3vsToD3ps,float detModStrength)
{
    lighting.lights.resize(2);                          // Ensure we have 2
    Scene                   scene;
    scene.mvm = worldToD3vs.transpose();                // D3D is column-major (of course)
    scene.projection = d3vsToD3ps.transpose();          // "
    scene.ambient = append(lighting.ambient,1.0f);
    for (uint ii=0; ii<2; ++ii) {
        Vec3F               d = lighting.lights[ii].direction;
        scene.lightDir[ii] = {d[0],d[1],-d[2],1.0f};    // OECS to D3VS homogenous position
        scene.lightColor[ii] = append(lighting.lights[ii].colour,1.0f);
    }
    scene.detTexMod = {detModStrength,0,0,0};
    return setScene(scene);
}

// Ambient-only version:
WinPtr<ID3D11Buffer> D3d::makeScene(Vec3F ambient,Mat44F worldToD3vs,Mat44F d3vsToD3ps,float detModStrength)
{
    Light             diffuseBlack(Vec3F(0),Vec3F(0,0,1));
    Lighting          lighting(ambient,{diffuseBlack,diffuseBlack});
    return makeScene(lighting,worldToD3vs,d3vsToD3ps,detModStrength);
}

void                D3d::setBgImage(BackgroundImage const & bgi)
{
    bgImg.reset();
    ImgRgba8 const &    img = bgi.imgN.val();
    if (img.empty())
        return;
    Vec2UI              p2dims = mapMin(mapPow2Ceil(img.dims()),maxMapSize);
    ImgRgba8            map(p2dims);
    imgResize_(img,map);
    bgImg = makeMap(map);
}

void                D3d::renderBgImg(BackgroundImage const & bgi, Vec2UI viewportSize, bool  transparentPass) {
    WinPtr<ID3D11RasterizerState> rasterizer;
    {
        D3D11_RASTERIZER_DESC       rd = {};
        rd.FillMode = D3D11_FILL_SOLID;
        rd.CullMode = D3D11_CULL_NONE; // : D3D11_CULL_BACK;
        ID3D11RasterizerState*      ptr;
        pDevice->CreateRasterizerState(&rd,&ptr);
        rasterizer.reset(ptr);
    }
    pContext->RSSetState(rasterizer.get());
    WinPtr<ID3D11Buffer>            bgImageVerts;
    {   // Background image polys:
        Vec2F        im = Vec2F(bgi.origDimsN.val()),
                        xr(im[0]*viewportSize[1],im[1]*viewportSize[0]);
        Vec2F        sz = Vec2F(xr) / cMaxElem(xr) * scast<float>(exp(bgi.lnScale.val())),
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
    scene.mvm = cMatDiag<float,4>(1);
    scene.projection = cMatDiag<float,4>(1);
    scene.ambient = Vec4F(1);
    if (transparentPass) {
        double                      ft = clamp(bgi.foregroundTransparency.val(),0.0,1.0);
        scene.ambient[3] = static_cast<float>(ft);
    }
    WinPtr<ID3D11Buffer>        sceneBuff = setScene(scene);
    pContext->Draw(6,0);
}

D3dMesh &           D3d::getD3dMesh(RendMesh const & rm) const
{
    Any &               gpuMesh = *rm.gpuData;
    if (!gpuMesh.valid())
        gpuMesh.reset(D3dMesh{cUpdateFlagT(rm.shapeVertsN)});
    return gpuMesh.ref<D3dMesh>();
}

D3dSurf &           D3d::getD3dSurf(RendSurf const & rs) const
{
    Any &               gpuSurf = *rs.gpuData;
    if (!gpuSurf.valid()) {
        gpuSurf.reset(D3dSurf{
            rs.smoothMapN,
            rs.modulationMapN,
            rs.specularMapN,
        });
    }
    return gpuSurf.ref<D3dSurf>();
}

void                D3d::updateMap_(DfFPtr const & flag,NPT<ImgRgba8> const & in,D3dMap & out)
{
    if (flag && flag->checkUpdate()) {
        out.reset();
        if (in.ptr && !in.val().empty())
            out = makeMap(in.val());
    }
}

void                D3d::renderBackBuffer(
    BackgroundImage const &     bgi,
    Lighting                    lighting,       // OECS
    Mat44F                      worldToD3vs,    // modelview
    Mat44F                      d3vsToD3ps,     // projection
    Vec2UI                      viewportSize,
    RendOptions const &         rendOpts,
    float                       texModStrength,
    bool                        backgroundTransparent)
{
    // No render view during create - created by first resize:
    if (pRTV == nullptr)
        return;
    // Update 'd3dMeshes' (mesh and map data) if required:
    RendMeshes const &      rendMeshes = rendMeshesN.val();
    for (RendMesh const & rendMesh : rendMeshes) {
        Mesh const &        origMesh = rendMesh.origMeshN.val();
        D3dMesh &           d3dMesh = getD3dMesh(rendMesh);
        bool                vertsChanged = d3dMesh.vertsFlag->checkUpdate();
        bool                trisChanged = vertsChanged || (flatShaded != rendOpts.flatShaded);                          
        bool                valid = !origMesh.verts.empty();
        // Easier to always update these than have a flag for each and only update when resp. option selected:
        if (vertsChanged) {
            d3dMesh.allVerts.reset();
            d3dMesh.markedPoints.reset();
            if (valid) {
                d3dMesh.allVerts = makeAllVerts(rendMesh.shapeVertsN.val());
                d3dMesh.markedPoints = makeMarkedVerts(rendMesh,origMesh);
            }
        }
        if (vertsChanged || logRelSize.checkUpdate()) {
            d3dMesh.surfPoints.reset();
            if (valid)
                d3dMesh.surfPoints = makeSurfPoints(rendMesh,origMesh);
        }
        // TODO: don't duplicate shared maps ! (eg. multisurface mesh)
        // Be forgiving to client; for instance model set may define different number of maps
        // than .fgmesh defines surfaces. If fewer defined then render without maps:
        size_t              S = cMin(rendMesh.rendSurfs.size(),origMesh.surfaces.size());
        for (size_t ss=0; ss<S; ++ss) {
            RendSurf const &        rs = rendMesh.rendSurfs[ss];
            D3dSurf &               d3dSurf = getD3dSurf(rs);
            updateMap_(d3dSurf.albedoMapFlag,rs.smoothMapN,d3dSurf.albedoMap);
            updateMap_(d3dSurf.modulationMapFlag,rs.modulationMapN,d3dSurf.modulationMap);
            updateMap_(d3dSurf.specularMapFlag,rs.specularMapN,d3dSurf.specularMap);
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

    // RENDER:
    
    WinPtr<ID3D11Buffer>        sceneBuff;
    WinPtr<ID3D11SamplerState>  samplerState = makeSamplerState();
    ID3D11SamplerState *        sss[] {samplerState.get()};
    pContext->PSSetSamplers(0,1,sss);
    //Opaque pass
    pContext->ClearDepthStencilView(pDSV.Get(),D3D11_CLEAR_DEPTH,1.0f,0);
    pContext->OMSetBlendState(pBlendStateDefault.Get(),nullptr,0xFFFFFFFF);
    // Needed for both paths since 11.1 with facets disabled doesn't overwrite this buffer:
    Arr<float,3>                bg = rendOpts.backgroundColor.m;
    Arr<float,4>                bgColor {bg[0],bg[1],bg[2],255};
    if (backgroundTransparent)
        bgColor[3] = 0;
    pContext->ClearRenderTargetView(pRTV.Get(),bgColor.begin());
    pContext->OMSetRenderTargets(1,dataPtr({pRTV.Get()}),pDSV.Get());
    opaquePassPSO.apply(pContext);
    if (bgImg.valid()) {
        pContext->OMSetDepthStencilState(pDepthStencilStateDisable.Get(),0);
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        renderBgImg(bgi,viewportSize,false);
    }
    pContext->OMSetDepthStencilState(pDepthStencilStateDefault.Get(),0);
    if (rendOpts.facets) {
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        sceneBuff = makeScene(lighting,worldToD3vs,d3vsToD3ps,texModStrength);
        WinPtr<ID3D11RasterizerState>   rasterizer;
        {
            D3D11_RASTERIZER_DESC       rd = {};
            rd.FillMode = D3D11_FILL_SOLID;
            rd.CullMode = rendOpts.twoSided ? D3D11_CULL_NONE : D3D11_CULL_BACK;
            if (rendOpts.wireframe || rendOpts.allVerts || rendOpts.surfPoints || rendOpts.markedVerts) {
                // Increase depth values small amount to wireframe & allverts get shown.
                // Only do this when necessary just in case some small confusing artifacts result.
                rd.DepthBias = 1000;
                rd.SlopeScaledDepthBias = 0.5f;
                rd.DepthBiasClamp = 0.001f;
            }
            ID3D11RasterizerState* ptr;
            pDevice->CreateRasterizerState(&rd,&ptr);
            rasterizer.reset(ptr);
        }
        pContext->RSSetState(rasterizer.get());
        renderTris(rendMeshes,rendOpts,false);
        if (supports11_1) {
            //  OIT Pass 1
            // Re-creating pUAVBufferLinkedListOIT here did not fix the GTX 980 visibility issue
            pContext->ClearUnorderedAccessViewUint(
                pUAVTextureHeadOIT.Get(),dataPtr({0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}));
            pContext->OMSetRenderTargetsAndUnorderedAccessViews(
                1,pRTV.GetAddressOf(),pDSV.Get(),1,2,
                dataPtr({pUAVTextureHeadOIT.Get(),pUAVBufferLinkedListOIT.Get()}),
                dataPtr({0x0u,0x0u }));
            pContext->OMSetBlendState(pBlendStateColorWriteDisable.Get(),nullptr,0xFFFFFFFF);
            pContext->OMSetDepthStencilState(pDepthStencilStateWriteDisable.Get(),0);
            transparentFirstPassPSO.apply(pContext);
            renderTris(rendMeshes,rendOpts,true);
            // OIT Pass 2
            pContext->OMSetBlendState(pBlendStateLerp.Get(),nullptr,0xFFFFFFFF);
            pContext->OMSetDepthStencilState(pDepthStencilStateDisable.Get(),0);
            transparentSecondPassPSO.apply(pContext);
            // This vertex shader uses 'SV_VertexID' so generates indices and doesn't need a bound vertex buffer:
            pContext->IASetVertexBuffers(0,0,nullptr,nullptr,nullptr);
            pContext->Draw(3,0);
        }
        else
            renderTris(rendMeshes,rendOpts,true);
    }
    pContext->OMSetRenderTargets(1,dataPtr({pRTV.Get()}),pDSV.Get());
    pContext->OMSetDepthStencilState(pDepthStencilStateDefault.Get(),0);
    opaquePassPSO.apply(pContext);
    if (rendOpts.surfPoints) {
        sceneBuff = makeScene(Lighting{Light{Vec3F{1,0,0}}},worldToD3vs,d3vsToD3ps);
        ID3D11ShaderResourceView*       mapViews[3];
        mapViews[0] = whiteMap.view.get();
        mapViews[1] = blackMap.view.get();              // No specular on point spheres
        mapViews[2] = noModulationMap.view.get();
        pContext->PSSetShaderResources(0,3,mapViews);
        for (RendMesh const & rendMesh : rendMeshes) {
            Mesh const &        origMesh = rendMesh.origMeshN.val();
            if (origMesh.surfPointNum() > 0) {
                D3dMesh &       d3dMesh = getD3dMesh(rendMesh);
                if (d3dMesh.surfPoints) {   // Can be null if no surf points
                    setVertexBuffer(d3dMesh.surfPoints.get());
                    pContext->Draw(uint(origMesh.surfPointNum()*60),0);
                }
            }
        }
    }
    if (rendOpts.wireframe) {
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        if (rendOpts.facets)        // Render wireframe blue over facets, white otherwise:
            sceneBuff = makeScene({0,0,1},worldToD3vs,d3vsToD3ps);
        else
            sceneBuff = makeScene({1,1,1},worldToD3vs,d3vsToD3ps);
        WinPtr<ID3D11RasterizerState>   rasterizer;
        {
            D3D11_RASTERIZER_DESC       rd = {};
            rd.FillMode = D3D11_FILL_WIREFRAME;
            rd.CullMode = rendOpts.twoSided ? D3D11_CULL_NONE : D3D11_CULL_BACK;
            ID3D11RasterizerState* ptr;
            pDevice->CreateRasterizerState(&rd,&ptr);
            rasterizer.reset(ptr);
        }
        pContext->RSSetState(rasterizer.get());
        for (RendMesh const & rendMesh : rendMeshes) {
            Mesh const &        origMesh = rendMesh.origMeshN.val();
            size_t              S = cMin(origMesh.surfaces.size(),rendMesh.rendSurfs.size());
            //if (origMesh.surfaces.size() != rendMesh.rendSurfs.size())
            //    fgout << fgnl << "WARNING: wireframe render surf counts differ: "
            //        << origMesh.surfaces.size() << " != " << rendMesh.rendSurfs.size();
            // Must loop through origMesh surfaces in case the mesh has been emptied (and the redsurfs remain):
            for (size_t ss=0; ss<S; ++ss) {
                D3dSurf &           d3dSurf = getD3dSurf(rendMesh.rendSurfs[ss]);
                Surf const &        origSurf = origMesh.surfaces[ss];
                if (origSurf.empty())
                    continue;
                if (!d3dSurf.lineVerts) {   // GPU needs updating
                    if (origSurf.edgeFlags.empty())
                        d3dSurf.lineVerts = makeVertBuff(makeLineVerts(rendMesh,origMesh,ss));
                    else
                        d3dSurf.lineVerts = makeVertBuff(makeFlagVerts(rendMesh,origMesh,ss));
                }
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
                pContext->PSSetShaderResources(0,3,mapViews);
                pContext->Draw(uint(origSurf.tris.size() * 6 + origSurf.quads.size() * 8), 0);
            }
        }
    }
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    ID3D11ShaderResourceView *  mapViews[3] {whiteMap.view.get(),blackMap.view.get(),noModulationMap.view.get()};
    if (rendOpts.allVerts) {
        sceneBuff = makeScene({1,1,1},worldToD3vs,d3vsToD3ps);
        for (size_t mm=0; mm<rendMeshes.size(); ++mm) {
            RendMesh const &        rendMesh = rendMeshes[mm];
            D3dMesh const &         d3dMesh = getD3dMesh(rendMesh);
            if (d3dMesh.allVerts) {
                Mesh const &        origMesh = rendMesh.origMeshN.val();
                mapViews[0] = tintMaps[selectTintMap(mm,rendMeshes.size())].strong.view.get();
                pContext->PSSetShaderResources(0,3,mapViews);
                setVertexBuffer(d3dMesh.allVerts.get());
                pContext->Draw(uint(origMesh.verts.size()),0);
            }
        }
    }
    mapViews[0] = whiteMap.view.get();      // modified by allverts render above
    pContext->PSSetShaderResources(0,3,mapViews);
    if (rendOpts.markedVerts) {
        sceneBuff = makeScene({1,1,0},worldToD3vs,d3vsToD3ps);
        for (RendMesh const & rendMesh : rendMeshes) {
            Mesh const &        origMesh = rendMesh.origMeshN.val();
            if (!origMesh.markedVerts.empty()) {
                D3dMesh &       d3dMesh = getD3dMesh(rendMesh);
                if (d3dMesh.markedPoints) {   // Can be null if no marked verts
                    setVertexBuffer(d3dMesh.markedPoints.get());
                    pContext->Draw(uint(origMesh.markedVerts.size()),0);
                }
            }
        }
    }
    if (bgImg.valid()) {
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pContext->OMSetDepthStencilState(pDepthStencilStateDisable.Get(),0);
        pContext->OMSetBlendState(pBlendStateLerp.Get(),nullptr,0xFFFFFFFF);
        renderBgImg(bgi,viewportSize,true);
    }
    pContext->OMSetRenderTargets(0,nullptr,nullptr);
}

void                D3d::showBackBuffer()
{
    // No render view during create - created by first resize:
    if (pRTV == nullptr)
        return;
    // TODO: arguments below can be used for frame sync.
    // TODO: switch to Present1()
    HRESULT  hr = pSwapChain->Present(0,0);      // Swap back buffer to display
    FG_ASSERT_D3D(hr);
}

ImgRgba8            D3d::capture(Vec2UI viewportSize)
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
    ImgRgba8                     ret(viewportSize);
    uchar*                      dst = (uchar*)resource.pData;
    for (size_t rr=0; rr<viewportSize[1]; ++rr) {
        memcpy(&ret.xy(0,rr),dst,4ULL*viewportSize[0]);
        dst += size_t(resource.RowPitch);
    }
    return ret;
}

void                D3d::setVertexBuffer(ID3D11Buffer * vertBuff)
{
    UINT                stride[] = { sizeof(Vert) };
    UINT                offset[] = { 0 };
    ID3D11Buffer*       buffers[] = { vertBuff };
    // The way these buffers are interpreted by the HLSL is determined by 'IASetInputLayout' above:
    pContext->IASetVertexBuffers(0,1,buffers,stride,offset);
}

size_t              D3d::selectTintMap(size_t idx,size_t num) const
{
    // maximize color diffs, handle M=0 and M>tintTransMaps.size():
    size_t              colorStep = cMax(tintMaps.size()/cMax(num,size_t(1)),size_t(1));
    return (idx*colorStep) % tintMaps.size();
}

void                D3d::renderTris(RendMeshes const & rendMeshes,RendOptions const & rendOpts,bool transparentPass)
{
    size_t              M = rendMeshes.size();
    for (size_t mm=0; mm<M; ++mm) {
        RendMesh const & rendMesh = rendMeshes[mm];
        if (rendMesh.shapeVertsN.val().empty())    // Not selected
            continue;
        Mesh const &        origMesh = rendMesh.origMeshN.val();
        size_t              S = cMin(origMesh.surfaces.size(),rendMesh.rendSurfs.size());
        for (size_t ss=0; ss<S; ++ss) {
            Surf const &        origSurf = origMesh.surfaces[ss];
            if (origSurf.empty())
                continue;
            RendSurf const &    rendSurf = rendMesh.rendSurfs[ss];
            bool                transparency {false};
            if (rendSurf.albedoHasTransparencyN.valid() && rendSurf.albedoHasTransparencyN.val())
                transparency = true;
            if (rendOpts.albedoMode==AlbedoMode::byMesh)
                transparency = true;
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
                else if (rendOpts.albedoMode==AlbedoMode::bySurf) {
                    size_t              tintIdx = selectTintMap(ss,S);
                    mapViews[0] = tintMaps[tintIdx].light.view.get();
                }
                else if (rendOpts.albedoMode==AlbedoMode::byMesh) {
                    size_t              tintIdx = selectTintMap(mm,M);
                    mapViews[0] = tintMaps[tintIdx].trans.view.get();
                }
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
    }
}

void                D3d::handleHResult(char const * fpath,uint lineNum,HRESULT hr)
{
    if (hr == DXGI_ERROR_DEVICE_REMOVED) {
        HRESULT             hr2 = pDevice->GetDeviceRemovedReason();
        if (hr2 == S_OK)
            return;                         // Not actually an error after all ... WTF
        if (hr2 == DXGI_ERROR_INVALID_CALL)
            hr = hr2;                       // Report this error instead
        else
            throw ExceptD3dDeviceRemoved {};
    }
    if (hr < 0) {
        string          v11_1 = supports11_1 ? " D3D11.1" : " D3D11.0",     // wrong way around throug Mod v3.22
                        flip = supportsFlip ? " flip" : " noflip";
        throwWindows("D3D HRESULT",
            pathToName(fpath)+":"+toStr(lineNum)+":HR="+toHexString(scast<uint32>(hr))+v11_1+flip);
    }
}

}

// */
