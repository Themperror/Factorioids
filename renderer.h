#pragma once

#include <d3d11.h>
#include <wrl.h>

#define FWD_DECL_HANDLE(name) struct name##__; typedef struct name##__ *name
FWD_DECL_HANDLE(HWND);

template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class Renderer
{
public:
	bool Init(HWND& hwnd, size_t width, size_t height);
	void Resize(size_t width, size_t height);

private:
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain> swapchain;

	ComPtr<ID3D11PixelShader> ui_pixelShader;
	ComPtr<ID3D11VertexShader> ui_vertexShader;
	ComPtr<ID3D11InputLayout> ui_inputLayout;

	ComPtr<ID3D11PixelShader> asteroid_pixelShader;
	ComPtr<ID3D11VertexShader> asteroid_vertexShader;
	ComPtr<ID3D11InputLayout> asteroid_inputLayout;

	ComPtr<ID3D11DepthStencilState> depthState;
	ComPtr<ID3D11RasterizerState> rasterState;
	ComPtr<ID3D11BlendState> blendState;
	ComPtr<ID3D11SamplerState> samplerState;

};