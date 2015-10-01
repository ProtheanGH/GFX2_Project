#pragma pack_matrix( row_major )

struct V_INPUT
{
	float4 posL : POSITION;
	float4 colorL : COLOR;
};

struct V_OUTPUT
{
	float4 posH : SV_POSITION;
	float4 colorH : COLOR;
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
	output.colorH = _input.colorL;

	return output;
}