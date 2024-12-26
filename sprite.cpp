#include "sprite.h"

void Sprite::Init(ComPtr<ID3D11ShaderResourceView> spriteSheet, int spriteNumX, int spriteNumY, float fps)
{
	texture = spriteSheet;
	
	ComPtr<ID3D11Texture2D> tex2d;
	ComPtr<ID3D11Resource> resource;
	texture->GetResource(resource.GetAddressOf());
	resource->QueryInterface(IID_PPV_ARGS(tex2d.GetAddressOf()));
	D3D11_TEXTURE2D_DESC desc;
	tex2d->GetDesc(&desc);
	imageWidth = desc.Width;
	imageHeight = desc.Height;
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
