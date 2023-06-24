#pragma once

#include <yttrium.h>
#include <glm/glm.hpp>
#include "InfMeshResource.h"

#include "ManagedGLBufferHandle.h"
#include "VAOHandle.h"

#include "../../libInfinite/tags/handles/modeHandle.h"

#include <stdint.h>

#include <vector>
#include <memory>

struct meshes;

class InfMesh : public ytr::MeshBase{
public:
	void setupData(int lod = -1);
	void setupShader();
	void draw(ytr::Camera* camera, ytr::Transform transform);

	GLuint shaderId = 0;

	InfMesh(render_geometry* geo_str, meshes* mesh, Tag* tag, ytr::ResourceManager* resourceManager);
	InfMesh(render_geometryHandle* hndl, uint32_t meshIndex);

private:

	void setupMesh(uint32_t meshIndex, uint32_t lod);

	//InfMeshResource* meshResource;
	render_geometry* geo_str;
	meshes* mesh;
	Tag* tag;
	glm::mat4 posCompression;

	render_geometryHandle* hndl;
	uint32_t meshIndex;

	bool usesNewStuff;

	//std::vector<std::shared_ptr<ManagedGLBufferHandle>> buffers;
	//std::shared_ptr<ManagedGLBufferHandle> vertexBuffers[19];
	//std::shared_ptr<ManagedGLBufferHandle> indexBuffer;
	std::shared_ptr<VAOHandle> vao;
	bool hasValidSetup;
	std::vector<partInfo> parts;

	ytr::ResourceManager* resourceManager;

};
