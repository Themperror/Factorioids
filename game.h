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
	std::array<std::array<ComPtr<ID3D11ShaderResourceView>, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesDiffuse;
	std::array<std::array<ComPtr<ID3D11ShaderResourceView>, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesNormal;
	std::array<std::array<ComPtr<ID3D11ShaderResourceView>, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesRoughness;
};