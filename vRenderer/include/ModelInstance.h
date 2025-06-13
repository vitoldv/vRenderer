#pragma once

#include <memory>

#include "Model.h"
#include "SceneGraphInstance.h"
#include "ITemplateInstance.h"

namespace VRD::Scene
{
	class ModelInstance : public SceneGraphInstance, public ITemplateInstance<Model>
	{
	public:

		ModelInstance(uint32_t id, std::string printName) : SceneGraphInstance(id, printName) {}
		ModelInstance(uint32_t id, std::string printName, const ModelInstance& other) :
			SceneGraphInstance(id, printName, dynamic_cast<const SceneGraphInstance&>(other))
		{
			modelTemplate = other.modelTemplate;
		}
		virtual ~ModelInstance() {}

		ModelInstance(const ModelInstance& other) = delete;
		ModelInstance& operator=(const ModelInstance& other) = delete;

		virtual ModelInstance* clone(uint32_t id, std::string printName) const
		{
			return new ModelInstance(id, printName, *this);
		}

		const Model& getTemplate() const
		{
			return *modelTemplate;
		}

		void createFromTemplate(const Model& templObj) override
		{
			modelTemplate.reset(&templObj);
		}

	private:

		std::shared_ptr<const Model> modelTemplate = nullptr;
	};
}