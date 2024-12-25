#include "game.h"
#include "renderer.h"
#include "asteroid.h"
#include "fileutils.h"

void Game::Init(Renderer& renderer)
{
	std::array<const char*, AsteroidCategory::ENUM_MAX> categoryStrings = {"small", "medium", "big", "huge"};
	std::array<const char*, AsteroidType::ENUM_MAX> typeStrings = {"carbonic", "metallic", "oxide", "promethium"};


	const char* factorioAsteroidPath = "C:/Program Files (x86)/Steam/steamapps/common/Factorio/data/space-age/graphics/entity/asteroid/";
	const char* factorioAsteroidExplosionPath = "C:/Program Files (x86)/Steam/steamapps/common/Factorio/data/space-age/graphics/entity/asteroid-explosions/";
	const char* factorioMechPath = "C:/Program Files (x86)/Steam/steamapps/common/Factorio/data/space-age/graphics/entity/mech-armor/mech-idle-air.png";

	player.Init(renderer.MakeTextureFrom(factorioMechPath, true),5,8,60.0f);
	player.SetSpriteRange(0,5);
	player.scale = 20;
	player.vertexBuffer = renderer.CreateVertexBuffer(6,6,sizeof(XMFLOAT4));
	player.constantBuffer = renderer.CreateSpriteConstantBuffer(5, 8);

	char buf[512];

	//cache the asteroid explosion textures
	sprintf_s(buf, sizeof(buf), "%s%s", factorioAsteroidExplosionPath, "asteroid-explosion-small.png");
	asteroidExplosionsTextures[AsteroidCategory::Small] = renderer.MakeTextureFrom(buf, true);
	asteroidExplosionConstantData[AsteroidCategory::Small] = renderer.CreateSpriteConstantBuffer(6,6);
	sprintf_s(buf, sizeof(buf), "%s%s", factorioAsteroidExplosionPath, "asteroid-explosion-medium.png");
	asteroidExplosionsTextures[AsteroidCategory::Medium] = renderer.MakeTextureFrom(buf, true);
	asteroidExplosionConstantData[AsteroidCategory::Medium] = asteroidExplosionConstantData[AsteroidCategory::Small];
	sprintf_s(buf, sizeof(buf), "%s%s", factorioAsteroidExplosionPath, "asteroid-explosion-big.png");
	asteroidExplosionsTextures[AsteroidCategory::Big] = renderer.MakeTextureFrom(buf, true);
	asteroidExplosionConstantData[AsteroidCategory::Big] = asteroidExplosionConstantData[AsteroidCategory::Small];
	sprintf_s(buf, sizeof(buf), "%s%s", factorioAsteroidExplosionPath, "asteroid-explosion-huge.png");
	asteroidExplosionsTextures[AsteroidCategory::Huge] = renderer.MakeTextureFrom(buf, true);
	asteroidExplosionConstantData[AsteroidCategory::Huge] = asteroidExplosionConstantData[AsteroidCategory::Small];


	std::vector<std::string> diffuseNames;
	std::vector<std::string> normalNames;
	std::vector<std::string> roughnessNames;

	//cache all the asteroids
	for (size_t i = 0; i < AsteroidType::ENUM_MAX; i++)
	{
		for (size_t j = 0; j < AsteroidCategory::ENUM_MAX; j++)
		{
			sprintf_s(buf, sizeof(buf), "%s%s/%s/", factorioAsteroidPath, typeStrings[i], categoryStrings[j]);
			std::vector<std::string> files = Util::LoadFilesFromDirectory(buf);
			files.erase(std::remove_if(files.begin(),files.end(),[](auto& a){return a.find("colour") == std::string::npos;}), files.end());
			for (unsigned int k = 0; k < files.size(); k++)
			{
				sprintf_s(buf, sizeof(buf), "%s%s/%s/asteroid-%s-%s-colour-0%i.png", factorioAsteroidPath, typeStrings[i], categoryStrings[j], typeStrings[i], categoryStrings[j], k+1);
				diffuseNames.push_back(buf);
				sprintf_s(buf, sizeof(buf), "%s%s/%s/asteroid-%s-%s-normal-0%i.png", factorioAsteroidPath, typeStrings[i], categoryStrings[j], typeStrings[i], categoryStrings[j], k+1);
				normalNames.push_back(buf);
				sprintf_s(buf, sizeof(buf), "%s%s/%s/asteroid-%s-%s-roughness-0%i.png", factorioAsteroidPath, typeStrings[i], categoryStrings[j], typeStrings[i], categoryStrings[j], k+1);
				roughnessNames.push_back(buf);
			}

			asteroidTexturesDiffuse[j][i] = renderer.MakeTextureArrayFrom(diffuseNames, true);
			asteroidTexturesNormal[j][i] = renderer.MakeTextureArrayFrom(normalNames, false);
			asteroidTexturesRoughness[j][i] = renderer.MakeTextureArrayFrom(roughnessNames, false);

			diffuseNames.clear();
			normalNames.clear();
			roughnessNames.clear();
		}
	}


	for (size_t i = 0; i < asteroidVertices.size(); i++)
	{
		for (size_t j = 0; j < asteroidVertices[i].size(); j++)
		{
			if (!asteroidVertices[i][j].buffer)
			{
				//keep some small minimum size
				asteroidVertices[i][j] = renderer.CreateVertexBuffer(0, 4096llu, sizeof(XMFLOAT4));
			}
		}
	}
	for (size_t i = 0; i < asteroidExplosionVertices.size(); i++)
	{
		asteroidExplosionVertices[i] = renderer.CreateVertexBuffer(0, 4096llu, sizeof(XMFLOAT4));
	}

	spawnTimer.Start(0.3);
}

template<typename T>
void SwapAndPop(T& container, size_t index)
{
	std::swap(container[index], container.back());
	container.pop_back();
}

Scene::Status Game::Update(double dt, Input& input)
{
	if (spawnTimer.HasFinished())
	{
		spawnTimer.RestartWithRemainder();
		SpawnRandomAsteroid();
	}

	for (size_t i = 0; i < asteroids.size(); i++)
	{
		Asteroid& asteroid = asteroids[i];
		asteroid.rot += static_cast<float>(dt * asteroid.rotSpeed);
		asteroid.pos.x += static_cast<float>(dt * asteroid.velocity.x);
		asteroid.pos.y += static_cast<float>(dt * asteroid.velocity.y);
		if (asteroid.pos.x < -1000 || asteroid.pos.x > 1000 || asteroid.pos.y < -500 || asteroid.pos.y > 500)
		{
			EraseAsteroid(i);
			i--;
		}
	}

	for (size_t i = 0; i < asteroidExplosions.size(); i++)
	{
		asteroidExplosions[i].Update();		
		if (asteroidExplosions[i].HasFinishedAnimation())
		{
			SwapAndPop(asteroidExplosions, i);
			i--;
		}
	}

	player.Update();

	return Status::Running;
}

void Game::EraseAsteroid(size_t asteroidIndex)
{
	SwapAndPop(asteroids,asteroidIndex);
}

void Game::BreakAsteroid(size_t asteroidIndex)
{
	assert(asteroidIndex < asteroids.size());
	Asteroid& asteroid = asteroids[asteroidIndex];
	AsteroidCategory::Category category = asteroid.category;
	AsteroidType::Type type = asteroid.type;
	XMFLOAT2 position = asteroid.pos;

	AsteroidExplosion& explosion = asteroidExplosions.emplace_back();
	explosion.Init(asteroidExplosionsTextures[category], 5,5, 60.0f);
	explosion.position = position;
	explosion.rotation = asteroid.rot;
	explosion.scale = asteroid.size;
	explosion.category = category;

	switch (category)
	{
		case AsteroidCategory::Small:
		{
			EraseAsteroid(asteroidIndex);
		}
		break;
		case AsteroidCategory::Medium:
		{
			SpawnAsteroid(type, (AsteroidCategory::Category)(category - 1),position);
			SpawnAsteroid(type, (AsteroidCategory::Category)(category - 1),position);
			if (rand() & 1)
			{
				SpawnAsteroid(type, (AsteroidCategory::Category)(category - 1), position);
			}
			EraseAsteroid(asteroidIndex);
		}
		break;
		case AsteroidCategory::Big:
		{
			SpawnAsteroid(type, (AsteroidCategory::Category)(category - 1),position);
			SpawnAsteroid(type, (AsteroidCategory::Category)(category - 1),position);
			if (rand() % 100  < 25)
			{
				SpawnAsteroid(type, (AsteroidCategory::Category)(category - 1), position);
			}
			EraseAsteroid(asteroidIndex);
		}
		break;
		case AsteroidCategory::Huge:
		{
			SpawnAsteroid(type, (AsteroidCategory::Category)(category - 1),position);
			SpawnAsteroid(type, (AsteroidCategory::Category)(category - 1),position);
			if (rand() % 100 < 10)
			{
				SpawnAsteroid(type, (AsteroidCategory::Category)(category - 1), position);
			}
			EraseAsteroid(asteroidIndex);
		}
		break;
	}
}

void Game::SpawnRandomAsteroid()
{
	Asteroid& asteroid = asteroids.emplace_back();
	asteroid.category = (AsteroidCategory::Category)(rand() % AsteroidCategory::ENUM_MAX);
	asteroid.type = (AsteroidType::Type)(rand() % AsteroidType::ENUM_MAX);
	asteroid.health = 100.0f;
	asteroid.rot = XMConvertToRadians(static_cast<float>(rand() % 360));
	asteroid.rotSpeed = ((rand() % 100) - 50) * 0.005f;
	asteroid.variation = rand()% 65536;

	float angle = XMConvertToRadians(static_cast<float>(rand() % 360));
	asteroid.pos.x = sinf(angle) * 1000.0f;
	asteroid.pos.y = cosf(angle) * 500.0f;

	float speed = static_cast<float>(rand() % 20 + 10);
	asteroid.velocity.x = -sinf(angle) * speed * ((rand() % 50) / 50.0f);
	asteroid.velocity.y = -cosf(angle) * speed * ((rand() % 50) / 50.0f);

	switch (asteroid.category)
	{
	case AsteroidCategory::Category::Small:
		asteroid.size = (rand() % 20) + 10.0f;
		break;
	case AsteroidCategory::Category::Medium:
		asteroid.size = (rand() % 25) + 15.0f;
		break;
	case AsteroidCategory::Category::Big:
		asteroid.size = (rand() % 30) + 20.0f;
		break;
	case AsteroidCategory::Category::Huge:
		asteroid.size = (rand() % 40) + 35.0f;
		break;
	}
}
void Game::SpawnAsteroid(AsteroidType::Type asteroidType, AsteroidCategory::Category asteroidCategory, XMFLOAT2 position)
{
	Asteroid& asteroid = asteroids.emplace_back();
	asteroid.category = asteroidCategory;
	asteroid.type = asteroidType;
	asteroid.health = 100.0f;
	asteroid.rot = static_cast<float>(rand() % 360);
	asteroid.rotSpeed = ((rand() % 100) - 50) * 0.005f;

	float angle = XMConvertToRadians(static_cast<float>(rand() % 360));
	float speed = static_cast<float>(rand() % 20 + 10);
	asteroid.velocity.x = -sinf(angle) * speed * ((rand() % 50) / 50.0f);
	asteroid.velocity.y = -cosf(angle) * speed * ((rand() % 50) / 50.0f);

	switch (asteroid.category)
	{
	case AsteroidCategory::Category::Small:
		asteroid.size = (rand() % 20) + 10.0f;
		break;
	case AsteroidCategory::Category::Medium:
		asteroid.size = (rand() % 25) + 15.0f;
		break;
	case AsteroidCategory::Category::Big:
		asteroid.size = (rand() % 30) + 20.0f;
		break;
	case AsteroidCategory::Category::Huge:
		asteroid.size = (rand() % 40) + 35.0f;
		break;
	}

	if (position.x == 0 && position.y == 0)
	{
		asteroid.pos.x = sinf(angle) * 1000.0f;
		asteroid.pos.y = cosf(angle) * 500.0f;
	}
	else
	{
		asteroid.pos.x = position.x + (rand() % (int)asteroid.size);
		asteroid.pos.y = position.y + (rand() % (int)asteroid.size);
	}
}


void CreateQuad(XMFLOAT2 position, float rotation, float size, float customData, XMMATRIX& ortho, std::vector<XMFLOAT4>& buffer)
{
	size_t vertexIndex = buffer.size();

	XMVECTOR pos = XMLoadFloat2(&position);
	XMMATRIX TRS = XMMatrixTransformation2D(XMVectorZero(), 0, XMVectorSplatOne(), XMVectorZero(), rotation, pos);
	buffer.push_back(XMFLOAT4(-(size), +(size), 0, 0));
	buffer.push_back(XMFLOAT4(+(size), +(size), 0, 0));
	buffer.push_back(XMFLOAT4(-(size), -(size), 0, 0));
	
	buffer.push_back(XMFLOAT4(+(size), +(size), 0, 0));
	buffer.push_back(XMFLOAT4(+(size), -(size), 0, 0));
	buffer.push_back(XMFLOAT4(-(size), -(size), 0, 0));

	XMVECTOR posTL = XMLoadFloat4(&buffer[vertexIndex + 0]);
	XMVECTOR posTR = XMLoadFloat4(&buffer[vertexIndex + 1]);
	XMVECTOR posBL = XMLoadFloat4(&buffer[vertexIndex + 2]);
	XMVECTOR posBR = XMLoadFloat4(&buffer[vertexIndex + 4]);

	posTL = XMVector2Transform(XMVector2Transform(posTL, TRS), ortho);
	posTR = XMVector2Transform(XMVector2Transform(posTR, TRS), ortho);
	posBL = XMVector2Transform(XMVector2Transform(posBL, TRS), ortho);
	posBR = XMVector2Transform(XMVector2Transform(posBR, TRS), ortho);

	XMStoreFloat4(&buffer[vertexIndex + 0], posTL);
	XMStoreFloat4(&buffer[vertexIndex + 1], posTR);
	XMStoreFloat4(&buffer[vertexIndex + 2], posBL);
	XMStoreFloat4(&buffer[vertexIndex + 3], posTR);
	XMStoreFloat4(&buffer[vertexIndex + 4], posBR);
	XMStoreFloat4(&buffer[vertexIndex + 5], posBL);

	//Have to set rotation value, as the Vector2 transforms get rid of Z and W
	buffer[vertexIndex + 0].z = rotation;
	buffer[vertexIndex + 1].z = rotation;
	buffer[vertexIndex + 2].z = rotation;
	buffer[vertexIndex + 3].z = rotation;
	buffer[vertexIndex + 4].z = rotation;
	buffer[vertexIndex + 5].z = rotation;

	buffer[vertexIndex + 0].w = customData;
	buffer[vertexIndex + 1].w = customData;
	buffer[vertexIndex + 2].w = customData;
	buffer[vertexIndex + 3].w = customData;
	buffer[vertexIndex + 4].w = customData;
	buffer[vertexIndex + 5].w = customData;
}


void Game::Render(Renderer& renderer)
{
	XMMATRIX ortho = XMMatrixOrthographicLH(1280,720.0f,0.0f,10.0f);
	
	//Asteroids
	{
		for (size_t cat = 0; cat < AsteroidCategory::ENUM_MAX; cat++)
		{
			for (size_t type = 0; type < AsteroidType::ENUM_MAX; type++)
			{
				auto& activeBuffer = asteroidBufferData[type][cat];
				activeBuffer.clear();
			}
		}

		for (size_t i = 0; i < asteroids.size(); i++)
		{
			Asteroid& asteroid = asteroids[i];
			auto& activeBuffer = asteroidBufferData[asteroid.category][asteroid.type];
			CreateQuad(asteroid.pos, asteroid.rot, asteroid.size, static_cast<float>(asteroid.variation), ortho, activeBuffer);
		}

		for (size_t i = 0; i < asteroidVertices.size(); i++)
		{
			for (size_t j = 0; j < asteroidVertices[i].size(); j++)
			{
				renderer.UpdateVertexBuffer(asteroidVertices[i][j], asteroidBufferData[i][j]);
			}
		}
	}

	//Explosions
	{
		for (size_t cat = 0; cat < AsteroidCategory::ENUM_MAX; cat++)
		{
			auto& activeBuffer = asteroidExplosionBufferData[cat];
			activeBuffer.clear();
		}

		for (size_t i = 0; i < asteroidExplosions.size(); i++)
		{
			auto& explode = asteroidExplosions[i];
			CreateQuad(explode.position, explode.rotation, explode.scale, static_cast<float>(explode.GetSpriteIndex()), ortho, asteroidExplosionBufferData[explode.category]);
		}

		for (size_t i = 0; i < asteroidExplosionVertices.size(); i++)
		{
			renderer.UpdateVertexBuffer(asteroidExplosionVertices[i], asteroidExplosionBufferData[i]);
		}
	}

	//Player
	{
		player.vertices.clear();
		CreateQuad(player.position, 0, player.scale, static_cast<float>(player.GetSpriteIndex()), ortho, player.vertices);
		renderer.UpdateVertexBuffer(player.vertexBuffer, player.vertices);
	}

	renderer.Clear(0.025f,0.025f,0.025f,1.0f);

	auto asteroidMat = renderer.GetAsteroidMaterial();
	for (size_t i = 0; i < asteroidVertices.size(); i++)
	{
		for (size_t j = 0; j < asteroidVertices[i].size(); j++)
		{
			std::array<ID3D11ShaderResourceView*, 3> srvs =
			{
				asteroidTexturesDiffuse[j][i].Get(),
				asteroidTexturesNormal[j][i].Get(),
				asteroidTexturesRoughness[j][i].Get(),
			};
			std::array<ID3D11Buffer*,0> cbs;

			renderer.Draw(asteroidVertices[j][i], asteroidMat, srvs, cbs);
		}
	}

	auto spriteMaterial = renderer.GetSpriteMaterial();
	for (size_t i = 0; i < asteroidExplosionVertices.size(); i++)
	{
		std::array<ID3D11ShaderResourceView*, 1> srvs =
		{
			asteroidExplosionsTextures[i].Get(),
		};
		std::array<ID3D11Buffer*, 1> cbs =
		{
			asteroidExplosionConstantData[i].Get(),
		};
		renderer.Draw(asteroidExplosionVertices[i], spriteMaterial, srvs, cbs);
	}
	
	{
		std::array<ID3D11ShaderResourceView*, 1> srvs =
		{
			player.GetTexture(),
		};
		std::array<ID3D11Buffer*, 1> cbs =
		{
			player.constantBuffer.Get(),
		};
		renderer.Draw(player.vertexBuffer, spriteMaterial, srvs, cbs);
	}
	
}
