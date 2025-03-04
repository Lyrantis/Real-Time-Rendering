#include "Terrain.h"
#include <fstream>

Terrain::Terrain()
{
	XMStoreFloat4x4(&m_world, XMMatrixIdentity() * XMMatrixScaling(m_size.x, m_size.y, m_size.z) * XMMatrixTranslation(m_position.x, m_position.y, m_position.z));
}

void Terrain::GenFlatGrid(ID3D11Device* _device, float width, float depth, int columns, int rows)
{
	std::vector<SimpleVertex> vertices = std::vector<SimpleVertex>();
	std::vector<UINT32> indices = std::vector<UINT32>();

	m_halfWidth = width * 0.5f;
	m_halfDepth = depth * 0.5f;

	m_dx = width / (columns - 1);
	m_dy = depth / (rows - 1);

	m_du = 1.0f / (columns - 1);
	m_dv = 1.0f / (rows - 1);

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			vertices.push_back(SimpleVertex(XMFLOAT3(-m_halfWidth + j * m_dx, 0, m_halfDepth - i * m_dy), XMFLOAT3(0, 1, 0), XMFLOAT2(j * m_du, i * m_dv)));
		}
	}

	for (int i = 0; i < rows - 1; i++)
	{
		for (int j = 0; j < columns - 1; j++)
		{
			indices.push_back((i * columns) + j);
			indices.push_back((i * columns) + j + 1);
			indices.push_back(((i + 1) * columns) + j);

			indices.push_back(((i + 1) * columns) + j);
			indices.push_back((i * columns) + j + 1);
			indices.push_back(((i + 1) * columns) + j + 1);
		}
	}

    m_vertexCount = vertices.size();;
	m_vertexArray = new SimpleVertex[m_vertexCount];
	for (int i = 0; i < m_vertexCount; i++)
	{
		m_vertexArray[i] = vertices[i];
	}

	m_indexCount = indices.size();
	m_indexArray = new UINT32[m_indexCount];
	for (int i = 0; i < m_indexCount; i++)
	{
		m_indexArray[i] = indices[i];
	}
}

void Terrain::HeightMapLoad(int heightMapWidth, int heightMapHeight, std::string heightMapFileName)
{
	std::vector<unsigned char> in(heightMapWidth * heightMapHeight);


	std::ifstream inFile;
	inFile.open(heightMapFileName.c_str(), std::ios_base::binary);

	if (inFile)
	{
		inFile.read((char*)&in[0], (std::streamsize)in.size());

		inFile.close();
	}

	m_heightMapData = std::vector<float>();
	m_heightMapData.resize(sizeof(float) * heightMapWidth * heightMapHeight);

	for (int i = 0; i < heightMapWidth * heightMapHeight; i++)
	{
		m_heightMapData[i] = (in[i] / 255.0f) * m_heightScale;
	}

	for (int i = 0; i < m_vertexCount; i++)
	{
		m_vertexArray[i].m_pos.y = m_heightMapData[i];
	}

}

void Terrain::Draw(ID3D11DeviceContext* _immediateContext)
{
	m_stride = sizeof(SimpleVertex);
	m_offset = 0;

	_immediateContext->PSSetShaderResources(0, 1, &m_texture);
	_immediateContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &m_stride, &m_offset);
	_immediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	_immediateContext->DrawIndexed(m_indexCount, 0, 0);
}
