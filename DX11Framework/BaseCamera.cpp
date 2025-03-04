#include "BaseCamera.h"
#include <windows.h>

BaseCamera::BaseCamera()
{

}

BaseCamera::BaseCamera(XMFLOAT3 eyeIn, XMFLOAT3 atIn, XMFLOAT3 upIn)
{
	m_eye = eyeIn;
	m_at = atIn;
	m_up = upIn;

	m_aspect = m_windowWidth / m_windowHeight;

	XMStoreFloat4x4(&m_view, XMMatrixLookToLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&m_at), XMLoadFloat3(&m_up)));
	XMStoreFloat4x4(&m_projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(90), m_aspect, m_nearDepth, m_farDepth));


	XMMATRIX cameraTransform = XMLoadFloat4x4(&m_view);

	cameraTransform = XMMatrixInverse(nullptr,cameraTransform);
	//r[0] = rightX,    rightY,    rightZ,    0
	//r[1] = upX,       upY,       upZ,       0
	//r[2] = forwardX,  forwardY,  forwardZ,  0
	//r[3] = positionX, positionY, positionZ, 1


	//Rotation around right axis example
	//XMMATRIX rot = XMMatrixRotationAxis(cameraTransform.r[0], XMConvertToRadians(15));
	//cameraTransform.r[1] = XMVector3TransformNormal(cameraTransform.r[1], rot);
	//cameraTransform.r[2] = XMVector3TransformNormal(cameraTransform.r[2], rot);

	m_right = { cameraTransform.r->m128_f32[0], cameraTransform.r->m128_f32[1], cameraTransform.r->m128_f32[2] };

}
