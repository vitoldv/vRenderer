#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include "AppContext.h"

#include "editor_settings.h"
#include "input_handler.h"

using namespace inp;

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
	
	glm::vec3 getForward() const;
	glm::vec3 getRight() const;
	glm::vec3 getUp() const;
	glm::vec3 getPosition() const;

	glm::mat4 getProjectionMatrix() const;
	glm::mat4 getViewMatrix() const;

	virtual void update() = 0;
	virtual void onMouseScroll(float amount, InputState input) = 0;
	virtual void onMouseMove(int xpos, int ypos, InputState input) = 0;
	virtual void onKey(InputState input) = 0;

protected:

	bool flipY;

	float fovAngles;
	float znear;
	float zfar;

	int viewportWidth;
	int viewportHeight;

	glm::vec3 position;
	glm::vec3 target;

	glm::vec3 up;
	glm::vec3 forward;
	glm::vec3 right;

	virtual void recalculateDirectionVectors();
};

// TODO
// 1. Get rid of redundant direction vectors recalculation (recalculateVectors() calls);
// 2. Remove initialCameraPosition vector and alter corresponding logic;