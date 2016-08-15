#include "Settings.h"


//the constructor sets up some workable defaults
Settings::Settings()
{

	// ----- Orbits

	//if an orbit has not been determined as being outside the set in at least this many iterations (n, AKA orbit depth), discard it
	maxIts = 10000;
	//if an orbit _has_ been determined as being outside the set in less than this many iterations, discard it
	minIts = 18;


	//how orbit start locations are decided
	orbitMode = 2; // 2; //0 = naive (full random), 1 = apertures (my strange method of finding detailed orbit starts), 2 = metropolis hastings


	//GA (Genetic Algorithm) settings, the GA allows for painting more detailed orbits (only for apertures orbit mode)

	//the size of the orbit casting "box", a smaller number makes a smaller box
	//orbits casts are begun from random seeds within the box (instead of the whole plane) 
	//until the box moves (a "set" of casts), to a new random location
	//mutStr = 0.005;
	//mutStr = 0.0025;
	mutStr = 0.0075;
	mutStrFocus = 0.0025; //smaller casting box for more exaggerated GA outcome when in "focus mode" (hold I by default)
	//mutation set "size", the box moves once this reaches 0
	//maxMutations = 4096;
	maxMutations = 2048;
	maxMutationsFocus = 4096;


	// ----- Painting

	xRes = 768;
	yRes = 768;

	brightness = 1.0f;
	gamma = 1.0f;

	//important: this applies when outputting the buddhabrot to an RGB bitmap (texture or image file). Not when painting orbits.
	colMethod = 1; //1 = sqrt (better), otherwise linear (poorly shows details).


	//viewport:

	//these settings are good for a 0.0, 0.0 julia seed and viewing the zR, zI plane ("default")
	xScale = 2.7;
	yScale = 2.5;
	xOffset = -0.48;
	yOffset = 0;

	/*
	xScale = 3.0;
	yScale = 2.5;
	xOffset = -0.5;
	yOffset = 0.0;
	*/
	
	/*
	xScale = .017;
	yScale = .017;
	xOffset = -0.158;
	yOffset = 1.034;
	*/

	//good for zooming into a minibrot on the stalk
	//xScale = 0.0002;
	//yScale = 0.0002;

	//xScale = 0.00018;
	//yScale = 0.0002;
	//xOffset = -1.484472;
	//yOffset = 0;
	


	//start - end orbit depths (n) for paint ranges
	//intensity is minimum at n=s and maximum at n=e (default painting method)
	//where n trends from minIts+start*(maxIts-minIts) to minIts+end*(maxIts-minIts) 
	
	rStart = 0.002f, rEnd = 0.50f;
	gStart = 0.020f, gEnd = 0.75f;
	bStart = 0.200f, bEnd = 1.00f;

	//rStart = 0.01f, rEnd = 0.25f;
	//gStart = 0.2f, gEnd = 0.3f;
	//bStart = 0.28f, bEnd = 1.0f;

	/*
	sRed = 16; eRed = 512;
	sGreen = 64; eGreen = 1024;
	sBlue = 96; eBlue = 1100;
	*/


	// ----- Advanced

	//interesting visuals can be achieved on planes other than Zr, Zi
	xAxis = 0; //0 = Zr, 1 = Zi, 2 = Cr
	yAxis = 0; //0 = Zi, 1 = Cr, 2 = Ci

	//seed for juliabrot (0.0, 0.0 = mandel/buddhabrot)
	sR = 0; // -0.7;
	sI = 0; // -0.7;


	fourDims = false; //if true, z is also seeded at random

	paintMirror = false; //if using metropolis hastings, this enables mirroring painting around the center x axis of the image (so it should be false if not rendering a symmetrical viewport)
	
	skipIsOutsideMandel = false; //if true, isOutsideMandel is skipped
	forceSkipIsOutsideMandel = false; //if true, override automation of this setting

	paintStartMap = false; //if true, a buffer will be saved of where orbits that resulted in painting began from

	//custom orbit start region on the Z complex plane (defaults to mandelbrot range if false)
	doCustomOrbitStart = false;
	//mirrorCustomOrbitStart = true;

	/*
	orbitXScale = 0.016;
	orbitYScale = 0.016;
	orbitXOffset = -.1585;
	orbitYOffset = 1.033;
	*/

	/*
		xScale = 0.00018;
		yScale = 0.0002;
		xOffset = -1.484472;
	*/
	/*
	orbitXScale = 0.00019;
	orbitYScale = 0.00021;
	orbitXOffset = -1.484474;
	orbitYOffset = 0.0;
	*/
}

void Settings::init(){
	
	// ----- Internal processing

	//start - end orbit depths (n) for paint ranges
	//intensity is minimum at n=s and maximum at n=e (default colouring method)

	paintStartIts = minIts;
	//paintStartIts = 4; //checking if there's any reason for paintStartIts to be different from minIts (maybe with some exotic colouring/painting)

	int intsDiff = maxIts - paintStartIts;
	sRed = paintStartIts + int(intsDiff * rStart);
	eRed = paintStartIts + int(intsDiff * rEnd);
	sGreen = paintStartIts + int(intsDiff * gStart);
	eGreen = paintStartIts + int(intsDiff * gEnd);
	sBlue = paintStartIts + int(intsDiff * bStart);
	eBlue = paintStartIts + int(intsDiff * bEnd);

	paintStartIts = (sRed < ( (sBlue < sGreen)? sBlue : sGreen))?  sRed : ( (sBlue < sGreen)? sBlue : sGreen); //the smallest


	//skip quick mandelbrot region check if the plane is not default and skipIsOutsideMandel is not forced
	if(!forceSkipIsOutsideMandel && (xAxis != 0 && yAxis!= 0))
		skipIsOutsideMandel = true;


	//sRed = minIts;
	//sGreen = minIts;
	//sBlue = minIts;

	/*
	sRed = 16; eRed = 512;
	sGreen = 64; eGreen = 1024;
	sBlue = 96; eBlue = 1100;
	*/

	/*
	if (xPlane == 1 && yPlane == 2){ //any other viewports are bad for the zI cI plane so override the setting
		xScale = 4.2;
		yScale = 2.3;
		xOffset = 0.0;
		yOffset = 0.0;
	}
	*/
	

}

Settings::~Settings()
{
}


using namespace tinyxml2;

int Settings::loadFromFile(const char* path){

	settingsFile = new XMLDocument ();
    
	XMLError xmlErr = settingsFile->LoadFile(path);
	
	if (xmlErr != XML_SUCCESS){

		FILE* xmlErrFile;
		//fopen_s(&xmlErrFile, "xmlErr.txt", "w");
		//fprintf_s(xmlErrFile, "settings load failed: %d\n", xmlErr);
		xmlErrFile = fopen("xmlErr.txt", "w");
		fprintf(xmlErrFile, "settings load failed: %d\n", xmlErr);
		fclose(xmlErrFile);

		if(xmlErr != XML_ERROR_FILE_NOT_FOUND)
			return -1; //failed

		return 0;
	}

	//find the first <brot> element with an "active" attribute set to true
	XMLElement* brot = settingsFile->FirstChildElement("brot");
	while(brot != NULL){

		bool bActive = false;
		brot->QueryBoolAttribute("active", &bActive);

		if(!bActive){
			brot = brot->NextSiblingElement("brot");
			continue;
		}

		//found our way here? then this is the (first, and only one we care about) active <brot>


		//~read window resolution
		

		// -- read texture resolution

		int sXRes, sYRes;
		int sRes = -1;
		brot->QueryIntAttribute("resolution", &sRes);
		switch(sRes){ //which resolutions can be used depends on the graphics device
			case 0:
				sXRes = sYRes = 512;
				break;
			case 1:
				sXRes = sYRes = 768;
				break;
			case 2:
				sXRes = sYRes = 1024;
				break;
			case 3:
				sXRes = sYRes = 2048;
				break;
			case 4:
				sXRes = sYRes = 4096;
				break;
			case 5:
				sXRes = sYRes = 8192;
				break;
		}

		//also read exact resolutions if specified
		brot->QueryIntAttribute("xRes", &sXRes);
		brot->QueryIntAttribute("yRes", &sYRes);
		
		//constrain values
		if(sXRes < 128) sXRes = 128;
		if(sYRes < 128) sYRes = 128;

		//apply the read values
		xRes = sXRes;
		yRes = sYRes;


		// -- read min and max iterations

		brot->QueryIntAttribute("minIts", &minIts);
		brot->QueryIntAttribute("maxIts", &maxIts);

		//constrain values
		if(minIts < 1) minIts = 1;
		if(maxIts < minIts) maxIts = minIts;

		
		// -- read colour start & end depths

		//XMLElement *col = brot->FirstChildElement("col");
		//if(col != NULL){
		
			//XMLElement *red = col->FirstChildElement("red");
			XMLElement *red = brot->FirstChildElement("red");
			if(red != NULL){
				red->QueryFloatAttribute("start", &rStart);
				red->QueryFloatAttribute("end", &rEnd);
				if(rStart < 0.0f) rStart = 0.0f;
				if(rStart > 1.0f) rStart = 1.0f;
				if(rEnd < rStart) rEnd = rStart;
			}
		
			//XMLElement *green = col->FirstChildElement("green");
			XMLElement *green = brot->FirstChildElement("green");
			if(red != NULL){
				green->QueryFloatAttribute("start", &gStart);
				green->QueryFloatAttribute("end", &gEnd);
				if(gStart < 0.0f) gStart = 0.0f;
				if(gStart > 1.0f) gStart = 1.0f;
				if(gEnd < gStart) gEnd = gStart;
			}
		
			//XMLElement *blue = col->FirstChildElement("blue");
			XMLElement *blue = brot->FirstChildElement("blue");
			if(red != NULL){
				blue->QueryFloatAttribute("start", &bStart);
				blue->QueryFloatAttribute("end", &bEnd);
				if(bStart < 0.0f) bStart = 0.0f;
				if(bStart > 1.0f) bStart = 1.0f;
				if(bEnd < bStart) bEnd = bStart;
			}

		//}


		// -- read orbit mode

		brot->QueryIntAttribute("orbitMode", &orbitMode);
		if(orbitMode < 0) orbitMode = 0;
		if(orbitMode > 2) orbitMode = 2;
		

		// -- read viewport settings (x/y scale and x/y offset) if set

		XMLElement* viewport = brot->FirstChildElement("viewport");
		if(viewport != NULL){
			float sXScale, sYScale, sXOffset, sYOffset;
			viewport->QueryFloatAttribute("xScale", &sXScale);
			viewport->QueryFloatAttribute("yScale", &sYScale);

			viewport->QueryFloatAttribute("xOffset", &sXOffset);
			viewport->QueryFloatAttribute("yOffset", &sYOffset);

			
			//apply read values (can only read floats, no problem, but it's a double internally)
			xScale = sXScale;
			yScale = sYScale;
			xOffset = sXOffset;
			yOffset = sYOffset;

			//clamp scales as their internal form (double)
			if(xScale < 0.) xScale = 0.;
			if(yScale < 0.) yScale = 0.;
		}


		// -- read GA settings if orbit mode is apertures

		if(orbitMode == 1){
			XMLElement* GA = brot->FirstChildElement("GA");

			if(GA != NULL){
				float sMutStr, sMutStrFocus;
				GA->QueryFloatAttribute("mutStr", &sMutStr);
				GA->QueryFloatAttribute("mutStrFocus", &sMutStrFocus);
			
				if(sMutStr < 0.f) sMutStr = 0.f; //0. is way too small, but let the user do what they want because it shouldn't cause bugs
				if(sMutStr > 1.f) sMutStr = 1.f; //1. is huge, same as above
			
				if(sMutStrFocus < 0.f) sMutStrFocus = 0.f;
				if(sMutStrFocus > 1.f) sMutStrFocus = 1.f;
				//don't even constrain this ^ to be sensible in relation to mutStr, it's the user's choice
			
				//apply read values (can only read floats, no problem, but it's a double internally)
				mutStr = sMutStr;
				mutStrFocus = sMutStrFocus;
			

				GA->QueryIntAttribute("maxMutations", &maxMutations);
				if(maxMutations < 0) maxMutations = 0;

				GA->QueryIntAttribute("maxMutationsFocus", &maxMutationsFocus);
				if(maxMutationsFocus < 0) maxMutationsFocus = 0;
				//again, don't even constrain this ^ to be sensible in relation to maxMutations, it's the user's choice
			}

		}


		// -- read complex planes to be rendered as x/y
		
		brot->QueryIntAttribute("xAxis", &xAxis);
		brot->QueryIntAttribute("yAxis", &yAxis);

		//clamp
		if(xAxis < 0) xAxis = 0;
			else if(xAxis > 2) xAxis = 2;

		if(yAxis < 0) yAxis = 0;
			else if(yAxis > 2) yAxis = 2;

		
		// -- read julia seed

		float sSR = float(sR), sSI = float(sI);
		brot->QueryFloatAttribute("sR", &sSR);
		brot->QueryFloatAttribute("sI", &sSI);
		
		sR = sSR;
		sI = sSI;
		

		// -- read custom orbit start, if set

		XMLElement* orbitStart = brot->FirstChildElement("orbitStart");
		if(orbitStart != NULL){

			doCustomOrbitStart = true;

			float sOXScale, sOYScale, sOXOffset, sOYOffset; //this "convention" is getting hard to read
			orbitStart->QueryFloatAttribute("xScale", &sOXScale);
			orbitStart->QueryFloatAttribute("yScale", &sOYScale);

			orbitStart->QueryFloatAttribute("xOffset", &sOXOffset);
			orbitStart->QueryFloatAttribute("yOffset", &sOYOffset);

			
			//apply read values (can only read floats, no problem, but it's a double internally)
			orbitXScale = sOXScale;
			orbitYScale = sOYScale;
			orbitXOffset = sOXOffset;
			orbitYOffset = sOYOffset;

			//clamp scales as their internal form (double)
			if(orbitXScale < 0.) orbitXScale = 0.;
			if(orbitYScale < 0.) orbitYScale = 0.;
		}


		// -- some more boolean values
		
		brot->QueryBoolAttribute("paintStartMap", &paintStartMap);
		
		brot->QueryBoolAttribute("randImaginary", &fourDims);

		brot->QueryBoolAttribute("paintMirror", &paintMirror);

		brot->QueryBoolAttribute("skipIsOutsideMandel", &skipIsOutsideMandel);
		brot->QueryBoolAttribute("forceSkipIsOutsideMandel", &forceSkipIsOutsideMandel);


		// -- should also add int-based colour ranges

		// -- allowing the start paint value to be specified instead of it being automated to the minimum colour range would also be good


		break; //don't infinite loop
	}

	delete settingsFile; //deconstruct the settings file

	return 1; //success
}
