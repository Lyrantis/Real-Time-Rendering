#include "FollowObjectCamera.h"

FollowObjectCamera::FollowObjectCamera(XMFLOAT3 eyeIn, XMFLOAT3 upIn, XMFLOAT3* objectToFollow)
{
	m_eye = eyeIn;
	m_objectPositionToFollow = objectToFollow;
	m_at = *objectToFollow;
	m_up = upIn;

	m_aspect = m_windowWidth / m_windowHeight;

	XMStoreFloat4x4(&m_view, XMMatrixLookAtLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&m_at), XMLoadFloat3(&m_up)));
	XMStoreFloat4x4(&m_projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(90), m_aspect, m_nearDepth, m_farDepth));


	XMMATRIX cameraTransform = XMLoadFloat4x4(&m_view);
	cameraTransform = XMMatrixInverse(nullptr, cameraTransform);
	m_right = { cameraTransform.r->m128_f32[0], cameraTransform.r->m128_f32[1], cameraTransform.r->m128_f32[2] };
}

void FollowObjectCamera::UpdateLookAtPos(XMFLOAT3 newPos)
{
	m_at = newPos;
	XMStoreFloat4x4(&m_view, XMMatrixLookToLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&m_at), XMLoadFloat3(&m_up)));
}

void FollowObjectCamera::Update(float deltaTime)
{
	m_at = *m_objectPositionToFollow;
	XMStoreFloat4x4(&m_view, XMMatrixLookAtLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&m_at), XMLoadFloat3(&m_up)));
}
