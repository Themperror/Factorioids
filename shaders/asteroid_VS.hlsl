struct VS_OUT
{
	float4 pos : SV_Position;
	float4 uv : UV0;
};

struct SpriteUniform
{
	int numX;
	int numY;
	float baseDepth;
	float pad;
	matrix ortho;
} spriteData : register(c0);

VS_OUT main( float4 pos : POS , uint vertexID : SV_VertexID)
{
	VS_OUT vsOut;
	vsOut.pos.xy = mul(pos.xy, (float2x4) spriteData.ortho).xy;
	vsOut.pos.zw = float2(spriteData.baseDepth + abs(pos.z) * 0.0001, 1.0);
	vertexID = vertexID % 6;
	if (vertexID == 0)
	{
		vsOut.uv.xy = float2(1, 0);
	}
	else if(vertexID == 1 || vertexID == 3)
	{
		vsOut.uv.xy = float2(0, 0);
	}
	else if(vertexID == 2 || vertexID == 5)
	{
		vsOut.uv.xy = float2(1, 1);
	}
	else
	{
		vsOut.uv.xy = float2(0, 1);
	}
	//rotation value
	vsOut.uv.z = pos.z;
	//variation
	vsOut.uv.w = pos.w;
	return vsOut;	
}