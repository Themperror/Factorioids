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
	int lives = 3;
	Timer invulnerabilityTimer;

	/////////
	//these variables are used to flicker the player sprite after being hit
	Timer lostLifeTimer;
	int lostLifeCounter = 0;
	bool shouldDraw = true;
	/////////

	std::vector<XMFLOAT4> vertices;
	VertexBuffer vertexBuffer;
	ComPtr<ID3D11Buffer> constantBuffer;
	ComPtr<ID3D11Buffer> queryConstantBuffer;
	std::array<ComPtr<ID3D11Query>, 3> occlusionQuery;
	std::array<uint64_t, 3> occlusionResults;
};

struct Rocket : Sprite
{
	std::vector<XMFLOAT4> vertices;
	VertexBuffer vertexBuffer;
	std::array<ComPtr<ID3D11Query>, 3> occlusionQuery;
	std::array<uint64_t, 3> occlusionResults;

	XMFLOAT2 velocity;
	XMFLOAT2 acceleration;
};

class Game : Scene
{
public:
	void SetFactorioPath(const std::string& path);
	virtual void Init(Renderer& renderer) final override;
	virtual Scene::Status Update(double dt, Input& input, Renderer& renderer) final override;
	void EraseAsteroid(size_t asteroidIndex);
	void BreakAsteroid(size_t asteroidIndex);
	void SpawnRandomAsteroid();
	void SpawnAsteroid(AsteroidType::Type asteroidType, AsteroidCategory::Category asteroidCategory, XMFLOAT2 position = {});
	virtual void Render(Renderer& renderer) final override;
private:
	std::string factorioPath;
	Timer spawnTimer;
	Timer rocketCooldownTimer;
	Player player;
	size_t currentQueryIndex = 0;

	ComPtr<ID3D11Buffer> asteroidConstantBuffer;

	std::vector<Rocket> rockets;
	ComPtr<ID3D11Buffer> rocketConstantBuffer;
	ComPtr<ID3D11Buffer> rocketQueryConstantBuffer;

	std::vector<Asteroid> asteroids;
	std::vector<AsteroidExplosion> asteroidExplosions;

	std::array<std::array<std::vector<int>, 36>, 64> asteroidGrid;
	
	std::array<std::array<std::vector<XMFLOAT4>, AsteroidType::ENUM_MAX>, AsteroidCategory::ENUM_MAX> asteroidBufferData;
	std::array<std::array<VertexBuffer, AsteroidType::ENUM_MAX>, AsteroidCategory::ENUM_MAX> asteroidVertices;

	std::array<std::array<ID3D11ShaderResourceView*, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesDiffuse;
	std::array<std::array<ID3D11ShaderResourceView*, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesNormal;
	std::array<std::array<ID3D11ShaderResourceView*, AsteroidType::ENUM_MAX>,AsteroidCategory::ENUM_MAX> asteroidTexturesRoughness;

	std::array<ID3D11ShaderResourceView*, AsteroidCategory::ENUM_MAX> asteroidExplosionsTextures;
	std::array<TextureInfo, AsteroidCategory::ENUM_MAX> asteroidExplosionTexInfos;
	std::array<ComPtr<ID3D11Buffer>, AsteroidCategory::ENUM_MAX> asteroidExplosionConstantData;
	ID3D11ShaderResourceView* mechArmorTexture;

	std::array<VertexBuffer, AsteroidCategory::ENUM_MAX> asteroidExplosionVertices;
	std::array<std::vector<XMFLOAT4>, AsteroidCategory::ENUM_MAX> asteroidExplosionBufferData;

	TextureInfo rocketTexInfo;
	ID3D11ShaderResourceView* rocketTexture;
	ID3D11ShaderResourceView* rocketLightTexture;
	ComPtr<ID3D11Buffer> rocketConstantData;

	


};