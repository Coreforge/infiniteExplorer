#pragma once
#include "../../libInfinite/tags/handles/baseClasses/GenericHandle.h"

#include "ManagedGLBufferHandle.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <memory>

class VAOHandle : public GenericHandle{
public:
	VAOHandle();
	~VAOHandle();
	GLuint name;
	bool setup;

	std::shared_ptr<ManagedGLBufferHandle> vertexBuffers[19];
	std::shared_ptr<ManagedGLBufferHandle> indexBuffer;

	uint16_t mesh_flags;
	uint8_t index_buffer_type;
};
