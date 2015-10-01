#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

class Object
{
public:
	// ===== Constructor / Destructor
	Object();
	~Object();

	// === Variables
	XMMATRIX WorldMatrix;
	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;
	ID3D11InputLayout* pInputLayout;
	ID3D11VertexShader* pVertexShader;
	ID3D11PixelShader* pPixelShader;
	ID3D11Texture2D* pTexture;
	unsigned int VertexSize;
	unsigned int NumIndexes;
};

