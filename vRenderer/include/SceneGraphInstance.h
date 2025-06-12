#pragma once

#include <cstdint>
#include <string>

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/quaternion.hpp>

namespace VRD::Scene
{
	class SceneGraphInstance
	{
	public:

		const uint32_t id;
		const std::string printName;

		SceneGraphInstance(uint32_t id, std::string printName) : id(id), printName(printName) {}
		virtual ~SceneGraphInstance() {}

		glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

		inline glm::mat4 getTransformMat() const
		{
			glm::mat4 t = glm::mat4(1.0f);
			t = glm::scale(t, scale);
			t = glm::rotate(t, rotation.x, glm::vec3(1.0f, 0, 0));
			t = glm::rotate(t, rotation.y, glm::vec3(0, 1.0f, 0));
			t = glm::rotate(t, rotation.z, glm::vec3(0, 0, 1.0f));
			t = glm::translate(t, position);
			return t;
		}
	};
}