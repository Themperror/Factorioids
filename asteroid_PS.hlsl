struct VS_OUT
{
	float4 pos : SV_Position;
	float3 uv : UV0;
};

Texture2DArray diffuseTex : register(t0);
Texture2DArray normalTex : register(t1);
Texture2DArray roughnessTex : register(t2);

sampler samplerState : register(s0);

float4 main(VS_OUT vsOut) : SV_TARGET
{
	uint elements = 0;
	uint dummy0 = 0;
	uint dummy1 = 0;
	uint dummy2 = 0;
	uint dummy3 = 0;
	diffuseTex.GetDimensions(dummy0, dummy1, dummy2, elements, dummy3);

	float3 samplePos = float3(vsOut.uv.xy, uint(vsOut.pos.z * 1000) % elements);
	float4 color = diffuseTex.Sample(samplerState, samplePos);
	if (color.a < 0.1)
		discard;
	
	float3 normal = (normalTex.Sample(samplerState, samplePos).xyz);
	normal *= 2.0.xxx;
	normal -= 1.0.xxx;
	
	float3x3 rotMat;
	//X
	rotMat._11_12_13 = float3(1, 0, 0);
	rotMat._21_22_23 = float3(0, cos(vsOut.uv.z), -sin(vsOut.uv.z));
	rotMat._31_32_33 = float3(0, sin(vsOut.uv.z), cos(vsOut.uv.z));
	//Y
	//rotMat._11_12_13 = float3(cos(vsOut.uv.z), 0, sin(vsOut.uv.z));
	//rotMat._21_22_23 = float3(0, 1, 0);
	//rotMat._31_32_33 = float3(-sin(vsOut.uv.z),0, cos(vsOut.uv.z));
	//Z
	//rotMat._11_12_13 = float3(cos(vsOut.uv.z), -sin(vsOut.uv.z),0);
	//rotMat._21_22_23 = float3(sin(vsOut.uv.z), cos(vsOut.uv.z),0);
	//rotMat._31_32_33 = float3(0, 0, 1);
	
	normal.xyz = mul(rotMat, normal.xyz);
	
	float4 roughness = roughnessTex.Sample(samplerState, samplePos);
	
	float3 lightDir = normalize(float3(0.33,0.8,0.8));
	float amount = dot(normalize(normal.xyz), lightDir);
	amount += roughness.x * 2.0;
	//return amount.xxxx;
	return float4(color.rgb * amount, color.a);
}