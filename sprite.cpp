#include "sprite.h"

void Sprite::Init(ID3D11ShaderResourceView* spriteSheet, const TextureInfo& spriteTextureInfo, int spriteNumX, int spriteNumY, float fps)
{
	texture = spriteSheet;

	imageWidth = spriteTextureInfo.width;
	imageHeight = spriteTextureInfo.height;

	spriteNumW = spriteNumX;
	spriteNumH = spriteNumY;
	spriteIndex = 0;
	didFinishAnim = false;
	spriteRangeStart = 0;
	spriteRangeEnd = spriteNumW * spriteNumH;
	animationTimer.Start(1.0f / fps);
}

void Sprite::Update()
{
	if (animationTimer.HasFinished())
	{
		spriteIndex++;
		if (spriteIndex >= spriteNumW * spriteNumH || spriteIndex >= spriteRangeEnd)
		{
			spriteIndex = spriteRangeStart;
			didFinishAnim = true;
		}
		animationTimer.RestartWithRemainder();
	}
}

bool Sprite::HasFinishedAnimation()
{
	return didFinishAnim;
}

void Sprite::SetSpriteRange(int start, int end)
{
	if(spriteRangeStart == start && spriteRangeEnd == end)
		return;

	spriteIndex = start;
	spriteRangeStart = start;
	spriteRangeEnd = end;
	didFinishAnim = false;
	animationTimer.Restart();
}

void Sprite::SetSpriteIndex(int index)
{
	didFinishAnim = false;
	spriteIndex = index;
	animationTimer.Restart();
}

TextureInfo GetTextureInfo(ID3D11ShaderResourceView* texture)
{
	ComPtr<ID3D11Texture2D> tex2d;
	ComPtr<ID3D11Resource> resource;
	texture->GetResource(resource.GetAddressOf());
	resource->QueryInterface(IID_PPV_ARGS(tex2d.GetAddressOf()));
	D3D11_TEXTURE2D_DESC desc;
	tex2d->GetDesc(&desc);
	
	TextureInfo info;
	info.width = desc.Width;
	info.height = desc.Height;

	return info;
}
