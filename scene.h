#pragma once

class Renderer;
class Input;
class Scene
{
	public:
	enum class Status { Running, Quitting, ToMenu, ToGame };

	virtual Status Update(double dt, Input& input) = 0;
	virtual void Init(Renderer& renderer) = 0;
	virtual void Render(Renderer& renderer) = 0;
};