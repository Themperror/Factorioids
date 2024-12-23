struct VS_OUT
{
	float4 pos : SV_Position;
	float2 uv : UV0;
};

Texture2D diffuseTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D roughnessTex : register(t2);

sampler samplerState : register(s0);

float4 main(VS_OUT vsOut) : SV_TARGET
{
	float4 color = diffuseTex.Sample(samplerState, vsOut.uv);
	float4 normal = normalTex.Sample(samplerState, vsOut.uv);
	float4 roughness = roughnessTex.Sample(samplerState, vsOut.uv);
	
	float3 lightDir = normalize(float3(0,1,1));
	float amount = dot(normal.xyz, lightDir);
	
	return float4(color.rgb * amount, color.a);
}