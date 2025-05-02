#include "Camera.h"

Camera::Camera()
{
}

Camera::Camera(float fov, float znear, float zfar, int viewportWidth, int viewportHeight, bool flipY)
{
	this->target = glm::vec3(0);
	this->position = glm::vec3(0, 0, 10.0f);
	this->initialCameraPosition = glm::normalize(this->position);
	recalculateVectors();
	
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
	recalculateVectors();
}

void Camera::setPosition(glm::vec3 position)
{
	this->position = position;
	recalculateVectors();
}

void Camera::setFov(float fov)
{
	this->fovAngles = fov;
}

void Camera::setUp(glm::vec3 up)
{
	this->up = up;
	recalculateVectors();
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

void Camera::update()
{
	// Camera rotation
	{
		// If [0, 360] y axis angle is in range from 180 to 270 degrees, up vector should point down so picture wasn't accidentally flipped
		auto val = (abs(cameraRotation.x) + 90) / 180;
		if ((int)val % 2 == 1)
		{
			setUp(glm::vec3(0, -1.0f, 0));
		}
		else
		{
			setUp(glm::vec3(0, 1.0f, 0));
		}

		glm::mat4 rx = glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotation.x), glm::vec3(1.0f, 0, 0));
		glm::mat4 ry = glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotation.y), glm::vec3(0, 1.0f, 0));

		float distanceToTarget = glm::length(position - target);

		position = ry * rx * glm::vec4(initialCameraPosition, 1.0f);
		position *= distanceToTarget;
	}
	recalculateVectors();
}

void Camera::OnMouseScroll(float amount)
{
	float currZoom = glm::length((position - target));
	float newZoom = currZoom - glm::sign(amount) * zoomStep;
	if (newZoom < 0.1f) newZoom = 0.1f;
	position = glm::normalize(position - target) * newZoom;
	recalculateVectors();
}

void Camera::OnMouseMove(int xpos, int ypos, bool pressed)
{
	if (!pressed)
	{
		return;
	}

	// Orbiting camera implementation
	{
		currMousePos = glm::vec2(xpos, ypos);

		glm::vec2 mouseDelta = glm::normalize(currMousePos - prevMousePos);

		if (!glm::isnan(mouseDelta.x) && !glm::isnan(mouseDelta.y))
		{
			cameraRotation.y -= glm::sign(mouseDelta.x) * rotationSpeed * deltaTime;
			cameraRotation.x -= glm::sign(mouseDelta.y) * rotationSpeed * deltaTime;
		}

		prevMousePos = currMousePos;
	}
}


void Camera::recalculateVectors()
{
	forward = glm::normalize(target - position);
	// At this point up vector should point either straight to up or down.
	right = glm::cross(up, forward);
	// Here the correct up vector is calculated.
	up = glm::cross(right, forward);
}


