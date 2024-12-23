#pragma once
#include "Scene.h"
#include <vector>

#include "renderer.h"
#include "asteroid.h"

class Asteroid;

class Game : Scene
{
public:
	virtual void Init(Renderer& renderer) final override;
	virtual Scene::Status Update(double dt) final override;
	virtual void Render(Renderer& renderer) final override;
private:
	std::vector<Asteroid> asteroids;
	VertexBuffer asteroidVertices;
	std::vector<ComPtr<ID3D11ShaderResourceView>> asteroidTexturesDiffuse;
	std::vector<ComPtr<ID3D11ShaderResourceView>> asteroidTexturesNormal;
	std::vector<ComPtr<ID3D11ShaderResourceView>> asteroidTexturesRoughness;
};