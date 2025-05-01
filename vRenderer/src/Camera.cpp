#include "Camera.h"

Camera::Camera()
{
}

Camera::Camera(float fov, float znear, float zfar, int viewportWidth, int viewportHeight, bool flipY)
{
	this->target = glm::vec3(0);
	this->position = glm::vec3(1.0f);
	this->up = glm::vec3(0, 1.0f, 0);
	
	this->fovAngles = fov;
	this->zfar = zfar;
	this->znear = znear;
	this->viewportWidth = viewportWidth;
	this->viewportHeight = viewportHeight;

	this->flipY = flipY;
}

Camera::~Camera()
{
}

void Camera::lookAt(glm::vec3 target)
{
	this->target = target;
}

void Camera::setPosition(glm::vec3 position)
{
	this->position = position;
}

void Camera::setFov(float fov)
{
	this->fovAngles = fov;
}

glm::mat4 Camera::getProjectionMatrix()
{
	glm::mat4 projectionMat = glm::perspective(glm::radians((float)fovAngles), (float)viewportWidth / (float)viewportHeight, znear, zfar);
	if (flipY)
		projectionMat[1][1] *= -1;
	return projectionMat;
}

glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(position, target, up);
}


