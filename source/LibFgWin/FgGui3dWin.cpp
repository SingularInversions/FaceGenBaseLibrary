//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// * Target D3D feature level 11.0 (released 11.02) and DXGI 1.1 (released 12.08) as the highest versions
//   guaranteed in Windows 7 (by software driver if necessary).
// * D3D 11.1 API is avail for Win7 but not necessarily supported by hardware and doesn't have anything
//   we need.
// * D3D 11.0 is the earliest version to use shader model 5.0. As of 19.08 you can't buy a standalone GPU
//   that doesn't support 11.0, and only the very lowest Intel CPU onboard GPU doesn't.
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

#include "stdafx.h"

#include <d3d11_1.h>
#include <d3dcompiler.h>

#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"d3dcompiler.lib")

#include "FgGuiApi3d.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgCoordSystem.hpp"

using namespace std;

namespace Fg {

template<class T>
using Unique = std::unique_ptr<T,Release<T> >;

template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace {

struct  Map
{
    Unique<ID3D11Texture2D>             map;        // Can be empty
    Unique<ID3D11ShaderResourceView>    view;       // Empty iff above is empty

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
    Unique<ID3D11Buffer>        triVerts;       // 3 Verts for each tri. Null if no facets for this surf.
    Unique<ID3D11Buffer>        lineVerts;      // 2 Verts for each edge. Null if no facets or computation delayed.
    Map                         albedoMap;      // Can be empty
    Map                         specularMap;    // "

    void reset() {triVerts.reset(); lineVerts.reset(); albedoMap.reset(); specularMap.reset(); }
};

struct D3dMesh
{
    Unique<ID3D11Buffer>        surfPoints;
    Unique<ID3D11Buffer>        markedPoints;
    Unique<ID3D11Buffer>        allVerts;

    void reset() {surfPoints.reset(); markedPoints.reset(); }
};



class GraphicsPSO {
public:
	enum class ShaderType {
		Vertex,
		Pixel
	};
public:

	GraphicsPSO(ComPtr<ID3D11Device> pDevice) {
		m_pDevice = pDevice;
	}

	auto attachShader(std::wstring const& fileName, std::string const& shaderName, ShaderType type) -> void {

		auto compileShader = [](auto file, auto entrypoint, auto target) -> ComPtr<ID3DBlob> {

			ComPtr<ID3DBlob> pCodeBlob;
			ComPtr<ID3DBlob> pErrorBlob;

			uint32_t shaderFlags = 0;
#ifdef _DEBUG
			shaderFlags |= D3DCOMPILE_DEBUG;
			shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
			shaderFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif


			if (FAILED(D3DCompileFromFile(file.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.c_str(), target, shaderFlags, 0, pCodeBlob.GetAddressOf(), pErrorBlob.GetAddressOf())))
				throw std::runtime_error(static_cast<const char*>(pErrorBlob->GetBufferPointer()));
			return pCodeBlob;
		};

		switch (type) {
		case ShaderType::Vertex: {
			auto pShaderBlob = compileShader(fileName, shaderName, "vs_5_0");
			DX::ThrowIfFailed(m_pDevice->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));

			Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pShaderReflection;
			DX::ThrowIfFailed(D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_PPV_ARGS(&pShaderReflection)));

			D3D11_SHADER_DESC shaderDesc = {};
			pShaderReflection->GetDesc(&shaderDesc);

			std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDescs;

			for (auto index = 0u; index < shaderDesc.InputParameters; index++) {
				D3D11_SIGNATURE_PARAMETER_DESC paramDesc = {};
				pShaderReflection->GetInputParameterDesc(index, &paramDesc);

				// fill out input element desc
				D3D11_INPUT_ELEMENT_DESC elementDesc = {};
				elementDesc.SemanticName = paramDesc.SemanticName;
				elementDesc.SemanticIndex = paramDesc.SemanticIndex;
				elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

				// determine DXGI format
				if (paramDesc.Mask == 1) {
					if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
				}
				else if (paramDesc.Mask <= 3) {
					if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
				}
				else if (paramDesc.Mask <= 7) {
					if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				}
				else if (paramDesc.Mask <= 15) {
					if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
					else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				}

				//save element desc
				inputLayoutDescs.push_back(elementDesc);
			}


			DX::ThrowIfFailed(m_pDevice->CreateInputLayout(std::data(inputLayoutDescs), static_cast<uint32_t>(std::size(inputLayoutDescs)), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), m_pInputLayout.ReleaseAndGetAddressOf()));
			break;
		}
		case ShaderType::Pixel: {
			auto pShaderBlob = compileShader(fileName, shaderName, "ps_5_0");
			DX::ThrowIfFailed(m_pDevice->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));
			break;
		}
		default:
			break;
		}

	}

	auto applay(ComPtr<ID3D11DeviceContext> pContext) -> void {

		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pContext->IASetInputLayout(m_pInputLayout.Get());
		pContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
		pContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
	
	}


private:
	ComPtr<ID3D11Device>            m_pDevice;
	ComPtr<ID3D11VertexShader>      m_pVertexShader;
	ComPtr<ID3D11PixelShader>       m_pPixelShader;
	ComPtr<ID3D11InputLayout>       m_pInputLayout;

};



//Better use CompPtr. This stadart praction for application on DX
struct D3d {
    uint                            maxMapSize = 4096;  // Play it safe
	uint32_t                        layerCount = 32;    //Layer count for trasnaperent
    // Created in constructor:
	ComPtr<ID3D11Device>            pDevice;            // Handle to driver instance for GPU or emulator
	ComPtr<ID3D11DeviceContext>     pImmediateContext;
	ComPtr<IDXGISwapChain>          pSwapChain;         // Created by DXGI factor from 'pDevice'



	ComPtr<ID3D11RenderTargetView>  pRTV;         // View into the back buffer of 'pSwapchain'
	ComPtr<ID3D11DepthStencilView>  pDSV;


    Map                             greyMap;            // For surfaces without an albedo map
    Map                             blackMap;           // For surfaces without a specular map
    Map                             whiteMap;           // 'shiny' rendering option specular map


	ComPtr<ID3D11DepthStencilState> pDepthStencilStateDisable;
	ComPtr<ID3D11DepthStencilState> pDepthStencilStateWriteDisable;
	ComPtr<ID3D11BlendState>        pBlendStateColorWriteDisable;       
	ComPtr<ID3D11BlendState>        pBlendStateDefault;
 

    // Created in render:
    Map                             bgImg;
    // TODO: this needs to live with D3dMesh struct:
    bool                            flatShaded = false;     // Are the tri lists currently flat shaded ?




	//ComPtr<ID3D11Buffer>			  pConstantBufferFrame;
	ComPtr<ID3D11UnorderedAccessView> pUAVTextureHeadOIT;
	ComPtr<ID3D11UnorderedAccessView> pUAVBufferLinkedListOIT;

	std::shared_ptr<GraphicsPSO>    pOpaquePassPSO;
	std::shared_ptr<GraphicsPSO>    pTransparentFirstPassPSO;
	std::shared_ptr<GraphicsPSO>    pTransparentSecondPassPSO;


    D3d(HWND hwnd) {

	

		{
			//Get Window size
			RECT windowRect;
			GetWindowRect(hwnd, &windowRect);

			uint32_t width  = 8;
			uint32_t height = 8;

			//Create device and swapchain
			{
			
				DXGI_SWAP_CHAIN_DESC desc = {};
				desc.BufferCount = 1;
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


				D3D_FEATURE_LEVEL pFutureLever[] = { D3D_FEATURE_LEVEL_11_1 };
				uint32_t createFlag = 0;

#ifdef _DEBUG
				createFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

				DX::ThrowIfFailed(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createFlag, pFutureLever, _countof(pFutureLever),
					D3D11_SDK_VERSION,
					&desc,
					pSwapChain.GetAddressOf(),
					pDevice.GetAddressOf(), nullptr,
					nullptr));

			}

			//Create RTV from BackBuffer
			{
				ComPtr<ID3D11Texture2D> pBackBuffer;
				DX::ThrowIfFailed(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(pBackBuffer.GetAddressOf())));
				DX::ThrowIfFailed(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, pRTV.GetAddressOf()));
			}

			//Create DSV from Depth Buffer
			{
				ComPtr<ID3D11Texture2D> pDepthBuffer;
				D3D11_TEXTURE2D_DESC desc = {};
				desc.ArraySize = 1;
				desc.SampleDesc.Count = 1;
				desc.SampleDesc.Quality = 0;
				desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				desc.Width  = width;
				desc.Height = height;
				desc.MipLevels = 1;
				desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
				DX::ThrowIfFailed(pDevice->CreateTexture2D(&desc, nullptr, pDepthBuffer.GetAddressOf()));
				DX::ThrowIfFailed(pDevice->CreateDepthStencilView(pDepthBuffer.Get(), nullptr, pDSV.GetAddressOf()));
			}
			
			//Create Head texture for OIT
			{
				ComPtr<ID3D11Texture2D> pTextureOIT;
				D3D11_TEXTURE2D_DESC desc = {};
				desc.ArraySize = 1;
				desc.Width = width;
				desc.Height = height;
				desc.MipLevels = 1;
				desc.SampleDesc.Count = 1;
				desc.SampleDesc.Quality = 0;
				desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
				desc.Format = DXGI_FORMAT_R32_UINT;


				DX::ThrowIfFailed(pDevice->CreateTexture2D(&desc, nullptr, pTextureOIT.GetAddressOf()));
				DX::ThrowIfFailed(pDevice->CreateUnorderedAccessView(pTextureOIT.Get(), nullptr, pUAVTextureHeadOIT.GetAddressOf()));
			}

			//Create LinkedList with atomic counter for OIT 
			{
				

				struct ListNode {
					uint32_t  Next;
					uint32_t  Color;
					float     Depth;

				};

				auto pBufferOIT = DX::CreateStructuredBuffer<ListNode>(pDevice, width * height * layerCount, false, true);

				{
					D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
					desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
					desc.Buffer.FirstElement = 0;
					desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
					desc.Buffer.NumElements = width * height * layerCount;
					DX::ThrowIfFailed(pDevice->CreateUnorderedAccessView(pBufferOIT.Get(), &desc, pUAVBufferLinkedListOIT.GetAddressOf()));
				}

			}



			pDevice->GetImmediateContext(pImmediateContext.GetAddressOf());
		}

	

		pTransparentFirstPassPSO  = std::make_shared<GraphicsPSO>(pDevice);
		pTransparentSecondPassPSO = std::make_shared<GraphicsPSO>(pDevice);
		pOpaquePassPSO            = std::make_shared<GraphicsPSO>(pDevice);


		pOpaquePassPSO->attachShader(L"base/shaders/directx11_main.hlsl", "VS", GraphicsPSO::ShaderType::Vertex);
		pOpaquePassPSO->attachShader(L"base/shaders/directx11_main.hlsl", "PS", GraphicsPSO::ShaderType::Pixel);

		pTransparentFirstPassPSO->attachShader(L"base/shaders/Transparent.hlsl", "VertexShaderTransparentFirstPass", GraphicsPSO::ShaderType::Vertex);
		pTransparentFirstPassPSO->attachShader(L"base/shaders/Transparent.hlsl", "PixelShaderTransparentFirstPass", GraphicsPSO::ShaderType::Pixel);
		
		pTransparentSecondPassPSO->attachShader(L"base/shaders/Transparent.hlsl", "VertexShaderTransparentSecondPass", GraphicsPSO::ShaderType::Vertex);
		pTransparentSecondPassPSO->attachShader(L"base/shaders/Transparent.hlsl", "PixelShaderTransparentSecondPass", GraphicsPSO::ShaderType::Pixel);
	

		{
			auto desc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
			desc.RenderTarget[0].RenderTargetWriteMask = 0;
		    DX::ThrowIfFailed(pDevice->CreateBlendState(&desc, pBlendStateColorWriteDisable.GetAddressOf()));
		}

		{
			auto desc = CD3D11_BLEND_DESC(CD3D11_DEFAULT());	
			DX::ThrowIfFailed(pDevice->CreateBlendState(&desc, pBlendStateDefault.GetAddressOf()));
		}

		{
			auto desc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
			desc.DepthEnable = false;
			DX::ThrowIfFailed(pDevice->CreateDepthStencilState(&desc, pDepthStencilStateDisable.GetAddressOf()));
		}

		{
			auto desc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			DX::ThrowIfFailed(pDevice->CreateDepthStencilState(&desc, pDepthStencilStateWriteDisable.GetAddressOf()));
		}

		

//
//        HRESULT         hr = S_OK;
//        {   // Get device and context:
//            UINT            createDeviceFlags = 0;
//#ifdef _DEBUG
//            createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif
//            D3D_DRIVER_TYPE driverTypes[] = {
//                D3D_DRIVER_TYPE_HARDWARE,
//                D3D_DRIVER_TYPE_WARP,       // High performance software renderer
//                D3D_DRIVER_TYPE_REFERENCE,  // Reference software renderer is very slow.
//            };
//            ID3D11Device*   pDev;
//            ID3D11DeviceContext* pCtxt;
//            for(UINT ii=0; ii<3; ++ii) {
//                D3D_FEATURE_LEVEL       featureLevel = D3D_FEATURE_LEVEL_11_0;
//                hr = D3D11CreateDevice(
//                    NULL,                   // Use first video adapter (card/driver) if more than one of this type
//                    driverTypes[ii],
//                    nullptr,                // No software rasterizer DLL handle
//                    createDeviceFlags,
//                    &featureLevel,1,        // We only want 11.0 so that's all that's in the list
//                    D3D11_SDK_VERSION,      // Required
//                    &pDev,                  // Returned
//                    NULL,                   // Don't need to know which featureLevel was chosen (only 1)
//                    &pCtxt);                // Returned
//                if(SUCCEEDED(hr))
//                    break;
//            }
//            if(hr < 0)
//                throwWindows("Microsoft Direct3D 11.0 not supported",hr);
//            pDevice.reset(pDev);
//            pContext.reset(pCtxt);
//        }
//        {   // Get swapchain:
//            Unique<IDXGIFactory1>   dxgiFactory;
//            {        // Obtain DXGI 1.1 factory:
//                Unique<IDXGIDevice>     dxgiDevice;
//                {
//                    IDXGIDevice*            pDd;
//                    hr = pDevice->QueryInterface(__uuidof(IDXGIDevice),reinterpret_cast<void**>(&pDd));
//                    FGASSERTWINOK(hr);
//                    dxgiDevice.reset(pDd);
//                }
//                Unique<IDXGIAdapter>        adapter;
//                {
//                    IDXGIAdapter*               pA;
//                    hr = dxgiDevice->GetAdapter(&pA);
//                    FGASSERTWINOK(hr);
//                    adapter.reset(pA);
//                }
//                IDXGIFactory1*      pDf;
//                hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDf));
//                FGASSERTWINOK(hr);
//                dxgiFactory.reset(pDf);
//                // Don't handle full-screen swapchains so block the ALT+ENTER shortcut
//                dxgiFactory->MakeWindowAssociation(hwnd,DXGI_MWA_NO_ALT_ENTER);
//            }
//            DXGI_SWAP_CHAIN_DESC sd = {};
//            sd.BufferDesc.Width = 8;        // Start with an arbitrary size that doesn't cause debug logging
//            sd.BufferDesc.Height = 8;
//            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//            sd.BufferDesc.RefreshRate.Numerator = 60;
//            sd.BufferDesc.RefreshRate.Denominator = 1;
//            sd.SampleDesc.Count = 1;
//            sd.SampleDesc.Quality = 0;
//            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//            sd.BufferCount = 1;                 // Number of back buffers (so 2 including front buffer)
//            sd.OutputWindow = hwnd;
//            sd.Windowed = TRUE;
//            // This gives debug logs on Win10 that we should use DXGI_SWAP_EFFECT_FLIP_DISCARD instead,
//            // but that can't be used on Win7 so leave it for now:
//            sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//            IDXGISwapChain*         pSc;
//            hr = dxgiFactory->CreateSwapChain(pDevice.get(),&sd,&pSc);
//            FGASSERTWINOK(hr);
//            pSwapchain.reset(pSc);
//        }


//        // Compile shader dynamically per Carmack's recommendation:
//        DWORD               dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
//#ifdef _DEBUG
//        dwShaderFlags |= D3DCOMPILE_DEBUG;
//        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
//        dwShaderFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
//#endif
//        Ustring             shaderPath = dataDir()+"base/shaders/directx11_main.hlsl";
//        string              src =  fgSlurp(shaderPath);
//        {   // Vertex shader
//            Unique<ID3DBlob>        pVSBlob;
//            {
//                ID3DBlob*           pErrorBlob = nullptr;
//                ID3DBlob*               ptr;
//                hr = D3DCompile(src.data(),src.size(),
//                    nullptr,            // Don't send a source file path from which include directories would be relative
//                    nullptr,            // No macro definitions
//                    nullptr,            // No include info since no #includes in source
//                    "VS",               // Entry point function. IGNORED due to below being specified.
//                    "vs_4_0",
//                    dwShaderFlags,      // See above
//                    0,                  // Does nothing
//                    &ptr,               // Returned
//                    &pErrorBlob);       // Only set if there were errors
//                if (pErrorBlob) {       // Compilation error is not the same as function error
//                    const char *    errPtr = static_cast<const char *>(pErrorBlob->GetBufferPointer());
//                    size_t          errSz = pErrorBlob->GetBufferSize();
//                    string          errStr(errPtr,errSz);
//                    pErrorBlob->Release();
//                    cerr << errStr;
//                    fgThrow("VS compilation failed",errStr);
//                }
//                pVSBlob.reset(ptr);
//            }
//            {
//                ID3D11VertexShader*     ptr;
//	            hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),nullptr,&ptr);
//                FGASSERTWINOK(hr);
//                pVertShader.reset(ptr);
//            }
//            {   // Define the input layout.
//                // * The driver can auto convert vect3 to vect4 if corresponding semantic label in the shader is vect4.
//                // * For non-interleaved indexed vertex buffers give them different input slots (must be same for non-indexed).
//                D3D11_INPUT_ELEMENT_DESC layout[] =
//                {
//                    {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0 ,D3D11_INPUT_PER_VERTEX_DATA,0},
//                    {"NORMAL",  0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
//                    {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,   0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
//	            };
//                ID3D11InputLayout*      ptr;
//	            hr = pDevice->CreateInputLayout(
//                    layout,ARRAYSIZE(layout),
//                    pVSBlob->GetBufferPointer(),        // Compiled vertex shader is validated against layout
//                    pVSBlob->GetBufferSize(),
//                    &ptr);                              // Returned
//                FGASSERTWINOK(hr);
//                pVertexLayout.reset(ptr);
//            }
//        }
//        pContext->IASetInputLayout(pVertexLayout.get());
//        {   // Pixel shader
//	        Unique<ID3DBlob>        pPSBlob;
//            {
//                ID3DBlob*           pErrorBlob = nullptr;
//                ID3DBlob*           ptr;
//                hr = D3DCompile(&src[0],src.size(),nullptr,nullptr,nullptr,"PS","ps_4_0",dwShaderFlags,0,&ptr,&pErrorBlob);
//                if (pErrorBlob) {       // Compilation error is not the same as function error
//                    const char *    errPtr = static_cast<const char *>(pErrorBlob->GetBufferPointer());
//                    size_t          errSz = pErrorBlob->GetBufferSize();
//                    string          errStr(errPtr,errSz);
//                    pErrorBlob->Release();
//                    cerr << errStr;
//                    fgThrow("PS compilation failed",errStr);
//                }
//                pPSBlob.reset(ptr);
//            }
//            {
//                ID3D11PixelShader*      ptr;
//	            hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),pPSBlob->GetBufferSize(),nullptr,&ptr);
//                FGASSERTWINOK(hr);
//                pPixelShader.reset(ptr);
//            }
//        }
//        {
//            D3D11_RENDER_TARGET_BLEND_DESC  rt = {};
//            rt.BlendEnable = true;
//            rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;        // Multiply RGB_shader by A_shader
//            rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;   // Multiply RGB_target by (1-A_shader)
//            rt.BlendOp = D3D11_BLEND_OP_ADD;            // Add the two
//            rt.SrcBlendAlpha = D3D11_BLEND_ONE;
//            rt.DestBlendAlpha = D3D11_BLEND_ONE;
//            rt.BlendOpAlpha = D3D11_BLEND_OP_MAX;
//            rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;    // Do not leave as zero !
//            D3D11_BLEND_DESC            bd = {};
//            bd.AlphaToCoverageEnable = false;
//            bd.IndependentBlendEnable = false;
//            bd.RenderTarget[0] = rt;                    // Only define blending for one render target
//            ID3D11BlendState*           ptr;
//            pDevice->CreateBlendState(&bd,&ptr);
//            pBlendState.reset(ptr);
//        }
        // Just in case 1x1 image has memory alignment and two-sided interpolation edge-case issues:
        greyMap = makeMap(ImgC4UC(Vec2UI(2,2),RgbaUC(200,200,200,255)));
        blackMap = makeMap(ImgC4UC(Vec2UI(2,2),RgbaUC(0,0,0,255)));
        whiteMap = makeMap(ImgC4UC(Vec2UI(2,2),RgbaUC(255,255,255,255)));
    }

    void resize(Vec2UI windowSize) {


		if (pImmediateContext == nullptr)
			return;

		pImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);

		pRTV.Reset();
		DX::ThrowIfFailed(pSwapChain->ResizeBuffers(1, windowSize[0], windowSize[1], DXGI_FORMAT_UNKNOWN, 0));

		{
			ComPtr<ID3D11Texture2D> pBackBuffer;
			DX::ThrowIfFailed(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(pBackBuffer.GetAddressOf())));
			DX::ThrowIfFailed(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, pRTV.ReleaseAndGetAddressOf()));
		}

		{
			ComPtr<ID3D11Texture2D> pDepthBuffer;
			D3D11_TEXTURE2D_DESC desc = {};
			desc.ArraySize = 1;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			desc.Width  = windowSize[0];
			desc.Height = windowSize[1];
			desc.MipLevels = 1;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			DX::ThrowIfFailed(pDevice->CreateTexture2D(&desc, nullptr, pDepthBuffer.GetAddressOf()));
			DX::ThrowIfFailed(pDevice->CreateDepthStencilView(pDepthBuffer.Get(), nullptr, pDSV.ReleaseAndGetAddressOf()));
		}
		{
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


			DX::ThrowIfFailed(pDevice->CreateTexture2D(&desc, nullptr, pTextureOIT.GetAddressOf()));
			DX::ThrowIfFailed(pDevice->CreateUnorderedAccessView(pTextureOIT.Get(), nullptr, pUAVTextureHeadOIT.ReleaseAndGetAddressOf()));
		}

	
		{
			

			struct ListNode {
				uint32_t  Next;
				uint32_t  Color;
				float     Depth;
				
			};

			auto pBufferOIT = DX::CreateStructuredBuffer<ListNode>(pDevice, windowSize[0] * windowSize[1] * layerCount, false, true);

			{
				D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
				desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				desc.Buffer.FirstElement = 0;
				desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
				desc.Buffer.NumElements = windowSize[0] * windowSize[1] * layerCount;
				DX::ThrowIfFailed(pDevice->CreateUnorderedAccessView(pBufferOIT.Get(), &desc, pUAVBufferLinkedListOIT.ReleaseAndGetAddressOf()));
			}

		}
	

		auto viwports = { CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(windowSize[0]),  static_cast<float>(windowSize[1])) };
		pImmediateContext->RSSetViewports(static_cast<uint32_t>(std::size(viwports)), std::data(viwports) );
	



        //if (!pContext)                          // Don't execute before D3D is set up
        //    return;
        //// All buffer references must be released before 'ResizeBuffers'. It only appears to care about
        //// pRenderView though, and unless called within WM_SIZE seems to complain regardless:
        //pContext->OMSetRenderTargets(0,nullptr,nullptr);    // Unbind RenderView & DepthStencilView
        //pDepthStencilView.reset();
        //pDepthStencilState.reset();
        //pDepthStencil.reset();
        //pRenderView.reset();
        //pBackBuffer.reset();
        //pContext->Flush();                                  // Wait for complete
        //HRESULT         hr;
        //hr = pSwapchain->ResizeBuffers(
        //    1,                                      // We only have 1 buffer
        //    sz[0],sz[1],
        //    DXGI_FORMAT_UNKNOWN,                    // Don't change the format
        //    0);                                     // No special flags
        //FGASSERTWINOK(hr);
        //{   // Create new viewport:
        //    {
        //        ID3D11Texture2D*                ptr;
        //        hr = pSwapchain->GetBuffer(0,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(&ptr));
        //        FGASSERTWINOK(hr);
        //        pBackBuffer.reset(ptr);
        //    }
        //    {
        //        ID3D11RenderTargetView*         ptr;
        //        hr = pDevice->CreateRenderTargetView(pBackBuffer.get(),nullptr,&ptr);
        //        FGASSERTWINOK(hr);
        //        pRenderView.reset(ptr);
        //    }
        //}
        //// D3D version of Z buffer
        //pDepthStencilView.reset();
        //pDepthStencil.reset();
        //{
        //    D3D11_TEXTURE2D_DESC    dd = {};
        //    dd.Width = sz[0];
        //    dd.Height = sz[1];
        //    dd.MipLevels = 1;
        //    dd.ArraySize = 1;
        //    dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        //    dd.SampleDesc.Count = 1;
        //    dd.SampleDesc.Quality = 0;
        //    dd.Usage = D3D11_USAGE_DEFAULT;
        //    dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        //    dd.CPUAccessFlags = 0;
        //    dd.MiscFlags = 0;
        //    {
        //        ID3D11Texture2D*            ptr;
        //        hr = pDevice->CreateTexture2D(&dd,nullptr,&ptr);
        //        FGASSERTWINOK(hr);
        //        pDepthStencil.reset(ptr);
        //    }
        //}
        //{
        //    // Copied from MSDN - not actually using stencils, only depth test:
        //    D3D11_DEPTH_STENCILOP_DESC      sod;
        //    sod.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        //    sod.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        //    sod.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        //    sod.StencilFunc = D3D11_COMPARISON_ALWAYS;
        //    D3D11_DEPTH_STENCIL_DESC        dsd;
        //    dsd.DepthEnable = true;
        //    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;    // Enable writing to depth buffer
        //    dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;        // Override default 'LESS'
        //    dsd.StencilEnable = true;
        //    dsd.StencilReadMask = 0xFF;
        //    dsd.StencilWriteMask = 0xFF;
        //    dsd.FrontFace = sod;
        //    dsd.BackFace = sod;
        //    dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        //    ID3D11DepthStencilState*        dss;
        //    pDevice->CreateDepthStencilState(&dsd,&dss);
        //    pDepthStencilState.reset(dss);
        //    pContext->OMSetDepthStencilState(pDepthStencilState.get(),1);
        //}
        //{
        //    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
        //    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        //    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        //    descDSV.Texture2D.MipSlice = 0;
        //    {
        //        ID3D11DepthStencilView*         ptr;
        //        hr = pDevice->CreateDepthStencilView(pDepthStencil.get(),&descDSV,&ptr);
        //        FGASSERTWINOK(hr);
        //        pDepthStencilView.reset(ptr);
        //    }
        //}
        //{
        //    ID3D11RenderTargetView*         pRvs[] = { pRenderView.get() };
        //    pContext->OMSetRenderTargets(1,pRvs,pDepthStencilView.get());
        //}
        //// D3D 11 supports fractional viewport coordinates in the range [-32768,32767]:
        //D3D11_VIEWPORT          vp;
        //vp.TopLeftX = 0;
        //vp.TopLeftY = 0;
        //vp.Width = FLOAT(sz[0]);
        //vp.Height = FLOAT(sz[1]);
        //vp.MinDepth = 0.0f;
        //vp.MaxDepth = 1.0f;
        //pContext->RSSetViewports(1,&vp);
    }

    // All member sizes must be multiples of 8 bytes (presumably for alignment).
    // HLSL uses column-major matrices so we need to transpose:
    struct Scene
    {
	    Mat44F    mvm;
	    Mat44F    projection;
        Vec4F    ambient;
        Vec4F    lightDir[2];        // Normalized direction TO light in CCS
        Vec4F    lightColor[2];
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

    Unique<ID3D11Buffer> makeConstBuff(const Scene & scene)
    {
        D3D11_BUFFER_DESC       bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;             // read/write by GPU only
        bd.ByteWidth = sizeof(Scene);
	    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	    bd.CPUAccessFlags = 0;
        ID3D11Buffer*           ptr;
        HRESULT                 hr = pDevice->CreateBuffer(&bd,nullptr,&ptr);
        FGASSERTWINOK(hr);
	    pImmediateContext->UpdateSubresource(ptr,0,nullptr,&scene,0,0);
        return Unique<ID3D11Buffer>(ptr);
    }

    Unique<ID3D11Buffer> makeVertBuff(const Verts & verts)
    {
        if (verts.empty())
            return Unique<ID3D11Buffer>();  // D3D gives error for zero size buffer creation
        ID3D11Buffer*           ptr;
        D3D11_BUFFER_DESC       bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;             // read/write by GPU only
        bd.ByteWidth = sizeof(Vert) * uint(verts.size());
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	    bd.CPUAccessFlags = 0;
        D3D11_SUBRESOURCE_DATA  initData = {};
        initData.pSysMem = verts.data();
        HRESULT                 hr = pDevice->CreateBuffer(&bd,&initData,&ptr);
        FGASSERTWINOK(hr);
        return Unique<ID3D11Buffer>(ptr);
    }

    // Returns null pointer if no surf points:
    Unique<ID3D11Buffer> makeSurfPoints(RendMesh const & rendMesh,Mesh const & origMesh)
    {
        Verts                   verts;
        Vec3Fs const &       rendVerts = rendMesh.posedVertsN.cref();
        for (Surf const & origSurf : origMesh.surfaces) {
            for (SurfPoint const & sp : origSurf.surfPoints) {
                Vert                v;
                v.pos = fgSurfPointPos(sp,origSurf.tris,origSurf.quads,rendVerts);
                v.norm = Vec3F(0,0,1);   // Random non-zero value can be normalized by shader
                v.uv = Vec2F(0);
                verts.push_back(v);
            }
        }
        return makeVertBuff(verts);
    }

    // Returns null pointer if no marked verts:
    Unique<ID3D11Buffer> makeMarkedVerts(RendMesh const & rendMesh,Mesh const & origMesh)
    {
        Verts                   verts;
        Vec3Fs const &       rendVerts = rendMesh.posedVertsN.cref();
        for (MarkedVert const & mv : origMesh.markedVerts) {
            Vert                v;
            v.pos = rendVerts[mv.idx];
            v.norm = Vec3F(0,0,1);   // Random non-zero value can be normalized by shader
            v.uv = Vec2F(0);
            verts.push_back(v);
        }
        return makeVertBuff(verts);
    }

    Unique<ID3D11Buffer> makeAllVerts(Vec3Fs const & verts)
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

    Verts makeVertList(
        RendMesh const &        rendMesh,
        Mesh const &            origMesh,
        size_t                  surfNum,
        bool                    shadeFlat)
    {
        Verts                   vertList;
        Surf const &            origSurf = origMesh.surfaces[surfNum];
        Normals const &         normals = rendMesh.normalsN.cref();
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

    Verts makeLineVerts(RendMesh const & rendMesh,Mesh const & origMesh,size_t surfNum) {
        Verts                       ret;
        Vec3Fs const &           verts = rendMesh.posedVertsN.cref();
        Surf const &         origSurf = origMesh.surfaces[surfNum];
        Vec3UIs const &          tris = origSurf.tris.vertInds;
        Vec4UIs const &          quads = origSurf.quads.vertInds;
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

    Unique<ID3D11Texture2D> loadMap(ImgC4UC const & map) {
        ImgC4UCs                  mip = fgMipMap(map);
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
        FGASSERTWINOK(hr);
        return Unique<ID3D11Texture2D>(ptr);
    }

    Unique<ID3D11ShaderResourceView>
    makeMapView(ID3D11Texture2D* mapPtr)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC     SRVDesc = {};
        SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        SRVDesc.Texture2D.MipLevels = uint(-1);   // Use all available mipmap levels
        ID3D11ShaderResourceView*           ptr;
        HRESULT         hr = pDevice->CreateShaderResourceView(mapPtr,&SRVDesc,&ptr);
        FGASSERTWINOK(hr);
        // Couldn't get this to work. mipmaps currently generated in CPU:
        //pContext->GenerateMips(ptr);
        return Unique<ID3D11ShaderResourceView>(ptr);
    }

    Map
    makeMap(ImgC4UC const & map)
    {
        Map             ret;
        ret.map = loadMap(map);
        ret.view = makeMapView(ret.map.get());
        return ret;
    }

    Unique<ID3D11SamplerState>
    makeSamplerState()
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
        FGASSERTWINOK(hr);
        return Unique<ID3D11SamplerState>(ptr);
    }

    Unique<ID3D11Buffer>
    setScene(Scene const & scene)
    {
        Unique<ID3D11Buffer>        sceneBuff = makeConstBuff(scene);
        {
            ID3D11Buffer*           sbs[] = { sceneBuff.get() };
	        pImmediateContext->VSSetConstantBuffers(0,1,sbs);
            pImmediateContext->PSSetConstantBuffers(0,1,sbs);
        }
        return sceneBuff;
    }

    Unique<ID3D11Buffer>
    makeScene(FgLighting lighting,Mat44F worldToD3vs,Mat44F d3vsToD3ps)
    {
        lighting.lights.resize(2);                          // Ensure we have 2
        Scene                   scene;
	    scene.mvm = worldToD3vs.transpose();                // D3D is column-major (of course)
	    scene.projection = d3vsToD3ps.transpose();          // "
        scene.ambient = fgAsHomogVec(lighting.ambient);
        for (uint ii=0; ii<2; ++ii) {
            scene.lightDir[ii] = fgAsHomogVec(-lighting.lights[ii].direction);
            scene.lightColor[ii] = fgAsHomogVec(lighting.lights[ii].colour);
        }
        return setScene(scene);
    }

    // Ambient-only version:
    Unique<ID3D11Buffer>
    makeScene(Vec3F ambient,Mat44F worldToD3vs,Mat44F d3vsToD3ps)
    {
        FgLight             diffuseBlack(Vec3F(0),Vec3F(0,0,1));
        FgLighting          lighting(ambient,fgSvec(diffuseBlack,diffuseBlack));
        return makeScene(lighting,worldToD3vs,d3vsToD3ps);
    }

    void
    setBgImage(BackgroundImage const & bgi)
    {
        bgImg.reset();
        ImgC4UC const &     img = bgi.imgN.cref();
        if (img.empty())
            return;
        Vec2UI              p2dims = clampHi(pow2Ceil(img.dims()),maxMapSize);
        ImgC4UC             map(p2dims);
        fgImgResize(img,map);
        bgImg = makeMap(map);
    }

    void renderBgImg(BackgroundImage const & bgi, Vec2UI viewportSize, bool  transparentPass) {
        Unique<ID3D11RasterizerState> rasterizer;
        {
            D3D11_RASTERIZER_DESC       rd = {};
            rd.FillMode = D3D11_FILL_SOLID;
            rd.CullMode = D3D11_CULL_NONE; // : D3D11_CULL_BACK;
            ID3D11RasterizerState*      ptr;
            pDevice->CreateRasterizerState(&rd,&ptr);
            rasterizer.reset(ptr);
        }
        pImmediateContext->RSSetState(rasterizer.get());
        Unique<ID3D11Buffer>            bgImageVerts;
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
        ID3D11ShaderResourceView*       mapViews[2];    // Albedo, specular resp.
        mapViews[0] = bgImg.view.get();
        mapViews[1] = blackMap.view.get();
        pImmediateContext->PSSetShaderResources(0,2,mapViews);
        Scene                       scene;
        scene.mvm = Mat44F::identity();
        scene.projection = Mat44F::identity();
        scene.ambient = Vec4F(1);
        if (transparentPass) {
            double                      ft = clampBounds(bgi.foregroundTransparency.val(),0.0,1.0);
            scene.ambient[3] = static_cast<float>(ft);
        }
        Unique<ID3D11Buffer>        sceneBuff = setScene(scene);
        pImmediateContext->Draw(6,0);
    }

    D3dMesh &
    getD3dMesh(RendMesh const & rm) const
    {
        Any &  gpuMesh = *rm.gpuData;
        if (gpuMesh.empty())
            gpuMesh.reset(D3dMesh());
        return gpuMesh.ref<D3dMesh>();
    }

    D3dSurf & getD3dSurf(RendSurf const & rs) const
    {
        Any &  gpuSurf = *rs.gpuData;
        if (gpuSurf.empty())
            gpuSurf.reset(D3dSurf());
        return gpuSurf.ref<D3dSurf>();
    }

    void renderBackBuffer(
        BackgroundImage const &     bgi,
        RendMeshes const &          rendMeshes,
        FgLighting                  lighting,
        Mat44F                      worldToD3vs,    // modelview
        Mat44F                      d3vsToD3ps,     // projection
        Vec2UI                      viewportSize,
        const RendOptions &         rendOpts)
    {
        // No render view during create - created by first resize:
        if (pRTV == nullptr)
            return;
        // Update 'd3dMeshes' (mesh and map data) if required:
        for (RendMesh const & rendMesh : rendMeshes) {
            Mesh const &            origMesh = rendMesh.origMeshN.cref();
            D3dMesh &               d3dMesh = getD3dMesh(rendMesh);
            bool                    vertsChanged = rendMesh.surfVertsFlag->checkUpdate(),
                                    trisChanged = vertsChanged || (flatShaded != rendOpts.flatShaded),
                                    // Is this mesh valid (ie. currently selected and has displayable content):
                                    valid = !origMesh.verts.empty();
            if (rendOpts.allVerts && rendMesh.allVertsFlag->checkUpdate()) {
                d3dMesh.allVerts.reset();
                if (valid)
                    d3dMesh.allVerts = makeAllVerts(rendMesh.posedVertsN.cref());
            }
            if (vertsChanged) {
                d3dMesh.surfPoints.reset();
                d3dMesh.markedPoints.reset();
                if (valid) {
                    d3dMesh.surfPoints = makeSurfPoints(rendMesh,origMesh);
                    d3dMesh.markedPoints = makeMarkedVerts(rendMesh,origMesh);
                }
            }
            if (valid)
                FGASSERT(rendMesh.rendSurfs.size() == origMesh.surfaces.size());
            for (size_t ss=0; ss<rendMesh.rendSurfs.size(); ++ss) {
                RendSurf const &        rs = rendMesh.rendSurfs[ss];
                D3dSurf &               d3dSurf = getD3dSurf(rs);
                if (rs.albedoMapFlag->checkUpdate()) {
                    d3dSurf.albedoMap.reset();
                    ImgC4UC const &     img = rs.albedoMap.cref();
                    if (!img.empty())
                        d3dSurf.albedoMap = makeMap(img);
                }
                if (rs.specularMapFlag->checkUpdate()) {
                    d3dSurf.specularMap.reset();
                    ImgC4UC const &     img = rs.specularMap.cref();
                    if (!img.empty())
                        d3dSurf.specularMap = makeMap(img);
                }
                if (trisChanged) {
                    d3dSurf.lineVerts.reset();  // Relase but delay computation in case not needed
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

   
        pImmediateContext->ClearRenderTargetView(pRTV.Get(), std::data(fgAsHomogVec(rendOpts.backgroundColor)));
        pImmediateContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		pImmediateContext->ClearUnorderedAccessViewUint(pUAVTextureHeadOIT.Get(), std::data({ 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }));
		pImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(1, pRTV.GetAddressOf(), pDSV.Get(), 1, 2, std::data({ pUAVTextureHeadOIT.Get(), pUAVBufferLinkedListOIT.Get() }), std::data({ 0x0u, 0x0u }));


        Unique<ID3D11Buffer>        sceneBuff;
        Unique<ID3D11SamplerState>  samplerState = makeSamplerState();
        {
            ID3D11SamplerState*         sss[] = { samplerState.get() };
            pImmediateContext->PSSetSamplers(0,1,sss);
        }
        // Currently use same shaders for all render options:



		//FIRST PASS

		pImmediateContext->OMSetBlendState(pBlendStateColorWriteDisable.Get(), nullptr, 0xFFFFFFFF);
		pImmediateContext->OMSetDepthStencilState(pDepthStencilStateWriteDisable.Get(), 0);
		pTransparentFirstPassPSO->applay(pImmediateContext);



        if (bgImg.valid()) {
            pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            renderBgImg(bgi,viewportSize,false);
        }
        // Triangle rendering (faces and wireframe):
        if (rendOpts.facets) {
            pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            sceneBuff = makeScene(lighting,worldToD3vs,d3vsToD3ps);
            Unique<ID3D11RasterizerState>   rasterizer;
            {
                D3D11_RASTERIZER_DESC       rd = {};
                rd.FillMode = D3D11_FILL_SOLID;
                rd.CullMode = rendOpts.twoSided ? D3D11_CULL_NONE : D3D11_CULL_BACK;
                if (rendOpts.wireframe || rendOpts.allVerts || rendOpts.surfPoints || rendOpts.markedVerts) {
                    // Increase depth values small amount to wireframe & allverts get shown.
                    // Only do this when necessary just in case some small confusting artifacts result.
                    rd.DepthBias = 1000;
                    rd.SlopeScaledDepthBias = 0.5f;
                    rd.DepthBiasClamp = 0.001f;
                }
                ID3D11RasterizerState*      ptr;
                pDevice->CreateRasterizerState(&rd,&ptr);
                rasterizer.reset(ptr);
            }
            pImmediateContext->RSSetState(rasterizer.get());
            renderTris(rendMeshes,rendOpts,false);
            renderTris(rendMeshes,rendOpts,true);
        }
        if (rendOpts.wireframe) {
            pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
            if (rendOpts.facets)        // Render wireframe blue over facets, white otherwise:
                sceneBuff = makeScene(Vec3F(0,0,1),worldToD3vs,d3vsToD3ps);
            else
                sceneBuff = makeScene(Vec3F(1,1,1),worldToD3vs,d3vsToD3ps);
            Unique<ID3D11RasterizerState>   rasterizer;
            {
                D3D11_RASTERIZER_DESC       rd = {};
                rd.FillMode = D3D11_FILL_WIREFRAME;
                rd.CullMode = rendOpts.twoSided ? D3D11_CULL_NONE : D3D11_CULL_BACK;
                ID3D11RasterizerState*      ptr;
                pDevice->CreateRasterizerState(&rd,&ptr);
                rasterizer.reset(ptr);
            }
            pImmediateContext->RSSetState(rasterizer.get());
            for (RendMesh const & rendMesh : rendMeshes) {
                Mesh const &            origMesh = rendMesh.origMeshN.cref();
                // Must loop through origMesh surfaces in case the mesh has been emptied (and the redsurfs remain):
                for (size_t ss=0; ss<origMesh.surfaces.size(); ++ss) {
                    D3dSurf &               d3dSurf = getD3dSurf(rendMesh.rendSurfs[ss]);
                    Surf const &            origSurf = origMesh.surfaces[ss];
                    if (origSurf.empty())
                        continue;
                    if (!d3dSurf.lineVerts)     // GPU needs updating
                        d3dSurf.lineVerts = makeVertBuff(makeLineVerts(rendMesh,origMesh,ss));
                    setVertexBuffer(d3dSurf.lineVerts.get());
                    ID3D11ShaderResourceView* mapViews[2];    // Albedo, specular resp.
                    mapViews[0] = greyMap.view.get();
                    if (rendOpts.shiny)
                        mapViews[1] = whiteMap.view.get();
                    else if (d3dSurf.specularMap.valid())
                        mapViews[1] = d3dSurf.specularMap.view.get();
                    else
                        mapViews[1] = blackMap.view.get();
                    pImmediateContext->PSSetShaderResources(0,2,mapViews);
                    pImmediateContext->Draw(uint(origSurf.numTris()*6+origSurf.numQuads()*8),0);
                }
            }
        }
        // Point rendering (doesn't use rasterizer):
        pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        {
            ID3D11ShaderResourceView*       mapViews[2];    // Albedo, specular resp.
            mapViews[0] = whiteMap.view.get();
            mapViews[1] = blackMap.view.get();
            pImmediateContext->PSSetShaderResources(0,2,mapViews);
        }
        if (rendOpts.allVerts) {
            sceneBuff = makeScene(Vec3F(0,1,0),worldToD3vs,d3vsToD3ps);
            for (RendMesh const & rendMesh : rendMeshes) {
                D3dMesh const &         d3dMesh = getD3dMesh(rendMesh);
                if (d3dMesh.allVerts) {
                    Mesh const &        origMesh = rendMesh.origMeshN.cref();
                    setVertexBuffer(d3dMesh.allVerts.get());
                    pImmediateContext->Draw(uint(origMesh.verts.size()),0);
                }
            }
        }
        if (rendOpts.surfPoints) {
            sceneBuff = makeScene(Vec3F(1,0,0),worldToD3vs,d3vsToD3ps);
            for (auto const & rendMesh : rendMeshes) {
                auto const & origMesh = rendMesh.origMeshN.cref();
                if (origMesh.surfPointNum()>0) {
                    auto&  d3dMesh = getD3dMesh(rendMesh);
                    if (d3dMesh.surfPoints) {   // Can be null if no surf points
                        setVertexBuffer(d3dMesh.surfPoints.get());
                        pImmediateContext->Draw(uint(origMesh.surfPointNum()),0);
                    }
                }
            }
        }
        if (rendOpts.markedVerts) {
            sceneBuff = makeScene(Vec3F(1,0,0),worldToD3vs,d3vsToD3ps);
            for (RendMesh const & rendMesh : rendMeshes) {
                Mesh const &            origMesh = rendMesh.origMeshN.cref();
                if (!origMesh.markedVerts.empty()) {
                    D3dMesh &                      d3dMesh = getD3dMesh(rendMesh);
                    if (d3dMesh.markedPoints) {   // Can be null if no marked verts
                        setVertexBuffer(d3dMesh.markedPoints.get());
                        pImmediateContext->Draw(uint(origMesh.surfPointNum()),0);
                    }
                }
            }
        }

		//Second PASS

		pImmediateContext->OMSetBlendState(pBlendStateDefault.Get(), nullptr, 0xFFFFFFFF);
		pImmediateContext->OMSetDepthStencilState(pDepthStencilStateDisable.Get(), 0);
		pTransparentSecondPassPSO->applay(pImmediateContext);
		pImmediateContext->Draw(3, 0);

		//TODO
        //if (bgImg.valid()) {
        //    pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        //    pImmediateContext->ClearDepthStencilView(pDSV.Get(),D3D11_CLEAR_DEPTH,1.0f,0);
        //    renderBgImg(bgi,viewportSize,true);
        //}
    }

    void showBackBuffer() {
        // No render view during create - created by first resize:
        if (pRTV == nullptr)
            return;
        HRESULT  hr = pSwapChain->Present(0,0);      // Swap back buffer to display
        FGASSERTWINOK(hr);
    }

    ImgC4UC capture(Vec2UI viewportSize) {
        HRESULT                     hr;
        Unique<ID3D11Texture2D>     pBuffer;
        {
            ID3D11Texture2D*        ptr;
            hr = pSwapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),(LPVOID*)&ptr);
            FGASSERTWINOK(hr);
            pBuffer.reset(ptr);
        }
        D3D11_TEXTURE2D_DESC    desc;
        pBuffer->GetDesc(&desc);
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        desc.Usage = D3D11_USAGE_STAGING;
        Unique<ID3D11Texture2D>     pTexture;
        {
            ID3D11Texture2D*        ptr;
            hr = pDevice->CreateTexture2D(&desc,NULL,&ptr);
            FGASSERTWINOK(hr);
            pTexture.reset(ptr);
        }
        pImmediateContext->CopyResource(pTexture.get(),pBuffer.get());
        D3D11_MAPPED_SUBRESOURCE    resource;
        unsigned int                subresource = D3D11CalcSubresource(0,0,0);
        hr = pImmediateContext->Map(pTexture.get(),subresource,D3D11_MAP_READ_WRITE,0,&resource);
        FGASSERTWINOK(hr);
        ImgC4UC                 ret(viewportSize);
        uchar*                      dst = (uchar*)resource.pData;
        for (size_t rr=0; rr<viewportSize[1]; ++rr) {
            memcpy(&ret.xy(0,rr),dst,4ULL*viewportSize[0]);
            dst += size_t(resource.RowPitch);
        }
        return ret;
    }

    void setVertexBuffer(ID3D11Buffer * vertBuff) {
        UINT                stride[] = { sizeof(Vert) };
        UINT                offset[] = { 0 };
        ID3D11Buffer*       buffers[] = { vertBuff };
        // The way these buffers are interpreted by the HLSL is determined by 'IASetInputLayout' above:
        pImmediateContext->IASetVertexBuffers(0,1,buffers,stride,offset);
    }

    void renderTris(RendMeshes const & rendMeshes, RendOptions const & rendOpts, bool transparentPass) {
        for (RendMesh const & rendMesh : rendMeshes) {
            if (rendMesh.posedVertsN.cref().empty())    // Not selected
                continue;
            Mesh const &                origMesh = rendMesh.origMeshN.cref();
            for (size_t ss=0; ss<origMesh.surfaces.size(); ++ss) {
                const Surf &         origSurf = origMesh.surfaces[ss];
                if (origSurf.empty())
                    continue;
                RendSurf const &            rendSurf = rendMesh.rendSurfs[ss];
                if (transparentPass == rendSurf.albedoHasTransparencyN.val()) {
                    D3dSurf &                      d3dSurf = getD3dSurf(rendSurf);
                    FGASSERT(d3dSurf.triVerts);
                    setVertexBuffer(d3dSurf.triVerts.get());
                    ID3D11ShaderResourceView*       mapViews[2];    // Albedo, specular resp.
                    if (rendOpts.useTexture && d3dSurf.albedoMap.valid())
                        mapViews[0] = d3dSurf.albedoMap.view.get();
                    else
                        mapViews[0] = greyMap.view.get();
                    if (rendOpts.shiny)
                        mapViews[1] = whiteMap.view.get();
                    else if (d3dSurf.specularMap.valid())
                        mapViews[1] = d3dSurf.specularMap.view.get();
                    else
                        mapViews[1] = blackMap.view.get();
                    pImmediateContext->PSSetShaderResources(0,2,mapViews);
                    pImmediateContext->Draw(uint(origSurf.numTriEquivs())*3,0);
                }
            }
        }
    }

    void reset()
    {
        if (pImmediateContext != nullptr)
            pImmediateContext->ClearState();
    }
};

// D3D projection from frustum:
Mat44F
calcProjection(const Frustum & f)
{
    // Projects from D3VS to D3PS. ie. maps:
    // * Near plane to Z=0, far plane to Z=1
    // * Left/right clip planes to X=-1/1
    // * Bottom/top clip planes to Y=-1/1
    double              a = f.nearDist / f.nearHalfWidth,
                        b = f.nearDist / f.nearHalfHeight,
                        del = f.farDist - f.nearDist,
                        c = f.farDist / del,
                        d = -f.nearDist*f.farDist / del;
    Mat44D            ret
    {{
        a, 0, 0, 0,
        0, b, 0, 0,
        0, 0, c, d,
        0, 0, 1, 0
    }};
    return Mat44F(ret);
}
}

struct  Gui3dWin : public GuiBaseImpl
{
    HWND                    m_hwnd;
    Gui3d                   m_api;
    Vec2UI               m_size;
    HDC                     m_hdc;
    Vec2I                m_lastPos;          // Last mouse position in CC (only valid if drag!=None)
    ULONGLONG               m_lastGestureVal;   // Need to keep last gesture val to calc differences.
    DfgFPtr                    m_updateBgImg;      // Update BG image ?
    Mat44F                m_worldToD3ps;      // Transform used for last render (for mouse-surface intersection)

    unique_ptr<D3d>         m_d3d;

    Gui3dWin(const Gui3d & api)
    : m_api(api)
    {
        // Including api.viewBounds, api.pan/tiltDegrees and api.logRelSize in the below caused a feedback
        // issue that broke updates of the other sliders:
        m_updateBgImg = makeUpdateFlag(api.bgImg.imgN);
    }

    virtual void
    create(HWND parentHwnd,int ident,Ustring const &,DWORD extStyle,bool visible)
    {
        WinCreateChild   cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        cc.useFillBrush = false;
        winCreateChild(parentHwnd,ident,this,cc);
        // Don't create D3D here this is parent context.
    }

    virtual void
    destroy()
    {
        // Don't release D3D here - this is parent context and isn't actually called.
        DestroyWindow(m_hwnd);
    }

    virtual Vec2UI
    getMinSize() const
    {return Vec2UI(400,500); }

    virtual Vec2B wantStretch() const { return Vec2B(true, true); }

    virtual void
    updateIfChanged()   // Just always render
    {
        // This flips the dirty bit (QS_PAINT) for the render window but Windows will not
        // actually send a WM_PAINT message until the message queue is empty for a fraction
        // of a second (regardless of background paint flag):
        InvalidateRect(m_hwnd,NULL,FALSE);
        // So tell the system we want this updated immediately, for fluid interactive display:
        UpdateWindow(m_hwnd);
    }

    virtual void moveWindow(Vec2I lo, Vec2I sz) { MoveWindow(m_hwnd,lo[0],lo[1],sz[0],sz[1],TRUE); }

    virtual void showWindow(bool s)  { ShowWindow(m_hwnd,s ? SW_SHOW : SW_HIDE); }

    LRESULT
    wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        if (msg == WM_CREATE) {
            m_hwnd = hwnd;
            m_hdc = GetDC(hwnd);
            PIXELFORMATDESCRIPTOR pfd;
            ZeroMemory(&pfd,sizeof(pfd));
            pfd.nSize      = sizeof(pfd);
            pfd.nVersion   = 1;
            pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = 32;
            pfd.cDepthBits = 24;
            pfd.cAlphaBits = 8;
            pfd.cStencilBits = 8;
            int format = ChoosePixelFormat(m_hdc,&pfd);
            FGASSERTWIN(format != 0);
            FGASSERTWIN(SetPixelFormat(m_hdc,format,&pfd));
            FGASSERTWIN(DescribePixelFormat(m_hdc,format,pfd.nSize,&pfd));
            m_d3d.reset(new D3d(hwnd));
            m_api.capture->getDims = bind(&Gui3dWin::dims,this);
            m_api.capture->getImg = bind(&Gui3dWin::capture,this,placeholders::_1);
            // The pinch-to-zoom gesture is enabled by default but not the rotate gesture, which we
            // must explicitly enable:
            GESTURECONFIG   config = {0};
            config.dwWant = GC_ROTATE;
            config.dwID = GID_ROTATE;
            config.dwBlock = 0;
            // This function returned a "not implemented" error on a German Windows 7 64bit SP1 system,
            // so don't throw on error, just continue and presumably no gesture messages will be received:
            SetGestureConfig(hwnd,0,1,&config,sizeof(GESTURECONFIG));
        }
        else if (msg == WM_SIZE) {
            uint        wid = LOWORD(lParam),
                        hgt = HIWORD(lParam);
            // In case 0 size sent at creation or minimize:
            if (wid*hgt > 0) {
                m_size = Vec2UI(wid,hgt);
                m_api.viewportDims.set(m_size);
                m_d3d->resize(m_size);
            }
        }
        else if (msg == WM_PAINT) {
            PAINTSTRUCT ps;
            // Begin/End Paint required for D3D render or other windows don't render properly,
            // but swapBuffers not required.
            // Validates the invalid region. Doesn't erase background in this case since brush is NULL.
            BeginPaint(hwnd,&ps);
            render();
            // One customer had a Windows termination ('the program has stopped working and will now close' -
            // Win 10 Pro, NVIDIA GeForce GTX 760)
            // here, even though 'm_hdc' was valid and a face was already displayed on the screen (just once,
            // when the program first started). Buggy video driver (perhaps OpenGL specific).
            EndPaint(hwnd,&ps);
        }
        else if (msg == WM_MBUTTONDOWN) {
            if (wParam == (MK_SHIFT | MK_MBUTTON)) {
                Vec2I    pos(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
                const CtrlShiftMiddleClickAction & fn = m_api.ctrlShiftMiddleClickActionI.cref();
                if (fn) {
                    fn(m_size,pos,m_worldToD3ps);
                    winUpdateScreen();
                }
            }
        }
        else if ((msg == WM_LBUTTONDOWN) || (msg == WM_RBUTTONDOWN)) {
            m_lastPos = Vec2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            // This can only be the case for WM_LBUTTONDOWN:
            if (wParam == (MK_SHIFT | MK_CONTROL | MK_LBUTTON)) {
                m_api.markSurfPoint(m_size,m_lastPos,m_worldToD3ps);
                winUpdateScreen();
            }
            if (wParam == (MK_SHIFT | MK_CONTROL | MK_RBUTTON)) {
                m_api.markVertex(m_size,m_lastPos,m_worldToD3ps);
                winUpdateScreen();
            }
            else if ((wParam == (MK_CONTROL | MK_LBUTTON)) || (wParam == (MK_CONTROL | MK_RBUTTON))) {
                m_api.ctlClick(m_size,m_lastPos,m_worldToD3ps);
            }
            POINT   point;
            point.x = 0;
            point.y = 0;
            FGASSERTWIN(ClientToScreen(hwnd,&point));
            RECT    rect;
            rect.left = point.x;
            rect.top = point.y;
            rect.right = point.x + m_size[0];
            rect.bottom = point.y + m_size[1];
            FGASSERTWIN(ClipCursor(&rect));     // Prevent mouse from moving outside this window
        }
        else if ((msg == WM_LBUTTONUP) || (msg == WM_RBUTTONUP)) {
            // Only if both buttons are released:
            if ((wParam & (MK_LBUTTON | MK_RBUTTON)) == 0)
                ClipCursor(NULL);
        }
        else if (msg == WM_MOUSEMOVE) {
            Vec2I    pos = Vec2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            Vec2I    delta = pos-m_lastPos;
            if (wParam == MK_LBUTTON) {
                m_api.panTilt(delta);
                winUpdateScreen();
            }
            else if (wParam == (MK_LBUTTON | MK_SHIFT)) {
                m_api.translate(delta);
                winUpdateScreen();
            }
            else if (wParam == MK_MBUTTON) {
                m_api.translate(delta);
                winUpdateScreen();
            }
            else if (wParam == MK_RBUTTON) {
                m_api.scale(delta[1]);
                winUpdateScreen();
            }
            else if (wParam == (MK_RBUTTON | MK_SHIFT)) {
                const ShiftRightDragAction & fn = m_api.shiftRightDragActionI.cref();
                if (fn) {
                    fn(m_size,pos,m_worldToD3ps);
                    winUpdateScreen();
                }
            }
            else if (wParam == (MK_LBUTTON | MK_CONTROL)) {
                if (m_api.ctlDragAction) {
                    m_api.ctlDrag(true,m_size,delta,m_worldToD3ps);
                    winUpdateScreen();
                }
            }
            else if (wParam == (MK_RBUTTON | MK_CONTROL)) {
                if (m_api.ctlDragAction) {
                    m_api.ctlDrag(false,m_size,delta,m_worldToD3ps);
                    winUpdateScreen();
                }
            }
            else if (wParam == (MK_LBUTTON | MK_RBUTTON)) {
                const BothButtonsDragAction & fn = m_api.bothButtonsDragActionI.cref();
                if (fn) {
                    fn(false,delta);
                    winUpdateScreen();
                }
            }
            else if (wParam == (MK_LBUTTON | MK_RBUTTON | MK_SHIFT)) {
                const BothButtonsDragAction & fn = m_api.bothButtonsDragActionI.cref();
                if (fn) {
                    fn(true,delta);
                    winUpdateScreen();
                }
            }
            else if (wParam == (MK_LBUTTON | MK_SHIFT | MK_CONTROL)) {
                m_api.ctrlShiftLeftDrag(m_size,delta);
                winUpdateScreen();
            }
            else if (wParam == (MK_RBUTTON | MK_SHIFT | MK_CONTROL)) {
                m_api.ctrlShiftRightDrag(m_size,delta);
                winUpdateScreen();
            }
            m_lastPos = pos;
        }
        else if (msg == WM_CAPTURECHANGED) {
            ClipCursor(NULL);
        }
        else if (msg == WM_DESTROY) {
            // Release GPU resources for render objects. Not strictly necessary since API releases automatically
            // and will ignore later Release() calls but better to be explicit:
            m_api.rendMeshesN = makeIPT(RendMeshes());
            m_d3d->reset();
            m_d3d.reset();
            if (m_hdc) {
                ReleaseDC(hwnd,m_hdc);
                m_hdc = NULL;
            }
        }
        else if (msg == WM_GESTURE){ 
            GESTUREINFO     gi;
            ZeroMemory(&gi,sizeof(GESTUREINFO));
            gi.cbSize = sizeof(GESTUREINFO);
            if (GetGestureInfo((HGESTUREINFO)lParam,&gi)) {
                if (gi.dwFlags & GF_BEGIN) {
                    // Set to zero to indicate the start of a new gesture, and don't store
                    // current value since it's not valid:
                    m_lastGestureVal = 0;
                }
                else if (gi.dwID == GID_ZOOM) {     // Enabled by default
                    if (m_lastGestureVal > 0) {
                        m_api.scale(int(gi.ullArguments)-int(m_lastGestureVal));
                        winUpdateScreen();
                    }
                    m_lastGestureVal = gi.ullArguments;
                    return 0;
                }
                else if (gi.dwID == GID_ROTATE) {   // Enabled in WM_CREATE
                    if (m_lastGestureVal > 0) {
                        m_api.roll((int(gi.ullArguments)-int(m_lastGestureVal)) / 32);  // by experiment
                        winUpdateScreen();
                    }
                    m_lastGestureVal = gi.ullArguments;
                    return 0;
                }
            }
        }
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }

    void renderBackBuffer() {
        if (m_updateBgImg->checkUpdate())
            m_d3d->setBgImage(m_api.bgImg);

        const Camera &   camera = m_api.xform.cref();
        // A negative determinant matrix as D3D is a LEFT handed coordinate system:
        Mat33D                    oecsToD3vs = 
            {
                1, 0, 0,
                0, 1, 0,
                0, 0,-1,
            };
        Mat44F                    worldToD3vs = Mat44F((oecsToD3vs * camera.modelview).asHomogenous()),
                                    d3vsToD3ps = calcProjection(camera.frustum);
        m_worldToD3ps = d3vsToD3ps * worldToD3vs;
        RendOptions               options = m_api.renderOptions.cref();
        options.colorBySurface = m_api.colorBySurface.val();
        m_d3d->renderBackBuffer(
            m_api.bgImg,
            m_api.rendMeshesN.cref(),
            m_api.light.cref(),
            worldToD3vs,
            d3vsToD3ps,
            m_size,
            options);
    }

    void render() {
        renderBackBuffer();
        m_d3d->showBackBuffer();
    }

    Vec2UI
    dims()
    {return m_size; }

    ImgC4UC
    capture(Vec2UI dims)
    {
        m_d3d->resize(dims);
        m_api.viewportDims.set(dims);
        renderBackBuffer();
        ImgC4UC         ret = m_d3d->capture(dims);
        m_api.viewportDims.set(m_size);
        m_d3d->resize(m_size);
        return ret;
    }
};

GuiImplPtr
guiGetOsImpl(const Gui3d & def) {
	return GuiImplPtr(new Gui3dWin(def)); }

}

// */
