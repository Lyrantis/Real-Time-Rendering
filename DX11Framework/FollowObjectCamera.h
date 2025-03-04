#pragma once
#include "BaseCamera.h"

class FollowObjectCamera : public BaseCamera
{
public:
	XMFLOAT3* m_objectPositionToFollow;

	FollowObjectCamera(XMFLOAT3 eyeIn, XMFLOAT3 upIn, XMFLOAT3* objectToFollow);

	void UpdateLookAtPos(XMFLOAT3 newPos);

	void Update(float deltaTime) override;
};

