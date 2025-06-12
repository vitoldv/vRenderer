#pragma once

class ISceneInstanceTemplate
{
public:

	const uint32_t id;
	const std::string name;

	ISceneInstanceTemplate(uint32_t id, std::string name) : id(id), name(name) {}
	virtual ~ISceneInstanceTemplate() {}
};