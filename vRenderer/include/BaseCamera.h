#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include "AppContext.h"

#include "editor_settings.h"

/// <summary>
/// Base camera abstract class.
/// </summary>
class BaseCamera
{
public:

	BaseCamera() = default;
	BaseCamera(float fov, float znear, float zfar, int viewportWidth, int viewportHeight, bool flipY = false);
	~BaseCamera() = default;

	void lookAt(glm::vec3 target);
	void setPosition(glm::vec3 position);
	void setFov(float fov);
	void setUp(glm::vec3 up);
	
	glm::mat4 getProjectionMatrix();
	glm::mat4 getViewMatrix();

	virtual void update() = 0;
	virtual void onMouseScroll(float amount) = 0;
	virtual void onMouseMove(int xpos, int ypos, bool pressed) = 0;

protected:

	bool flipY;

	glm::vec3 position;
	glm::vec3 target;

	glm::vec3 up;
	glm::vec3 forward;
	glm::vec3 right;

	float fovAngles;
	float znear;
	float zfar;

	int viewportWidth;
	int viewportHeight;

	glm::vec2 prevMousePos;
	glm::vec2 currMousePos;

	glm::vec3 initialCameraPosition;
	glm::vec3 cameraRotation = { 0, 0, 0 };

	void recalculateVectors();
};

// TODO
// 1. Get rid of redundant direction vectors recalculation (recalculateVectors() calls);
// 2. Remove initialCameraPosition vector and alter corresponding logic;