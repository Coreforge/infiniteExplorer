#include "ManagedGLBufferHandle.h"

#include <stdio.h>

ManagedGLBufferHandle::ManagedGLBufferHandle(GLBufferHandle* actualHandle, void* userData){
	// nothing special for now
	initialized = false;
	size = 0;
	count = 0;
	stride = 0;
	name = 0;
}

ManagedGLBufferHandle::ManagedGLBufferHandle(){
	initialized = false;
	size = 0;
	count = 0;
	stride = 0;
	name = 0;
}

void ManagedGLBufferHandle::setup(int size, int count, int stride, void* data){
	if(name != 0){
		// This is not intended to happen, but this should prevent buffers from being leaked
		printf("ManagedGLBufferHandle: setup called when a buffer was already assigned!\n");
		glDeleteBuffers(1,&name);
	}
	this->size = size;
	this->count = count;
	this->stride = stride;
	glCreateBuffers(1,&name);
	glNamedBufferData(name, size, data, GL_STATIC_DRAW);
	initialized = true;
}

ManagedGLBufferHandle::~ManagedGLBufferHandle(){
	// no GC for now, just delete
	//delete handle;
	if(name != 0){
		glDeleteBuffers(1,&name);
	}
}
