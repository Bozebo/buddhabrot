#ifndef HEADER_BUFFER
#define HEADER_BUFFER

#include <GL/glew.h>


class Buffer{
private:
	GLuint mBufferID;
	GLuint mComponentCount;
public:
	Buffer(GLfloat *data, GLsizei count, GLuint componentCount);
	~Buffer();

	void bind() const;
	void unbind() const;

	inline GLuint getComponentCount() const { return mComponentCount; };
};

#endif
