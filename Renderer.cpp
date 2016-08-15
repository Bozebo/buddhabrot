

#include "Buddhabrot.h"
#include "Renderer.h"

using namespace std;

void sleepFor(int timeInMs){	
	#ifdef __linux
		usleep(timeInMs * 10);
	#elif _WIN32
		Sleep(timeInMs);
	#endif
}

//glfw makes it awkward to avoid globals :(
bool viewportChanged;
GLint windowWidth, windowHeight;
Renderer* __renderer;

GLchar* readFile(char const* name) {
  FILE* f;
  int len;
  GLchar* s = 0;

  // open file an get its length
  /*
  if (!(f = fopen(name, "r"))) goto readFileError1;
  fseek(f, 0, SEEK_END);
  len = ftell(f);
  */
  
  f = fopen(name, "r");
  if (!f) goto readFileError1;
  fseek(f, 0, SEEK_END);
  len = ftell(f);

  // read the file in an allocated buffer
  if (!(s = (GLchar*) malloc(len+1))) goto readFileError2;
  rewind(f);
  len = fread(s, 1, len, f);
  s[len] = '\0';

  readFileError2: fclose(f);
  readFileError1: return s;
}

void Renderer::applySettings(Settings* toApply){
	mutStr = toApply->mutStr;
	mutStrFocus = toApply->mutStrFocus;
	maxMutations = toApply->maxMutations;
	maxMutationsFocus = toApply->maxMutationsFocus;
	orbitMode = toApply->orbitMode;
	paintStartMap = toApply->paintStartMap;
}

void Renderer::init(){

	__renderer = this;

	mutStr = 0.01;
	mutStrFocus = 0.0025;
	maxMutations = 1024;
	maxMutationsFocus = 4096;
	boxing = false;
	orbitMode = 1;

	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

	/*
	const GLchar* vertMandelBox = 
	{
		"#version 420\n"\
		
		"uniform mat4 viewMatrix;\n"\

		"uniform float fovX;\n"\
		"uniform float fovY;\n"\

		"in vec2 in_position;\n"\

		//eye location is constant
		"flat out vec3 eye;\n"\
		//1 ray for each main edge of the fustrum, smoothed across the quad
		"out vec3 dir;\n"\

		
		"float fov2scale(float fov){\n"\
		"	return tan(radians(fov/2.0));\n"\
		"}\n"\
		
		
		"void main(void){\n"\

		"	eye = vec3(viewMatrix[3]);\n"\

		"	dir = vec3(viewMatrix * vec4(fov2scale(fovX)*in_position.x, fov2scale(fovY)*in_position.y, 1.0, 0.0));\n"\

		"	gl_Position = vec4(in_position, 1.0, 1.0);\n"\
		"}\n"

	};
	*/
		const GLchar* VertexShader = 
		{
			"#version 420\n"\
			
			"layout(location=0) in vec2 in_position;\n"\
			"layout(location=1) in vec2 in_complexValue;\n"\

			"smooth out vec2 complexPlane;\n"\
			
			"void main(void){\n"\
				
			"   complexPlane = in_complexValue;\n"\
			"   gl_Position = vec4(in_position, 1.0, 1.0);\n"\

			"}\n"
		};

		/*
		//buddhabrot generation doesn't lend itself well to OpenGL, perhaps OpenCL
		const GLchar* makeBuddhaVert = 
		{
			"#version 420\n"\
			
			"layout(location=0) in vec2 in_position;\n"\
			
			"void main(void){\n"\
			"	gl_PointSize = 16.0;\n"\
			"   gl_Position = vec4(in_position, 1.0, 1.0);\n"\

			"}\n"
		};
		
		const GLchar* makeBuddhaFrag = 
		{
			"#version 420\n"\

			"layout(binding=0) uniform sampler2D buddha;\n"\

			//"noperspective in vec2 complexPlane;\n"\
			
			"uniform dvec2 center;\n"\
			"uniform float scale;\n"\
			"uniform uint iter;\n"\

			"uniform uint wipe;\n"\

			"out vec4 out_complexState;\n"\


			"void main(void){\n"\

			//clear

			"	if(wipe == 1){\n"\
			"		out_complexState.r = 0.0;\n"\
			"		out_complexState.g = 0.0;\n"\
			"		out_complexState.b = 0.0;\n"\
			"		out_complexState.a = 1.0;\n"\
			"		return;\n"\
			"	}\n"\


			"	vec2 complexPlane;"\
			"	complexPlane.x = gl_FragCoord.x / 640;\n"\
			"	complexPlane.y = gl_FragCoord.y / 480;\n"\
		
			
			//"			out_complexState.r = 1.0;\n"\
			//"			out_complexState.g = 1.0;\n"\
			//"			out_complexState.b = 1.0;\n"\
			//"			out_complexState.a = 1.0;\n"\

			//"			return;"\

			"	float it = float(0.0);\n"\

			"	dvec2 z;\n"\

			"	float i = 0;\n"\
			
			"	dvec2 c = dvec2((complexPlane.x - 0.5) * scale - center.x, (complexPlane.y - 0.5) * scale - center.y);\n"\
			
			//"	c.x = ;\n"\
			"	c.y = ;\n"\
			
			"	double zxsq = c.x * c.x;\n"\
			"	double zysq = c.y * c.y;\n"\

			"	z = c;\n"\

			//"	dvec2 temp;\n"\
			
			
			//skip starting numbers which are within the main cardioid or period 2 bulb
			"	float zxminquart = z.x - 0.25;\n"\
			"	float q = zxminquart*zxminquart + zysq;"\
			"	if(q*(q + zxminquart) < zysq*0.25){\n"\
			"		i = iter;\n"\

			//acquire state at previous iteration
			
			//"		out_complexState.r = 0.0;\n"\
			"		out_complexState.g = 0.0;\n"\
			"		out_complexState.b = 0.0;\n"\
			"		out_complexState.a = 1.0;\n"\
			

			"	}"\

			"		vec4 oldState = texture(buddha, complexPlane);\n"\
			"	for(i; i < iter; i ++){\n"\


			//if point is not in the mandelbrot set
			"		if(zxsq + zysq > 4.0){"\
			


			//not interesting if it took only a few iterations
			"			if(i < 3.0){\n"\
			"				break;\n"\
			"			}\n"\
			"			float distCol = i/iter;"\
			//make red
			"			out_complexState.r = distCol * 0.01 + oldState;\n"\

			//"			out_complexState.g = distCol;\n"\
			"			out_complexState.b = distCol;\n"\

			"			out_complexState.a = 1.0;\n"\

	
			"			break;\n"\
			"		}\n"\
			
			
			"		z.y = z.x * z.y;\n"\
			"		z.y += z.y;\n"\
			"		z.y += c.y;\n"\
			"		z.x = zxsq - zysq + c.x;\n"\
			
			"		zxsq = z.x * z.x;\n"\
			"		zysq = z.y * z.y;\n"\

			"	}\n"\

			"}\n"

		};
		*/

		const GLchar* showBuddhaFrag = 
		{
			"#version 420\n"\
			
			"layout(binding=0) uniform sampler2D buddha;\n"\


			"smooth in vec2 complexPlane;\n"\
			
			"out vec4 out_Color;\n"\


			"uniform float rScale;\n"\
			"uniform float gScale;\n"\
			"uniform float bScale;\n"\
			
			"void main(void){\n"\

			"	vec4 col = texture(buddha, complexPlane);\n"\

			//"	float newR = col.r*rScale;\n"\
			//"	float newG = col.g*gScale;\n"/
			//"	float newB = col.b*bScale;\n"/
			
			//"	if(rScale > 0.5) newR = 1.0;\n"\
			//"	if(gScale == 0) newG = 1.0;\n"\

			"	out_Color = vec4(col.r*rScale, col.g*gScale, col.b*bScale, 1.0);\n"\

			//"	out_Color = vec4(col.r, col.g, col.b, 1.0);\n"\

			//"	out_Color = vec4(newR, newG, newB, 1.0);\n"\

			//"	out_Color = vec4(0.0, 0.8, 0.8, 1.0);\n"\

			"}\n"
		};



		/*
		const GLchar* FragmentShaderOld = 
		{
			"#version 420\n"\
			
			//"layout(binding=0) uniform sampler2D buddha;\n"\
			
			"noperspective in vec2 complexPlane;\n"\

			"uniform vec2 seed;\n"\

			"uniform dvec2 center;\n"\
			"uniform float scale;\n"\
			"uniform uint iter;\n"\
			
			
			"out vec4 out_Color;\n"\

			"void main(void){\n"\
			
			
			"	float it = float(0.0);\n"\
			"	dvec2 z;\n"\
			"	float i;\n"\
			
			

			//mandelbrot
			
			"	dvec2 c;\n"\
			"	c.x = (complexPlane.x - 0.5) * scale - center.x;\n"\
			"	c.y = (complexPlane.y - 0.5) * scale - center.y;\n"\
			
			"	double zxsq = c.x * c.x;\n"\
			"	double zysq = c.y * c.y;\n"\

			"	z = c;\n"\

			"	dvec2 temp;\n"\

			"	out_Color = vec4(0.0, 0.0, 0.0, 1.0);\n"\
			"	for(i = 0; i < iter; i ++){\n"\

			//"		temp = dvec2((z.x * z.x - z.y * z.y) + c.x, (z.y * z.x + z.x * z.y) + c.y);\n"\

			//"		double x = (z.x * z.x - z.y * z.y) + c.x;\n"\
			"		double y = (z.y * z.x + z.x * z.y) + c.y;\n"\

					//stop iterating if it will not be part of the set (tends towards infinity if the magnitude of z is beyond 2)
			//"		float magnitude = float(x*x + y*y);\n"\

			//"		if(x*x + y*y > 4.0){\n"\

			//"		if(sqrt(float(x)*float(x) + float(y)*float(y)) > 2.0){;"\
			
			//"		if(length(temp) > 2.0){"\

			"		if(zxsq + zysq > 4.0){"\
			
			//2 more iterations
		//	"			if(i < 30){\n"\

			"			float x = (z.x * z.x - z.y * z.y) + c.x;\n"\
			"			float y = (z.y * z.x + z.x * z.y) + c.y;\n"\
			"			i ++;\n"\

			"			x = (z.x * z.x - z.y * z.y) + c.x;\n"\
			"			y = (z.y * z.x + z.x * z.y) + c.y;\n"\
			"			i ++;\n"\

		//	"			}\n"\
			
			"			float modulus = sqrt(x*x + y*y);\n"\

			//"			float modulus = x*x + y*y / 2.0;\n"\

			"			float mu = i - (log(log(modulus))/log(2.0));\n"\
		
			"			out_Color = vec4((-cos(0.15*mu)+1.0)/2.0,(-cos(0.1*mu)+1.0)/2.0, (-cos(0.05*mu)+1.0)/2.0, 1.0);\n"\


			"			break;\n"\
			"		}\n"\
			
			
			"		z.y = z.x * z.y;\n"\
			"		z.y += z.y;\n"\
			"		z.y += c.y;\n"\
			"		z.x = zxsq - zysq + c.x;\n"\
			
			"		zxsq = z.x * z.x;\n"\
			"		zysq = z.y * z.y;\n"\

			//"		z = temp;\n"\

			//"		z.x = x;\n"\
			"		z.y = y;\n"\

			"	}\n"\

			//"	out_Color = float(it) * out_Color;\n"\
			
			
			
			
			//julia
			
			"	out_Color = vec4(0.0, 0.0, 0.0, 1.0);\n"\
			"	z.x = (complexPlane.x - 0.5) * scale - center.x;\n"\
			"	z.y = (complexPlane.y - 0.5) * scale - center.y;\n"\
			
			"	double zxsq = z.x * z.x;\n"\
			"	double zysq = z.y * z.y;\n"\

			"	for(i = 0; i < iter; i++){\n"\
			
			//"		float x = (z.x * z.x - z.y * z.y) + seed.x;\n"\
			"		float y = (z.y * z.x + z.x * z.y) + seed.y;\n"\
			
			//"		temp = dvec2((z.x * z.x - z.y * z.y) + seed.x, (z.y * z.x + z.x * z.y) + seed.y);\n"\
			
			
			"		if(zxsq + zysq > 4.0){\n"\

			//"		if(length(temp) > 2.0){\n"\

			

			
			//a few extra iterations
			"			float x = (z.x * z.x - z.y * z.y) + seed.x;\n"\
			"			float y = (z.y * z.x + z.x * z.y) + seed.y;\n"\
			"			i ++;\n"\

			"			x = (z.x * z.x - z.y * z.y) + seed.x;\n"\
			"			y = (z.y * z.x + z.x * z.y) + seed.y;\n"\
			"			i ++;\n"\

			"			float modulus = sqrt(x*x + y*y);\n"\
			"			float mu = i - (log(log(modulus))/log(2.0));\n"\
		
			//"			out_Color = vec4(0.9, 0.5, 0.1, 1.0)*mu\n;"\


					   "out_Color = vec4((-cos(0.025*mu)+1.0)/2.0,(-cos(0.08*mu)+1.0)/2.0, (-cos(0.12*mu)+1.0)/2.0, 1.0);\n"\


			"			break;\n"\
			"		}\n"\

			//"		z.y = ((z.x + z.y)*(z.x + z.y) - zxsq - zysq) + seed.y;\n"\
			"		z.x = zxsq - zysq + seed.x;\n"\
			
			"		z.y = z.x * z.y;\n"\
			"		z.y += z.y;\n"\
			"		z.y += seed.y;\n"\
			"		z.x = zxsq - zysq + seed.x;\n"\
			
			"		zxsq = z.x * z.x;\n"\
			"		zysq = z.y * z.y;\n"\

			//"		z.x = x;\n"\
			"		z.y = y;\n"\

			"}\n"\

			

			//"	out_Color = float(it) * out_Color;\n"\
			
			
			
			

			


			"}\n"
		};
		*/


		glewInit();

		/*

		//make buddhabrot (cpu side)

		GLuint vShader, fShader;
		vShader = glCreateShader(GL_VERTEX_SHADER);
		fShader = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(vShader, 1, &makeBuddhaVert, NULL);
		glShaderSource(fShader, 1, &makeBuddhaFrag, NULL);

		glCompileShader(vShader);
		glCompileShader(fShader);

		//check for fragment shader compilation errors
		GLint status = 0;
		glGetShaderiv(fShader, GL_COMPILE_STATUS, &status);
		if(!status){
			GLchar errLog[1024];
			GLint maxLen = 1024;
			glGetShaderInfoLog(fShader, maxLen, &maxLen, errLog);

			ofstream out("buddhaLog.txt", ios::out | ios::trunc);
			out.write(errLog, maxLen);
		}
		
		progBuddha = glCreateProgram();

		glAttachShader(progBuddha, vShader);
		glAttachShader(progBuddha, fShader);
		glLinkProgram(progBuddha);
		*/




		//buddhabrot GL init
		
		GLuint vShader2 = glCreateShader(GL_VERTEX_SHADER);
		GLuint fShader2 = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(vShader2, 1, &VertexShader, NULL);
		glShaderSource(fShader2, 1, &showBuddhaFrag, NULL);
		
		glCompileShader(vShader2);
		glCompileShader(fShader2);

		//check for fragment shader compilation errors
		GLint statusB = 0;
		glGetShaderiv(fShader2, GL_COMPILE_STATUS, &statusB);
		if(!statusB){
			GLchar errLog2[1024];
			GLint maxLen2 = 1024;
			glGetShaderInfoLog(fShader2, maxLen2, &maxLen2, errLog2);

			ofstream out("drawLog.txt", ios::out | ios::trunc);
			out.write(errLog2, maxLen2);
		}

		progDraw = glCreateProgram();
		glAttachShader(progDraw, vShader2);
		glAttachShader(progDraw, fShader2);
		glLinkProgram(progDraw);

		//check link status
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(progDraw, GL_LINK_STATUS, &linkStatus);
		if(linkStatus != GL_TRUE){
			GLchar infoLog[2048];
			GLint maxLen2 = 2040;
			glGetProgramInfoLog(progDraw, maxLen2, &maxLen2, infoLog);
		
			GLint maxVAttrs;
			glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVAttrs);

			ofstream out("linkLog.txt", ios::out | ios::trunc);
			out.write(infoLog, maxLen2);
			out.write(infoLog, sprintf(infoLog, "\nGL_MAX_VERTEX_ATTRIBS: %d\n", maxVAttrs));
			

			out.close();
		}
		errCheck(313);
		

		/*
		//mandelbox code I found on a forum

		GLuint vShader3 = glCreateShader(GL_VERTEX_SHADER);
		GLuint fShader3 = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(vShader3, 1, &vertMandelBox, NULL);

		GLchar const* mandelFrag;
		mandelFrag = readFile("mandelBox.glsl");
		glShaderSource(fShader3, 1, &mandelFrag, NULL);



		glCompileShader(vShader3);
		//check for vertex shader compilation errors
		GLint estatus = 0;
		glGetShaderiv(vShader3,	GL_COMPILE_STATUS, &estatus);
		if (!estatus){
			GLchar errLog2[1024];
			GLint maxLen2 = 1024;
			glGetShaderInfoLog(vShader3, maxLen2, &maxLen2, errLog2);

			ofstream out("vertLog.txt", ios::out | ios::trunc);
			out.write(errLog2, maxLen2);
		}


		glCompileShader(fShader3);
		//check for fragment shader compilation errors
		GLint statusB = 0;
		glGetShaderiv(fShader3, GL_COMPILE_STATUS, &statusB);
		if(!statusB){
			GLchar errLog2[1024];
			GLint maxLen2 = 1024;
			glGetShaderInfoLog(fShader3, maxLen2, &maxLen2, errLog2);

			ofstream out("fragLog.txt", ios::out | ios::trunc);
			out.write(errLog2, maxLen2);
		}



		
		errCheck(40);
		progMandelBox = glCreateProgram();
		errCheck(45);
		glAttachShader(progMandelBox, vShader3);
		errCheck(50);
		glAttachShader(progMandelBox, fShader3);
		errCheck(55);
		glLinkProgram(progMandelBox);
		errCheck(60);
		


		//check link status
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(progMandelBox, GL_LINK_STATUS, &linkStatus);
		if(linkStatus != GL_TRUE){
			GLchar infoLog[2048];
			GLint maxLen2 = 2040;
			glGetProgramInfoLog(progMandelBox, maxLen2, &maxLen2, infoLog);
		
			GLint maxVAttrs;
			glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVAttrs);

			ofstream out("mandelInfoLog.txt", ios::out | ios::trunc);
			out.write(infoLog, maxLen2);
			out.write(infoLog, sprintf(infoLog, "\nGL_MAX_VERTEX_ATTRIBS: %d\n", maxVAttrs));
			

			out.close();
		}
		errCheck(61);
		*/

		// texture to store buddhabrot info in
		
	//	glGenTextures(1, &texFbA);
	//	glGenTextures(1, &texFbB);
		
		
	/*	
		glBindTexture(GL_TEXTURE_2D, texFbA);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 640, 480, 0,
             GL_RGB, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glBindTexture(GL_TEXTURE_2D, texFbB);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 640, 480, 0,
             GL_RGB, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		*/
		
		
		// -- create the ui renderer

		ui = new TextRenderer(&textLines);
		int fontLoadResult = ui->LoadFont("OpenSansSemibold.xml");
		if(fontLoadResult != 1){

			FILE* fontLoadErrorFile;
			fontLoadErrorFile = fopen("fontError.txt", "w");
			fprintf(fontLoadErrorFile, "%d", fontLoadResult);
			fclose(fontLoadErrorFile);

			printf("Font load error: %d\n", fontLoadResult);

			glfwDestroyWindow(window);
		}

		errCheck(506070);
		
		/*
		TextLine *testTextLine;
		testTextLine = new TextLine(maths::vec2(-.6f, .3f), "Testing Lines!!!");
		testTextLine->SetDisplayLength(2);
		textLines.push_back(testTextLine);
		*/
		//statusText = new TextLine(maths::vec2(-1.f, 1.f), maths::vec3(1.f, .2f, .2f), 0.2f, "Rendering paused (P). Space updates the buddhabrot image.");
		statusText = new TextLine(maths::vec2(4.f, 0.f), maths::vec3(1.f, .2f, .2f), 72.f, statusStr);
		updateStatusText(768, 768);
		statusText->SetEnabled(false);
		textLines.push_back(statusText);

		/*
		ui->TestTextLine(-.5f, .2f, 0.8f, 0.0f, 0.0f, 1.f, "Testing it!");
		errCheck(506071);

		
		TextLine *testTextLine;

		testTextLine = new TextLine(maths::vec2(-.6f, .3f), "Testing Lines!!!");
		testTextLine->SetDisplayLength(2);
		textLines.push_back(testTextLine);
		
		testTextLine = new TextLine(maths::vec2(-.1f, -.15f), "Again :O");
		textLines.push_back(testTextLine);

		testTextLine = new TextLine(maths::vec2(-.9f, .9f), .1f, "small little one");
		textLines.push_back(testTextLine);
		
		testTextLine = new TextLine(maths::vec2(-.7f, -.25f), maths::vec4(.8f, .4f, .4f, 1.f), .2f, "another small 0123456789 set");
		testTextLine->SetAlpha(.7f);
		textLines.push_back(testTextLine);

		testTextLine = new TextLine(maths::vec2(-.6f, -.65f), .4f, "unknown character: ");
		testTextLine->SetAlpha(.7f);
		textLines.push_back(testTextLine);
		*/

		errCheck(506072);

		
		
		// -- create the buddhabrot object		

		brot = new Buddhabrot();
		errCheck(23233);

		Settings* testSettings = new Settings();

		
		int loadResult = testSettings->loadFromFile("settings.xml");

		if(loadResult == -1){
			printf("Settings load error: %d\n", loadResult);

			glfwDestroyWindow(window);
		}

		testSettings->init();

		applySettings(testSettings);
		brot->applySettings(testSettings);


		glGenTextures(1, &texBuddha);
		errCheck(23234);
		glBindTexture(GL_TEXTURE_2D, texBuddha);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, brot->width, brot->height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
            

		//glBindTexture(GL_TEXTURE_2D, 0);

		/*
			GLchar errLog3[1024];
			GLint maxLen3 = 1024;

			maxLen3 = sprintf(errLog3, "%d %d", testBrot->width, testBrot->height);

			ofstream out("00wot.txt", ios::out | ios::trunc);
			out.write(errLog3, maxLen3);
			*/
		//frame buffer object for rendering to texture

		//glGenFramebuffers(1, &fbBuddha);
		//glBindFramebuffer(GL_FRAMEBUFFER, fbBuddha);
		
		//glBindRenderbuffer(GL_RENDERBUFFER, fbBuddha);
		//glRenderbufferStorage(GL_RENDERBUFFER,
		//					GL_RGBA8,
		//					640,
		//					480);
		



						//fbUseA = false; //using texture B first

		
		//need a draw buffer
		//GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0 };
		//glDrawBuffers(1, draw_bufs);

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		

		/*
		GLenum statusC = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(statusC != GL_FRAMEBUFFER_COMPLETE){
			GLchar errLog3[1024];
			GLint maxLen3 = 1024;

			maxLen3 = sprintf(errLog3, "Error creating FB: %d", statusC);

			ofstream out("fbLog.txt", ios::out | ios::trunc);
			out.write(errLog3, maxLen3);
		}
		*/
		
		
		glUseProgram(progDraw);
		//prepare draw list for screen quad (vbo & ibo)


		//corner locations for quad as triangle strip with complex plane limits
		vertices[0] = -1.0f; vertices[1] = 1.0f; vertices[2] =  0.0f; vertices[3] = 1.0f;
		vertices[4] = 1.0f; vertices[5] = 1.0f; vertices[6] =  1.0f; vertices[7] = 1.0f;
		vertices[8] = -1.0f; vertices[9] = -1.0f; vertices[10] =  0.0f; vertices[11] = 0.0f;
		vertices[12] = 1.0f; vertices[13] = -1.0f; vertices[14] =  1.0f; vertices[15] = 0.0f;

		vertices[16] = 0.0f; vertices[17] = 0.0f; vertices[18] =  0.5f; vertices[19] = 0.5f;

		GLubyte indices[] = {
			0, 1, 2, 3, 4
		};

		glGenVertexArrays(1, &vao);

		glGenBuffers(1, &vBuffer);
		glGenBuffers(1, &iBuffer);

		glBindVertexArray(vao);

			glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuffer);

			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, 0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, (void*) 8);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

		glBindVertexArray(0);


		glBindBuffer(GL_ARRAY_BUFFER, vBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuffer);

		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


		errCheck(1001);
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		

		errCheck(2990);
		/*
		uCenter = glGetUniformLocation(progBuddha, "center");
		uScale = glGetUniformLocation(progBuddha, "scale");
		uIter = glGetUniformLocation(progBuddha, "iter");
		//uSeed = glGetUniformLocation(progBuddha, "seed");
		*/
		//uWipe = glGetUniformLocation(progDraw, "wipe");
		
		
		errCheck(2991);

		
		uRScale = glGetUniformLocation(progDraw, "rScale");
		uGScale = glGetUniformLocation(progDraw, "gScale");
		uBScale = glGetUniformLocation(progDraw, "bScale");
		errCheck(2992);
		rScale = 1;
		gScale = 1;
		bScale = 1;
		glUniform1f(uRScale, rScale);
		glUniform1f(uGScale, gScale);
		glUniform1f(uBScale, bScale);
		colChange = .005f;
		maxColScale = 256.f;

		maxGamBright = 2.5f;
		gamBrightChange = .005f;

		/*
		errCheck(1002);
		centX = 1.0;
		centY = 0.0;
		*/

		srand(clock());
		
		/*
		glUniform2d(uCenter, centX, centY);
		errCheck(10031);
		scale = 3.0;
		glUniform1f(uScale, scale);
		errCheck(10032);
		iters = (GLuint)8;
		glUniform1ui(uIter, iters);
		errCheck(10033);
		wipe = 0;
		glUniform1ui(uWipe, wipe);
		*/
		

		/*
		seedX = (GLfloat) rand()/(RAND_MAX + 1);
		seedY = (GLfloat) rand()/(RAND_MAX + 1);
		glUniform2f(uSeed, seedX, seedY);
		*/
		
		lastSpacePress = false;
		lastChangeIters = false;
		keepShowing = false;
		viewportChanged = false;

		//~ vsync (mostly irrelevant for the buddhabrot)
		glfwSwapInterval(0);

		//no depth testing
		glDisable(GL_DEPTH_TEST);
		
		//turn on blending (for UI/text)
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//texture 0 is in use
		//glActiveTexture(GL_TEXTURE0);

		errCheck(999);
		//Sleep(1000);


		//glUseProgram(progMandelBox);
		errCheck(91003);
		
		//set up mandelbox parameters
		//params[0][0] = 0.25;
		//params[0][1] = -1.77;
		
		//grab uniform handles
		/*
		GLint uParams, uFovX, uFovY, uMaxSteps, uMinDist, uIters, uColIters, 
			uAoEps, uAoStr,

			uGlowStr, uDistToCol;
		*/

		/*
		errCheck(91004);	

		uFovX = glGetUniformLocation(progMandelBox, "fovX");
		if(uFovX == -1){
			int bob = 929292;
		}

		uFovY = glGetUniformLocation(progMandelBox, "fovY");
		errCheck(91005);

		uParams = glGetUniformLocation(progMandelBox, "par");
		errCheck(91006);
		
		uMaxSteps = glGetUniformLocation(progMandelBox, "maxSteps");
		uMinDist = glGetUniformLocation(progMandelBox, "minDist");
		errCheck(91007);
		
		uIters = glGetUniformLocation(progMandelBox, "iters");
		uColIters = glGetUniformLocation(progMandelBox, "colIters");
		errCheck(91008);
		
		uAoEps = glGetUniformLocation(progMandelBox, "aoEps");
		uAoStr = glGetUniformLocation(progMandelBox, "aoStr");
		errCheck(91009);
		
		uGlowStr = glGetUniformLocation(progMandelBox, "glowStr");
		uDistToCol = glGetUniformLocation(progMandelBox, "distToCol");
		errCheck(91010);
		
		uCamera = glGetUniformLocation(progMandelBox, "viewMatrix");

		errCheck(91919);
		
		multiSamples = 1;
		moveSpeed = 0.04f;
		kbdRotSpeed = 2;
		sensitivity = 0.6f;

		fovX = 91;
		fovY = 75;
		maxSteps = 128;
		minDist = 0.0001f;
		iters = 13;
		colIters = 9;
		aoEps = 0.0005f;
		aoStr = 0.1f;
		glowStr = 0.5f;
		distToCol = 0.2f;

		*/

		/*


		//memset(mCamera, 0, 16*sizeof(float));
		mCamera[0] = mCamera[1] = mCamera[2] = mCamera[3] = 0.0f;
		mCamera[4] = mCamera[6] = mCamera[7] = 0.0f;
		mCamera[8] = mCamera[9] = mCamera[11] = 0.0f;
		mCamera[5] = 1.0f;
		mCamera[10] = 1.0f;
		mCamera[12] = -1.82f;
		mCamera[13] = 0.9f;
		mCamera[14] = -2.0f;
		mCamera[15] = 1.0f;
		
		camSpeed = 0.3f;
		//checkParams();
		errCheck(91920);

		//updateBoxUniforms();
		*/
		

		//frameChanged = false;
		//glActiveTexture(GL_TEXTURE0);

		errCheck(92229);

	if(glfwWindowShouldClose(window)){
		printf("Error during init\n");
		//while(!_kbhit())
			//Sleep(50);
	}

	#ifdef _WIN32
	
	else
		FreeConsole();
	#endif

}
/*
void Renderer::updateBoxUniforms(){
  glUniform2fv(uParams, sizeof(params)/sizeof((params)[0]), (float*)params);
  
		errCheck(919193);

  glUniform1f(uFovX, fovX);
  glUniform1f(uFovY, fovY);



  glUniform1ui(uMaxSteps, maxSteps);
  glUniform1f(uMinDist, minDist);
		errCheck(919194);

  glUniform1ui(uIters, iters);
  glUniform1ui(uColIters, colIters);
		errCheck(919195);

  glUniform1f(uAoEps, aoEps);
  glUniform1f(uAoStr, aoStr);
		errCheck(919196);

  glUniform1f(uGlowStr, glowStr);
  glUniform1f(uDistToCol, distToCol);
		errCheck(919197);
}
*/

void framebuffer_size_callback(GLFWwindow* window, int sWidth, int sHeight){
	glViewport(0, 0, sWidth, sHeight);

	windowWidth = sWidth;
	windowHeight = sHeight;

	viewportChanged = true;

	__renderer->updateStatusText(windowWidth, windowHeight);
	__renderer->ui->SetWindowSize(windowWidth, windowHeight);
}

bool Renderer::run(){
	

	if(!glfwInit())
		return false;

	bmpLastSaved = keepShowing = lastPPressed = false;
	
	window = glfwCreateWindow(768, 768, "Buddhabrot", NULL, NULL);
	windowHeight = 768;
	windowWidth = 768;


	if(!window){
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);


	init();
	errCheck(23232);
	glfwSetFramebufferSizeCallback(window, &framebuffer_size_callback);


	ui->SetWindowSize(windowWidth, windowHeight);
	

	showGap = 50;
	doRender = true;
	frameChanged = true; //draw the initial frame (black)

	glUseProgram(progDraw);

	errCheck(454545);

	while(!glfwWindowShouldClose(window)){
		
		bool updateData = false;
		
		//don't let the frame update too often (only every showGap render # of runs), to avoid it hogging cpu time
		if(keepShowing){
			if(showIn <= 0){
				showIn = showGap;
				frameChanged = true;
				updateData = true;
			} else frameChanged = false;
		}

		if(viewportChanged){
			frameChanged = true;
			viewportChanged = false;
		}

		//glUseProgram(progBuddha);

		//glUseProgram(progMandelBox);
		
		//if(!frameChanged){


			//clear the canvas and re-load settings
			if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){

				Settings* testSettings = new Settings();

		
				int loadResult = testSettings->loadFromFile("settings.xml");

				testSettings->init();

				applySettings(testSettings);
				brot->applySettings(testSettings);

				delete testSettings;

				//brot->init();
			}

			
			//r channel
			if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
				rScale += colChange;
				if(rScale > maxColScale) rScale = maxColScale;
				glUniform1f(uRScale, rScale);
				frameChanged = true;
			} else if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
				rScale -= colChange;
				if(rScale < 0) rScale = 0;
				glUniform1f(uRScale, rScale);
				frameChanged = true;
			}

			//g channel
			if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS){
				gScale += colChange;
				if(gScale > maxColScale) gScale = maxColScale;
				glUniform1f(uGScale, gScale);
				frameChanged = true;
			} else if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS){
				gScale -= colChange;
				if(gScale < 0) gScale = 0;
				glUniform1f(uGScale, gScale);
				frameChanged = true;
			}

			//b channel
			if(glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS){
				bScale += colChange;
				if(bScale > maxColScale) bScale = maxColScale;
				glUniform1f(uBScale, bScale);
				frameChanged = true;
			} else if(glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS){
				bScale -= colChange;
				if(bScale < 0) bScale = 0;
				glUniform1f(uBScale, bScale);
				frameChanged = true;
			}

			//gamma
			if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS){
				brot->gamma += gamBrightChange;
				if (brot->gamma > maxGamBright) brot->brightness = maxGamBright;
			}
			else if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS){
				brot->gamma -= gamBrightChange;
				if (brot->gamma < 0.) brot->brightness = 0.f;
			}

			//brightness
			if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS){
				brot->brightness += gamBrightChange;
				if (brot->brightness > maxGamBright) brot->brightness = maxGamBright;
			}
			else if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS){
				brot->brightness -= gamBrightChange;
				if (brot->brightness < 0.) brot->brightness = 0.f;
			}
			

			//wipe image
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
				brot->wipeImage();
			}
			
			//default colour
			if(glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS){
				bScale = gScale = rScale = 1.0f;

				glUniform1f(uGScale, gScale);
				glUniform1f(uRScale, rScale);
				glUniform1f(uBScale, bScale);

				brot->gamma = 1.f;
				brot->brightness = 1.f;

				frameChanged = true;
			}
			
			
			//doRender = true;
			

			//toggle rendering on or off
			if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !lastPPressed){
				statusText->SetEnabled(doRender);
				doRender = !doRender;
				lastPPressed = true;
				frameChanged = true;
			} else if(glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE && lastPPressed){
				lastPPressed = false;
			}
			

		/*
			//iterations
			if(!lastChangeIters){
				if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS){
					iters --;
					if(iters < 1) iters = 1;
					//glUniform1ui(uIter, iters);

					frameChanged = true;
				} else if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
					iters ++;
					if(iters > maxIters) iters = maxIters;
					//glUniform1ui(uIter, iters);

					frameChanged = true;
				}
			} else if(glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE || glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE)
				lastChangeIters = false;
*/

			//zoom
			if(glfwGetKey(window, GLFW_KEY_Q)){
				/*
				scale += 0.045*scale;
				if(scale > 3.0) scale = 3.0;
				//glUniform1f(uScale, scale);
				*/

				//frameChanged = true;
			} else if(glfwGetKey(window, GLFW_KEY_E)){
				/*
				scale -= 0.035*scale;
				if(scale < 0.00000001) scale = 0.00000001;
				//glUniform1f(uScale, scale);
				*/

				//frameChanged = true;
			}		
			
			//pan
			if(glfwGetKey(window, GLFW_KEY_W) || glfwGetKey(window, GLFW_KEY_A) || glfwGetKey(window, GLFW_KEY_S) || glfwGetKey(window, GLFW_KEY_D)){


				/*
				//ui->TestTextLine(-.7f, -.2f, 0.6f, 0.9f, 0.7f, 0.5f, "More test");
				

				float line0Alpha = textLines[0]->GetAlpha();
				if(line0Alpha < 0.99f){
					line0Alpha += 0.012f;
				} else {
					line0Alpha = 0.00f;
				}
				textLines[0]->SetAlpha(line0Alpha);
				
				
				float line2Size = textLines[2]->GetSize();
				if(line2Size > 1.7f){
					textLines[2]->SetText("... small again :)");
					line2Size = 0.05f;
				} else if(line2Size > 1.1f){
					textLines[2]->SetText("IS NOW HUUUGE!");
					line2Size += 0.003f;
				} else if(line2Size > .7f){
					textLines[2]->SetText("Is getting big...");
					line2Size += 0.0028f;
				} else if(line2Size > 0.45f){
					textLines[2]->SetText("Is fairly normal");
					line2Size += 0.0022f;
				} else if(line2Size > 0.25f){
					textLines[2]->SetText("Is less small");
					line2Size += 0.0018f;
				} else if(line2Size > 0.15f){
					textLines[2]->SetText("Is small");
					line2Size += 0.0015f;
				} else {
					line2Size += 0.001f;
				}
				textLines[2]->SetSize(line2Size);

				textLines[0]->SetDisplayLength(textLines[0]->GetDisplayLength() + 1);
				if(textLines[0]->GetDisplayLength() == textLines[0]->GetLength())
					textLines[0]->SetDisplayLength(0);

				keepShowing = true;
				frameChanged = true;
				showIn = showGap / 5;

				*/


				/*
				if(glfwGetKey(window, GLFW_KEY_W))
					centY -= (centY > -1.8) ? 0.01*scale : 0.0;

				else if(glfwGetKey(window, GLFW_KEY_A))
					centX += (centX < 1.8) ? 0.01*scale : 0.0;

				else if(glfwGetKey(window, GLFW_KEY_S))
					centY += (centY < 1.8) ? 0.01*scale : 0.0;

				else
					centX -= (centX > -1.8) ? 0.01*scale : 0.0;

				//glUniform2d(uCenter, centX, centY);
				*/

				/*
				if(glfwGetKey(window, GLFW_KEY_W))
					moveCamera(0, 0, camSpeed);

				if(glfwGetKey(window, GLFW_KEY_A))
					moveCamera(-camSpeed, 0, 0);

				if(glfwGetKey(window, GLFW_KEY_S))
					moveCamera(0, 0, -camSpeed);
				
				if(glfwGetKey(window, GLFW_KEY_D))
					moveCamera(camSpeed, 0, 0);

					*/

				//frameChanged = true;

			}
			
			

			

			if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !lastSpacePress){
				
				//seedX = (GLfloat) rand()/(RAND_MAX + 1);
				//seedY = (GLfloat) rand()/(RAND_MAX + 1);
				//glUniform2f(uSeed, seedX, seedY);
				
			
				keepShowing = true;
				showIn = showGap;
				//keepShowing = !keepShowing;
				lastSpacePress = true;
				frameChanged = true;
			} else if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && lastSpacePress){
				lastSpacePress = false;
				keepShowing = false;
				updateData = true;
				frameChanged = true;
			}
			
			

			//detail = ;
			
			
			if(glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !bmpLastSaved){
				bmpLastSaved = true;
				frameChanged = true;
				brot->saveToBMP("brotSave.bmp", rScale, gScale, bScale);
				brot->saveToFile("brotSave.txt");
				if (paintStartMap){
					brot->saveStartMapToBMP("brotStartMap.bmp");
				}
			} else if(glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE)
				bmpLastSaved = false;
			

			/*
			if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS){
				seedX += 0.001f;
				glUniform2f(uSeed, seedX, seedY);

				frameChanged = true;
			} else if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS){
				seedX -= 0.001f;
				glUniform2f(uSeed, seedX, seedY);

				frameChanged = true;
			}

			if(glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS){
				seedY += 0.001f;
				glUniform2f(uSeed, seedX, seedY);

				frameChanged = true;
			} else if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS){
				seedY -= 0.001f;
				glUniform2f(uSeed, seedX, seedY);

				frameChanged = true;
			}
			*/

		//}


		/*
		//render mandelbox/cube
		if(frameChanged){

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(progMandelBox);
			setCamera();
			updateBoxUniforms();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuffer);

			glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, NULL);

			glfwSwapBuffers(window);
			
			frameChanged = false;


		} else 
			Sleep(10);
		*/


		//display
		if(frameChanged){

			//generate buddhabrot texture data, if required
			if(updateData){
				//build data
				brot->genTexData();
				//bind the texture
				glBindTexture(GL_TEXTURE_2D, texBuddha);
				//upload the buddhabrot image to the texture
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, brot->width, brot->height, 0, GL_RGB, GL_UNSIGNED_BYTE, brot->texData);
				glBindTexture(GL_TEXTURE_2D, 0);
			}


			//clear the screen
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				
			// -- draw the buddhabrot
			glBindTexture(GL_TEXTURE_2D, texBuddha);
			glBindVertexArray(vao);
				glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, NULL);
			glBindVertexArray(0);
			glBindTexture(GL_TEXTURE_2D, 0);

			// -- draw info/UI
			if(ui->Draw()){
				glUseProgram(progDraw); //reset shader state, because the UI changed it
			}

			errCheck(506073);

			//present to the display
			glfwSwapBuffers(window);
			errCheck(506074);


			//dont draw again until required
			frameChanged = false;

					
		}


		//render orbits
		if(doRender){
			
			if (orbitMode == 1){ //apertures

				//do some orbits (create a mutation aperture and begin n orbits inside it)

				if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS){

					//uses a tight mutator for detail highlighting
					brot->mutStr = mutStrFocus;
					brot->doSamplesAperture(maxMutationsFocus);

				} else {
					//more "dusty/scratchy" and averagely painted
					brot->mutStr = mutStr;
					brot->doSamplesAperture(maxMutations);

				}

			} else if(orbitMode == 2){ //metropolis hastings

				brot->doSamplesMH(2048);

			} else { //naive method (fully random orbit starts)

				//do orbits beginning anywhere in the z complex plane
				brot->doSamplesNaive(512);

			}

			if (keepShowing){
				showIn--;
			}

			//sleepFor(10);
			
			
		} else {
			sleepFor(15); //wait a little to not hog CPU for no reason
			
			keepShowing = false;
		}
		

		glfwPollEvents();

		if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
			glfwSetWindowShouldClose(window, 1);
		}

	}
	
	glfwTerminate();


	return true;
}

/*
void Renderer::checkParams(){
	if(fovX <= 0){
		if(fovY <= 0) fovY = 75;
		fovX = atan(tan(fovY*PI/180/2)*width/height)/PI*180*2;
	}

	if (fovY <= 0) fovY = atan(tan(fovX*PI/180/2)*height/width)/PI*180*2;

	if (multiSamples < 1) multiSamples = 1;
	if (moveSpeed <= 0) moveSpeed = 0.004f;  // units/frame
	if (kbdRotSpeed <= 0) kbdRotSpeed = 1;  // degrees/frame
	if (sensitivity <= 0) sensitivity = 0.2f;  // degrees/pixel

	if (maxSteps < 1) maxSteps = 96;
	if (minDist <= 0) minDist = 0.0001f;
	if (iters < 1) iters = 9;
	if (colIters < 1) colIters = 9;
	if (aoEps <= 0) aoEps = 0.0005f;
	if (aoStr <= 0) aoStr = 0.1f;
	if (glowStr <= 0) glowStr = 0.5f;
	if (distToCol <= 0) distToCol = 0.2f;

	orthogonalizeCamera();
}
*/

