#include "InfMeshResource.h"

//#include <../yttrium/GLRenderer/GLIncludes.h>
#include <GL/glew.h>

void InfMeshResource::incRefCount(){
	refCount++;
}

void InfMeshResource::decRefCount(){
	refCount--;
}

InfMeshResource::InfMeshResource(render_geometry* geo_str, LOD_render_data* lodData, Tag* tag){
	this->geo_str = geo_str;
	this->lodData = lodData;
	this->tag = tag;
	//InfGLModelRenderer::ensureBuffersForPart(geo_str, lodData, tag);
	/*this->mesh = mesh;
	this->vertexBuffer = vertexBuffer;
	glGenVertexArrays(1,&vao);
	glBindVertexArray(vao);

	vertexBuffer->bind(GL_ARRAY_BUFFER);
	vertexBuffer->bind(GL_ELEMENT_ARRAY_BUFFER);

	// just position so far, and this isn't very portable, but whatever, idc
	uint64_t pos_off = mesh->VertexBuffers[0].offset;
	glVertexAttribPointer(0,4,GL_UNSIGNED_SHORT,GL_TRUE,mesh->VertexBuffers[0].stride,(const void*)pos_off);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);*/
}

void InfMeshResource::bindForDraw(){
	//glBindVertexArray(vao);
}

void InfMeshResource::unbindAfterDraw(){
	//glBindVertexArray(0);
}

void InfMeshResource::draw(int part){
	//glDrawElements(GL_TRIANGLES, mesh->parts[part].indexCount/3, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(mesh->IndexBuffer.offset));
}
