#pragma once
#include "Scene.h"

class Game : Scene
{
public:
	virtual void Init() final override;
	virtual Scene::Status Update(double dt) final override;
	virtual void Render(Renderer& renderer) final override;
};