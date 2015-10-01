#include "Object.h"

#define SAFE_RELEASE(p) { if(p) { p->Release(); p = nullptr; } }

// ===== Constructor / Destructor ===== //
Object::Object()
{
	WorldMatrix = XMMatrixIdentity();
	pVertexBuffer = nullptr;
	pIndexBuffer = nullptr;
	pInputLayout = nullptr;
	pVertexShader = nullptr;
	pPixelShader = nullptr;
	pTexture = nullptr;
	VertexSize = 0;
	NumIndexes = 0;
}

Object::~Object()
{
	SAFE_RELEASE(pVertexBuffer);
	SAFE_RELEASE(pIndexBuffer);
	SAFE_RELEASE(pInputLayout);
	SAFE_RELEASE(pVertexShader);
	SAFE_RELEASE(pPixelShader);
	SAFE_RELEASE(pTexture);
}
// ==================================== //
