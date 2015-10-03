struct P_INPUT
{
	float4 posH : SV_POSITION;
	float2 UVCoords : TEXCOORD0;
};

texture2D baseTexture : register(t0);

SamplerState filter : register (s0);

float4 main(P_INPUT _input) : SV_TARGET
{
	float2 uv = float2(_input.UVCoords[0], _input.UVCoords[1]);
	float4 color = baseTexture.Sample(filter, uv);
	return color;
}