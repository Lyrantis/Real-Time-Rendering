#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class BaseCamera
{
public:

	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_projection;

	XMFLOAT3 m_eye;
	XMFLOAT3 m_at;
	XMFLOAT3 m_up;
	XMFLOAT3 m_right;

	float m_windowWidth = 1280;
	float m_windowHeight = 768;
	float m_aspect;

	float m_nearDepth = 0.01f;
	float m_farDepth = 100.0f;

	BaseCamera();
	BaseCamera(XMFLOAT3 eyeIn, XMFLOAT3 atIn, XMFLOAT3 upIn);

	virtual void Update(float deltaTime) { ; }

	XMFLOAT3 GetEye() { return m_eye; }
	void SetEye(XMFLOAT3 eye) { m_eye = eye; }

	XMFLOAT3 GetAt() { return m_at; }
	void SetAt(XMFLOAT3 at) { m_at = at; }

	XMFLOAT4X4 GetView() { return m_view; }
	XMFLOAT4X4 GetProjection() { return m_projection; }

};
