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

	char buf[512];
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
	spawnTimer.Start(1.0);
}

Scene::Status Game::Update(double dt)
{
	if (spawnTimer.HasFinished())
	{
		spawnTimer.Restart();
		SpawnRandomAsteroid();
	}

	for (size_t i = 0; i < asteroids.size(); i++)
	{
		asteroids[i].rot += dt * asteroids[i].rotSpeed;
		asteroids[i].pos.x += dt * asteroids[i].velocity.x;
		asteroids[i].pos.y += dt * asteroids[i].velocity.y;
	}

	return Status::Running;
}

void Game::EraseAsteroid(int asteroidIndex)
{
	std::swap(asteroids[asteroidIndex], asteroids.back());
	asteroids.erase(asteroids.begin() + asteroids.size() - 1);
}

void Game::BreakAsteroid(int asteroidIndex)
{
	assert(asteroidIndex < asteroids.size());
	Asteroid& asteroid = asteroids[asteroidIndex];
	switch (asteroid.category)
	{
		case AsteroidCategory::Small:
		{
			EraseAsteroid(asteroidIndex);
		}
		break;
		case AsteroidCategory::Medium:
		{
			SpawnAsteroid(asteroid.type, (AsteroidCategory::Category)(asteroid.category - 1));
			SpawnAsteroid(asteroid.type, (AsteroidCategory::Category)(asteroid.category - 1));
			if (rand() & 1)
			{
				SpawnAsteroid(asteroid.type, (AsteroidCategory::Category)(asteroid.category - 1));
			}
		}
		break;
		case AsteroidCategory::Big:
		{
			SpawnAsteroid(asteroid.type, (AsteroidCategory::Category)(asteroid.category - 1));
			SpawnAsteroid(asteroid.type, (AsteroidCategory::Category)(asteroid.category - 1));
			if (rand() % 100  < 25)
			{
				SpawnAsteroid(asteroid.type, (AsteroidCategory::Category)(asteroid.category - 1));
			}
		}
		break;
		case AsteroidCategory::Huge:
		{
			SpawnAsteroid(asteroid.type, (AsteroidCategory::Category)(asteroid.category - 1));
			SpawnAsteroid(asteroid.type, (AsteroidCategory::Category)(asteroid.category - 1));
			if (rand() % 100 < 10)
			{
				SpawnAsteroid(asteroid.type, (AsteroidCategory::Category)(asteroid.category - 1));
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
	asteroid.rot = rand() % 360;
	asteroid.rotSpeed = ((rand() % 100) - 50) * 0.005;
	asteroid.variation = rand()% 65536;

	float angle = ((rand() % 360) * 0.0174533f);
	float angleDeviation = (rand() % 25);
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
void Game::SpawnAsteroid(AsteroidType::Type asteroidType, AsteroidCategory::Category asteroidCategory)
{
	Asteroid& asteroid = asteroids.emplace_back();
	asteroid.category = asteroidCategory;
	asteroid.type = asteroidType;
	asteroid.health = 100.0f;
	asteroid.rot = rand() % 360;
	asteroid.rotSpeed = ((rand() % 100) - 50) * 0.005;

	float angle = ((rand() % 360) * 0.0174533f);
	float angleDeviation = (rand() % 25);
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

void Game::Render(Renderer& renderer)
{
	XMMATRIX ortho = XMMatrixOrthographicLH(1280,720.0f,0.0f,10.0f);
	
	{
		std::array<std::array<std::vector<XMFLOAT4>, AsteroidType::ENUM_MAX>, AsteroidCategory::ENUM_MAX> newBufferData;
		for (size_t i = 0; i < asteroids.size(); i++)
		{
			Asteroid& asteroid = asteroids[i];
			auto& activeBuffer = newBufferData[asteroid.type][asteroid.category];
			size_t vertexIndex = activeBuffer.size();

			XMVECTOR pos = XMLoadFloat2(&asteroid.pos);
			XMMATRIX TRS = XMMatrixTransformation2D(XMVectorZero(),0, XMVectorSplatOne(), XMVectorZero(), asteroid.rot, pos);
			activeBuffer.push_back(XMFLOAT4(-(asteroid.size), +(asteroid.size), 0, 0));
			activeBuffer.push_back(XMFLOAT4(+(asteroid.size), +(asteroid.size), 0, 0));
			activeBuffer.push_back(XMFLOAT4(-(asteroid.size), -(asteroid.size), 0, 0));

			activeBuffer.push_back(XMFLOAT4(+(asteroid.size), +(asteroid.size), 0, 0));
			activeBuffer.push_back(XMFLOAT4(+(asteroid.size), -(asteroid.size), 0, 0));
			activeBuffer.push_back(XMFLOAT4(-(asteroid.size), -(asteroid.size), 0, 0));

			XMVECTOR posTL = XMLoadFloat4(&activeBuffer[vertexIndex+0]);
			XMVECTOR posTR = XMLoadFloat4(&activeBuffer[vertexIndex+1]);
			XMVECTOR posBL = XMLoadFloat4(&activeBuffer[vertexIndex+2]);
			XMVECTOR posBR = XMLoadFloat4(&activeBuffer[vertexIndex+4]);

			posTL = XMVector2Transform(XMVector2Transform(posTL, TRS), ortho);
			posTR = XMVector2Transform(XMVector2Transform(posTR, TRS), ortho);
			posBL = XMVector2Transform(XMVector2Transform(posBL, TRS), ortho);
			posBR = XMVector2Transform(XMVector2Transform(posBR, TRS), ortho);

			XMStoreFloat4(&activeBuffer[vertexIndex+0], posTL);
			XMStoreFloat4(&activeBuffer[vertexIndex+1], posTR);
			XMStoreFloat4(&activeBuffer[vertexIndex+2], posBL);
			XMStoreFloat4(&activeBuffer[vertexIndex+3], posTR);
			XMStoreFloat4(&activeBuffer[vertexIndex+4], posBR);
			XMStoreFloat4(&activeBuffer[vertexIndex+5], posBL);

			//Have to set rotation value, as the Vector2 transforms get rid of Z and W
			activeBuffer[vertexIndex + 0].z = asteroid.rot;
			activeBuffer[vertexIndex + 1].z = asteroid.rot;
			activeBuffer[vertexIndex + 2].z = asteroid.rot;
			activeBuffer[vertexIndex + 3].z = asteroid.rot;
			activeBuffer[vertexIndex + 4].z = asteroid.rot;
			activeBuffer[vertexIndex + 5].z = asteroid.rot;

			activeBuffer[vertexIndex + 0].w = asteroid.variation;
			activeBuffer[vertexIndex + 1].w = asteroid.variation;
			activeBuffer[vertexIndex + 2].w = asteroid.variation;
			activeBuffer[vertexIndex + 3].w = asteroid.variation;
			activeBuffer[vertexIndex + 4].w = asteroid.variation;
			activeBuffer[vertexIndex + 5].w = asteroid.variation;
		}
		for (size_t i = 0; i < asteroidVertices.size(); i++)
		{
			for (size_t j = 0; j < asteroidVertices[i].size(); j++)
			{
				renderer.UpdateVertexBuffer(asteroidVertices[i][j], newBufferData[i][j]);
			}
		}
	}
	
	renderer.Clear(0.025,0.025,0.025,1);

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
			renderer.Draw(asteroidVertices[i][j], renderer.GetAsteroidPixelShader(), renderer.GetAsteroidVertexShader(), srvs);
		}
	}
	

}
