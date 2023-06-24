#pragma once

#include "../../libInfinite/tags/handles/baseClasses/GenericHandle.h"
#include "GLBufferHandle.h"

#include <GL/glew.h>
#include <GL/gl.h>

class ManagedGLBufferHandle : public GenericHandle{
public:
	ManagedGLBufferHandle(GLBufferHandle* actualHandle, void* userData);
	ManagedGLBufferHandle();
	~ManagedGLBufferHandle();

	void setup(int size, int count, int stride, void* data);

	bool initialized;

	int size, count, stride;

	// unlikely to stay this way, but this way it works without any central resource manager
	GLuint name;
};
