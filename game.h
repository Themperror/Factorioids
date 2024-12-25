#pragma once
#include "Scene.h"
#include <vector>

#include "renderer.h"
#include "asteroid.h"
#include "sprite.h"
#include "timer.h"

struct Asteroid;

struct Player : Sprite
{
	
};

class Game : Scene
{
public:
	virtual void Init(Renderer& renderer) final override;
	virtual Scene::Status Update(double dt, Input& input) final override;
	void EraseAsteroid(size_t asteroidIndex);
	void BreakAsteroid(size_t asteroidIndex);
	void SpawnRandomAsteroid();
	void SpawnAsteroid(AsteroidType::Type asteroidType, AsteroidCategory::Category asteroidCategory, XMFLOAT2 position = {});
	virtual void Render(Renderer& renderer) final override;
private:
	Timer spawnTimer;
	Player player;
	std::vector<Asteroid> asteroids;
	std::vector<AsteroidExplosion> asteroidExplosions;
	

	std::array<std::array<std::vector<XMFLOAT4>, AsteroidType::ENUM_MAX>, AsteroidCategory::ENUM_MAX> asteroidBufferData;
	std::array<std::array<VertexBuffer, AsteroidType::ENUM_MAX>, AsteroidCategory::ENUM_MAX> asteroidVertices;

	std::array<std::array<ComPtr<ID3D11ShaderResourceView>, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesDiffuse;
	std::array<std::array<ComPtr<ID3D11ShaderResourceView>, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesNormal;
	std::array<std::array<ComPtr<ID3D11ShaderResourceView>, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesRoughness;

	std::array<ComPtr<ID3D11Buffer>, AsteroidCategory::ENUM_MAX> asteroidExplosionConstantData;
	std::array<ComPtr<ID3D11ShaderResourceView>, AsteroidCategory::ENUM_MAX> asteroidExplosionsTextures;
	ComPtr<ID3D11ShaderResourceView> mechArmorTexture;

	std::array<VertexBuffer, AsteroidCategory::ENUM_MAX> asteroidExplosionVertices;
	std::array<std::vector<XMFLOAT4>, AsteroidCategory::ENUM_MAX> asteroidExplosionBufferData;
};