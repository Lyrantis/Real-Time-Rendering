#pragma once
#include "DirectXMath.h"
#include "vcruntime_string.h"
#include <string>
#include <d3d11_4.h>

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 m_pos = XMFLOAT3();
	XMFLOAT3 m_normal = XMFLOAT3();
	XMFLOAT2 m_texCoords = XMFLOAT2();

	bool operator<(const SimpleVertex other) const
	{
		return memcmp((void*)this, (void*)&other, sizeof(SimpleVertex)) > 0;
	};

	SimpleVertex()
	{
	}

	SimpleVertex(XMFLOAT3 PosIn, XMFLOAT3 NormalIn, XMFLOAT2 TexCoordIn)
	{
		m_pos = PosIn;
		m_normal = NormalIn;
		m_texCoords = TexCoordIn;
	}
};

struct MeshData
{
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	UINT m_VBStride;
	UINT m_VBOffset;
	UINT m_indexCount;
};

struct Prefab
{
	std::string m_meshData;
	std::string m_textureData;
	std::string m_specMapData = "None";

	Prefab()
	{

	}

	Prefab(std::string meshIn, std::string textureIn, std::string specIn)
	{
		m_meshData = meshIn;
		m_textureData = textureIn;
		m_specMapData = specIn;
	}
};

struct Skybox
{
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	MeshData m_meshData;
	ID3D11ShaderResourceView* m_texture = nullptr;
	XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_size = { 1000.0f, 1000.0f, 1000.0f };
	XMFLOAT4X4 m_world;
	bool m_enabled = true;

	Skybox()
	{
		XMStoreFloat4x4(&m_world, XMMatrixIdentity() * XMMatrixScaling(m_size.x, m_size.y, m_size.z) * XMMatrixTranslation(m_position.x, m_position.y, m_position.z));
		m_meshData = MeshData();
	}

	~Skybox()
	{
		if (m_vertexShader)m_vertexShader->Release();
		if (m_pixelShader)m_pixelShader->Release();
	}
};

struct PositionalLight
{
	XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4 m_colour = { 0.0f, 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_direction = { 0.0f, 0.0f, 0.0f };;
	float m_maxDistance = 0.0f;
	bool m_enabled = false;

	PositionalLight()
	{

	}

	PositionalLight(XMFLOAT3 pos, bool on, XMFLOAT4 col, XMFLOAT3 dir, float maxDist)
	{
		m_position = pos;
		m_enabled = on;
		m_colour = col;
		m_direction = dir;
		m_maxDistance = maxDist;
	}
};