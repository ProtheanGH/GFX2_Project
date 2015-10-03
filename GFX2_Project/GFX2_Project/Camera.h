#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <Windows.h>

using namespace DirectX;

class Camera
{
private:
	XMFLOAT4X4	ViewMatrix;
	POINT		CursorPosition;
	float		m_fMovementSpeed;
	float		m_fRotationSpeed;

public:
	// ===== Constructor / Destructor
	Camera();
	~Camera();

	// ===== Interface
	void HandleInput(float _deltaTime);

	// ===== Accessors / Mutators
	XMFLOAT4X4 GetViewMatrix();
	XMMATRIX GetViewXMMatrix();
	XMFLOAT3 GetPosition();
};

