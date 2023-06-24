#pragma once

#include "../../libInfinite/tags/handles/baseClasses/BufferHandleBase.h"
#include <atomic>

#include <GL/glew.h>
#include <GL/gl.h>

class GLBufferHandle{
public:
	GLBufferHandle(void* data, int size, int stride, void* userptr, std::atomic_uint8_t* indicator);
	~GLBufferHandle();


	GLuint name;

	int size;
	int stride;
};
