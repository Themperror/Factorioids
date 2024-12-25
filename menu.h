#pragma once

#include "Scene.h"

class Menu : Scene
{
public:
	virtual void Init(Renderer& renderer) final override;
	virtual Scene::Status Update(double dt, Input& input) final override;
	virtual void Render(Renderer& renderer) final override;
};