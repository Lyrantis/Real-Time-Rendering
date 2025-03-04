#pragma once
#include "Structures.h"
#include <d3d11_4.h>
#include <string>
#include <vector>

class Terrain
{
public:

	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;

	UINT m_stride;
	UINT m_offset;

	SimpleVertex* m_vertexArray;
	UINT32* m_indexArray;

	int m_vertexCount;
	int m_indexCount;

	ID3D11ShaderResourceView* m_texture;



private:
	float m_halfWidth;
	float m_halfDepth;

	float m_dx;
	float m_dy;

	float m_du;
	float m_dv;

	//Height 
	std::vector<float> m_heightMapData;
	float m_heightScale = 10.0f;

	XMFLOAT4X4 m_world;

	XMFLOAT3 m_position = { 0, -10, 0 };
	XMFLOAT3 m_size = { 1, 1, 1 };
public:
	Terrain();

	XMFLOAT4X4 GetWorld() { return m_world; }

	void GenFlatGrid(ID3D11Device* _device, float width, float depth, int columns, int rows);
	void HeightMapLoad(int heightMapWidth, int heightMapHeight, std::string heightMapFileName);

	void Draw(ID3D11DeviceContext* _immediateContext);
};

