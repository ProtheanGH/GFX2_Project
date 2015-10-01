#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <Windows.h>

using namespace DirectX;

class Camera
{
private:
	XMMATRIX	ViewMatrix;
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
	XMMATRIX GetViewMatrix();
};

