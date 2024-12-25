#pragma once
#include "renderer.h"
#include "timer.h"
class Sprite
{
public:
	void Init(ComPtr<ID3D11ShaderResourceView> spriteSheet, int spriteNumX, int spriteNumY, float fps);
	void Update();
	bool HasFinishedAnimation();
	void SetSpriteRange(int start, int end);
	void SetSpriteIndex(int index);
	int GetSpriteIndex() { return spriteIndex; }
	ID3D11ShaderResourceView* GetTexture() { return texture.Get(); }
	
	XMFLOAT2 position;
	float rotation;
	float scale;

private:
	int spriteRangeStart;
	int spriteRangeEnd;
	int spriteIndex;
	int spriteNumW;
	int spriteNumH;
	int imageWidth;
	int imageHeight;
	bool didFinishAnim;
	Timer animationTimer;
	ComPtr<ID3D11ShaderResourceView> texture;
};