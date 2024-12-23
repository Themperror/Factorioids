#include "renderer.h"
#include "fileutils.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

bool Renderer::Init(HWND& hwnd, size_t width, size_t height)
{
	//////////////////
	// Device & Swapchain setup
	DXGI_SWAP_CHAIN_DESC swapDesc{};
	swapDesc.BufferDesc.Width = width;
	swapDesc.BufferDesc.Height = height;
	swapDesc.BufferDesc.RefreshRate = {};
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
	swapDesc.SampleDesc = { 1, 0 };
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.BufferCount = 2;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapDesc.Windowed = true;
	swapDesc.OutputWindow = hwnd;

	UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if _DEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, deviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &swapDesc, swapchain.GetAddressOf(), device.GetAddressOf(), nullptr, context.GetAddressOf());
	if(res != S_OK)
		return false;

	//////////////////
	
	//////////////////
	// Shaders & ILs
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements = {
		//LPCSTR SemanticName;
		//UINT SemanticIndex;
		//DXGI_FORMAT Format;
		//UINT InputSlot;
		//UINT AlignedByteOffset;
		//D3D11_INPUT_CLASSIFICATION InputSlotClass;
		//UINT InstanceDataStepRate;
		D3D11_INPUT_ELEMENT_DESC{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	{
		std::vector<uint8_t> uiVS = Util::ReadFileToVector("ui_VS.cso");
		if (uiVS.size() > 0)
		{
			res = device->CreateVertexShader(uiVS.data(), uiVS.size(), nullptr, ui_vertexShader.GetAddressOf());
			if (res != S_OK)
				return false;

			res = device->CreateInputLayout(inputElements.data(), inputElements.size(), uiVS.data(), uiVS.size(), ui_inputLayout.GetAddressOf());
			if (res != S_OK)
				return false;
		}
	}
	{
		std::vector<uint8_t> uiPS = Util::ReadFileToVector("ui_PS.cso");
		if (uiPS.size() > 0)
		{
			res = device->CreatePixelShader(uiPS.data(), uiPS.size(), nullptr, ui_pixelShader.GetAddressOf());
			if (res != S_OK)
				return false;
		}
	}
	{
		std::vector<uint8_t> asteroidVS = Util::ReadFileToVector("asteroid_VS.cso");
		if (asteroidVS.size() > 0)
		{
			res = device->CreatePixelShader(asteroidVS.data(), asteroidVS.size(), nullptr, asteroid_pixelShader.GetAddressOf());
			if (res != S_OK)
				return false;

			res = device->CreateInputLayout(inputElements.data(), inputElements.size(), asteroidVS.data(), asteroidVS.size(), asteroid_inputLayout.GetAddressOf());
			if (res != S_OK)
				return false;
		}
	}
	{
		std::vector<uint8_t> asteroidPS = Util::ReadFileToVector("asteroid_PS.cso");
		if (asteroidPS.size() > 0)
		{
			res = device->CreatePixelShader(asteroidPS.data(), asteroidPS.size(), nullptr, asteroid_pixelShader.GetAddressOf());
			if (res != S_OK)
				return false;
		}
	}
	//////////////////
	
	//////////////////
	// Default States & Objects
	{
		D3D11_DEPTH_STENCIL_DESC depthDesc{};
		depthDesc.DepthEnable = true;
		depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthDesc.StencilEnable = false;
		device->CreateDepthStencilState(&depthDesc, depthState.GetAddressOf());

		D3D11_RASTERIZER_DESC rasterDesc{};
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		device->CreateRasterizerState(&rasterDesc, rasterState.GetAddressOf());

		D3D11_BLEND_DESC blendDesc{};
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_COLOR;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = 0xF;

		device->CreateBlendState(&blendDesc, blendState.GetAddressOf());

		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
		device->CreateSamplerState(&samplerDesc,samplerState.GetAddressOf());
	}
	//////////////////

	return res == S_OK;
}

void Renderer::Resize(size_t width, size_t height)
{
	
}