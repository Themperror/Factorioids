#pragma once

class Renderer;
class Scene
{
	public:
	enum class Status { Running, Quitting, ToMenu, ToGame };

	virtual Status Update(double dt) = 0;
	virtual void Init() = 0;
	virtual void Render(Renderer& renderer) = 0;
};