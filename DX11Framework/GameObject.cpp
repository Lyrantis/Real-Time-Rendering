#include "GameObject.h"

GameObject::GameObject()
{
	m_world = XMFLOAT4X4();
}

GameObject::GameObject(Prefab prefab, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, ID3D11Device* device, std::string type)
{
	m_objectType = type;

	m_meshData = OBJLoader::Load(prefab.m_meshData.c_str(), device);
	CreateDDSTextureFromFile(device, std::wstring(prefab.m_textureData.begin(), prefab.m_textureData.end()).c_str(), nullptr, &m_texture);
	if (prefab.m_specMapData != "None")
	{
		m_hasSpecMap = true;
		CreateDDSTextureFromFile(device, std::wstring(prefab.m_specMapData.begin(), prefab.m_specMapData.end()).c_str(), nullptr, &m_specMapTexture);
	}

	m_position = position;
	m_rotation = rotation;
	m_scale = scale;

	XMStoreFloat4x4(&m_world, XMMatrixIdentity() * XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) 
		* XMMatrixRotationX(rotation.x) * XMMatrixRotationY(rotation.y) * XMMatrixRotationZ(rotation.z) * XMMatrixTranslation(position.x, position.y, position.z));
}

void GameObject::SetSpecularMap(ID3D11ShaderResourceView* in)
{
	m_specMapTexture = in;
	m_hasSpecMap = true;
}

void GameObject::Update(float deltaTime)
{
	if (m_isRotating)
	{
		m_rotation.x += m_rotationSpeed.x * deltaTime;
		m_rotation.y += m_rotationSpeed.y * deltaTime;
		m_rotation.z += m_rotationSpeed.z * deltaTime;
	}
	XMStoreFloat4x4(&m_world, XMMatrixIdentity() * XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z)
		* XMMatrixRotationX(m_rotation.x) * XMMatrixRotationY(m_rotation.y) * XMMatrixRotationZ(m_rotation.z) * XMMatrixTranslation(m_position.x, m_position.y, m_position.z));
}

void GameObject::Draw(ID3D11DeviceContext* _immediateContext)
{
    _immediateContext->PSSetShaderResources(0, 1, &m_texture);
    _immediateContext->IASetVertexBuffers(0, 1, &m_meshData.m_vertexBuffer, &m_meshData.m_VBStride, &m_meshData.m_VBOffset);
    _immediateContext->IASetIndexBuffer(m_meshData.m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	_immediateContext->DrawIndexed(m_meshData.m_indexCount, 0, 0);
}

void GameObject::Move(XMFLOAT3 toMove)
{

}

void GameObject::Rotate(char axis, float amount)
{
}
