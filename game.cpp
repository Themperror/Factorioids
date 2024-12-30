#include "game.h"
#include "renderer.h"
#include "asteroid.h"
#include "fileutils.h"
#include "input.h"

#include <algorithm>

XMMATRIX ortho = XMMatrixOrthographicLH(1280, 720.0f, 0.0f, 10.0f);

void Game::Init(Renderer& renderer)
{
	std::array<const char*, AsteroidCategory::ENUM_MAX> categoryStrings = {"small", "medium", "big", "huge"};
	std::array<const char*, AsteroidType::ENUM_MAX> typeStrings = {"carbonic", "metallic", "oxide", "promethium"};


	const char* factorioAsteroidPath = "C:/Program Files (x86)/Steam/steamapps/common/Factorio/data/space-age/graphics/entity/asteroid/";
	const char* factorioAsteroidExplosionPath = "C:/Program Files (x86)/Steam/steamapps/common/Factorio/data/space-age/graphics/entity/asteroid-explosions/";
	const char* factorioMechPath = "C:/Program Files (x86)/Steam/steamapps/common/Factorio/data/space-age/graphics/entity/mech-armor/mech-idle-air.png";
	const char* factorioRocketPath = "C:/Program Files (x86)/Steam/steamapps/common/Factorio/data/base/graphics/entity/rocket/";

	int playerTextureHandle= renderer.MakeTextureFrom(factorioMechPath, true);

	std::array<std::array<int, AsteroidType::ENUM_MAX>, AsteroidCategory::ENUM_MAX> asteroidTexturesDiffuseHandles;
	std::array<std::array<int, AsteroidType::ENUM_MAX>, AsteroidCategory::ENUM_MAX> asteroidTexturesNormalHandles;
	std::array<std::array<int, AsteroidType::ENUM_MAX>, AsteroidCategory::ENUM_MAX> asteroidTexturesRoughnessHandles;
	std::array<int, AsteroidCategory::ENUM_MAX> asteroidExplosionsTextureHandles;
	char buf[512];

	//cache the rocket texture
	sprintf_s(buf, sizeof(buf), "%s%s", factorioRocketPath, "rocket.png");
	int rocketColHandle = renderer.MakeTextureFrom(buf,true);
	sprintf_s(buf, sizeof(buf), "%s%s", factorioRocketPath, "rocket-lights.png");
	int rocketLightHandle = renderer.MakeTextureFrom(buf,true);

	//cache the asteroid explosion textures
	sprintf_s(buf, sizeof(buf), "%s%s", factorioAsteroidExplosionPath, "asteroid-explosion-small.png");
	asteroidExplosionsTextureHandles[AsteroidCategory::Small] = renderer.MakeTextureFrom(buf, true);
	asteroidExplosionConstantData[AsteroidCategory::Small] = renderer.CreateSpriteConstantBuffer(6,6, 0.2f, ortho);
	sprintf_s(buf, sizeof(buf), "%s%s", factorioAsteroidExplosionPath, "asteroid-explosion-medium.png");
	asteroidExplosionsTextureHandles[AsteroidCategory::Medium] = renderer.MakeTextureFrom(buf, true);
	asteroidExplosionConstantData[AsteroidCategory::Medium] = asteroidExplosionConstantData[AsteroidCategory::Small];
	sprintf_s(buf, sizeof(buf), "%s%s", factorioAsteroidExplosionPath, "asteroid-explosion-big.png");
	asteroidExplosionsTextureHandles[AsteroidCategory::Big] = renderer.MakeTextureFrom(buf, true);
	asteroidExplosionConstantData[AsteroidCategory::Big] = asteroidExplosionConstantData[AsteroidCategory::Small];
	sprintf_s(buf, sizeof(buf), "%s%s", factorioAsteroidExplosionPath, "asteroid-explosion-huge.png");
	asteroidExplosionsTextureHandles[AsteroidCategory::Huge] = renderer.MakeTextureFrom(buf, true);
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

			asteroidTexturesDiffuseHandles[j][i] = renderer.MakeTextureArrayFrom(diffuseNames, true);
			asteroidTexturesNormalHandles[j][i] = renderer.MakeTextureArrayFrom(normalNames, false);
			asteroidTexturesRoughnessHandles[j][i] = renderer.MakeTextureArrayFrom(roughnessNames, false);

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

	player.vertexBuffer = renderer.CreateVertexBuffer(6, 6, sizeof(XMFLOAT4));
	player.constantBuffer = renderer.CreateSpriteConstantBuffer(5, 8, 0.1f,  ortho);
	player.queryConstantBuffer = renderer.CreateSpriteConstantBuffer(5, 8, 0.8f, ortho);
	
	for (int i = 0; i < player.occlusionQuery.size(); i++)
	{
		player.occlusionQuery[i] = renderer.CreateOcclusionQuery();
	}

	rocketConstantBuffer = renderer.CreateSpriteConstantBuffer(8, 1, 0.2f, ortho);
	rocketQueryConstantBuffer = renderer.CreateSpriteConstantBuffer(8, 1, 0.8f, ortho);

	asteroidConstantBuffer = renderer.CreateSpriteConstantBuffer(0, 0, 0.5f, ortho);

	renderer.FlushLoading();

	for (size_t i = 0; i < AsteroidType::ENUM_MAX; i++)
	{
		for (size_t j = 0; j < AsteroidCategory::ENUM_MAX; j++)
		{
			asteroidTexturesDiffuse[j][i] = renderer.GetTexture(asteroidTexturesDiffuseHandles[j][i]);
			asteroidTexturesNormal[j][i] = renderer.GetTexture(asteroidTexturesNormalHandles[j][i]);
			asteroidTexturesRoughness[j][i] = renderer.GetTexture(asteroidTexturesRoughnessHandles[j][i]);
		}
	}
	for (size_t i = 0; i < asteroidExplosionsTextures.size(); i++)
	{
		asteroidExplosionsTextures[i] = renderer.GetTexture(asteroidExplosionsTextureHandles[i]);
	}

	rocketTexture = renderer.GetTexture(rocketColHandle);
	rocketLightTexture = renderer.GetTexture(rocketLightHandle);
	rocketTexInfo = GetTextureInfo(rocketLightTexture);

	auto playerTexInfo = GetTextureInfo(renderer.GetTexture(playerTextureHandle));
	player.Init(renderer.GetTexture(playerTextureHandle), playerTexInfo, 5, 8, 60.0f);
	player.SetSpriteRange(0, 5);
	player.scale = 20;
	player.invulnerabilityTimer.Start(3.0);

	spawnTimer.Start(0.3);
	rocketCooldownTimer.Start(0.25);
}


XMFLOAT2 ToUnifiedSpace(float posX, float posY)
{
	return XMFLOAT2((posX + 640.0f) / 1280.0f, (posY + 360.0f) / 720.0f);
}

XMFLOAT2 ToUnifiedSpace(XMFLOAT2 pos)
{
	return ToUnifiedSpace(pos.x,pos.y);
}

Scene::Status Game::Update(double dt, Input& input, Renderer& renderer)
{
	//for (int i = 0; i < asteroidGrid.size(); i++)
	//{
	//	for (int j = 0; j < asteroidGrid[i].size(); j++)
	//	{
	//		asteroidGrid[i][j].clear();
	//	}
	//}

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
	//for (size_t i = 0; i < asteroids.size(); i++)
	//{
	//	Asteroid& asteroid = asteroids[i];
	//	int minX,maxX,minY,maxY;
	//	XMFLOAT2 minPos = ToUnifiedSpace(asteroid.pos.x - asteroid.size, asteroid.pos.y - asteroid.size);
	//	XMFLOAT2 maxPos = ToUnifiedSpace(asteroid.pos.x + asteroid.size, asteroid.pos.y + asteroid.size);
	//
	//	minX = (int)floor(minPos.x * static_cast<float>(asteroidGrid.size()));
	//	maxX = (int)ceil(maxPos.x * static_cast<float>(asteroidGrid.size()));
	//	minY = (int)floor(minPos.y * static_cast<float>(asteroidGrid[0].size()));
	//	maxY = (int)ceil(maxPos.y * static_cast<float>(asteroidGrid[0].size()));
	//
	//	minX = std::clamp(minX,0, static_cast<int>(asteroidGrid.size()));
	//	maxX = std::clamp(maxX,0, static_cast<int>(asteroidGrid.size()));
	//	minY = std::clamp(minY,0, static_cast<int>(asteroidGrid[0].size()));
	//	maxY = std::clamp(maxY,0, static_cast<int>(asteroidGrid[0].size()));
	//
	//	for (int y = minY; y < maxY; y++)
	//	{
	//		for (int x = minX; x < maxX; x++)
	//		{
	//			asteroidGrid[x][y].push_back(static_cast<int>(i));
	//		}
	//	}
	//}

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
	if (input.GetKeyState('W') == KeyState::Down)
	{
		player.position.y += static_cast<float>(100.0 * dt);
	}
	if (input.GetKeyState('A') == KeyState::Down)
	{
		player.position.x -= static_cast<float>(100.0 * dt);
	}
	if (input.GetKeyState('S') == KeyState::Down)
	{
		player.position.y -= static_cast<float>(100.0 * dt);
	}
	if (input.GetKeyState('D') == KeyState::Down)
	{
		player.position.x += static_cast<float>(100.0 * dt);
	}

	if (player.occlusionResults[currentQueryIndex] && player.invulnerabilityTimer.HasFinished())
	{
		player.invulnerabilityTimer.Restart();
		player.lives--;
		player.lostLifeTimer.Start(0.33);
		player.lostLifeCounter = 0;
		player.shouldDraw = false;


		AsteroidExplosion& explosion = asteroidExplosions.emplace_back();
		explosion.Init(asteroidExplosionsTextures[AsteroidCategory::Huge], asteroidExplosionTexInfos[AsteroidCategory::Huge], 5, 5, 60.0f);
		explosion.position = player.position;
		explosion.rotation = 0;
		explosion.scale = player.scale * 2.0f;
		explosion.category = AsteroidCategory::Huge;

		if (player.lives < 0)
		{
			player.position.x = 0;
			player.position.y = 0;
		}

	}
	if (player.lostLifeTimer.HasFinished())
	{
		player.lostLifeCounter++;
		player.shouldDraw = !player.shouldDraw;
		player.lostLifeTimer.RestartWithRemainder();
	}
	if (player.invulnerabilityTimer.HasFinished())
	{
		player.lostLifeTimer.Stop();
		player.shouldDraw = true;
	}



	//XMFLOAT2 playerPosUnified = ToUnifiedSpace(player.position);
	//playerPosUnified.x = std::clamp(playerPosUnified.x * static_cast<float>(asteroidGrid.size()),0.0f,static_cast<float>(asteroidGrid.size()));
	//playerPosUnified.y = std::clamp(playerPosUnified.y * static_cast<float>(asteroidGrid[0].size()), 0.0f, static_cast<float>(asteroidGrid[0].size()));
	//Util::Print("Player GridPos: %f  %f", playerPosUnified.x, playerPosUnified.y);
	//auto& asteroidsInCell = asteroidGrid[static_cast<int>(playerPosUnified.x)][static_cast<int>(playerPosUnified.y)];
	//if (asteroidsInCell.size())
	//{
	//	BreakAsteroid(asteroidsInCell[0]);
	//}

	//Util::Print("Query Results: [%08llu]  [%08llu]  [%08llu]", playerQueryResults[0], playerQueryResults[1], playerQueryResults[2]);

	//Player rotation towards mouse
	{
		XMFLOAT2 mousePos = input.GetMousePos();
		mousePos.x *= 2.0;
		mousePos.x -= 1.0;
		mousePos.y *= 2.0;
		mousePos.y -= 1.0;
		mousePos.y *= -1.0;
		mousePos.x *= 640.0f;
		mousePos.y *= 360.0f;

		XMFLOAT2 up = XMFLOAT2(0,1);
		XMFLOAT2 right = XMFLOAT2(1,0);

		XMVECTOR mouseVec = XMLoadFloat2(&mousePos);
		XMVECTOR upVec = XMLoadFloat2(&up);
		XMVECTOR rightVec = XMLoadFloat2(&right);
		XMVECTOR playerVec = XMLoadFloat2(&player.position);
			
		XMVECTOR dirToCursor = XMVectorSubtract(mouseVec, playerVec);
		dirToCursor = XMVector2Normalize(dirToCursor);

		XMVECTOR dotVec = XMVector2Dot(dirToCursor, rightVec);
		float dot;
		XMStoreFloat(&dot, dotVec);
		int direction = dot < 0.0f ? -1 : 1;

		dotVec = XMVector2Dot(dirToCursor, upVec);
		XMStoreFloat(&dot, dotVec);
		float angle = acos(std::min(std::max(dot, -1.0f), 1.0f)) + XM_PI / 8.0f;
		int sector = (int)std::min(std::max(4.0f * angle / XM_PI, 0.0f), 4.0f);
		if (direction < 0 && sector > 0 && sector < 4)
		{
			sector = 8-sector;
		}
		player.SetSpriteRange(sector * 5, (sector * 5)+5);


		if (input.GetKeyState(VK_SPACE) == KeyState::Down)
		{
			if (rocketCooldownTimer.HasFinished())
			{
				Rocket& rocket = rockets.emplace_back();
				rocket.Init(rocketTexture, rocketTexInfo, 8,1,60.0f);
				rocket.vertexBuffer = renderer.CreateVertexBuffer(6,6,sizeof(XMFLOAT4));
				rocket.position = player.position;
				for (size_t i = 0; i < rocket.occlusionQuery.size(); i++)
				{
					rocket.occlusionQuery[i] = renderer.CreateOcclusionQuery();
				}
			
				float rotation = (direction <= 0) ? (1.0f - dot) / 4.0f : (dot + 3.0f) / 4.0f;

				rotation *= 2.0 * XM_PI;
				rocket.rotation = rotation;
				rocket.scale = 2.5f;

				XMFLOAT2 speed = XMFLOAT2(60,60);
				XMVECTOR speedVec =	XMLoadFloat2(&speed);
				XMVECTOR accelVec = XMVectorMultiply(dirToCursor, speedVec);
				XMStoreFloat2(&rocket.acceleration, accelVec);

				rocketCooldownTimer.RestartWithRemainder();
			}
		}
	}


	for (size_t i = 0; i < rockets.size(); i++)
	{
		Rocket& rocket = rockets[i];
		rocket.Update();

		rocket.velocity.x += rocket.acceleration.x * dt;
		rocket.velocity.y += rocket.acceleration.y * dt;

		rocket.position.x += rocket.velocity.x * dt;
		rocket.position.y += rocket.velocity.y * dt;

		for (size_t j = 0; j < rocket.occlusionResults.size(); j++)
		{
			if (rocket.occlusionResults[j])
			{
				int closestAsteroidIndex = -1;
				float dist = FLT_MAX;
				XMVECTOR rocketPos = XMLoadFloat2(&rocket.position);
				for (size_t x = 0; x < asteroids.size(); x++)
				{
					XMVECTOR asteroidPos = XMLoadFloat2(&asteroids[x].pos);
					float currentDist; 
					XMStoreFloat(&currentDist, XMVector2LengthSq(XMVectorSubtract(rocketPos, asteroidPos)));
					if (currentDist < dist)
					{
						dist = currentDist;
						closestAsteroidIndex = x;
					}
				}

				BreakAsteroid(closestAsteroidIndex);

				SwapAndPop(rockets,i);
				i--;
				break;
			}
		}

		if (rocket.position.x < -1000 || rocket.position.x > 1000 || rocket.position.y < -500 || rocket.position.y > 500)
		{

			SwapAndPop(rockets, i);
			i--;
		}
	}

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
	explosion.Init(asteroidExplosionsTextures[category], asteroidExplosionTexInfos[category], 5,5, 60.0f);
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


void CreateQuad(XMFLOAT2 position, float rotation, float sizeX, float sizeY, float customData, std::vector<XMFLOAT4>& buffer)
{
	size_t vertexIndex = buffer.size();

	XMVECTOR pos = XMLoadFloat2(&position);
	XMMATRIX TRS = XMMatrixTransformation2D(XMVectorZero(), 0, XMVectorSplatOne(), XMVectorZero(), rotation, pos);
	buffer.push_back(XMFLOAT4(-(sizeX), +(sizeY), 0, 0));
	buffer.push_back(XMFLOAT4(+(sizeX), +(sizeY), 0, 0));
	buffer.push_back(XMFLOAT4(-(sizeX), -(sizeY), 0, 0));
	
	buffer.push_back(XMFLOAT4(+(sizeX), +(sizeY), 0, 0));
	buffer.push_back(XMFLOAT4(+(sizeX), -(sizeY), 0, 0));
	buffer.push_back(XMFLOAT4(-(sizeX), -(sizeY), 0, 0));

	XMVECTOR posTL = XMLoadFloat4(&buffer[vertexIndex + 0]);
	XMVECTOR posTR = XMLoadFloat4(&buffer[vertexIndex + 1]);
	XMVECTOR posBL = XMLoadFloat4(&buffer[vertexIndex + 2]);
	XMVECTOR posBR = XMLoadFloat4(&buffer[vertexIndex + 4]);

	posTL = XMVector2Transform(posTL, TRS);
	posTR = XMVector2Transform(posTR, TRS);
	posBL = XMVector2Transform(posBL, TRS);
	posBR = XMVector2Transform(posBR, TRS);

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

	renderer.SetDepthLesser();
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
			CreateQuad(asteroid.pos, asteroid.rot, asteroid.size, asteroid.size, static_cast<float>(asteroid.variation), activeBuffer);
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
			CreateQuad(explode.position, explode.rotation, explode.scale, explode.scale, static_cast<float>(explode.GetSpriteIndex()), asteroidExplosionBufferData[explode.category]);
		}

		for (size_t i = 0; i < asteroidExplosionVertices.size(); i++)
		{
			renderer.UpdateVertexBuffer(asteroidExplosionVertices[i], asteroidExplosionBufferData[i]);
		}
	}

	for (size_t i = 0; i < rockets.size(); i++)
	{
		Rocket& rocket = rockets[i];
		rocket.vertices.clear();
		CreateQuad(rocket.position, rocket.rotation, rocket.scale, rocket.scale * 5.0f, static_cast<float>(rocket.GetSpriteIndex()), rocket.vertices);
		renderer.UpdateVertexBuffer(rocket.vertexBuffer, rocket.vertices);
	}

	//Player
	{
		player.vertices.clear();
		CreateQuad(player.position, 0, player.scale, player.scale, static_cast<float>(player.GetSpriteIndex()), player.vertices);
		renderer.UpdateVertexBuffer(player.vertexBuffer, player.vertices);
	}

	renderer.Clear(0.025f,0.025f,0.025f,1.0f);
	//renderer.Clear(1,1,1,1.0f);

	auto asteroidMat = renderer.GetAsteroidMaterial();
	for (size_t i = 0; i < asteroidVertices.size(); i++)
	{
		for (size_t j = 0; j < asteroidVertices[i].size(); j++)
		{
			std::array<ID3D11ShaderResourceView*, 3> srvs =
			{
				asteroidTexturesDiffuse[j][i],
				asteroidTexturesNormal[j][i],
				asteroidTexturesRoughness[j][i],
			};
			//only need the ortho matrix, so player's CBV will do
			std::array<ID3D11Buffer*, 1> cbs =
			{
				asteroidConstantBuffer.Get(),
			};

			renderer.Draw(asteroidVertices[j][i], asteroidMat, srvs, cbs);
		}
	}

	auto spriteMaterial = renderer.GetSpriteMaterial();
	auto rocketMaterial = renderer.GetRocketMaterial();
	for (size_t i = 0; i < asteroidExplosionVertices.size(); i++)
	{
		std::array<ID3D11ShaderResourceView*, 1> srvs =
		{
			asteroidExplosionsTextures[i],
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
		std::array<ID3D11ShaderResourceView*, 2> srvsRocket =
		{
			rocketTexture,
			rocketLightTexture,
		};
		std::array<ID3D11Buffer*, 1> cbsQuery =
		{
			player.queryConstantBuffer.Get(),
		};
		std::array<ID3D11Buffer*, 1> cbsPlayer =
		{
			player.constantBuffer.Get(),
		};
		std::array<ID3D11Buffer*, 1> cbsRocket =
		{
			rocketConstantBuffer.Get(),
		};


		//Inverse the depth check so we only get pass results from query if we're overlapping an asteroid
		renderer.SetDepthGreater();

		renderer.BeginQuery(player.occlusionQuery[currentQueryIndex].Get());
		renderer.Draw(player.vertexBuffer, spriteMaterial, srvs, cbsQuery);
		renderer.EndQuery(player.occlusionQuery[currentQueryIndex].Get());

		for (size_t i = 0; i < rockets.size(); i++)
		{
			renderer.BeginQuery(rockets[i].occlusionQuery[currentQueryIndex].Get());
			renderer.Draw(rockets[i].vertexBuffer, rocketMaterial, srvs, cbsQuery);
			renderer.EndQuery(rockets[i].occlusionQuery[currentQueryIndex].Get());
		}

		//back to "normal" depth
		renderer.SetDepthLesser();
		if (player.shouldDraw)
		{
			renderer.Draw(player.vertexBuffer, spriteMaterial, srvs, cbsPlayer);
		}

		for (size_t i = 0; i < rockets.size(); i++)
		{
			renderer.Draw(rockets[i].vertexBuffer, rocketMaterial, srvsRocket, cbsRocket);
		}
	}
	currentQueryIndex = (currentQueryIndex+1) % player.occlusionQuery.size();

	bool res = renderer.GetQueryResult(player.occlusionQuery[currentQueryIndex].Get(), player.occlusionResults[currentQueryIndex]);
	if (!res)
	{
		player.occlusionResults[currentQueryIndex] = 0;
	}
	for (size_t i = 0; i < rockets.size(); i++)
	{
		res = renderer.GetQueryResult(rockets[i].occlusionQuery[currentQueryIndex].Get(), rockets[i].occlusionResults[currentQueryIndex]);
	}

}
