#include "DebugFlyCamera.h"
#include <windows.h>

DebugFlyCamera::DebugFlyCamera()
{
}

DebugFlyCamera::DebugFlyCamera(XMFLOAT3 eyeIn, XMFLOAT3 toIn, XMFLOAT3 upIn)
{
	m_eye = eyeIn;
	m_to = toIn;
	m_up = upIn;

	m_aspect = m_windowWidth / m_windowHeight;

	XMStoreFloat4x4(&m_view, XMMatrixLookToLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&m_to), XMLoadFloat3(&m_up)));
	XMStoreFloat4x4(&m_projection, XMMatrixPerspectiveFovLH(XMConvertToRadians(90), m_aspect, m_nearDepth, m_farDepth));


	XMMATRIX cameraTransform = XMLoadFloat4x4(&m_view);

	cameraTransform = XMMatrixInverse(nullptr, cameraTransform);

	m_right = { cameraTransform.r->m128_f32[0], cameraTransform.r->m128_f32[1], cameraTransform.r->m128_f32[2] };
}

void DebugFlyCamera::Update(float deltaTime)
{
	XMFLOAT3 input = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT2 rotationInput = XMFLOAT2(0.0f, 0.0f);

	if (GetKeyState('W') < 0)
	{
		input.y += 1.0f;
	}
	if (GetKeyState('S') < 0)
	{
		input.y -= 1.0f;
	}
	if (GetKeyState('A') < 0)
	{
		input.x -= 1.0f;
	}
	if (GetKeyState('D') < 0)
	{
		input.x += 1.0f;
	}
	if (GetAsyncKeyState(VK_SPACE))
	{
		input.z += 1.0f;
	}
	if (GetAsyncKeyState(VK_LSHIFT))
	{
		input.z -= 1.0f;
	}
	if ((input.x != 0.0f) || (input.y != 0.0f) || (input.z != 0.0f))
	{
		Move(input, deltaTime);
	}

	if (GetAsyncKeyState(VK_LEFT))
	{
		rotationInput.x -= 1.0f;
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		rotationInput.x += 1.0f;
	}
	if (GetAsyncKeyState(VK_UP))
	{
		rotationInput.y -= 1.0f;
	}
	if (GetAsyncKeyState(VK_DOWN))
	{
		rotationInput.y += 1.0f;
	}
	if ((rotationInput.x != 0.0f) || (rotationInput.y != 0.0f))
	{
		Rotate(rotationInput, deltaTime);
	}

}

void DebugFlyCamera::Move(XMFLOAT3 input, float deltaTime)
{
	float magnitude = sqrt((input.x * input.x) + (input.y * input.y) + (input.z * input.z));
	input.x /= magnitude;
	input.y /= magnitude;
	input.z /= magnitude;

	XMMATRIX cameraTransform = XMLoadFloat4x4(&m_view);
	cameraTransform = XMMatrixInverse(nullptr, cameraTransform); //Get World Matrix and transpose to be readable

	XMFLOAT3 directionToMove = XMFLOAT3((m_to.x * input.y) + (m_right.x * input.x) + (m_up.x * input.z), (m_to.y * input.y) + (m_right.y * input.x) + (m_up.y * input.z), (m_to.z * input.y) + (m_right.z * input.x) + (m_up.z * input.z));
	float mag = sqrt((directionToMove.x * directionToMove.x) + (directionToMove.y * directionToMove.y) + (directionToMove.z * directionToMove.z));
	directionToMove = { directionToMove.x / mag, directionToMove.y / mag, directionToMove.z / mag };
	m_eye.x += directionToMove.x * m_speed * deltaTime / 10.0f;
	m_eye.y += directionToMove.y * m_speed * deltaTime / 10.0f;
	m_eye.z += directionToMove.z * m_speed * deltaTime / 10.0f;


	XMStoreFloat4x4(&m_view, XMMatrixLookToLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&m_to), XMLoadFloat3(&m_up)));
}

void DebugFlyCamera::Rotate(XMFLOAT2 rotationInput, float deltaTime)
{
	XMMATRIX cameraTransform = XMLoadFloat4x4(&m_view);

	cameraTransform = XMMatrixInverse(nullptr, cameraTransform);

	XMMATRIX xRot = XMMatrixRotationAxis(cameraTransform.r[1], XMConvertToRadians(rotationInput.x * m_rotationSpeed * deltaTime));
	cameraTransform.r[0] = XMVector3TransformNormal(cameraTransform.r[0], xRot);
	cameraTransform.r[2] = XMVector3TransformNormal(cameraTransform.r[2], xRot);

	XMMATRIX yRot = XMMatrixRotationAxis(cameraTransform.r[0], XMConvertToRadians(rotationInput.y * m_rotationSpeed * deltaTime));
	cameraTransform.r[1] = XMVector3TransformNormal(cameraTransform.r[1], yRot);
	cameraTransform.r[2] = XMVector3TransformNormal(cameraTransform.r[2], yRot);

	m_up = { cameraTransform.r[1].m128_f32[0], cameraTransform.r[1].m128_f32[1], cameraTransform.r[1].m128_f32[2] };
	m_to = { cameraTransform.r[2].m128_f32[0],cameraTransform.r[2].m128_f32[1], cameraTransform.r[2].m128_f32[2] };
	m_right = { cameraTransform.r->m128_f32[0], cameraTransform.r->m128_f32[1], cameraTransform.r->m128_f32[2] };

	XMStoreFloat4x4(&m_view, XMMatrixLookToLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&m_to), XMLoadFloat3(&m_up)));
}
