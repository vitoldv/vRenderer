#pragma once

namespace VRD::Scene
{
	template<typename TemplateType>
	class ITemplateInstance
	{

	public:

		virtual void createFromTemplate(const TemplateType& templObj) = 0;
		virtual const TemplateType& getTemplate() const = 0;

	};
}
