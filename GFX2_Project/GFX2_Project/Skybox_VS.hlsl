#pragma pack_matrix( row_major )

struct V_INPUT
{
	float4 posL : POSITION;
	float3 uvL : TEXTCOORDS;
	float3 normalsL : NORMALS;
};

struct V_OUTPUT
{
	float4 posH : SV_POSITION;
	float2 UVCoords : TEXCOORD0;
};

cbuffer OBJECT : register (b0)
{
	float4x4 worldMatrix;
}

cbuffer SCENE : register (b1)
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}

V_OUTPUT main(V_INPUT _input)
{
	V_OUTPUT output = (V_OUTPUT)0;

	float4 localH = float4(_input.posL);
	// === Local -> World
	localH = mul(localH, worldMatrix);
	// === World -> View
	localH = mul(localH, viewMatrix);
	// === View -> Projection
	localH = mul(localH, projectionMatrix);

	output.posH = localH;
	output.UVCoords = float2(_input.uvL[0], _input.uvL[1]);

	return output;
}