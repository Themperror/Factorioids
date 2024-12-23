#include "game.h"
#include "renderer.h"
#include "asteroid.h"

void Game::Init(Renderer& renderer)
{
	asteroids.resize(20);
	for (size_t i = 0; i < 20; i++)
	{
		asteroids[i].pos = XMFLOAT2(rand() % 2000 - 1000, rand() % 1000 - 500);
		asteroids[i].size = (rand() % 50) + 70.0f;
		asteroids[i].rot = rand() % 360;
	}

	for (size_t i = 1; i < 8; i++)
	{
		asteroidTexturesDiffuse.push_back( renderer.MakeTextureFrom("C:/Program Files (x86)/Steam/steamapps/common/Factorio/data/space-age/graphics/entity/asteroid/carbonic/huge/asteroid-carbonic-huge-colour-0" + std::to_string(i) + ".png", true));
		asteroidTexturesNormal.push_back( renderer.MakeTextureFrom("C:/Program Files (x86)/Steam/steamapps/common/Factorio/data/space-age/graphics/entity/asteroid/carbonic/huge/asteroid-carbonic-huge-normal-0" + std::to_string(i) + ".png", false) );
		asteroidTexturesRoughness.push_back( renderer.MakeTextureFrom("C:/Program Files (x86)/Steam/steamapps/common/Factorio/data/space-age/graphics/entity/asteroid/carbonic/huge/asteroid-carbonic-huge-roughness-0" + std::to_string(i) + ".png", false) );
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
		asteroidVertices = renderer.CreateVertexBuffer( asteroids.size()*6, std::max(asteroids.size()*6, 4096llu), sizeof(XMFLOAT2));
	}

	{
		std::vector<XMFLOAT2> newBufferData;
		newBufferData.reserve(asteroids.size() * 6);
		for (size_t i = 0; i < asteroids.size(); i++)
		{
			Asteroid& asteroid = asteroids[i];

			XMVECTOR pos = XMLoadFloat2(&asteroid.pos);
			XMMATRIX TRS = XMMatrixTransformation2D(XMVectorZero(),0, XMVectorSplatOne(), XMVectorZero(), asteroid.rot, pos);
			newBufferData.push_back(XMFLOAT2(-(asteroid.size), +(asteroid.size)));
			newBufferData.push_back(XMFLOAT2(+(asteroid.size), +(asteroid.size)));
			newBufferData.push_back(XMFLOAT2(-(asteroid.size), -(asteroid.size)));

			newBufferData.push_back(XMFLOAT2(+(asteroid.size), +(asteroid.size)));
			newBufferData.push_back(XMFLOAT2(+(asteroid.size), -(asteroid.size)));
			newBufferData.push_back(XMFLOAT2(-(asteroid.size), -(asteroid.size)));

			XMVECTOR posTL = XMLoadFloat2(&newBufferData[i*6+0]);
			XMVECTOR posTR = XMLoadFloat2(&newBufferData[i*6+1]);
			XMVECTOR posBL = XMLoadFloat2(&newBufferData[i*6+2]);
			XMVECTOR posBR = XMLoadFloat2(&newBufferData[i*6+4]);


			posTL = XMVector2Transform(XMVector2Transform(posTL, TRS), ortho);
			posTR = XMVector2Transform(XMVector2Transform(posTR, TRS), ortho);
			posBL = XMVector2Transform(XMVector2Transform(posBL, TRS), ortho);
			posBR = XMVector2Transform(XMVector2Transform(posBR, TRS), ortho);


			XMStoreFloat2(&newBufferData[i*6+0], posTL);
			XMStoreFloat2(&newBufferData[i*6+1], posTR);
			XMStoreFloat2(&newBufferData[i*6+2], posBL);
			XMStoreFloat2(&newBufferData[i*6+3], posTR);
			XMStoreFloat2(&newBufferData[i*6+4], posBR);
			XMStoreFloat2(&newBufferData[i*6+5], posBL);
		}

		renderer.UpdateVertexBuffer(asteroidVertices, newBufferData);
	}
	
	renderer.Clear(0,0,0,1);

	std::vector<ID3D11ShaderResourceView*> srvs = 
	{
		asteroidTexturesDiffuse[0].Get(),
		asteroidTexturesNormal[0].Get(),
		asteroidTexturesRoughness[0].Get(),
	};

	renderer.Draw( asteroidVertices, renderer.GetAsteroidPixelShader(), renderer.GetAsteroidVertexShader(), srvs);

}
