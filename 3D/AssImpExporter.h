#pragma once

#include "ExporterBase.h"
#include <assimp/scene.h>
#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "AssimpHandle.h"

#include <vector>
#include <memory>

class AssImpExporter : public ExporterBase{
public:
	void newScene();
	void addRenderGeo(render_geometryHandle* handle, uint32_t index, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::string name);
	void addRenderGeo(render_geometryHandle* handle, uint32_t index, glm::vec3 position, glm::mat3 rotation, glm::vec3 scale, std::string name, glm::mat4 meshbasetransform = glm::mat4(1.0));
	void setLogger(Logger* logger);
	void exportScene(std::string path);
	//AssImpExporter();
private:
	aiScene* scene = nullptr;
	Assimp::Exporter assExporter;
	Assimp::Importer importer;	// only used to create a empty scene

	bool initialized = false;

	std::vector<std::shared_ptr<AssimpHandle>> meshinfoHandles;
};
