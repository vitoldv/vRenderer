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
		virtual ~ModelInstance() {}

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