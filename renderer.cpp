#include "renderer.h"
#include "fileutils.h"

#define FREEIMAGE_LIB
//RGB colororder for FreeImage
#define FREEIMAGE_COLORORDER 1
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


	//LPCSTR SemanticName;
	//UINT SemanticIndex;
	//DXGI_FORMAT Format;
	//UINT InputSlot;
	//UINT AlignedByteOffset;
	//D3D11_INPUT_CLASSIFICATION InputSlotClass;
	//UINT InstanceDataStepRate;
	std::vector<D3D11_INPUT_ELEMENT_DESC> asteroidInputElements = {
		D3D11_INPUT_ELEMENT_DESC{"POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
	};	
	std::vector<D3D11_INPUT_ELEMENT_DESC> spriteInputElements = {
		D3D11_INPUT_ELEMENT_DESC{"POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	std::vector<D3D11_INPUT_ELEMENT_DESC> UIInputElements = {
		D3D11_INPUT_ELEMENT_DESC{"POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	{
		std::vector<uint8_t> uiVS = Util::ReadFileToVector("ui_VS.cso");
		if (uiVS.size() > 0)
		{
			res = device->CreateVertexShader(uiVS.data(), uiVS.size(), nullptr, UIMaterial.vertexShader.GetAddressOf());
			if (res != S_OK)
				return false;

			res = device->CreateInputLayout(UIInputElements.data(), static_cast<UINT>(UIInputElements.size()), uiVS.data(), static_cast<UINT>(uiVS.size()), UIMaterial.inputLayout.GetAddressOf());
			if (res != S_OK)
				return false;
		}
	}
	{
		std::vector<uint8_t> uiPS = Util::ReadFileToVector("ui_PS.cso");
		if (uiPS.size() > 0)
		{
			res = device->CreatePixelShader(uiPS.data(), static_cast<UINT>(uiPS.size()), nullptr, UIMaterial.pixelShader.GetAddressOf());
			if (res != S_OK)
				return false;
		}
	}
	{
		std::vector<uint8_t> asteroidVS = Util::ReadFileToVector("asteroid_VS.cso");
		if (asteroidVS.size() > 0)
		{
			res = device->CreateVertexShader(asteroidVS.data(), asteroidVS.size(), nullptr, asteroidMaterial.vertexShader.GetAddressOf());
			if (res != S_OK)
				return false;

			res = device->CreateInputLayout(asteroidInputElements.data(), static_cast<UINT>(asteroidInputElements.size()), asteroidVS.data(), static_cast<UINT>(asteroidVS.size()), asteroidMaterial.inputLayout.GetAddressOf());
			if (res != S_OK)
				return false;
		}
	}
	{
		std::vector<uint8_t> asteroidPS = Util::ReadFileToVector("asteroid_PS.cso");
		if (asteroidPS.size() > 0)
		{
			res = device->CreatePixelShader(asteroidPS.data(), static_cast<UINT>(asteroidPS.size()), nullptr, asteroidMaterial.pixelShader.GetAddressOf());
			if (res != S_OK)
				return false;
		}
	}
	{
		std::vector<uint8_t> spriteVS = Util::ReadFileToVector("sprite_VS.cso");
		if (spriteVS.size() > 0)
		{
			res = device->CreateVertexShader(spriteVS.data(), spriteVS.size(), nullptr, spriteMaterial.vertexShader.GetAddressOf());
			if (res != S_OK)
				return false;

			res = device->CreateInputLayout(spriteInputElements.data(), static_cast<UINT>(spriteInputElements.size()), spriteVS.data(), static_cast<UINT>(spriteVS.size()), spriteMaterial.inputLayout.GetAddressOf());
			if (res != S_OK)
				return false;
		}
	}
	{
		std::vector<uint8_t> spritePS = Util::ReadFileToVector("sprite_PS.cso");
		if (spritePS.size() > 0)
		{
			res = device->CreatePixelShader(spritePS.data(), static_cast<UINT>(spritePS.size()), nullptr, spriteMaterial.pixelShader.GetAddressOf());
			if (res != S_OK)
				return false;
		}
	}
	//////////////////
	
	//////////////////
	// Default States & Objects
	{
		backBufferWidth = static_cast<UINT>(width);
		backBufferHeight = static_cast<UINT>(height);

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
		blendDesc.RenderTarget[0].BlendEnable = false;
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
		samplerDesc.MaxAnisotropy = 16;
		samplerDesc.MinLOD = -D3D11_FLOAT32_MAX;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
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

ComPtr<ID3D11Buffer> Renderer::CreateSpriteConstantBuffer(int numSpriteX, int numSpriteY)
{
	struct
	{
		int numX;
		int numY;
		int pad0;
		int pad1;

	} SpriteData;

	SpriteData.numX = numSpriteX;
	SpriteData.numY = numSpriteY;

	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = static_cast<UINT>(sizeof(SpriteData));
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_IMMUTABLE;

	ComPtr<ID3D11Buffer> buffer;
	D3D11_SUBRESOURCE_DATA initialData{};
	initialData.pSysMem = &SpriteData;
	initialData.SysMemPitch = sizeof(SpriteData);

	device->CreateBuffer(&desc, &initialData, buffer.GetAddressOf());

	return buffer;
}

VertexBuffer Renderer::CreateVertexBuffer(size_t vertexAmount, size_t vertexCapacity, size_t vertexSize)
{
	vertexCapacity = std::max(vertexAmount, vertexCapacity);

	VertexBuffer buffer{};
	buffer.maxVertexCount = vertexCapacity;
	buffer.currentVertexCount = vertexAmount;
	buffer.stride = static_cast<UINT>(vertexSize);
	
	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = static_cast<UINT>(vertexSize * buffer.maxVertexCount);
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	
	device->CreateBuffer(&desc, nullptr, buffer.buffer.GetAddressOf());

	return buffer;
}

ComPtr<ID3D11ShaderResourceView> Renderer::MakeTextureFrom(const std::string& filePath, bool isSrgb)
{
	const auto& it = textureCache.find(filePath);
	if (it != textureCache.end())
	{
		return it->second;
	}

	FIBITMAP* image;
	FIBITMAP* image2;
	{
		std::vector<uint8_t> fileData = Util::ReadFileToVector(filePath);
		FIMEMORY* mem = FreeImage_OpenMemory(fileData.data(), static_cast<DWORD>(fileData.size()));
		FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeFromMemory(mem);
		if (format == FREE_IMAGE_FORMAT::FIF_UNKNOWN)
			return {};

		image = FreeImage_LoadFromMemory(format, mem);
		FreeImage_CloseMemory(mem);
	}
	//Factorio images aren't always 32 bits, and BGR instead of RGB for some reason.. fixup here..
	image2 = FreeImage_ConvertTo32Bits(image);
	FreeImage_Unload(image);

	FIBITMAP* rChannel = FreeImage_GetChannel(image2, FREE_IMAGE_COLOR_CHANNEL::FICC_RED);
	FIBITMAP* bChannel = FreeImage_GetChannel(image2, FREE_IMAGE_COLOR_CHANNEL::FICC_BLUE);
	FreeImage_SetChannel(image2, bChannel, FREE_IMAGE_COLOR_CHANNEL::FICC_RED);
	FreeImage_SetChannel(image2, rChannel, FREE_IMAGE_COLOR_CHANNEL::FICC_BLUE);
	FreeImage_Unload(bChannel);
	FreeImage_Unload(rChannel);
	FreeImage_FlipVertical(image2);

	BYTE* bits = FreeImage_GetBits(image2);
	uint32_t height = FreeImage_GetHeight(image2);
	uint32_t width = FreeImage_GetWidth(image2);

	D3D11_SUBRESOURCE_DATA initialData{};
	initialData.pSysMem = bits;
	initialData.SysMemPitch = width * 4;

	D3D11_TEXTURE2D_DESC texDesc{};

	texDesc.Format = isSrgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.Height = height;
	texDesc.Width = width;
	texDesc.SampleDesc.Count = 1;
	//TODO generate mips
	texDesc.MipLevels = 1;

	ComPtr<ID3D11Texture2D> newTexture;
	HRESULT res = device->CreateTexture2D(&texDesc, &initialData, newTexture.GetAddressOf());
	FreeImage_Unload(image2);
	if (res != S_OK)
		return {};

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = isSrgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D10_1_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	ComPtr<ID3D11ShaderResourceView> newSRV;
	res = device->CreateShaderResourceView(newTexture.Get(), &srvDesc, newSRV.GetAddressOf());

	if (res != S_OK)
		return {};

	textureCache.emplace(filePath, newSRV);
	return newSRV;
}

ComPtr<ID3D11ShaderResourceView> Renderer::MakeTextureArrayFrom(const std::vector<std::string>& filePath, bool isSrgb)
{
	for (size_t i = 0; i < filePath.size(); i++)
	{
		const auto& it = textureCache.find(filePath[i]);
		if (it != textureCache.end())
		{
			return it->second;
		}
	}

	UINT textureWidth{}, textureHeight{};
	uint32_t imageSize{};
	std::vector<uint8_t> data;

	for (size_t i = 0; i < filePath.size(); i++)
	{
		FIBITMAP* image;
		FIBITMAP* image2;
		{
			std::vector<uint8_t> fileData = Util::ReadFileToVector(filePath[i]);
			FIMEMORY* mem = FreeImage_OpenMemory(fileData.data(), static_cast<DWORD>(fileData.size()));
			FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeFromMemory(mem);
			if (format == FREE_IMAGE_FORMAT::FIF_UNKNOWN)
				return {};

			image = FreeImage_LoadFromMemory(format, mem);
			FreeImage_CloseMemory(mem);
		}

		//Factorio images aren't always 32 bits, and BGR instead of RGB for some reason.. fixup here..
		image2 = FreeImage_ConvertTo32Bits(image);
		FreeImage_Unload(image);

		FIBITMAP* rChannel = FreeImage_GetChannel(image2, FREE_IMAGE_COLOR_CHANNEL::FICC_RED);
		FIBITMAP* bChannel = FreeImage_GetChannel(image2, FREE_IMAGE_COLOR_CHANNEL::FICC_BLUE);
		FreeImage_SetChannel(image2,bChannel, FREE_IMAGE_COLOR_CHANNEL::FICC_RED);
		FreeImage_SetChannel(image2,rChannel, FREE_IMAGE_COLOR_CHANNEL::FICC_BLUE);
		FreeImage_Unload(bChannel);
		FreeImage_Unload(rChannel);
		FreeImage_FlipVertical(image2);
		
		BYTE* bits = FreeImage_GetBits(image2);
		textureHeight = FreeImage_GetHeight(image2);
		textureWidth = FreeImage_GetWidth(image2);

		imageSize = textureHeight * textureWidth * 4;
		if (data.size() == 0)
		{
			data.resize(imageSize * filePath.size());
		}
		memcpy(data.data() + (imageSize * i), bits, imageSize );
		FreeImage_Unload(image2);
	}


	std::vector<D3D11_SUBRESOURCE_DATA> initialData;
	initialData.resize(filePath.size());
	for (size_t i = 0; i < filePath.size(); i++)
	{
		initialData[i].pSysMem = data.data() + imageSize * i;
		initialData[i].SysMemPitch = textureWidth * 4;
		initialData[i].SysMemSlicePitch = 0;
	}

	D3D11_TEXTURE2D_DESC texDesc{};

	texDesc.Format = isSrgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.ArraySize = static_cast<UINT>(filePath.size());
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.Height = textureHeight;
	texDesc.Width = textureWidth;
	texDesc.SampleDesc.Count = 1;

	//TODO generate mips
	texDesc.MipLevels = 1;

	ComPtr<ID3D11Texture2D> newTexture;
	HRESULT res = device->CreateTexture2D(&texDesc, initialData.data(), newTexture.GetAddressOf());
	if (res != S_OK)
		return {};

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = isSrgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.ArraySize = static_cast<UINT>(filePath.size());
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	ComPtr<ID3D11ShaderResourceView> newSRV;
	res = device->CreateShaderResourceView(newTexture.Get(), &srvDesc, newSRV.GetAddressOf());

	if (res != S_OK)
		return {};

	for (size_t i = 0; i < filePath.size(); i++)
	{
		textureCache.emplace(filePath[i], newSRV);
	}
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
	context->RSSetState(rasterState.Get());
	const float blendFactor[4] = {1,1,1,1};
	context->OMSetBlendState(blendState.Get(),blendFactor, 0xF);
	context->PSSetSamplers(0,1,samplerState.GetAddressOf());

	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(backBufferWidth);
	viewPort.Height = static_cast<float>(backBufferHeight);
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1;
	context->RSSetViewports(1, &viewPort);
}

void Renderer::Present()
{
	swapchain->Present(1,0);
}

