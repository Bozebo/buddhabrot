#ifndef HEADER_RENDERER
#define HEADER_RENDERER

#ifdef __linux
	#include <unistd.h>
	#include <GL/glew.h>
#elif _WIN32
	#include <Windows.h>
	#include "conio.h"
	#include <GL/glew.h>
#else
	#error "Compiler target platform is not supported"
#endif


#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>

#include "time.h"


#include "Buddhabrot.h"
#include "TextRenderer.h"


using namespace std;

//platform independent thread sleep
void sleepFor(int timeInMs);

class Renderer{
private:
	GLFWwindow* window;

	GLuint progDraw;
	//GLuint fbBuddha;
	
	//GLuint progBuddha;
	
	//GLuint texFbA;
	//GLuint texFbB;
	GLuint texBuddha;
	//bool fbUseA;

	//GLuint samplerTex;
	//GLuint uCenter, uScale, uIter, uSeed, uWipe;

	GLfloat rScale, gScale, bScale;
	GLuint uRScale, uGScale, uBScale;

	GLfloat colChange;
	GLfloat maxColScale;

	float maxGamBright;
	float gamBrightChange;

	//GLfloat scale;
	//GLuint iters;
	//GLubyte wipe;
	//GLdouble centX, centY;
	//GLfloat seedX, seedY;

	//each line of text to be rendered on the ui
	std::vector<TextLine*> textLines;


	//basic status information text
	TextLine* statusText;
	char statusStr[512];



	Buddhabrot* brot;



	double mutStr, mutStrFocus;
	int maxMutations, maxMutationsFocus;

	GLuint vao;
	GLuint vBuffer;
	GLuint iBuffer;


	bool doRender, keepShowing;
	unsigned int showIn, showGap;

	bool lastSpacePress, lastChangeIters, lastPPressed, frameChanged, bmpLastSaved;
	bool boxing;
	double boxT, boxR, boxB, boxL;

	static const int maxIters = 256;

	int orbitMode;

	bool paintStartMap;

	GLfloat vertices[20];


	void errCheck(int identifier){
		
		GLenum err = glGetError();

		if(err != GL_NO_ERROR){

			GLchar errLog3[1024];
			GLint maxLen3 = 1024;


			maxLen3 = sprintf(errLog3, "(%d) Generic OpenGL error: %d => %s\n", identifier, err, (char*)((err == GL_OUT_OF_MEMORY) ? L"table too large" : L""));

			ofstream out("glLog.txt", ios::out | ios::trunc);
			out.write(errLog3, maxLen3);

			printf(errLog3);
			glfwSetWindowShouldClose(window, 1);
		}
		else {
			GLchar errLog3[1024];
			GLint maxLen3 = 1024;

			maxLen3 = sprintf(errLog3, "(%d) no OpenGL error", identifier);

			ofstream out("glLog.txt", ios::out | ios::trunc);
			out.write(errLog3, maxLen3);

		}
		
	}


	void applySettings(Settings* toApply);

	void init();

	/*
	//mandelbox code (cpu-side) I found on a forum

	GLuint progMandelBox;

	//mandelbox parameters
	GLint uParams, uFovX, uFovY, uMaxSteps, uMinDist, uIters, uColIters, 
		uAoEps, uAoStr,

		uGlowStr, uDistToCol,

		uCamera;

	//values
	GLfloat params[10][2];
	GLfloat mCamera[16];

	float camSpeed;

	GLint multiSamples;
	GLfloat moveSpeed;		//units/frame
	GLfloat kbdRotSpeed;	//degrees/frame
	GLfloat sensitivity;	//degrees/pixel

	GLfloat fovX, fovY, minDist;
	GLuint	maxSteps, iters, colIters, 

		//ambient occlusion
		aoEps, aoStr,

		glowStr, distToCol;


	void updateBoxUniforms();

	void checkParams();
	
	#define rightDirection (&mCamera[0])
	#define upDirection (&mCamera[4])
	#define direction (&mCamera[8])
	#define position (&mCamera[12])

	// Compute the dot product of two vectors.
	float dot(float x[3], float y[3]) {
	  return x[0]*y[0] + x[1]*y[1] + x[2]*y[2];
	}

	// Normalize a vector. If it was zero, return 0.
	int normalize(float x[3]) {
	  float len = dot(x, x); if (len == 0) return 0;
	  len = 1/sqrt(len); x[0] *= len; x[1] *= len; x[2] *= len;
	  return 1;
	}

	// Set the OpenGL modelview matrix to the camera matrix.
	void setCamera(void) {
	  glUniformMatrix4fv(uCamera, 1, GL_FALSE, mCamera);
	}

	// Orthogonalize the camera matrix.
	void orthogonalizeCamera(void) {
	  int i; float l;

	  if (!normalize(direction)) { direction[0]=direction[1]=0; direction[2]=1; }

	  // Orthogonalize and normalize upDirection.
	  l = dot(direction, upDirection);
	  for (i=0; i<3; i++) upDirection[i] -= l*direction[i];
	  if (!normalize(upDirection)) {  // Error? Make upDirection.z = 0.
		upDirection[2] = 0;
		if (fabs(direction[2]) == 1) { upDirection[0] = 0; upDirection[1] = 1; }
		else {
		  upDirection[0] = -direction[1]; upDirection[1] = direction[0];
		  normalize(upDirection);
		}
	  }

	  // Compute rightDirection as a cross product of upDirection and direction.
	  for (i=0; i<3; i++) {
		int j = (i+1)%3, k = (i+2)%3;
		rightDirection[i] = upDirection[j]*direction[k] - upDirection[k]*direction[j];
	  }
	}

	// Move camera in a direction relative to the view direction.
	// Behaves like `glTranslate`.
	void moveCamera(float x, float y, float z) {
	  int i; for (i=0; i<3; i++) {
		position[i] += rightDirection[i]*x + upDirection[i]*y + direction[i]*z;
	  }
	}

	// Move camera in the normalized absolute direction `dir` by `len` units.
	void moveCameraAbsolute(float* dir, float len) {
	  int i; for (i=0; i<3; i++) {
		position[i] += len * dir[i];
	  }
	}

	// Rotate the camera by `deg` degrees around a normalized axis.
	// Behaves like `glRotate` without normalizing the axis.
	void rotateCamera(float deg, float x, float y, float z) {
	  int i, j;
	  float s = sin(deg*PI/180), c = cos(deg*PI/180), t = 1-c;
	  float r[3][3] = {
		{ x*x*t +   c, x*y*t + z*s, x*z*t - y*s },
		{ y*x*t - z*s, y*y*t +   c, y*z*t + x*s },
		{ z*x*t + y*s, z*y*t - x*s, z*z*t +   c }
	  };
	  for (i=0; i<3; i++) {
		float c[3];
		for (j=0; j<3; j++) c[j] = mCamera[i+j*4];
		for (j=0; j<3; j++) mCamera[i+j*4] = dot(c, r[j]);
	  }
	}


	*/


public:
	bool run();

	void updateStatusText(int w, int h){
		sprintf(statusStr, "Rendering paused (P). Space updates the buddhabrot image. (dimensions: %dx%d)", w, h);
		statusText->SetText(statusStr);
	}
	
	TextRenderer* ui;

};


#endif
