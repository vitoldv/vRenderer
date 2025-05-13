#include "BaseCamera.h"

BaseCamera::BaseCamera(float fov, float znear, float zfar, int viewportWidth, int viewportHeight, bool flipY)
{
	this->fovAngles = fov;
	this->zfar = zfar;
	this->znear = znear;
	this->viewportWidth = viewportWidth;
	this->viewportHeight = viewportHeight;

	this->flipY = flipY;
}

void BaseCamera::lookAt(glm::vec3 target)
{
	this->target = target;
	recalculateDirectionVectors();
}

void BaseCamera::setPosition(glm::vec3 position)
{
	this->position = position;
	recalculateDirectionVectors();
}

void BaseCamera::setFov(float fov)
{
	this->fovAngles = fov;
}

glm::vec3 BaseCamera::getForward() const
{
	return forward;
}

glm::vec3 BaseCamera::getRight() const
{
	return right;
}

glm::vec3 BaseCamera::getUp() const
{
	return up;
}

glm::vec3 BaseCamera::getPosition() const
{
	return position;
}

glm::mat4 BaseCamera::getProjectionMatrix() const
{
	glm::mat4 projectionMat = glm::perspective(glm::radians((float)fovAngles), (float)viewportWidth / (float)viewportHeight, znear, zfar);
	if (flipY)
		projectionMat[1][1] *= -1;
	return projectionMat;
}

glm::mat4 BaseCamera::getViewMatrix() const
{
	return glm::lookAt(position, target, up);
}

void BaseCamera::recalculateDirectionVectors()
{
	forward = glm::normalize(target - position);
	// At this point up vector should point either straight to up or down.
	right = glm::cross(forward, up);
	// Here the correct up vector is calculated.
	up = glm::cross(right, forward);
}


