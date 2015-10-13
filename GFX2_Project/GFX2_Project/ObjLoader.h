#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "Object.h"
#include "Vertex_Inputs.h"

using namespace DirectX;
using std::vector;

struct Vector3
{
	float x, y, z;
};

struct ObjectData
{
	const char * path;
	vector<Vector3> vertices;
	vector<Vector3> uvs;
	vector<Vector3> normals;
};

struct ModelData
{
	const char*		path;
	Object*			object;
	ID3D11Device*	device;
};

// === Loading Obj File Data
bool LoadObjFile(ObjectData* _objectData)
{
	vector<unsigned int> vertexIndexes, uvIndexes, normalIndexes;
	vector<Vector3> tempVertices, tempUVs, tempNormals;
	FILE* file;
	fopen_s(&file, _objectData->path, "r");
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
		_objectData->vertices.push_back(vertex);
		Vector3 uv = tempUVs[uvIndexes[i] - 1];
		_objectData->uvs.push_back(uv);
		Vector3 normal = tempNormals[normalIndexes[i] - 1];
		_objectData->normals.push_back(normal);
	}
	
	return true;
}

void LoadObjFile_Thread(ModelData* _modelData)
{
	ObjectData objData;
	objData.path = _modelData->path;

	// === Load the Data from the file
	LoadObjFile(&objData);

	// === Cycle through the data, setting up the actaul object
	Vertex* objectVertices = new Vertex[objData.vertices.size()];
	unsigned int* objectIndexes = new unsigned int[objData.vertices.size()];
	for (unsigned int i = 0; i < objData.vertices.size(); i++) {
		objectVertices[i] = Vertex(objData.vertices[i].x, objData.vertices[i].y, objData.vertices[i].z, 1, objData.uvs[i].x, objData.uvs[i].y, objData.uvs[i].z, objData.normals[i].x, objData.normals[i].y, objData.normals[i].z);
		objectIndexes[i] = i;
	}

	// === Setup the Object
	// == Vertex Buffer
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(Vertex) * objData.vertices.size();
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.MiscFlags = 0;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	initData.pSysMem = objectVertices;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	_modelData->device->CreateBuffer(&bufferDesc, &initData, &_modelData->object->pVertexBuffer);
	// == Index Buffer
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(unsigned int) * objData.vertices.size();
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.MiscFlags = 0;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;

	initData.pSysMem = objectIndexes;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	_modelData->device->CreateBuffer(&bufferDesc, &initData, &_modelData->object->pIndexBuffer);
	// == Set the VertexSize
	_modelData->object->VertexSize = sizeof(Vertex);
	// == Set the Number of Vertices
	_modelData->object->NumIndexes = objData.vertices.size();
}

