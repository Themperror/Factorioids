Texture2D spriteTex : register(t0);
SamplerState samplerState : register(s0);

struct VS_OUT
{
	float4 pos : SV_Position;
	float2 uv : UV0;
};


float4 main(VS_OUT psIn) : SV_TARGET
{
	float4 spriteCol = spriteTex.Sample(samplerState, psIn.uv);
	if (spriteCol.a < 0.2)
	{
		discard;
	}
	return spriteCol;
}