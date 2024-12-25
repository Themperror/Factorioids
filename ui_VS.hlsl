struct VS_OUT
{
	float4 pos : SV_Position;
	float2 uv : UV0;
};

VS_OUT main(float3 pos : POS, uint vertexID : SV_VertexID)
{
	VS_OUT vsOut;
	vsOut.pos = float4(pos.x, pos.y, vertexID / 6, 1.0);
	if (vertexID == 0)
	{
		vsOut.uv.xy = float2(1, 0);
	}
	else if (vertexID == 1 || vertexID == 3)
	{
		vsOut.uv.xy = float2(0, 0);
	}
	else if (vertexID == 2 || vertexID == 5)
	{
		vsOut.uv.xy = float2(1, 1);
	}
	else
	{
		vsOut.uv.xy = float2(0, 1);
	}
	return vsOut;
}