#pragma once

#include <DirectXMath.h>

using namespace DirectX;

#define MatrixToXMMatrix(Matrix) { XMMatrix(Matrix.VectorData[0].x, Matrix.VectorData[0].y, Matrix.VectorData[0].z, Matrix.VectorData[0].w), Matrix.VectorData[1].x, Matrix.VectorData[1].y, Matrix.VectorData[1].z, Matrix.VectorData[1].w, Matrix.VectorData[2].x, Matrix.VectorData[2].y, Matrix.VectorData[2].z, Matrix.VectorData[2].w, Matrix.VectorData[3].x, Matrix.VectorData[3].y, Matrix.VectorData[3].z, Matrix.VectorData[3].w }

struct Vector
{
	float x, y, z, w;
};

struct Matrix
{
	Vector VectorData[4];
};