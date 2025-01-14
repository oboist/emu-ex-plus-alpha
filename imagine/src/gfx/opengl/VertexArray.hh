#pragma once

#include <imagine/gfx/VertexArray.hh>

namespace IG::Gfx
{

void VertexArray::init(const void *data, uint32_t size)
{
	glGenBuffers(1, &ref);
	logMsg("new VBO id %d", ref);
	glBindBuffer(GL_ARRAY_BUFFER, ref);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

void VertexArray::write(const void *data, uint32_t offset, uint32_t size)
{
	glBindBuffer(GL_ARRAY_BUFFER, ref);
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}

void VertexArray::deinit()
{
	logMsg("deleting VBO id %d", ref);
	glDeleteBuffers(1, &ref);
}

}
