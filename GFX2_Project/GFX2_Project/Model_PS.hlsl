struct P_INPUT
{
	float4 posH : SV_POSITION;
	float2 UVCoords : TEXCOORD0;
	float3 normal : NORMAL;
};

cbuffer LIGHT : register(b0)
{
	float4 LightDirection;
	float4 LightColor;
}

texture2D baseTexture : register(t0);

SamplerState filter : register (s0);

float4 main(P_INPUT _input) : SV_TARGET
{
	// === Get the pixel from the Texture
	float4 color = baseTexture.Sample(filter, _input.UVCoords);
	// === Handle Lighting
	float lightRatio = clamp(dot(-LightDirection, _input.normal), 0, 1);
	if (lightRatio > 0) {
		color[0] *= LightColor * lightRatio;
		color[1] *= LightColor * lightRatio;
		color[2] *= LightColor * lightRatio;
	}
	else {
		color[0] *= 0.1f;
		color[1] *= 0.1f;
		color[2] *= 0.1f;
	}
	return color;
}