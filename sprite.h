#pragma once
#include "renderer.h"
#include "timer.h"

struct TextureInfo
{
	int width, height;
};

TextureInfo GetTextureInfo(ID3D11ShaderResourceView* texture);

class Sprite
{
public:
	void Init(ID3D11ShaderResourceView* spriteSheet, const TextureInfo& spriteTextureInfo, int spriteNumX, int spriteNumY, float fps);
	void Update();
	bool HasFinishedAnimation();
	void SetSpriteRange(int start, int end);
	void SetSpriteIndex(int index);
	int GetSpriteIndex() { return spriteIndex; }
	ID3D11ShaderResourceView* GetTexture() { return texture; }
	
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
	ID3D11ShaderResourceView* texture;
};