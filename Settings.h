#ifndef HEADER_SETTINGS
#define HEADER_SETTINGS


#include <fstream>

#include "tinyxml2.h"

class Settings
{

private:

    tinyxml2::XMLDocument* settingsFile; //handle to tinyxml2 document


public:


	int xRes;
	int yRes;

	int maxIts; //max iterations to perform for one orbit
	int minIts; //min iterations to count as valid orbit to paint

	//start - end orbit depths (n) for paint ranges
	unsigned int sRed, eRed;
	unsigned int sGreen, eGreen;
	unsigned int sBlue, eBlue;
	float rStart, rEnd, gStart, gEnd, bStart, bEnd; //a more human-readable method of setting these

	unsigned int paintStartIts; //any iteration locations below this count, in an orbit, are not assessed for painting

	//1 = sqrt, otherwise linear
	int colMethod;

	float brightness, gamma;

	double mutStr;  //casting box size
	double mutStrFocus; //casting box size when in "foucs mode"
	int maxMutations; //number of mutations (orbit attempts) within a box before moving the box
	int maxMutationsFocus; //number of mutations when in focus mode

	//viewport
	double xScale, yScale;
	double xOffset, yOffset;

	//orbit start area
	bool doCustomOrbitStart;
	double orbitXScale, orbitYScale, orbitXOffset, orbitYOffset;

	double sI, sR; //julia seed
	int xAxis, yAxis; //defines the complex plane to paint
	bool fourDims; //experimantal (additional randomization for orbit start seed)
	
	bool paintMirror;

	bool skipIsOutsideMandel;
	bool forceSkipIsOutsideMandel;

	bool paintStartMap;

	int orbitMode;

	Settings();
	~Settings();

	void init(); //post-processes settings to clean them up

	int loadFromFile(const char* path);
};

#endif
