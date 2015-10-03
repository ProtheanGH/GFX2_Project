#include "Camera.h"

#define Float4x4ToXMMAtrix(float4x4) { XMMATRIX(float4x4._11, float4x4._12, float4x4._13, float4x4._14, float4x4._21, float4x4._22, float4x4._23, float4x4._24, float4x4._31, float4x4._32, float4x4._33, float4x4._34, float4x4._41, float4x4._42, float4x4._43, float4x4._44) }

// ===== Constructor / Destructor ===== //
Camera::Camera()
{
	XMStoreFloat4x4(&ViewMatrix, XMMatrixIdentity());
	CursorPosition.x = -1;
	m_fMovementSpeed = 1;
	m_fRotationSpeed = 1;
}

Camera::~Camera()
{

}
// ==================================== //

// ===== Interface ===== //
void Camera::HandleInput(float _deltaTime)
{
	XMMATRIX matrix = Float4x4ToXMMAtrix(ViewMatrix);
	XMVECTOR determinant = XMMatrixDeterminant(matrix);
	matrix = XMMatrixInverse(&determinant, matrix);
	// Forward / Backward Movement (Z-Axis)
	if (GetAsyncKeyState('W')) {
		matrix = XMMatrixMultiply(XMMatrixTranslation(0, 0, _deltaTime * m_fMovementSpeed), matrix);
	}
	else if (GetAsyncKeyState('S')) {
		matrix = XMMatrixMultiply(XMMatrixTranslation(0, 0, _deltaTime * -m_fMovementSpeed), matrix);
	}

	// Sidewards Movement (X-Axis)
	if (GetAsyncKeyState('A')) {
		matrix = XMMatrixMultiply(XMMatrixTranslation(_deltaTime * -m_fMovementSpeed, 0, 0), matrix);
	}
	else if (GetAsyncKeyState('D')) {
		matrix = XMMatrixMultiply(XMMatrixTranslation(_deltaTime * m_fMovementSpeed, 0, 0), matrix);
	}

	// Fly Up / Down (Y-Axis)
	if (GetAsyncKeyState('E')) {
		matrix = XMMatrixMultiply(matrix, XMMatrixTranslation(0, _deltaTime * m_fMovementSpeed, 0));
	}
	else if (GetAsyncKeyState('Q')) {
		matrix = XMMatrixMultiply(matrix, XMMatrixTranslation(0, _deltaTime * -m_fMovementSpeed, 0));
	}

	// Camera Rotation
	if (GetAsyncKeyState(VK_RBUTTON)) {
		if (CursorPosition.x == -1)
			GetCursorPos(&CursorPosition);
		// == Get the New Cursor Position
		POINT newCursorPos;
		GetCursorPos(&newCursorPos);
		float distance;
		// === Left / Right Rotation
		if (newCursorPos.x != CursorPosition.x) {
			XMVECTOR position = matrix.r[3];
			matrix = XMMatrixMultiply(matrix, XMMatrixTranslation(0, 0, 0));
			distance = CursorPosition.x - newCursorPos.x;
			matrix = XMMatrixMultiply(matrix, XMMatrixRotationY(-distance * 0.0174532925f));
			matrix.r[3] = position;
		}
		// === Up / Down Rotation
		if (newCursorPos.y != CursorPosition.y) {
			distance = CursorPosition.y - newCursorPos.y;
			matrix = XMMatrixMultiply(XMMatrixRotationX(-distance * 0.0174532925f), matrix);
		}

		CursorPosition = newCursorPos;
	}
	else {
		CursorPosition.x = -1;
	}

	// Quick Reset
	if (GetAsyncKeyState(VK_SPACE)) {
		matrix = XMMatrixIdentity();
	}

	// === Update the View Matrix
	determinant = XMMatrixDeterminant(matrix);
	matrix = XMMatrixInverse(&determinant, matrix);
	XMStoreFloat4x4(&ViewMatrix, matrix);
}
// ===================== //

// ===== Accessors / Mutators ===== //
XMFLOAT4X4 Camera::GetViewMatrix()
{
	return ViewMatrix;
}

XMMATRIX Camera::GetViewXMMatrix()
{
	return XMMATRIX(ViewMatrix._11, ViewMatrix._12, ViewMatrix._13, ViewMatrix._14,
		ViewMatrix._21, ViewMatrix._22, ViewMatrix._23, ViewMatrix._24,
		ViewMatrix._31, ViewMatrix._32, ViewMatrix._33, ViewMatrix._34,
		ViewMatrix._41, ViewMatrix._42, ViewMatrix._43, ViewMatrix._44);
}

XMFLOAT3 Camera::GetPosition()
{
	XMMATRIX matrix = Float4x4ToXMMAtrix(ViewMatrix);
	XMVECTOR determinant = XMMatrixDeterminant(matrix);
	XMFLOAT3 position;
	XMStoreFloat3(&position, XMMatrixInverse(&determinant, matrix).r[3]);
	return position;
}
// ================================ //