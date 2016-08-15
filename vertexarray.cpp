#include "vertexarray.h"


VertexArray::VertexArray(){
	glGenVertexArrays(1, &mArrayID);
}

VertexArray::~VertexArray(){
	for(GLuint i = 0; i < mBuffers.size(); i++)
		delete mBuffers[i];

	glDeleteVertexArrays(1, &mArrayID);
}

void VertexArray::addBuffer(Buffer *buffer, GLuint index){
	bind();
	buffer->bind();

	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, buffer->getComponentCount(), GL_FLOAT, GL_FALSE, 0, 0);

	buffer->unbind();
	unbind();
}

void VertexArray::bind() const{
	glBindVertexArray(mArrayID);
}

void VertexArray::unbind() const{
	glBindVertexArray(0);
}
