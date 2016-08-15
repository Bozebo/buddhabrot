#ifndef HEADER_INDEXBUFFER
#define HEADER_INDEXBUFFER

#include <GL/glew.h>


class IndexBuffer{
private:
	GLuint mBufferID;
	GLuint mCount;
public:
	IndexBuffer(GLushort *data, GLsizei count);
	~IndexBuffer();

	void bind() const;
	void unbind() const;

	inline GLuint getCount() const { return mCount; };
};

#endif
