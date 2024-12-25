#pragma once
#include "Scene.h"
#include <vector>

#include "renderer.h"
#include "asteroid.h"
#include "timer.h"

class Asteroid;

class Game : Scene
{
public:
	virtual void Init(Renderer& renderer) final override;
	virtual Scene::Status Update(double dt) final override;
	void BreakAsteroid(int asteroidIndex);
	void SpawnRandomAsteroid();
	void SpawnAsteroid(AsteroidType::Type asteroidType, AsteroidCategory::Category asteroidCategory);
	virtual void Render(Renderer& renderer) final override;
private:
	Timer spawnTimer;
	std::vector<Asteroid> asteroids;
	std::array<std::array<VertexBuffer, AsteroidType::ENUM_MAX>, AsteroidCategory::ENUM_MAX> asteroidVertices;

	std::array<std::array<ComPtr<ID3D11ShaderResourceView>, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesDiffuse;
	std::array<std::array<ComPtr<ID3D11ShaderResourceView>, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesNormal;
	std::array<std::array<ComPtr<ID3D11ShaderResourceView>, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesRoughness;
};