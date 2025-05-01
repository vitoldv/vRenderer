#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:

	Camera();
	Camera(float fov, float znear, float zfar, int viewportWidth, int viewportHeight, bool flipY = false);
	~Camera();

	void lookAt(glm::vec3 target);
	void setPosition(glm::vec3 position);\
	void setFov(float fov);
	glm::mat4 getProjectionMatrix();
	glm::mat4 getViewMatrix();

private:

	bool flipY;

	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 up;
	
	float fovAngles;
	float znear;
	float zfar;

	int viewportWidth;
	int viewportHeight;
};