#include "game.h"
#include "renderer.h"
#include "asteroid.h"
#include "fileutils.h"

void Game::Init(Renderer& renderer)
{
	asteroids.resize(20);
	for (size_t i = 0; i < 20; i++)
	{
		asteroids[i].pos = XMFLOAT2(rand() % 2000 - 1000, rand() % 1000 - 500);
		asteroids[i].size = (rand() % 50) + 70.0f;
		asteroids[i].rot = rand() % 360;
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
			for (size_t k = 0; k < files.size(); k++)
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

	for (size_t i = 0; i < AsteroidType::ENUM_MAX; i++)
	{
	}
}

Scene::Status Game::Update(double dt)
{
	for (size_t i = 0; i < asteroids.size(); i++)
	{
		asteroids[i].rot += dt;
	}
	return Status::Running;
}

void Game::Render(Renderer& renderer)
{
	XMMATRIX ortho = XMMatrixOrthographicLH(1280,720.0f,0.0f,10.0f);
	
	if (!asteroidVertices.buffer)
	{
		//keep some small minimum size
		asteroidVertices = renderer.CreateVertexBuffer( asteroids.size()*6, std::max(asteroids.size()*6, 4096llu), sizeof(XMFLOAT3));
	}

	{
		std::vector<XMFLOAT3> newBufferData;
		newBufferData.reserve(asteroids.size() * 6);
		for (size_t i = 0; i < asteroids.size(); i++)
		{
			Asteroid& asteroid = asteroids[i];

			XMVECTOR pos = XMLoadFloat2(&asteroid.pos);
			XMMATRIX TRS = XMMatrixTransformation2D(XMVectorZero(),0, XMVectorSplatOne(), XMVectorZero(), asteroid.rot, pos);
			newBufferData.push_back(XMFLOAT3(-(asteroid.size), +(asteroid.size), 0));
			newBufferData.push_back(XMFLOAT3(+(asteroid.size), +(asteroid.size), 0));
			newBufferData.push_back(XMFLOAT3(-(asteroid.size), -(asteroid.size), 0));
																				 
			newBufferData.push_back(XMFLOAT3(+(asteroid.size), +(asteroid.size), 0));
			newBufferData.push_back(XMFLOAT3(+(asteroid.size), -(asteroid.size), 0));
			newBufferData.push_back(XMFLOAT3(-(asteroid.size), -(asteroid.size), 0));

			XMVECTOR posTL = XMLoadFloat3(&newBufferData[i*6+0]);
			XMVECTOR posTR = XMLoadFloat3(&newBufferData[i*6+1]);
			XMVECTOR posBL = XMLoadFloat3(&newBufferData[i*6+2]);
			XMVECTOR posBR = XMLoadFloat3(&newBufferData[i*6+4]);

			posTL = XMVector2Transform(XMVector2Transform(posTL, TRS), ortho);
			posTR = XMVector2Transform(XMVector2Transform(posTR, TRS), ortho);
			posBL = XMVector2Transform(XMVector2Transform(posBL, TRS), ortho);
			posBR = XMVector2Transform(XMVector2Transform(posBR, TRS), ortho);

			XMStoreFloat3(&newBufferData[i*6+0], posTL);
			XMStoreFloat3(&newBufferData[i*6+1], posTR);
			XMStoreFloat3(&newBufferData[i*6+2], posBL);
			XMStoreFloat3(&newBufferData[i*6+3], posTR);
			XMStoreFloat3(&newBufferData[i*6+4], posBR);
			XMStoreFloat3(&newBufferData[i*6+5], posBL);

			//Have to set rotation value, as the Vector2 transforms get rid of Z
			newBufferData[i * 6 + 0].z = asteroid.rot;
			newBufferData[i * 6 + 1].z = asteroid.rot;
			newBufferData[i * 6 + 2].z = asteroid.rot;
			newBufferData[i * 6 + 3].z = asteroid.rot;
			newBufferData[i * 6 + 4].z = asteroid.rot;
			newBufferData[i * 6 + 5].z = asteroid.rot;
		}

		renderer.UpdateVertexBuffer(asteroidVertices, newBufferData);
	}
	
	renderer.Clear(0.025,0.025,0.025,1);

	std::vector<ID3D11ShaderResourceView*> srvs = 
	{
		asteroidTexturesDiffuse[3][0].Get(),
		asteroidTexturesNormal[3][0].Get(),
		asteroidTexturesRoughness[3][0].Get(),
	};

	renderer.Draw( asteroidVertices, renderer.GetAsteroidPixelShader(), renderer.GetAsteroidVertexShader(), srvs);

}
