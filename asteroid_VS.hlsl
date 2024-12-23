struct VS_OUT
{
	float4 pos : SV_Position;
	float2 uv : UV0;
};

VS_OUT main( float2 pos : POSITION , uint vertexID : SV_VertexID)
{
	VS_OUT vsOut;
	vsOut.pos = float4(pos.x, pos.y, vertexID * 0.0001, 1.0);
	vertexID = vertexID % 6;
	if (vertexID == 0)
	{
		vsOut.uv = float2(0, 0);
	}
	else if(vertexID == 1 || vertexID == 3)
	{
		vsOut.uv = float2(1, 0);
	}
	else if(vertexID == 2 || vertexID == 5)
	{
		vsOut.uv = float2(0, 1);
	}
	else
	{
		vsOut.uv = float2(1, 1);
	}
	return vsOut;	
}