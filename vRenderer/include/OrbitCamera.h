#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include "AppContext.h"
#include "BaseCamera.h"

/// <summary>
/// Orbiting camera implementation.
/// </summary>
class OrbitCamera : public BaseCamera
{
public:

	OrbitCamera() = default;
	OrbitCamera(float fov, float znear, float zfar, int viewportWidth, int viewportHeight, bool flipY = false);
	~OrbitCamera() = default;

	void update() override;
	void onMouseScroll(float amount, InputState input) override;
	void onMouseMove(int xpos, int ypos, InputState input) override;
	void onKey(InputState input) override;

private:

	bool firstClick = false;
	glm::vec2 prevMousePos = { 0, 0 };
	glm::vec2 currMousePos = { 0, 0 };

	glm::vec3 initialCameraPosition;
	// Rotation around camera target point (orbiting point)
	glm::vec3 cameraRotation = { 0, 0, 0 };
};