#include "FpvCamera.h"

FpvCamera::FpvCamera(float fov, float znear, float zfar, int viewportWidth, int viewportHeight, bool flipY)
	: BaseCamera(fov, znear, zfar, viewportWidth, viewportHeight, flipY)
{
	this->position = CAMERA_INITIAL_POSITION;
	this->target = glm::vec3(0);
	this->up = { 0, 1.0f, 0 };

	recalculateDirectionVectors();
}

void FpvCamera::update()
{
	// empty
}

void FpvCamera::onMouseScroll(float amount, InputState input)
{
	// empty
}

void FpvCamera::onMouseMove(int xpos, int ypos, InputState input)
{
	// FPV camera implementation
	{

		currMousePos = { static_cast<float>(xpos), static_cast<float>(ypos) };

		if (firstTime)
		{
			firstTime = false;
			prevMousePos = currMousePos;
			return;
		}

		glm::vec2 mouseDelta = currMousePos - prevMousePos;
		prevMousePos = currMousePos;

		bool pressed = input & InputKeys::MOUSE_LEFT;
		if (!pressed)
		{
			return;
		}

		if (!glm::isnan(mouseDelta.x) && !glm::isnan(mouseDelta.y))
		{
			cameraRotation.y -= mouseDelta.x * CAMERA_FPV_SENSETIVITY_HORIZONTAL * AppContext::instance().deltaTime;
			cameraRotation.x -= mouseDelta.y * CAMERA_FPV_SENSETIVITY_VERTICAL * AppContext::instance().deltaTime;
		}
		
		// constrain pitch to avoid camera flip
		if (cameraRotation.x > PITCH_LIMIT_TOP)
			cameraRotation.x = PITCH_LIMIT_TOP;
		if (cameraRotation.x < PITCH_LIMIT_BOTTOM)
			cameraRotation.x = PITCH_LIMIT_BOTTOM;

		glm::mat4 rx = glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotation.x), glm::vec3(1.0f, 0, 0));
		glm::mat4 ry = glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotation.y), glm::vec3(0, 1.0f, 0));

		// rotation should be applied to initial forward vector state
		forward = { 0, 0, -1.0f };
		forward = glm::normalize(ry * rx * glm::vec4(forward, 1.0f));
		
	}
	
	recalculateDirectionVectors();
}

void FpvCamera::onKey(InputState input)
{
	float cameraSpeed = isPressed(input, InputKeys::KEY_SHIFT) ? CAMERA_FPV_INCREASED_SPEED :  CAMERA_FPV_SPEED;
	cameraSpeed *= AppContext::instance().deltaTime;
	if (isPressed(input, InputKeys::KEY_W))
		position += cameraSpeed * forward;
	if (isPressed(input, InputKeys::KEY_S))
		position -= cameraSpeed * forward;
	if (isPressed(input, InputKeys::KEY_A))
		position -= cameraSpeed * right;
	if (isPressed(input, InputKeys::KEY_D))
		position += cameraSpeed * right;
	if (isPressed(input, InputKeys::KEY_E))
		position += cameraSpeed * up;
	if (isPressed(input, InputKeys::KEY_Q))
		position -= cameraSpeed * up;
}

void FpvCamera::recalculateDirectionVectors()
{
	target = position + forward;
	// forward vector is defined in mouse handler
	// up vector always remains the same and points up
	right = glm::normalize(glm::cross(forward, up));
}