#ifndef HEADER_BUDDHABROT
#define HEADER_BUDDHABROT

#include <iostream>
#include <fstream>
#include "time.h"
#include <GL/glew.h>
#include <assert.h>
#include <cmath>

#include "Settings.h"

#ifdef PI
#undef PI
#endif
#define PI		3.141592653589793
#define TAU		6.283185307179586

/*
void
hsvtorgb(unsigned char *r, unsigned char *g, unsigned char *b, unsigned char h, unsigned char s, unsigned char v)
{
	unsigned char region, fpart, p, q, t;

	if (s == 0) {
		// color is grayscale 
		*r = *g = *b = v;
		return;
	}

	// make hue 0-5 
	region = h / 43;
	// find remainder part, make it from 0-255 
	fpart = (h - (region * 43)) * 6;

	// calculate temp vars, doing integer multiplication
	p = (v * (255 - s)) >> 8;
	q = (v * (255 - ((s * fpart) >> 8))) >> 8;
	t = (v * (255 - ((s * (255 - fpart)) >> 8))) >> 8;

	// assign temp vars based on color cone region 
	switch (region) {
	case 0:
		*r = v; *g = t; *b = p; break;
	case 1:
		*r = q; *g = v; *b = p; break;
	case 2:
		*r = p; *g = v; *b = t; break;
	case 3:
		*r = p; *g = q; *b = v; break;
	case 4:
		*r = t; *g = p; *b = v; break;
	default:
		*r = v; *g = p; *b = q; break;
	}

	return;
}
*/

class Buddhabrot{

public:

	void applySettings(Settings* toApply);

	Buddhabrot(){

		image = nullptr;
		scores = nullptr;
		texData = nullptr;
		orbitHistory = nullptr;
		nullZones = nullptr;
		orbitStartMap = nullptr;
		
		//start and end orbit lengths for colour components
		//sRed = 600, eRed = 2048;
		//sGreen = 340, eGreen = 1000;
		//sBlue = 128, eBlue = 750;

		
		// ----- begin settings -----
		
		width = 512;
		height = 512;
		
		iterations = 700;

		brightness = 1.0f;
		gamma = 1.0f;

		minIts = 28; //bailout

		sRed = 20; eRed = 64;
		sGreen = 18; eGreen = 450;
		sBlue = 45; eBlue = 800;

		//paintStartIts = min(sRed, min(sGreen, sBlue));
		paintStartIts = std::min(sRed, std::min(sGreen, sBlue));
		

		colMethod = 1; //1 = sqrt, otherwise linear
		
		/*
		iterations = 1400;
		brightness = 0.98f; //multiplier for colour channel intensity
		gamma = 1.0; //multiplier for brightest image colour component to grade other hits in the same channel by
		minIts = 32; //dont draw orbits for rays which iterate less than this
		sRed = 72; eRed = 1400;
		sGreen = 50; eGreen = 1100;
		sBlue = 32; eBlue = 72;
		*/

		mutStr = 0.001; //smaller numbers tightly highlight details more

		
		//greyscale
		//sRed = sGreen = sBlue = minIts;
		//eRed = eGreen = eBlue = iterations;
		

		fourDims = false; //true = random initial c instead of 0

		//axes in 4D space to render to the x/y image plane
		xAxis = 0; //0 = zR, 1 = zI, 2 = cR
		yAxis = 0; //0 = zI, 1 = cR, 2 = cI

		//viewport region
		xScale = 2.6;
		yScale = 2.3;
		xOffset = 0.0;
		yOffset = 0.0;

		//orbit start region
		orbitXScale = mandelbrotXScale;
		orbitYScale = mandelbrotYScale;
		orbitXOffset = mandelbrotXOffset;
		orbitYOffset = mandelbrotYOffset;

		doCustomOrbitStart = false;

		//julia seed
		sR = 0.0;
		sI = 0.0;

		paintMirror = false;

		skipIsOutsideMandel = false;

		// ----- end settings -----

		bInitialized = false;

		//init(); //initialize the canvas for the default settings

		
	};

	void init();

	void doSamplesNaive(int numOrbits);
	void doSamplesAperture(int maxMutations);
	void doSamplesAperture2(int maxMutations);
	void doSamplesMH(int numOrbits);
	void saveToFile(const char* fileName);
	void genTexData();
	void saveToBMP(const char* fileName, float rColScale, float gColScale, float bColScale);
	void saveStartMapToBMP(const char* fileName);

	void fillRegion(int sX, int sY, int w, int h, float val = -1);
	void brightenRegion(int sX, int sY, int w, int h, float val = -1);
	void wipeImage();
	int width;
	int height;

	float gamma;
	float brightness;

	unsigned int mostHits;
	double mutStr;
	GLubyte* texData; //texture data prepared for OpenGL


private:

	int halfHeight;

	int noMoves;

	int numScores;
	int nextScore;

	int avgDepth;

	bool bInitialized;

	int iterations;
	int minIts;
	char debugStr[256];
	unsigned long totalSamples;
	unsigned int totalR, totalG, totalB;
	double xScale, yScale;
	double xOffset, yOffset;

	//orbit start region (mandelbrot region of the Z complex plane)
	const double mandelbrotXScale = 2.425;
	const double mandelbrotYScale = 2.325;
	const double mandelbrotXOffset = -0.685;
	const double mandelbrotYOffset = 0.0;

	//for a specific region for orbits to start within
	double orbitXScale;
	double orbitYScale;
	double orbitXOffset;
	double orbitYOffset;

	bool doCustomOrbitStart;

	//start-end colour regions
	unsigned int sRed, eRed;
	unsigned int sGreen, eGreen;
	unsigned int sBlue, eBlue;

	unsigned int paintStartIts;

	double bRed, bGreen, bBlue;



	double sI, sR;

	bool fourDims;

	int yAxis, xAxis;


	int colMethod;

	bool paintMirror;

	bool skipIsOutsideMandel;

	bool paintStartMap;

	bool isOutsideMandel(double r, double i);
	
	struct MutationScore{
		double x, y;

		unsigned long score;
		unsigned int numOrbits;

		unsigned long avgOrbitLength;

		double halfBoxWidth;

		MutationScore(){
			x = y = 0.0;
			score = 0;
			numOrbits = 0;
			halfBoxWidth = 0.0;
			avgOrbitLength = 0;
		}

	};

	struct OrbitLoc{
		double x, y;

		OrbitLoc(){
			x = y = 0.0;
		}
	};

	struct Pixel{
		double r, g, b;
		double hits;

		Pixel(){
			clear();
		}

		void clear(){
			r = 0;
			g = 0;
			b = 0;

			hits = 1;
		}

	};

	struct NullCircle{
		double r,  i,  radius;

		void setTo(double sR, double sI, double sRadius){
			r = sR;
			i = sI;
			radius = sRadius;
		}

		//true is returned if the value given is within the circle
		bool test(double tR, double tI){
			//return false;
			return (sqrt((r - tR) * (r - tR) + (i - tI)*(i - tI)) < radius);
		}
	};
	int nullZonesC;

	//stores areas of the z plane to ignore, as they are certainly part of the set
	NullCircle* nullZones;

	MutationScore* scores;
	
	Pixel* image; //image data for current fractal
	unsigned long* orbitStartMap; //image data for where orbits that painted started on the complex plane
	unsigned long startMapBrightest;

	OrbitLoc *orbitHistory; //locations passed through by one orbit
	//NOTE: This would be better on the stack but as it is of undetermined size I will trust the compiler and CPU cache to optimise as necessary (the assembly should be tested at some point)

	//stats/measures
	unsigned long totalAvgBoxOrbitLengths;
	unsigned long orbitsPainted;
	unsigned long orbitsChecked;
	unsigned long boxesDone;

};



#endif
