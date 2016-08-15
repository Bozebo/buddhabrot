#include "buffer.h"

Buffer::~Buffer(){
	glDeleteBuffers(1, &mBufferID);
}

Buffer::Buffer(GLfloat *data, GLsizei count, GLuint componentCount)
	: mComponentCount(componentCount)
{
	glGenBuffers(1, &mBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, mBufferID);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(GLfloat), data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Buffer::bind() const{
	glBindBuffer(GL_ARRAY_BUFFER, mBufferID);
}
void Buffer::unbind() const{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

