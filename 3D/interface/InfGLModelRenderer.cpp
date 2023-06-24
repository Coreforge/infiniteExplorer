#include "InfGLModelRenderer.h"

#include "VAOHandle.h"

#include <tags/xml/generated/mode.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <cstdio>

int InfGLModelRenderer::ensureBuffersForPart(render_geometry* geo_str, LOD_render_data* lodData, Tag* tag, ytr::ResourceManager* resourceManager){

	GLuint vaoName;

	// indicates that this is a new mesh/LOD, relevant for the VAO and the refcounts
	bool newMesh = false;
	if((lodData->flags_lod_render_flags & INFGLMODELRENDERER_FLAGS_SET_UP) == 0){
		// At least the VAO has to be created, everything else may be fine, or not
		glGenVertexArrays(1,&vaoName);
		glBindVertexArray(vaoName);
		lodData->per_mesh_temporary_ent.block = (per_mesh_temporary*)vaoName;
		newMesh = true;
	}


	uint16_t idx_idx = lodData->index_buffer_index;
	mode::render_geometry* actual_geo_str = ((mode::render_geometry*)geo_str);

	mesh_resource* res_str = actual_geo_str->mesh_package_ent.mesh_resource_groups_ent.block->mesh_resource_res.block;
	pc_index_buffers* idxbuf = &res_str->pc_index_buffers_ent.block[idx_idx];

	// check if the index buffer is loaded or not. If it isn't, do that
	if(idxbuf->ownsD3DResource == 0){
		uint32_t bufsize = idxbuf->count * idxbuf->stride;
		void* bufData = tag->getResource(idxbuf->offset, bufsize);
		GLuint idxBufName;
		glGenBuffers(1,&idxBufName);
		printf("%d, %d\n",__LINE__,glGetError());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,idxBufName);
		printf("%d, %d\n",__LINE__,glGetError());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,bufsize,bufData, GL_STATIC_DRAW);
		printf("%d, %d\n",__LINE__,glGetError());
		printf("GLName: %d Count: %d\n",idxBufName,idxbuf->count);
		idxbuf->m_resource = idxBufName;
		idxbuf->ownsD3DResource = 1;

		// also create the resource for the resource manager
		ytr::GLBufferResource* indexBufRes = new ytr::GLBufferResource(resourceManager,idxBufName);
		RESOURCE_HANDLE_TYPE idxBufResHandle = resourceManager->createResource(indexBufRes);
		// refcount starts at 0, but it's used here, so it has to be increased
		idxbuf->m_resourceView = idxBufResHandle;

	} else {
		if(newMesh){
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,idxbuf->m_resource);
			resourceManager->getResource(idxbuf->m_resourceView)->incRefCount();
		}
	}

	for(int i = 0; i < 19; i++){
		// There are 19 types of vertex buffer, each one gets its own VBO, since it's just easier to do so, even if it's less efficient maybe
		uint16_t vtx_idx = lodData->vertex_buffer_index[i];

		if(vtx_idx == 0xffff){
			continue;
		}
		pc_vertex_buffers* vtxbuf = &res_str->pc_vertex_buffers_ent.block[vtx_idx];
		if(vtxbuf->ownsD3DResource != 0){
			continue;
		}
		printf("Loading vertex buffer %d index %d\n",i,vtx_idx);
		uint32_t bufsize = vtxbuf->count * vtxbuf->stride;
		printf("Buffer size: %d\n",bufsize);
		void* bufData = tag->getResource(vtxbuf->offset, bufsize);
		GLuint vtxBufName;
		glGenBuffers(1,&vtxBufName);
		printf("%d, %d\n",__LINE__,glGetError());
		glBindBuffer(GL_ARRAY_BUFFER, vtxBufName);
		printf("%d, %d\n",__LINE__,glGetError());
		glBufferData(GL_ARRAY_BUFFER,bufsize,bufData,GL_STATIC_DRAW);
		printf("%d, %d\n",__LINE__,glGetError());
		// setup vertex attributes

		printf("%d, %d\n",__LINE__,glGetError());

		printf("GLName: %d Count: %d\n",vtxBufName,vtxbuf->count);
		vtxbuf->m_resource = vtxBufName;
		vtxbuf->ownsD3DResource = 1;

		// also create the resource for the resource manager
		ytr::GLBufferResource* vtxBufRes = new ytr::GLBufferResource(resourceManager,vtxBufName);
		RESOURCE_HANDLE_TYPE vtxBufResHandle = resourceManager->createResource(vtxBufRes);
		// refcount starts at 0, but it's used here, so it has to be increased
		vtxbuf->m_resourceView = vtxBufResHandle;
	}

	// set up the vertex attributes in the VAO. This needs to be done even if the buffers are already loaded, as the VAO for this mesh might not be set up yet
	if(newMesh){
		for(int i = 0; i < 19; i++){
			uint16_t vtx_idx = lodData->vertex_buffer_index[i];
			if(vtx_idx == 0xffff){
				continue;
			}
			pc_vertex_buffers* vtxbuf = &res_str->pc_vertex_buffers_ent.block[vtx_idx];
			GLuint vtxBufName = vtxbuf->m_resource;
			resourceManager->getResource(vtxbuf->m_resourceView)->incRefCount();

			if(i == 0){
				// The vertices are stored as uint16_t and then scaled according to the scale values in the vertex shader
				glBindBuffer(GL_ARRAY_BUFFER, vtxBufName);
				glVertexAttribPointer(0,3,GL_UNSIGNED_SHORT,GL_TRUE,vtxbuf->stride,(void*)0);
				glEnableVertexAttribArray(0);
			}

			if(i == 5){
				glBindBuffer(GL_ARRAY_BUFFER, vtxBufName);
				glVertexAttribPointer(5,4,GL_UNSIGNED_SHORT,GL_TRUE,vtxbuf->stride,(void*)0);
				glEnableVertexAttribArray(5);
			}

		}

		glBindVertexArray(0);
		lodData->flags_lod_render_flags |= INFGLMODELRENDERER_FLAGS_SET_UP;
	}

	return idxbuf->stride;
}



void InfGLModelRenderer::preDraw(LOD_render_data* lodData){
	uint64_t tmp = (uint64_t)lodData->per_mesh_temporary_ent.block;
	tmp = tmp & 0xffffffff;
	GLuint vaoName = tmp;
	glBindVertexArray(vaoName);
}

void InfGLModelRenderer::postDraw(){
	glBindVertexArray(0);
}

void InfGLModelRenderer::drawPart(render_geometry* geo_str, LOD_render_data* lodData, int part){
	mode::render_geometry* actual_geo_str = ((mode::render_geometry*)geo_str);
	uint16_t idx_idx = lodData->index_buffer_index;
	mesh_resource* res_str = actual_geo_str->mesh_package_ent.mesh_resource_groups_ent.block->mesh_resource_res.block;
	pc_index_buffers* idxbuf = &res_str->pc_index_buffers_ent.block[idx_idx];

	parts* p = &lodData->parts_ent.block[part];

	GLenum idxType = GL_UNSIGNED_INT;

	if(idxbuf->stride == 2){
		idxType = GL_UNSIGNED_SHORT;
	} else if(idxbuf->stride == 4){
		idxType = GL_UNSIGNED_INT;
	} else {
		printf("Unsupported vertex Stride: %d\n", idxbuf->stride);
	}
	uint64_t idx_start = p->index_start;
	glDrawElements(GL_TRIANGLES, p->index_count, idxType, (void*)idx_start);
	//glDrawArrays(GL_POINTS, 0, 60);

}
