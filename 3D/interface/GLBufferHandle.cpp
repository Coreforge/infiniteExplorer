#include "GLBufferHandle.h"



GLBufferHandle::GLBufferHandle(void* data, int size, int stride, void* userptr, std::atomic_uint8_t* indicator){
	//status = BUFFER_IN_USE;
	this->size = size;
	this->stride = stride;

	glGenBuffers(1, &name);
	glNamedBufferData(name, size, data, GL_STATIC_DRAW);
}

GLBufferHandle::~GLBufferHandle(){
	//indicator = 0;
	glDeleteBuffers(1,&name);
}
