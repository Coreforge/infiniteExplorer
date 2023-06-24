#pragma once

#include <yttriumGL.h>
#include <yttrium.h>
#include "../../libInfinite/models/Rendermodel.h"
#include "InfGLModelRenderer.h"
#include <GL/glew.h>

class InfMeshResource : public ytr::Resource{
public:
	void setVertexArrayAttribPointers();
	void incRefCount();
	void decRefCount();
	InfMeshResource(render_geometry* geo_str, LOD_render_data* lodData, Tag* tag);

	void bindForDraw();
	void unbindAfterDraw();
	void draw(int part);

	//LODMesh* mesh;
private:

	render_geometry* geo_str;
	LOD_render_data* lodData;
	Tag* tag;
	//ytr::GLBufferResource* vertexBuffer;
	//GLuint vao;
};
