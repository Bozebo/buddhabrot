#include "indexbuffer.h"

IndexBuffer::~IndexBuffer(){
	glDeleteBuffers(1, &mBufferID);
}


IndexBuffer::IndexBuffer(GLushort *data, GLsizei count)
	: mCount(count)
{
	glGenBuffers(1, &mBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLushort), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::bind() const{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferID);
}
void IndexBuffer::unbind() const{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

