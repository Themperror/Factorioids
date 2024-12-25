#include "game.h"
#include "renderer.h"
#include "asteroid.h"
#include "fileutils.h"

void Game::Init(Renderer& renderer)
{
	for (size_t i = 0; i < 40; i++)
	{
		SpawnRandomAsteroid();
	}

	std::vector<std::string> diffuseNames;
	std::vector<std::string> normalNames;
	std::vector<std::string> roughnessNames;

	std::array<const char*, AsteroidCategory::ENUM_MAX> categoryStrings = {"small", "medium", "big", "huge"};
	std::array<const char*, AsteroidType::ENUM_MAX> typeStrings = {"carbonic", "metallic", "oxide", "promethium"};


	const char* factorioAsteroidPath = "C:/Program Files (x86)/Steam/steamapps/common/Factorio/data/space-age/graphics/entity/asteroid/";
	const char* factorioAsteroidExplosionPath = "C:/Program Files (x86)/Steam/steamapps/common/Factorio/data/space-age/graphics/entity/asteroid-explosions/";

	char buf[512];

	//cache the asteroid explosion textures
	sprintf_s(buf, sizeof(buf), "%s%s", factorioAsteroidExplosionPath, "asteroid-explosion-small.png");
	asteroidExplosionsTextures[AsteroidCategory::Small] = renderer.MakeTextureFrom(buf, true);
	sprintf_s(buf, sizeof(buf), "%s%s", factorioAsteroidExplosionPath, "asteroid-explosion-medium.png");
	asteroidExplosionsTextures[AsteroidCategory::Medium] = renderer.MakeTextureFrom(buf, true);
	sprintf_s(buf, sizeof(buf), "%s%s", factorioAsteroidExplosionPath, "asteroid-explosion-big.png");
	asteroidExplosionsTextures[AsteroidCategory::Big] = renderer.MakeTextureFrom(buf, true);
	sprintf_s(buf, sizeof(buf), "%s%s", factorioAsteroidExplosionPath, "asteroid-explosion-huge.png");
	asteroidExplosionsTextures[AsteroidCategory::Huge] = renderer.MakeTextureFrom(buf, true);

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

	spawnTimer.Start(1.0);
}

template<typename T>
void SwapAndPop(T& container, size_t index)
{
	std::swap(container[index], container.back());
	container.pop_back();
}

Scene::Status Game::Update(double dt)
{
	if (spawnTimer.HasFinished())
	{
		spawnTimer.Restart();
		BreakAsteroid(rand() % asteroids.size());
	}

	for (size_t i = 0; i < asteroids.size(); i++)
	{
		asteroids[i].rot += dt * asteroids[i].rotSpeed;
		asteroids[i].pos.x += dt * asteroids[i].velocity.x;
		asteroids[i].pos.y += dt * asteroids[i].velocity.y;
	}

	for (size_t i = asteroidExplosions.size()-1; i < asteroidExplosions.size(); i--)
	{
		asteroidExplosions[i].Update();
		if (asteroidExplosions[i].HasFinishedAnimation())
		{
			SwapAndPop(asteroidExplosions, i);
		}
	}

	return Status::Running;
}

void Game::EraseAsteroid(int asteroidIndex)
{
	SwapAndPop(asteroids,asteroidIndex);
}

void Game::BreakAsteroid(int asteroidIndex)
{
	assert(asteroidIndex < asteroids.size());
	Asteroid& asteroid = asteroids[asteroidIndex];
	AsteroidCategory::Category category = asteroid.category;
	AsteroidType::Type type = asteroid.type;
	XMFLOAT2 position = asteroid.pos;

	AsteroidExplosion& explosion = asteroidExplosions.emplace_back();
	explosion.Init(asteroidExplosionsTextures[category], 5,5, 30.0f);
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
	asteroid.rot = XMConvertToRadians(rand() % 360);
	asteroid.rotSpeed = ((rand() % 100) - 50) * 0.005;
	asteroid.variation = rand()% 65536;

	float angle = XMConvertToRadians(rand() % 360);
	asteroid.pos.x = sin(angle) * 1000.0;
	asteroid.pos.y = cos(angle) * 500.0;

	float speed = rand() % 20 + 10;
	asteroid.velocity.x = -sin(angle) * speed * ((rand() % 50) / 50.0f);
	asteroid.velocity.y = -cos(angle) * speed * ((rand() % 50) / 50.0f);

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
	asteroid.rot = rand() % 360;
	asteroid.rotSpeed = ((rand() % 100) - 50) * 0.005;

	float angle = XMConvertToRadians(rand() % 360);
	float speed = rand() % 20 + 10;
	asteroid.velocity.x = -sin(angle) * speed * ((rand() % 50) / 50.0f);
	asteroid.velocity.y = -cos(angle) * speed * ((rand() % 50) / 50.0f);

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
		asteroid.pos.x = sin(angle) * 1000.0;
		asteroid.pos.y = cos(angle) * 500.0;
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
		for (size_t type = 0; type < AsteroidType::ENUM_MAX; type++)
		{
			for (size_t cat = 0; cat < AsteroidCategory::ENUM_MAX; cat++)
			{
				auto& activeBuffer = asteroidBufferData[type][cat];
				activeBuffer.clear();
			}
		}

		for (size_t i = 0; i < asteroids.size(); i++)
		{
			Asteroid& asteroid = asteroids[i];
			auto& activeBuffer = asteroidBufferData[asteroid.type][asteroid.category];
			CreateQuad(asteroid.pos, asteroid.rot, asteroid.size, asteroid.variation, ortho, activeBuffer);
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
			CreateQuad(explode.position, explode.rotation, explode.scale, 0, ortho, asteroidExplosionBufferData[explode.category]);
		}

		for (size_t i = 0; i < asteroidExplosionVertices.size(); i++)
		{
			renderer.UpdateVertexBuffer(asteroidExplosionVertices[i], asteroidExplosionBufferData[i]);
		}
	}
	
	renderer.Clear(0.025,0.025,0.025,1);

	auto asteroidMat = renderer.GetAsteroidMaterial();
	for (size_t i = 0; i < asteroidVertices.size(); i++)
	{
		for (size_t j = 0; j < asteroidVertices[i].size(); j++)
		{
			std::array<ID3D11ShaderResourceView*, 3> srvs =
			{
				asteroidTexturesDiffuse[i][j].Get(),
				asteroidTexturesNormal[i][j].Get(),
				asteroidTexturesRoughness[i][j].Get(),
			};
			renderer.Draw(asteroidVertices[i][j], asteroidMat, srvs);
		}
	}

	auto explosionMat = renderer.GetSpriteMaterial();
	for (size_t i = 0; i < asteroidExplosionVertices.size(); i++)
	{
		std::array<ID3D11ShaderResourceView*, 1> srvs =
		{
			asteroidExplosionsTextures[i].Get(),
		};
		renderer.Draw(asteroidExplosionVertices[i], explosionMat, srvs);
	}
	
}
