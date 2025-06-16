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
		SceneGraphInstance(uint32_t id, std::string printName, const SceneGraphInstance& other) :
			id(id), printName(printName)
		{
			position = other.position;
			rotation = other.rotation;
			scale = other.scale;
		}
		virtual ~SceneGraphInstance() {}

		SceneGraphInstance(const SceneGraphInstance& other) = delete;
		SceneGraphInstance& operator=(const SceneGraphInstance& other) = delete;
		
		virtual SceneGraphInstance* clone(uint32_t id, std::string printName) const
		{
			return new SceneGraphInstance(id, printName, *this);
		}

		glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

		inline glm::mat4 getTransformMat() const
		{
			glm::mat4 t = glm::translate(glm::mat4(1.0f), position);
			glm::mat4 rx = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0, 0));
			glm::mat4 ry = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0, 1.0f, 0));
			glm::mat4 rz = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0, 0, 1.0f));
			glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
			return t * rz * ry * rx * s;
		}
	};
}