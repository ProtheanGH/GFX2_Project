#pragma once

#include <DirectXMath.h>

using namespace DirectX;

// ( 32 Bytes )
struct DirectionalLight
{
	XMFLOAT4 LightDirection;
	XMFLOAT4 LightColor;
	DirectionalLight() {
		LightDirection = XMFLOAT4(0, 0, 0, 0);
		LightColor = XMFLOAT4(0, 0, 0, 0);
	}
};

// ( 32 Bytes )
struct PointLight
{
	XMFLOAT4 Position;
	XMFLOAT4 LightColor;
	float Radius;
	XMFLOAT3 Padding; // Needed for 16 byte alignment
	PointLight() {
		Position = XMFLOAT4(0, 0, 0, 0);
		LightColor = XMFLOAT4(0, 0, 0, 0);
		Radius = 0;
		Padding = XMFLOAT3(0, 0, 0);
	}
};

// ( 52 Bytes )
struct SpotLight
{
	XMFLOAT4 Position;
	XMFLOAT4 LightColor;
	XMFLOAT4 ConeDirection;
	float ConeRatio;
	float Radius;
	XMFLOAT2 Padding; // Needed for 16 byte alignment

	SpotLight() {
		Position = XMFLOAT4(0, 0, 0, 0);
		LightColor = XMFLOAT4(0, 0, 0, 0);
		ConeDirection = XMFLOAT4(0, 0, 0, 0);
		ConeRatio = 0.0f;
		Padding = XMFLOAT2(0, 0);
	}

	void HandleInput(float _deltaTime);
};

// (16 Bytes )
struct AmbientLight
{
	XMFLOAT4 LightColor;
	AmbientLight() {
		LightColor = XMFLOAT4(0, 0, 0, 0);
	}
};

struct Lights
{
	DirectionalLight mDirectionalLight;
	PointLight mPointLight;
	SpotLight mSpotLight;
	AmbientLight mAmbientLight;
};

// ===== Functions ===== //
// === Spot Light
void SpotLight::HandleInput(float _deltaTime)
{
	// === Movement
	if (GetAsyncKeyState(VK_NUMPAD8)) {
		// == Forward
		Position.z += _deltaTime;
	}
	if (GetAsyncKeyState(VK_NUMPAD2)) {
		// == Backward
		Position.z -= _deltaTime;
	}
	if (GetAsyncKeyState(VK_NUMPAD4)) {
		// == Left
		Position.x -= _deltaTime;
	}
	if (GetAsyncKeyState(VK_NUMPAD6)) {
		// == Right
		Position.x += _deltaTime;
	}

	// === Direction
	if (GetAsyncKeyState(VK_ADD)) {
		ConeDirection.z += _deltaTime;
	}
	else if (GetAsyncKeyState(VK_SUBTRACT)) {
		ConeDirection.z -= _deltaTime;
	}
	// === Intensity

}
// ===================== //