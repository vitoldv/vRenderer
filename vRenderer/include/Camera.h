#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include "editor_settings.h"

class Camera
{
public:

	Camera();
	Camera(float fov, float znear, float zfar, int viewportWidth, int viewportHeight, bool flipY = false);
	~Camera();

	// Temporal
	float deltaTime = 0;

	void lookAt(glm::vec3 target);
	void setPosition(glm::vec3 position);
	void setFov(float fov);
	void setUp(glm::vec3 up);
	glm::mat4 getProjectionMatrix();
	glm::mat4 getViewMatrix();

	void update();

	void OnMouseScroll(float amount);
	void OnMouseMove(int xpos, int ypos, bool pressed);

private:

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