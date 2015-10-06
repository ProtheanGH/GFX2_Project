// ===== Structures ===== //
// == Light Structures
struct DirectionalLight
{
	float4 LightDirection;
	float4 LightColor;
};

struct PointLight
{
	float4 Position;
	float4 LightColor;
	float Radius;
	float3 Padding;
};

struct SpotLight
{
	float4 Position;
	float4 LightColor;
	float4 ConeDirection;
	float ConeRatio;
	float Radius;
	float2 Padding;
};

struct AmbientLight
{
	float4 LightColor;
};

// == Input from VS
struct P_INPUT
{
	float4 posH : SV_POSITION;
	float4 surfacePos : SURFACEPOS;
	float2 UVCoords : TEXCOORD0;
	float3 normal : NORMAL;
};
// ====================== //

cbuffer LIGHTS : register(b0)
{
	DirectionalLight Light_Directional;
	PointLight Light_Point;
	SpotLight Light_Spot;
	AmbientLight Light_Ambient;
}

texture2D baseTexture : register(t0);

SamplerState filter : register (s0);

float4 main(P_INPUT _input) : SV_TARGET
{
	// === Get the pixel from the Texture
	float4 color = baseTexture.Sample(filter, _input.UVCoords);
	// === Handle Lighting
	float lightRatio;
	float4 lightDir;
	float attenuation;
	// == Ambient Lighting
	float4 ambientColor = color;
	ambientColor[0] *= Light_Ambient.LightColor[0];
	ambientColor[1] *= Light_Ambient.LightColor[1];
	ambientColor[2] *= Light_Ambient.LightColor[2];

	// == Directional Lighting
	float4 directionalColor = color;
	lightRatio = clamp(dot(-Light_Directional.LightDirection, _input.normal), 0, 1);
	directionalColor[0] *= Light_Directional.LightColor[0] * lightRatio;
	directionalColor[1] *= Light_Directional.LightColor[1] * lightRatio;
	directionalColor[2] *= Light_Directional.LightColor[2] * lightRatio;

	// == Point Lighting
	float4 pointColor = color;
	lightDir = normalize(Light_Point.Position - _input.surfacePos);
	lightRatio = clamp(dot(lightDir, _input.normal), 0, 1);
	attenuation = 1.0 - clamp(length(lightDir / Light_Point.Radius), 0, 1);
	pointColor[0] *= Light_Point.LightColor[0] * lightRatio * attenuation;
	pointColor[1] *= Light_Point.LightColor[1] * lightRatio * attenuation;
	pointColor[2] *= Light_Point.LightColor[2] * lightRatio * attenuation;

	// == SpotLight
	float4 spotColor = color;
	lightDir = normalize(Light_Spot.Position - _input.surfacePos);
	float surfaceRatio = clamp(dot(-lightDir, Light_Spot.ConeDirection), 0, 1);
	float spotFactor = (surfaceRatio > Light_Spot.ConeRatio) ? 1 : 0;
	lightRatio = clamp(dot(lightDir, _input.normal), 0, 1);
	spotColor[0] *= spotFactor * lightRatio * Light_Spot.LightColor[0];
	spotColor[1] *= spotFactor * lightRatio * Light_Spot.LightColor[1];
	spotColor[2] *= spotFactor * lightRatio * Light_Spot.LightColor[2];

	// === Combine the Colors
	color[0] = ambientColor[0] + directionalColor[0] + pointColor[0] + spotColor[0];
	color[1] = ambientColor[1] + directionalColor[1] + pointColor[1] + spotColor[1];
	color[2] = ambientColor[2] + directionalColor[2] + pointColor[2] + spotColor[2];

	return color;
}