#include "Object.h"

#define SAFE_RELEASE(p) { if(p) { p->Release(); p = nullptr; } }

// ===== Constructor / Destructor ===== //
Object::Object()
{
	// === Initialize Members
	XMStoreFloat4x4(&WorldMatrix, XMMatrixIdentity());
	pVertexBuffer = nullptr;
	pIndexBuffer = nullptr;
	pInputLayout = nullptr;
	pVertexShader = nullptr;
	pPixelShader = nullptr;
	pTexture = nullptr;
	VertexSize = 0;
	NumIndexes = 0;

	// === Initialize Components
	pMoveComponent = nullptr;
}

Object::~Object()
{
	SAFE_RELEASE(pVertexBuffer);
	SAFE_RELEASE(pIndexBuffer);
	SAFE_RELEASE(pInputLayout);
	SAFE_RELEASE(pTexture);
	SAFE_RELEASE(pShaderResourceView);
	SAFE_RELEASE(pSamplerState);
	/* No need to release the shaders, as the main application will handle that */
	// SAFE_RELEASE(pVertexShader);
	// SAFE_RELEASE(pPixelShader);
}
// ==================================== //

// ===== Interface ===== //
void Object::Update(float _deltaTime)
{
	if (pMoveComponent != nullptr)
		pMoveComponent->Update(_deltaTime);
}
// ===================== //
