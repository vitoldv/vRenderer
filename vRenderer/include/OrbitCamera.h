#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include "AppContext.h"
#include "BaseCamera.h"

#include "editor_settings.h"

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
	void onMouseScroll(float amount) override;
	void onMouseMove(int xpos, int ypos, bool pressed) override;
};

// TODO
// 1. Get rid of redundant direction vectors recalculation (recalculateVectors() calls);
// 2. Remove initialCameraPosition vector and alter corresponding logic;