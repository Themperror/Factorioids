struct VS_OUT
{
	float4 pos : SV_Position;
	float4 uv : UV0;
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

	float3 samplePos = float3(vsOut.uv.xy, uint(vsOut.uv.w) % elements);
	float4 color = diffuseTex.Sample(samplerState, samplePos);
	if (color.a < 0.1)
		discard;

	float normalStrength = 1.0;
	float3 normal = (normalTex.Sample(samplerState, samplePos));
	float2 normalxy = (normal.xy * 2.0f) - 1.0f.xx;
	normal = float3(normalxy.x, normalxy.y, normal.z);
	normal.z = sqrt(clamp(1.0f - dot(normal.xy, normal.xy), 0.0f, 1.0f));
	normal = normalize(float3(0.0f, 0.0f, 1.0f) + ((normal - float3(0.0f, 0.0f, 1.0f)) * normalStrength));

	float3x3 rotMat;
	//X
	//rotMat._11_12_13 = float3(1, 0, 0);
	//rotMat._21_22_23 = float3(0, cos(vsOut.uv.z), -sin(vsOut.uv.z));
	//rotMat._31_32_33 = float3(0, sin(vsOut.uv.z), cos(vsOut.uv.z));
	//Y
	//rotMat._11_12_13 = float3(cos(vsOut.uv.z), 0, sin(vsOut.uv.z));
	//rotMat._21_22_23 = float3(0, 1, 0);
	//rotMat._31_32_33 = float3(-sin(vsOut.uv.z),0, cos(vsOut.uv.z));
	//Z
	rotMat._11_12_13 = float3(cos(vsOut.uv.z), -sin(vsOut.uv.z), 0);
	rotMat._21_22_23 = float3(sin(vsOut.uv.z), cos(vsOut.uv.z), 0);
	rotMat._31_32_33 = float3(0, 0, 1);
	
	normal.xyz = mul(rotMat, normal.xyz);
	
	float4 SSS_Roughness = roughnessTex.Sample(samplerState, samplePos);
	float roughness = SSS_Roughness.w;
	float3 SSSColor = SSS_Roughness.xyz;
	float3 lightDir = normalize(float3(0.33, 0.8, 0.8));
		
	float3 illumination = 0.0f.xxx;
	float3 lights_diffuse = 0.0f.xxx;
	float3 lights_spec = 0.0f.xxx;
	float lightWidth = 0.1;
	float brightness = 2.2;
	float specPower = 0.9;
	float specPurity = 1.2;
	float specStrength = 1.2;
	float SSSContrast = 0.3;
	float SSSAmount = 0.1;
	float3 ambientLight = 0.5.xxx;
	{
		//float3 light_color = _74_lights[i].color.xyz;
		float3 light_color = 1.0.xxx; // float3(1.0, 0.76, 0.286); // orangey
		//float3 light_direction = normalize(_74_lights[i].direction.xyz);
		float3 light_direction = lightDir;
		float lighting = max(0.0f, dot(normal, -light_direction) + lightWidth);
		float3 light_diffuse = (light_color * lighting) * brightness;
		float3 reflect_dir = reflect(normal, -light_direction);
		float3 view_dir = float3(0.0f, 0.0f, -1.0f);
		float spec = pow(max(0.0f, dot(reflect_dir, view_dir) + lightWidth) * (1.0f - roughness), specPower);
		float3 specularColor = color.xyz + ((1.0f.xxx - color.xyz) * specPurity);
		float3 specLight = ((specularColor * spec) * specStrength) * light_color;
		illumination += (light_color * max(-1.0f, dot(normal, -light_direction) + lightWidth));
		lights_diffuse += light_diffuse;
		lights_spec += specLight;
	}
	float3 SSS = float3(clamp((2.0f.xxx - illumination) - SSSContrast.xxx, 0.0f.xxx, 1.0f.xxx) * SSSAmount.xxx);
	float4 finalColor = float4(((lights_spec + lights_diffuse) + ambientLight.xyz) * color.xyz, color.w) * 1.0f;
	
	float4 outColor = float4(finalColor.xyz + clamp(SSSColor * SSS, 0.0f.xxx, 1.0f.xxx), color.w) * clamp((color.w - 0.89999997615814208984375f) * 10.0f, 0.0f, 1.0f);
	//outColor *= vOpacity;
	//if ((_74_flags & 16) != 0)
	//{
	//	outColor = float4(lights_spec * color.w, 1.0f);
	//}
	return outColor;
}