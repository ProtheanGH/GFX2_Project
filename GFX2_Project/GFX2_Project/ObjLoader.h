#pragma once

#include <DirectXMath.h>
#include <vector>

#include "Object.h"

using namespace DirectX;
using std::vector;

struct Vector3
{
	float x, y, z;
};

// === Loading Obj File Data
bool LoadObjFile(const char * _path, vector<Vector3>& _vertices, vector<Vector3>& _uvs, vector<Vector3>& _normals)
{
	vector<unsigned int> vertexIndexes, uvIndexes, normalIndexes;
	vector<Vector3> tempVertices, tempUVs, tempNormals;
	FILE* file;
	fopen_s(&file, _path, "r");
	if (file == NULL)
		return false;

	while (true) {
		char header[1024];
		int res = fscanf_s(file, "%s", header, sizeof(header));
		
		// === Reach the end of file?
		if (res == EOF)
			break;
		// === Parse the read in data
		if (strcmp(header, "v") == 0) {
			Vector3 vertex;
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			tempVertices.push_back(vertex);
		}
		else if (strcmp(header, "vt") == 0) {
			Vector3 uv;
			fscanf_s(file, "%f %f", &uv.x, &uv.y);
			uv.y = 1 - uv.y;
			tempUVs.push_back(uv);
		}
		else if (strcmp(header, "vn") == 0) {
			Vector3 normal;
			fscanf_s(file, "%f %f %f", &normal.x, &normal.y, &normal.z);
			tempNormals.push_back(normal);
		}
		else if (strcmp(header, "f") == 0) {
			unsigned int vertIndex[3], uvIndex[3], normalIndex[3];
			int readIn = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertIndex[0], &uvIndex[0], &normalIndex[0], &vertIndex[1], &uvIndex[1], &normalIndex[1], &vertIndex[2], &uvIndex[2], &normalIndex[2]);
			if (readIn != 9)
				return false;
			vertexIndexes.push_back(vertIndex[0]);
			vertexIndexes.push_back(vertIndex[1]);
			vertexIndexes.push_back(vertIndex[2]);
			uvIndexes.push_back(uvIndex[0]);
			uvIndexes.push_back(uvIndex[1]);
			uvIndexes.push_back(uvIndex[2]);
			normalIndexes.push_back(normalIndex[0]);
			normalIndexes.push_back(normalIndex[1]);
			normalIndexes.push_back(normalIndex[2]);
		}
	}

	// === Cycle through each triangle
	for (unsigned int i = 0; i < vertexIndexes.size(); i++) {
		Vector3 vertex = tempVertices[vertexIndexes[i] - 1];
		_vertices.push_back(vertex);
		Vector3 uv = tempUVs[uvIndexes[i] - 1];
		_uvs.push_back(uv);
		Vector3 normal = tempNormals[normalIndexes[i] - 1];
		_normals.push_back(normal);
	}
	
	return true;
}