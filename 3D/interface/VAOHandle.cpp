#include "VAOHandle.h"

VAOHandle::VAOHandle(){
	glGenVertexArrays(1,&name);
	setup = false;
}

VAOHandle::~VAOHandle(){
	glDeleteVertexArrays(1,&name);
}
