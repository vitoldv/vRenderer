#include "OrbitCamera.h"

OrbitCamera::OrbitCamera(float fov, float znear, float zfar, int viewportWidth, int viewportHeight, bool flipY)
	: BaseCamera(fov, znear, zfar, viewportWidth, viewportHeight, flipY) 
{
	this->position = CAMERA_INITIAL_POSITION;
	this->target = glm::vec3(0);
	this->up = { 0.0f, 1.0, 0.0f };

	this->initialCameraPosition = glm::normalize(this->position);
	recalculateDirectionVectors();
}

void OrbitCamera::update()
{
	// Camera rotation
	{
		// If [0, 360] y axis angle is in range from 180 to 270 degrees, up vector should point down so picture wasn't accidentally flipped
		auto val = (abs(cameraRotation.x) + 90) / 180;
		if ((int)val % 2 == 1)
		{
			up = { 0, -1.0f, 0 };
		}
		else
		{
			up = { 0, 1.0f, 0 };
		}

		glm::mat4 rx = glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotation.x), glm::vec3(1.0f, 0, 0));
		glm::mat4 ry = glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotation.y), glm::vec3(0, 1.0f, 0));

		float distanceToTarget = glm::length(position - target);

		position = ry * rx * glm::vec4(initialCameraPosition, 1.0f);
		position *= distanceToTarget;
	}

	recalculateDirectionVectors();
}

void OrbitCamera::onMouseScroll(float amount, InputState input)
{
	float currZoom = glm::length((position - target));
	float newZoom = currZoom - glm::sign(amount) * CAMERA_ZOOM_STEP;
	if (newZoom < 1.0f) newZoom = 1.0f;
	position = glm::normalize(position - target) * newZoom;
	recalculateDirectionVectors();
}


void OrbitCamera::onMouseMove(int xpos, int ypos, InputState input)
{
	if (!isPressed(input, InputKeys::MOUSE_LEFT))
	{
		firstClick = false;
		return;
	}

	if(!firstClick)
	{
		firstClick = true;
		prevMousePos = glm::vec2(xpos, ypos);
	}

	// Orbiting camera implementation
	{
		currMousePos = glm::vec2(xpos, ypos);

		glm::vec2 mouseDelta = currMousePos - prevMousePos;

		if (!glm::isnan(mouseDelta.x) && !glm::isnan(mouseDelta.y))
		{
			cameraRotation.y -= mouseDelta.x * CAMERA_ROTATION_SPEED;
			cameraRotation.x -= mouseDelta.y * CAMERA_ROTATION_SPEED;
		}

		prevMousePos = currMousePos;
	}
}

void OrbitCamera::onKey(InputState input)
{

}


