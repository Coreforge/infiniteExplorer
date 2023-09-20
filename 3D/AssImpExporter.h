#pragma once

#include "ExporterBase.h"
#include <assimp/scene.h>
#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "AssimpHandle.h"
#include "../libInfinite/StringIDLUT.h"
#include "../libInfinite/BitmapHandle.h"
#include "../libInfinite/module/ModuleManager.h"


#include <vector>
#include <memory>

#include <unordered_map>
#include <unordered_set>

class AssImpExporter : public ExporterBase{
public:
	void newScene();
	void addRenderGeo(render_geometryHandle* handle, uint32_t index, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::string name);
	void addRenderGeo(render_geometryHandle* handle, uint32_t index, glm::vec3 position, glm::mat3 rotation, glm::vec3 scale, std::string name, std::vector<mat_Handle*> materials = std::vector<mat_Handle*>(), glm::mat4 meshbasetransform = glm::mat4(1.0));
	void setLogger(Logger* logger);
	void exportScene(std::string path);
	void exportBitmaps(std::string path, ModuleManager* modman, int mipmap = 0);
	void setStringIDLUT(StringIDLUT lut);
	//AssImpExporter();
private:
	aiScene* scene = nullptr;
	Assimp::Exporter assExporter;
	Assimp::Importer importer;	// only used to create a empty scene

	int addGeoPart(render_geometryHandle* handle, uint32_t index, uint32_t lod, uint32_t start, uint32_t count, unsigned int material = 0);

	unsigned int useMaterial(mat_Handle* matHandle);

	bool initialized = false;

	std::vector<std::shared_ptr<AssimpHandle>> meshinfoHandles;
	std::unordered_map<uint32_t, unsigned int> materialMap;		// global ID to material index
	std::unordered_set<mat_Handle*> usedMaterials;

	std::vector<uint32_t> usedBitmaps;

	StringIDLUT idLUT;


	const char* getParameterName(std::string name);

	class indexReducerMap{
	public:
		std::unordered_map<int, int> indexMap;
		std::vector<int> usedIndicies;

		int getSize();
		int addIndex(int index);
		int getIndex(int index);
	};

	void addUV(render_geometryHandle* handle, bufferInfo& inf, aiMesh* aimesh, indexReducerMap& reducermap, int uvChannel);
};
