#pragma once

#include "../libInfinite/tags/Tag.h"
#include "../libInfinite/tags/handles/render_geometryHandle.h"
#include "../libInfinite/logger/logger.h"
#include <glm/glm.hpp>

#include <stdint.h>
#include <string>

class ExporterBase{
public:
	//ExporterBase();
	virtual ~ExporterBase() = default;
	virtual void addRenderGeo(render_geometryHandle* handle, uint32_t index, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::string name) = 0;
	virtual void addRenderGeo(render_geometryHandle* handle, uint32_t index, glm::vec3 position, glm::mat3 rotation, glm::vec3 scale, std::string name, glm::mat4 meshbasetransform = glm::mat4(1.0)) = 0;
	virtual void newScene() = 0;

protected:
	Logger* logger;
};
