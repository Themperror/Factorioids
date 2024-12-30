Texture2D spriteTex : register(t0);
Texture2D lightTex : register(t1);
SamplerState samplerState : register(s0);

struct VS_OUT
{
	float4 pos : SV_Position;
	float4 uv : UV0;
};


float4 main(VS_OUT psIn) : SV_TARGET
{
	float4 spriteCol = spriteTex.Sample(samplerState, psIn.uv.xy);
	float4 lightCol = lightTex.Sample(samplerState, psIn.uv.zw);
	float4 color = spriteCol + lightCol;
	if (color.a < 0.2)
	{
		discard;
	}
	return color;
}