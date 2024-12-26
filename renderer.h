#pragma once

#define NOMINMAX
#define NONEARFAR
#define WIN32_LEAN_AND_MEAN

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>
#include <array>
#include <string>
#include "print.h"
#include "break.h"

#include <thread>
#include <mutex>

using namespace DirectX;

#define FWD_DECL_HANDLE(name) struct name##__; typedef struct name##__ *name
FWD_DECL_HANDLE(HWND);

template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

template<typename T>
void SwapAndPop(T& container, size_t index)
{
	std::swap(container[index], (*(container.begin() + container.size()-1)));
	container.pop_back();
}

struct VertexBuffer
{
	ComPtr<ID3D11Buffer> buffer;
	size_t currentVertexCount;
	size_t maxVertexCount;
	UINT stride;
};

struct Material
{
	ComPtr<ID3D11PixelShader> pixelShader;
	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11InputLayout> inputLayout;
};

class Renderer
{
public:
	~Renderer();
	bool Init(HWND& hwnd, size_t width, size_t height);
	void Resize(size_t width, size_t height);

	ComPtr<ID3D11Buffer> CreateSpriteConstantBuffer(int numSpriteX, int numSpriteY);

	VertexBuffer CreateVertexBuffer(size_t vertexAmount, size_t vertexCapacity, size_t vertexSize);

	template<typename T>
	void UpdateVertexBuffer(VertexBuffer& buffer, std::vector<T>& vertices) 
	{
		if (vertices.size() > buffer.maxVertexCount)
		{
			buffer = CreateVertexBuffer(vertices.size(), vertices.size() * 2, sizeof(T));
		}

		D3D11_MAPPED_SUBRESOURCE ms;
		if (context->Map(buffer.buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &ms) == S_OK)
		{
			memcpy(ms.pData, vertices.data(), vertices.size() * sizeof(T));
			context->Unmap(buffer.buffer.Get(), 0);
		}
		else
		{
			Util::Print("Failed to map vertex buffer! Count: %llu, Size: %llu", vertices.size(), vertices.size() * sizeof(T));
			Util::Break();
		}
		buffer.currentVertexCount = vertices.size();
	}

	Material* GetAsteroidMaterial()
	{
		return &asteroidMaterial;
	}

	Material* GetSpriteMaterial()
	{
		return &spriteMaterial;
	}

	Material* GetUIMaterial()
	{
		return &UIMaterial;
	}


	int MakeTextureFrom(const std::string& filePath, bool isSrgb);
	int MakeTextureArrayFrom(const std::vector<std::string>& filePath, bool isSrgb);
	ID3D11ShaderResourceView* GetTexture(int handle) { return textures[handle].Get();};
	void Clear(float r, float g, float b, float a);
	void BeginDraw();

	template<typename T, typename U>
	void Draw(VertexBuffer& vertexBuffer, Material* material, const T& textures, const U& VSConstantBuffers)
	{
		if (currentMaterial != material)
		{
			context->PSSetShader(material->pixelShader.Get(), nullptr, 0);
			context->VSSetShader(material->vertexShader.Get(), nullptr, 0);
			context->IASetInputLayout(material->inputLayout.Get());
		}

		context->VSSetConstantBuffers(0, static_cast<UINT>(VSConstantBuffers.size()), VSConstantBuffers.data());
		context->PSSetShaderResources(0, static_cast<UINT>(textures.size()), textures.data());

		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, vertexBuffer.buffer.GetAddressOf(), &vertexBuffer.stride, &offset);
		context->Draw(static_cast<UINT>(vertexBuffer.currentVertexCount), 0);
	}

	void FlushLoading();
	void Present();
private:
	struct TextureLoad
	{
		std::vector<std::string> path;
		int texHandle;
		bool srgb;
	};
	std::mutex textureLock;
	std::mutex loadLock;
	std::vector<TextureLoad> texturesToLoad;
	void LoadingThread();
	ComPtr<ID3D11ShaderResourceView> MakeTextureFrom_internal(const std::string& filePath, bool isSrgb);
	ComPtr<ID3D11ShaderResourceView> MakeTextureArrayFrom_internal(const std::vector<std::string>& filePath, bool isSrgb);

	std::atomic_bool shuttingDown = false;

	std::vector<ComPtr<ID3D11ShaderResourceView>> textures;

	UINT backBufferWidth{};
	UINT backBufferHeight{};

	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain> swapchain;

	ComPtr<ID3D11RenderTargetView> backBuffer;
	ComPtr<ID3D11DepthStencilView> depthTarget;

	Material* currentMaterial{};

	Material asteroidMaterial;
	Material spriteMaterial;
	Material UIMaterial;

	std::vector<std::thread> loadingThreads;

	ComPtr<ID3D11DepthStencilState> depthState;
	ComPtr<ID3D11RasterizerState> rasterState;
	ComPtr<ID3D11BlendState> blendState;
	ComPtr<ID3D11SamplerState> samplerState;

};