#ifndef HEADER_TEXTRENDERER
#define HEADER_TEXTRENDERER

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>

#include <map>
#include <vector>


#include "tinyxml2.h"

#include "TextLine.h"

class TextRenderer{

public:
	
	TextRenderer(std::vector<TextLine*> *sTextLines){
		init = false;

		numIndices = 0;

		//padding = spacing = 0;

		//lineHeight = base = 0;
		
		scaleW = scaleH = 0;

		//translation matrix is constant, shift origin to top left
		translation = maths::mat4::translation(maths::vec3(-1.f, 1.f, 0.f));

		SetWindowSize(768, 768);

		textLines = sTextLines;
		
		indices = nullptr;
		characterVertices = nullptr;
	}

	~TextRenderer(){

		//todo: clean up text bodies
		
		glDeleteBuffers(1, &characterBuffer);
		glDeleteBuffers(1, &indexBuffer);

		glDeleteVertexArrays(1, &mVAO);

	}

	//load a font file (from libgdx's hiero tool converted to xml and slightly simplified). Return value of 1 means success.
	int LoadFont(const char* path);

	//set a text string without any wrapping or bounds checks.
	void TestTextLine(float x, float y, float r, float g, float b, float size, const char* text);

	//draws the text line, returns true if anything was to be drawn
	bool Draw();

	void SetWindowSize(int width, int height){
		windowWidth = width;
		windowHeight = height;

		GLfloat fWidth = float(width);
		GLfloat fHeight = float(height);
		
		GLfloat halfWidth = fWidth/2.f;
		GLfloat halfHeight = fHeight/2.f;

		GLfloat aspect = float(width)/float(height);
		
		transformation =
			translation * //first, translate left and up, in screen space
			maths::mat4::orthographic(-halfWidth, halfWidth, -halfHeight, halfHeight, -2, 2); //and then apply orthographic projection

	}

private:
	bool init;

	int windowWidth;
	int windowHeight;

	GLint uiTransformUniform;
	maths::mat4 translation; //translation in eye space
	maths::mat4 transformation; //transformation matrix sent to UI shader

	//int padding;
	//int spacing;

	//int lineHeight;
	//int base;
	int scaleW;
	int scaleH;

	const char* pageFile; //path for the character page's raw image file
	std::vector<char>* pageData; //data loaded from the image file

	//holds data for a parsed character, relative to it's texture file
	struct Chr{
		char id;
		
		GLfloat atlasTop;
		GLfloat atlasLeft;
		GLfloat atlasBottom;
		GLfloat atlasRight;
		GLfloat width;
		GLfloat height;

		//offset from the "cursor" and base line
		GLfloat xOffset;
		GLfloat yOffset;

		GLfloat xAdvance; //x distance to advance to place the next character

		std::map<char, GLfloat> kernings;

	};
	std::map<char, Chr*> chars; //store characters mapped by their char


	//bodies of text to be rendered
	std::vector<TextLine*> *textLines;
	void ConsumeTextLines(); //will iterate over textLines

	//OpenGL

	const GLchar* VertShader = 
	{
		"#version 420\n"\
			
		"layout(location=0) in vec2 in_position;\n"\
		"layout(location=1) in vec2 in_texCoords;\n"\
		"layout(location=2) in vec4 in_textCol;\n"\
		
		//"uniform vec2 uiScale\n;"\

		//"uniform mat3 uiScale\n;"\

		"uniform mat4 uiScale\n;"\

		"smooth out vec2 pass_texCoords;\n"\
		"smooth out vec4 pass_textCol;\n"\

		"void main(void){\n"\
		
		//"   gl_Position = vec4(in_position * uiScale, 0.0, 1.0);\n"\
		
		//"   gl_Position = vec4(uiScale * vec3(in_position, 1.0f), 1.0f);\n"\

		"   gl_Position = uiScale * vec4(in_position, 1.0f, 1.0f);\n"\
		
		"	pass_texCoords = in_texCoords;"\
		"	pass_textCol = in_textCol;"\

		"}\n"
	};
	
	const GLchar* FragShader = 
	{
		"#version 420\n"\
		
		"layout(binding=0) uniform sampler2D atlas;\n"\
		
		"smooth in vec2 pass_texCoords;\n"\
		"smooth in vec4 pass_textCol;\n"\

		"out vec4 out_Color;\n"\

		"void main(void){\n"\
		
		//standard
		"	out_Color = vec4(pass_textCol.r, pass_textCol.g, pass_textCol.b, texture(atlas, pass_texCoords).r * pass_textCol.a);\n"\

		//saturate alpha, to test mipmapping etc. (could also just change blend method)
		//"	out_Color = vec4(pass_textCol.r, pass_textCol.g, pass_textCol.b, texture(atlas, pass_texCoords).r * 90);\n"\

		//blocks (looks really cool)
		//"	out_Color = vec4(pass_textCol.r, pass_textCol.g, pass_textCol.b, pass_textCol.a);\n"\

		"}\n"
	};

	GLuint progDraw;

	GLuint fontTexture;  //sampler uniform location

	struct CharacterVertexData
	{
		maths::vec2 pos;		 //location of the vertex in the quad
		maths::vec2 atlasCoords; //texture atlas coordinates for the vertex
		maths::vec4 col;		 //colour
	};
	
	GLuint mVAO;

	GLuint characterBuffer; //translations for each vertex in a draw call
	CharacterVertexData* characterVertices;
	std::vector<CharacterVertexData> vertexData;

	GLuint indexBuffer; //indices for a draw call
	GLushort *indices; 
	std::vector<GLushort> indexData;
	GLuint numIndices;

};

#endif
