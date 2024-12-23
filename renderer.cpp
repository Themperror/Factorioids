#include "renderer.h"
#include "fileutils.h"
#include "print.h"
#include "break.h"

#define FREEIMAGE_LIB
#include "FreeImage.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

#if _DEBUG
#pragma comment(lib, "FreeImageLibd.lib")
#else
#pragma comment(lib, "FreeImageLib.lib")
#endif

bool Renderer::Init(HWND& hwnd, size_t width, size_t height)
{
	//////////////////
	// Device & Swapchain setup
	DXGI_SWAP_CHAIN_DESC swapDesc{};
	swapDesc.BufferDesc.Width = static_cast<UINT>(width);
	swapDesc.BufferDesc.Height = static_cast<UINT>(height);
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

			res = device->CreateInputLayout(inputElements.data(), static_cast<UINT>(inputElements.size()), uiVS.data(), static_cast<UINT>(uiVS.size()), ui_inputLayout.GetAddressOf());
			if (res != S_OK)
				return false;
		}
	}
	{
		std::vector<uint8_t> uiPS = Util::ReadFileToVector("ui_PS.cso");
		if (uiPS.size() > 0)
		{
			res = device->CreatePixelShader(uiPS.data(), static_cast<UINT>(uiPS.size()), nullptr, ui_pixelShader.GetAddressOf());
			if (res != S_OK)
				return false;
		}
	}
	{
		std::vector<uint8_t> asteroidVS = Util::ReadFileToVector("asteroid_VS.cso");
		if (asteroidVS.size() > 0)
		{
			res = device->CreateVertexShader(asteroidVS.data(), asteroidVS.size(), nullptr, asteroid_vertexShader.GetAddressOf());
			if (res != S_OK)
				return false;

			res = device->CreateInputLayout(inputElements.data(), static_cast<UINT>(inputElements.size()), asteroidVS.data(), static_cast<UINT>(asteroidVS.size()), asteroid_inputLayout.GetAddressOf());
			if (res != S_OK)
				return false;
		}
	}
	{
		std::vector<uint8_t> asteroidPS = Util::ReadFileToVector("asteroid_PS.cso");
		if (asteroidPS.size() > 0)
		{
			res = device->CreatePixelShader(asteroidPS.data(), static_cast<UINT>(asteroidPS.size()), nullptr, asteroid_pixelShader.GetAddressOf());
			if (res != S_OK)
				return false;
		}
	}
	//////////////////
	
	//////////////////
	// Default States & Objects
	{
		backBufferWidth = width;
		backBufferHeight = height;

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
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
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

	ID3D11Texture2D* backBufferTexture;
	swapchain->GetBuffer(0, IID_PPV_ARGS(&backBufferTexture));

	D3D11_TEXTURE2D_DESC tex2DDesc{};
	backBufferTexture->GetDesc(&tex2DDesc);

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = tex2DDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	res = device->CreateRenderTargetView(backBufferTexture,&rtvDesc, &backBuffer);
	if (res != S_OK)
	{
		Util::Print("Failed to create RTV from backbuffer");
		Util::Break();
	}

	D3D11_TEXTURE2D_DESC dsvTexDesc{};
	dsvTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvTexDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
	dsvTexDesc.Height = tex2DDesc.Height;
	dsvTexDesc.Width = tex2DDesc.Width;
	dsvTexDesc.Usage = D3D11_USAGE_DEFAULT;
	dsvTexDesc.MipLevels = 1;
	dsvTexDesc.SampleDesc.Count = 1;
	dsvTexDesc.ArraySize = 1;
	
	ID3D11Texture2D* dsvTex;
	device->CreateTexture2D(&dsvTexDesc, nullptr, &dsvTex);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = dsvTexDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(dsvTex, &dsvDesc, depthTarget.GetAddressOf());

	FreeImage_Initialise();

	return res == S_OK;
}

void Renderer::Resize(size_t width, size_t height)
{
	
}

VertexBuffer Renderer::CreateVertexBuffer(size_t vertexAmount, size_t vertexCapacity, size_t vertexSize)
{
	vertexCapacity = std::max(vertexAmount, vertexCapacity);

	VertexBuffer buffer{};
	buffer.maxVertexCount = vertexCapacity;
	buffer.currentVertexCount = vertexAmount;
	
	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = static_cast<UINT>(vertexSize * buffer.maxVertexCount);
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	
	device->CreateBuffer(&desc, nullptr, buffer.buffer.GetAddressOf());

	return buffer;
}

void Renderer::UpdateVertexBuffer(VertexBuffer& buffer, std::vector<XMFLOAT2>& vertices)
{
	if (vertices.size() > buffer.maxVertexCount)
	{
		buffer = CreateVertexBuffer(vertices.size(), vertices.size() * 2, sizeof(XMFLOAT2));
	}
	
	D3D11_MAPPED_SUBRESOURCE ms;
	if (context->Map(buffer.buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms) == S_OK)
	{
		memcpy(ms.pData,vertices.data(), vertices.size() * sizeof(XMFLOAT2));
		context->Unmap(buffer.buffer.Get(), 0);
	}
	else
	{
		Util::Print("Failed to map vertex buffer! Count: %llu, Size: %llu", vertices.size(), vertices.size() * sizeof(XMFLOAT2));
		Util::Break();
	}
}

ID3D11PixelShader* Renderer::GetAsteroidPixelShader()
{
	return asteroid_pixelShader.Get();
}

ID3D11VertexShader* Renderer::GetAsteroidVertexShader()
{
	return asteroid_vertexShader.Get();
}

ID3D11PixelShader* Renderer::GetUIPixelShader()
{
	return ui_pixelShader.Get();
}

ID3D11VertexShader* Renderer::GetUIVertexShader()
{
	return ui_vertexShader.Get();
}

ComPtr<ID3D11ShaderResourceView> Renderer::MakeTextureFrom(const std::string& filePath, bool isSrgb)
{
	FIBITMAP* image;
	{
		std::vector<uint8_t> fileData = Util::ReadFileToVector(filePath);
		FIMEMORY* mem = FreeImage_OpenMemory(fileData.data(),fileData.size());
		FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeFromMemory(mem);
		if(format == FREE_IMAGE_FORMAT::FIF_UNKNOWN)
			return {};

		image = FreeImage_LoadFromMemory(format, mem);
		FreeImage_CloseMemory(mem);
	}
	image = FreeImage_ConvertTo32Bits(image);
	BYTE* bits = FreeImage_GetBits(image);
	uint32_t height = FreeImage_GetHeight(image);
	uint32_t width = FreeImage_GetWidth(image);
	
	D3D11_SUBRESOURCE_DATA initialData{};
	initialData.pSysMem = bits;
	initialData.SysMemPitch = width * 4;

	D3D11_TEXTURE2D_DESC texDesc{};
	
	texDesc.Format = isSrgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.Height = height;
	texDesc.Width =width;
	texDesc.SampleDesc.Count = 1;
	//TODO generate mips
	texDesc.MipLevels = 1;
	
	ComPtr<ID3D11Texture2D> newTexture;
	HRESULT res = device->CreateTexture2D(&texDesc,&initialData, newTexture.GetAddressOf() );
	FreeImage_Unload(image);
	if(res != S_OK)
		return {};

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = isSrgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D10_1_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	ComPtr<ID3D11ShaderResourceView> newSRV;
	res = device->CreateShaderResourceView(newTexture.Get(), &srvDesc, newSRV.GetAddressOf());

	if (res != S_OK)
		return {};

	return newSRV;
}

void Renderer::Clear(float r, float g, float b, float a)
{
	float clearColor[4] = {r,g,b,a};
	context->ClearRenderTargetView(backBuffer.Get(), clearColor);
	context->ClearDepthStencilView(depthTarget.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Renderer::BeginDraw()
{
	context->OMSetRenderTargets(1, backBuffer.GetAddressOf(), depthTarget.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(asteroid_inputLayout.Get());
	context->RSSetState(rasterState.Get());
	const float blendFactor[4] = {1,1,1,1};
	context->OMSetBlendState(blendState.Get(),blendFactor, 0xF);
	context->PSSetSamplers(0,1,samplerState.GetAddressOf());

	D3D11_VIEWPORT viewPort{};
	viewPort.Width = backBufferWidth;
	viewPort.Height = backBufferHeight;
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1;
	context->RSSetViewports(1, &viewPort);
}

void Renderer::Draw(VertexBuffer& vertexBuffer, ID3D11PixelShader* pixelShader, ID3D11VertexShader* vertexShader, const std::vector<ID3D11ShaderResourceView*>& textures)
{
	if (currentPS != pixelShader)
	{
		currentPS = pixelShader;
		currentVS = vertexShader;
		context->PSSetShader(pixelShader, nullptr, 0);
		context->VSSetShader(vertexShader, nullptr, 0);
	}

	context->PSSetShaderResources(0, textures.size(), textures.data());

	UINT stride = sizeof(XMFLOAT2);
	UINT offset = 0;
	context->IASetVertexBuffers(0,1, vertexBuffer.buffer.GetAddressOf(), &stride, &offset);
	context->Draw(vertexBuffer.currentVertexCount, 0);
}

void Renderer::Present()
{
	swapchain->Present(1,0);
}

