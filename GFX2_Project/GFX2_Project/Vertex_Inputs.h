#pragma once

#include <d3d11.h>

// ===== Vertex Structures ===== //
struct Vertex_PositionColor
{
	float x, y, z, w;
	float color[4];
	Vertex_PositionColor() {
		x = y = z = 0; w = 1;
		color[0] = 1; color[1] = 1; color[2] = 1; color[3] = 1;
	}
	Vertex_PositionColor(float _x, float _y, float _z, float _w) {
		x = _x; y = _y; z = _z; w = _w;
		color[0] = 1; color[1] = 0; color[2] = 0; color[3] = 1;
	}
	Vertex_PositionColor(float _x, float _y, float _z, float _w, float* _color) {
		x = _x; y = _y; z = _z; w = _w;
		color[0] = _color[0]; color[1] = _color[1]; color[2] = _color[2]; color[3] = _color[3];
	}
	Vertex_PositionColor(float _x, float _y, float _z, float _w, float _cR, float _cG, float _cB, float _cA) {
		x = _x; y = _y; z = _z; w = _w;
		color[0] = _cR; color[1] = _cG; color[2] = _cB; color[3] = _cA;
	}
};

struct Vertex
{
	float x, y, z, w;
	float u, v;

	Vertex() {
		x = y = z = 0; w = 1;
		u = z = 0;
	}
	Vertex(float _x, float _y, float _z, float _w = 1, float _u = 0, float _v = 0) {
		x = _x; y = _y; z = _z; w = _w;
		u = _u; v = _v;
	}
};
// ============================= //

// ===== Input Layouts ===== //
D3D11_INPUT_ELEMENT_DESC Layout_Vertex_PositionColor[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

D3D11_INPUT_ELEMENT_DESC Layout_Vertex[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORDS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
// ========================= //