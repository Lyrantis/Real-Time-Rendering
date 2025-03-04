#pragma once
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "Structures.h"
#include "OBJLoader.h"
#include "DDSTextureLoader.h"

class GameObject
{
private:
	MeshData m_meshData;

public:

	std::string m_objectType = "NULL";
	XMFLOAT3 m_position;
	XMFLOAT3 m_rotation;
	XMFLOAT3 m_scale;

	bool m_hasSpecMap = false;
	DirectX::XMFLOAT4X4 m_world;
	ID3D11ShaderResourceView* m_texture = nullptr;
	ID3D11ShaderResourceView* m_specMapTexture = nullptr;

	bool m_isRotating = false;
	XMFLOAT3 m_rotationSpeed = {0.0f, 0.0f, 0.0f};

	GameObject();
	GameObject(Prefab prefab, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, ID3D11Device* device, std::string type);

	void SetShaderResource(ID3D11ShaderResourceView* in) { m_texture = in; }
	void SetSpecularMap(ID3D11ShaderResourceView* in);
	void SetMeshData(MeshData in) { m_meshData = in; }
	void SetWorld(DirectX::XMFLOAT4X4 in) { m_world = in; }

	ID3D11ShaderResourceView** GetShaderResource() { return &m_texture; }
	MeshData GetMeshData() { return m_meshData; }
	DirectX::XMFLOAT4X4 GetWorld() { return m_world; }

	void Update(float deltaTime);
	void Draw(ID3D11DeviceContext* _immediateContext);
	void Move(XMFLOAT3 toMove);
	void Rotate(char axis, float amount);
};

