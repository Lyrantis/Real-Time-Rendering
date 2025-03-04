#pragma once
#include "BaseCamera.h"

class DebugFlyCamera : public BaseCamera
{
private:

	XMFLOAT3 m_to;
	float m_speed = 50.0f;
	float m_rotationSpeed = 180.0f;

public:
	DebugFlyCamera();
	DebugFlyCamera(XMFLOAT3 eyeIn, XMFLOAT3 atIn, XMFLOAT3 upIn);

	XMFLOAT3 GetTo() { return m_to; }
	void SetTo(XMFLOAT3 to) { m_to = to; }

	void Update(float deltaTime) override;

	void Move(XMFLOAT3 input, float deltaTime);
	void Rotate(XMFLOAT2 rotationInput, float deltaTime);
};