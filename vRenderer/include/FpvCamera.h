#pragma once

#include <iostream>

#include "BaseCamera.h"


class FpvCamera : public BaseCamera
{
public:

	FpvCamera() = default;
	FpvCamera(float fov, float znear, float zfar, int viewportWidth, int viewportHeight, bool flipY = false);
	~FpvCamera() = default;

	void update() override;
	void onMouseScroll(float amount, InputState inputState) override;
	void onMouseMove(int xpos, int ypos, InputState inputState) override;
	void onKey(InputState input) override;

protected:

	void recalculateDirectionVectors() override;

private:

	const float PITCH_LIMIT_TOP = 89.0f;
	const float PITCH_LIMIT_BOTTOM = -89.0f;
	
	// Required for proper initial input handling 
	bool firstTime = true;

	glm::vec2 prevMousePos = { 0, 0 };
	glm::vec2 currMousePos = { 0, 0 };

	// Rotation around local camera basis
	glm::vec3 cameraRotation = { 0, 0, 0 };
};