#include "InfMesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <tags/xml/generated/mode.h>

#include <cstdio>

void InfMesh::setupData(int lod){
	//meshResource->bindForDraw();
}

void InfMesh::setupShader(){
	glUseProgram(shaderId);
	//printf("Shader ID: %d\n",shaderId);
}

GLenum getDrawMode(uint8_t indexBufferType){
	switch(indexBufferType){
	case INDEX_BUFFER_TYPE_DEFAULT:
		return GL_TRIANGLES;
	case INDEX_BUFFER_TYPE_LINE_LIST:
		return GL_LINES;
	case INDEX_BUFFER_TYPE_LINE_STRIP:
		return GL_LINE_STRIP;
	case INDEX_BUFFER_TYPE_QUAD_LIST:
		return GL_POINTS;	// I don't think GL really supports this
	case INDEX_BUFFER_TYPE_TRIANGLE_LIST:
		return GL_TRIANGLES;
	case INDEX_BUFFER_TYPE_TRIANGLE_PATCH:
		return GL_PATCHES;
	case INDEX_BUFFER_TYPE_TRIANGLE_STRIP:
		return GL_TRIANGLE_STRIP;
	default:
		return GL_POINTS;
	}
}

void InfMesh::draw(ytr::Camera* camera, ytr::Transform transform){
	glm::mat4 mat(1.0f);
	// glm::rotate(glm::radians(-90.0f), glm::vec3(1.0,0.0,0.0))
	mat = posCompression; // decompress the mesh (and rotate it to align with this coordinate system, which differs from the one infinite uses)
	glm::mat4 scalemat(1.0f);
	scalemat = glm::scale(transform.scale);
	glm::mat4 rotmat(1.0f);
	if(transform.rotationType == transform.ROTATION_EULER){
		rotmat = glm::rotate(rotmat, glm::radians(transform.rotation.x), glm::vec3(1.0,0.0,0.0));
		rotmat = glm::rotate(rotmat, glm::radians(transform.rotation.y), glm::vec3(0.0,1.0,0.0));
		rotmat = glm::rotate(rotmat, glm::radians(transform.rotation.z), glm::vec3(0.0,0.0,1.0));
	} else {
		rotmat = transform.rotationMat;
	}
	glm::mat4 transmat(1.0f);
	transmat = glm::translate(transmat, transform.position);
	mat = transmat * rotmat * scalemat * mat;// * camera->projection);

	//GLint trans_loc = glGetUniformLocation(shaderId,"transmat");
	glUniformMatrix4fv(20,1,GL_FALSE,glm::value_ptr(camera->projection));
	glUniformMatrix4fv(21,1,GL_FALSE,glm::value_ptr(camera->renderMatrix));
	glUniformMatrix4fv(22,1,GL_FALSE,glm::value_ptr(mat));

	if(!usesNewStuff){
		LOD_render_data* loddata = &mesh->LOD_render_data_ent.block[0];
		InfGLModelRenderer::ensureBuffersForPart(geo_str, loddata, tag, resourceManager);

		InfGLModelRenderer::preDraw(loddata);
		for(int part = 0; part < loddata->parts_ent.count; part++){
			InfGLModelRenderer::drawPart(geo_str, loddata, part);
			//glDrawElements(GL_TRIANGLES, meshResource->mesh->parts[part].indexCount/3, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(meshResource->mesh->IndexBuffer.offset));
		}
		InfGLModelRenderer::postDraw();
	} else {
		// This is the fancy new way
		if(!hasValidSetup){
			printf("Trying to draw InfMesh without a valid Setup!\n");
			// set it up now, though this really isn't that great
			setupMesh(meshIndex, 0);
		}
		if(hasError){
			return;
		}
		// I'll have to abstract this once (or if) I add vulkan, but for now this is fine
		glBindVertexArray(vao->name);

		GLenum idxType = GL_UNSIGNED_INT;
		if(!(vao->mesh_flags & MESH_FLAGS_MESH_IS_UNINDEXED_DO_NOT_MODIFY)){
			// only dp this if there even is an index buffer
			if(vao->indexBuffer->stride == 2){
				idxType = GL_UNSIGNED_SHORT;
			} else if(vao->indexBuffer->stride == 4){
				idxType = GL_UNSIGNED_INT;
			} else {
				printf("Unsupported vertex Stride: %d\n", vao->indexBuffer->stride);
			}
		}

		for(int part = 0; part < parts.size(); part++){
			uint64_t start = parts[part].indexStart;
			if(!(vao->mesh_flags & MESH_FLAGS_MESH_IS_UNINDEXED_DO_NOT_MODIFY)){
				glDrawElements(getDrawMode(vao->index_buffer_type), parts[part].indexCount, idxType, (void*)start);
			} else {
				// no index buffer
				glDrawArrays(getDrawMode(vao->index_buffer_type), start, parts[part].indexCount);
			}
		}
		glBindVertexArray(0);
	}
}

void InfMesh::setupMesh(uint32_t meshIndex, uint32_t lod){
	vao = hndl->getMeshInfo<VAOHandle>(meshIndex, lod);

	if(vao->setup){
		/*
		 * The VAO stores the index buffer binding and vertex buffer bindings, so those aren't interesting anymore
		 * Since the VAOHandle also stores shared_ptrs to all the buffer handles that get used, the InfMesh doesn't have to track those either
		 * So the only thing needed (for a raw mesh at least, without materials so far) is to store a shared_ptr to the VAOHandle, and that should keep everything else valid
		 */
		printf("VAO is already setup for this LOD/Mesh. Not doing anything\n");
		hasValidSetup = true;
		return;
	}

	glBindVertexArray(vao->name);

	// Index Buffer
	uint16_t idx_idx = hndl->getIndexBufferIndex(meshIndex, lod);
	if(idx_idx == 0xffff){
		printf("Invalid Index Buffer!\n");
		glBindVertexArray(0);
		hasError = true;
		vao->setup = true;
		return;
	}

	vao->mesh_flags = hndl->getMeshFlags(meshIndex);
	vao->index_buffer_type = hndl->getIndexBufferType(meshIndex);

	if(!(vao->mesh_flags & MESH_FLAGS_MESH_IS_UNINDEXED_DO_NOT_MODIFY)){
		// hey, there's actually an index buffer!
		vao->indexBuffer = hndl->getIndexBufferInfo<ManagedGLBufferHandle>(idx_idx);
		if(!vao->indexBuffer->initialized){
			bufferInfo inf = hndl->getIndexBuffer(idx_idx);
			vao->indexBuffer->setup(inf.size, inf.count, inf.stride, inf.data);
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vao->indexBuffer->name);
	}

	int vertCount = 0;
	// Vertex buffers
	for(int i = 0; i < 19; i++){
		uint16_t vtx_idx = hndl->getVertexBufferIndex(meshIndex, lod, i);
		if(vtx_idx == 0xffff){
			vao->vertexBuffers[i] = nullptr;
			continue;
		}
		//auto vp = hndl->geoHandle.getVertexBuffer<ManagedGLBufferHandle,GLBufferHandle>(vtx_idx, nullptr);
		//buffers.emplace_back(vp);	// the order doesn't really matter, this is just to keep the reference and refcount

		vao->vertexBuffers[i] = hndl->getVertexBufferInfo<ManagedGLBufferHandle>(vtx_idx);
		if(!vao->vertexBuffers[i]->initialized){
			// buffer needs to be created
			auto inf = hndl->getVertexBuffer(vtx_idx);
			vao->vertexBuffers[i]->setup(inf.size, inf.count, inf.stride, inf.data);
		}
		if((vao->mesh_flags & MESH_FLAGS_MESH_IS_UNINDEXED_DO_NOT_MODIFY) && vertCount == 0){
			auto inf = hndl->getVertexBuffer(vtx_idx);
			vertCount = inf.count;
		}


		switch(i){
		case 0:
			glBindBuffer(GL_ARRAY_BUFFER, vao->vertexBuffers[i]->name);
			glVertexAttribPointer(0,3,GL_UNSIGNED_SHORT,GL_TRUE,vao->vertexBuffers[i]->stride,(void*)0);
			glEnableVertexAttribArray(0);
			break;
		case 5:
			glBindBuffer(GL_ARRAY_BUFFER, vao->vertexBuffers[i]->name);
			glVertexAttribPointer(5,3,GL_UNSIGNED_SHORT,GL_TRUE,vao->vertexBuffers[i]->stride,(void*)0);
			glEnableVertexAttribArray(5);
			break;
		default:
			break;
		}

	}

	vao->setup = true;
	glBindVertexArray(0);

	// save part info
	int pCount = hndl->getPartCount(meshIndex, lod);
	parts.clear();
	parts.reserve(pCount);
	for(int i = 0; i < pCount; i++){
		parts.emplace_back(hndl->getPartInfo(meshIndex, lod, i));
	}
	if(pCount == 0){
		// add a single part for all vertices
		partInfo pInfo;
		pInfo.indexStart = 0;
		pInfo.indexCount = vertCount;
		pInfo.materialIndex = 0;
		parts.emplace_back(pInfo);
	}

	hasValidSetup = true;
}

InfMesh::InfMesh(render_geometry* geo_str, meshes* mesh, Tag* tag, ytr::ResourceManager* resourceManager){
	this->geo_str = geo_str;
	this->mesh = mesh;
	this->tag = tag;
	this->resourceManager = resourceManager;
	usesNewStuff = false;
	shaderId = 0;

	point3D* minP = &((mode::render_geometry*)geo_str)->compression_info_ent.block->position_bounds_0;
	point3D* maxP = &((mode::render_geometry*)geo_str)->compression_info_ent.block->position_bounds_1;

	glm::vec3 scl(minP->y - minP->x,maxP->x - minP->z,maxP->z - maxP->y);
	glm::vec3 offs(minP->x, minP->z, maxP->y);
	//glm::vec3 scl(minP->x, minP->y, minP->z);
	//glm::vec3 offs(maxP->x, maxP->y, maxP->z);
	posCompression = glm::mat4(1.0f);
	posCompression = glm::scale(posCompression, scl);
	posCompression = glm::translate(posCompression, offs);
	hasError = false;
}

InfMesh::InfMesh(render_geometryHandle* hndl, uint32_t meshIndex){
	usesNewStuff = true;
	this->hndl = hndl;
	this->meshIndex = meshIndex;

	float posComp[3];
	hndl->positionCompressionOffset(posComp);
	float posCompScale[3];
	hndl->positionCompressionScale(posCompScale);
	glm::vec3 scl(posCompScale[0],posCompScale[1],posCompScale[2]);
	glm::vec3 offs(posComp[0],posComp[1],posComp[2]);

	posCompression = glm::mat4(1.0f);
	posCompression = glm::scale(posCompression, scl);
	posCompression = glm::translate(posCompression, offs);
	hasValidSetup = false;
	hasError = false;
}
