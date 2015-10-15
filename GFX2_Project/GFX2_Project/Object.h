#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

#include "MoveComponent.h"

using namespace DirectX;

class Object
{
public:
	// ===== Constructor / Destructor
	Object();
	~Object();

	// === Variables
	XMFLOAT4X4 WorldMatrix;
	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;
	ID3D11InputLayout* pInputLayout;
	ID3D11VertexShader* pVertexShader;
	ID3D11PixelShader* pPixelShader;
	ID3D11Resource* pTexture;
	ID3D11ShaderResourceView* pShaderResourceView;
	ID3D11SamplerState*	pSamplerState;
	unsigned int VertexSize;
	unsigned int NumIndexes;
	float DistanceFromCamera;

	// === Componets
	MoveComponent* pMoveComponent;

	// ===== Functions
	void Update(float _deltaTime);
	XMFLOAT3 GetPosition() { return XMFLOAT3(WorldMatrix._41, WorldMatrix._42, WorldMatrix._43); }
};

