struct VS_OUT
{
	float4 pos : SV_Position;
	float2 uv : UV0;
};

struct SpriteUniform
{
	int numX;
	int numY;
	int2 pad;
	matrix ortho;
} spriteData : register(c0);

VS_OUT main(float4 pos : POS, uint vertexID : SV_VertexID)
{
	VS_OUT vsOut;
	vsOut.pos.xy = mul(pos.xy, (float2x4)spriteData.ortho).xy;
	vsOut.pos.zw = float2(pos.z * 0.0001, 1.0);
	vertexID = vertexID % 6;
	
	float spriteWidth = 1.0 / (float) spriteData.numX;
	float spriteHeight = 1.0 / (float) (spriteData.numY);
	float spriteIndexY = (uint) pos.w / spriteData.numX;
	float spriteIndexX = ((uint) pos.w) % spriteData.numX;
	if (vertexID == 0)
	{
		vsOut.uv = float2(spriteIndexX * spriteWidth, spriteIndexY * spriteHeight);
	}
	else if (vertexID == 1 || vertexID == 3)
	{
		vsOut.uv = float2((spriteIndexX + 1.0) * spriteWidth, spriteIndexY * spriteHeight);
	}
	else if (vertexID == 2 || vertexID == 5)
	{
		vsOut.uv = float2(spriteIndexX * spriteWidth, (spriteIndexY + 1.0) * spriteHeight);
	}
	else
	{
		vsOut.uv = float2((spriteIndexX + 1.0) * spriteWidth, (spriteIndexY + 1.0) * spriteHeight);
	}

	return vsOut;
}