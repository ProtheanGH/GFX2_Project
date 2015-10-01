#include "Camera.h"

// ===== Constructor / Destructor ===== //
Camera::Camera()
{
	ViewMatrix = XMMatrixIdentity();
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
	XMVECTOR determinant = XMMatrixDeterminant(ViewMatrix);
	ViewMatrix = XMMatrixInverse(&determinant, ViewMatrix);
	// Forward / Backward Movement (Z-Axis)
	if (GetAsyncKeyState('W')) {
		ViewMatrix = XMMatrixMultiply(XMMatrixTranslation(0, 0, _deltaTime * m_fMovementSpeed), ViewMatrix);
	}
	else if (GetAsyncKeyState('S')) {
		ViewMatrix = XMMatrixMultiply(XMMatrixTranslation(0, 0, _deltaTime * -m_fMovementSpeed), ViewMatrix);
	}

	// Sidewards Movement (X-Axis)
	if (GetAsyncKeyState('A')) {
		ViewMatrix = XMMatrixMultiply(XMMatrixTranslation(_deltaTime * -m_fMovementSpeed, 0, 0), ViewMatrix);
	}
	else if (GetAsyncKeyState('D')) {
		ViewMatrix = XMMatrixMultiply(XMMatrixTranslation(_deltaTime * m_fMovementSpeed, 0, 0), ViewMatrix);
	}

	// Fly Up / Down (Y-Axis)
	if (GetAsyncKeyState('E')) {
		ViewMatrix = XMMatrixMultiply(ViewMatrix, XMMatrixTranslation(0, _deltaTime * m_fMovementSpeed, 0));
	}
	else if (GetAsyncKeyState('Q')) {
		ViewMatrix = XMMatrixMultiply(ViewMatrix, XMMatrixTranslation(0, _deltaTime * -m_fMovementSpeed, 0));
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
			XMVECTOR position = ViewMatrix.r[3];
			ViewMatrix = XMMatrixMultiply(ViewMatrix, XMMatrixTranslation(0, 0, 0));
			distance = CursorPosition.x - newCursorPos.x;
			ViewMatrix = XMMatrixMultiply(ViewMatrix, XMMatrixRotationY(-distance * 0.0174532925f));
			ViewMatrix.r[3] = position;
		}
		// === Up / Down Rotation
		if (newCursorPos.y != CursorPosition.y) {
			distance = CursorPosition.y - newCursorPos.y;
			ViewMatrix = XMMatrixMultiply(XMMatrixRotationX(-distance * 0.0174532925f), ViewMatrix);
		}

		CursorPosition = newCursorPos;
	}
	else {
		CursorPosition.x = -1;
	}

	// Quick Reset
	if (GetAsyncKeyState(VK_SPACE)) {
		ViewMatrix = XMMatrixIdentity();
	}

	// === Update the View Matrix
	determinant = XMMatrixDeterminant(ViewMatrix);
	ViewMatrix = XMMatrixInverse(&determinant, ViewMatrix);
}
// ===================== //

// ===== Accessors / Mutators ===== //
XMMATRIX Camera::GetViewMatrix()
{
	return ViewMatrix;
}
// ================================ //