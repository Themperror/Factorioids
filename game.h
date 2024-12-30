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
	std::vector<XMFLOAT4> vertices;
	VertexBuffer vertexBuffer;
	ComPtr<ID3D11Buffer> constantBuffer;
	int lives = 3;
	Timer invulnerabilityTimer;

	//these variables are used to flicker the player sprite after being hit
	Timer lostLifeTimer;
	int lostLifeCounter = 0;
	bool shouldDraw = true;
	/////////

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
	std::array<ComPtr<ID3D11Query>,3> playerQuery;
	std::array<uint64_t,3> playerQueryResults;
	size_t currentQueryIndex = 0;

	ComPtr<ID3D11Buffer> asteroidConstantBuffer;
	ComPtr<ID3D11Buffer> queryConstantBuffer;

	std::vector<Asteroid> asteroids;
	std::vector<AsteroidExplosion> asteroidExplosions;

	std::array<std::array<std::vector<int>, 36>, 64> asteroidGrid;
	

	std::array<std::array<std::vector<XMFLOAT4>, AsteroidType::ENUM_MAX>, AsteroidCategory::ENUM_MAX> asteroidBufferData;
	std::array<std::array<VertexBuffer, AsteroidType::ENUM_MAX>, AsteroidCategory::ENUM_MAX> asteroidVertices;

	std::array<std::array<ComPtr<ID3D11ShaderResourceView>, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesDiffuse;
	std::array<std::array<ComPtr<ID3D11ShaderResourceView>, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesNormal;
	std::array<std::array<ComPtr<ID3D11ShaderResourceView>, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesRoughness;

	std::array<ComPtr<ID3D11ShaderResourceView>, AsteroidCategory::ENUM_MAX> asteroidExplosionsTextures;
	std::array<ComPtr<ID3D11Buffer>, AsteroidCategory::ENUM_MAX> asteroidExplosionConstantData;
	ComPtr<ID3D11ShaderResourceView> mechArmorTexture;

	std::array<VertexBuffer, AsteroidCategory::ENUM_MAX> asteroidExplosionVertices;
	std::array<std::vector<XMFLOAT4>, AsteroidCategory::ENUM_MAX> asteroidExplosionBufferData;
};