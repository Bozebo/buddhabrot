#ifndef HEADER_VERTEXARRAY
#define HEADER_VERTEXARRAY

#include "buffer.h"

#include <vector>
#include <GL/glew.h>


class VertexArray{
private:
	GLuint mArrayID;
	std::vector<Buffer*> mBuffers;
public:
	VertexArray();
	~VertexArray();

	void bind() const;
	void unbind() const;

	void addBuffer(Buffer *buffer, GLuint index);
		

};

#endif
