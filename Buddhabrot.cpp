#include "Buddhabrot.h"
#include "mt.h"

void Buddhabrot::applySettings(Settings* toApply){

	width = toApply->xRes;
	height = toApply->yRes;

	iterations = toApply->maxIts;

	brightness = toApply->brightness;
	gamma = toApply->gamma;

	colMethod = toApply->colMethod;

	minIts = toApply->minIts;

	sRed = toApply->sRed; eRed = toApply->eRed;
	sGreen = toApply->sGreen; eGreen = toApply->eGreen;
	sBlue = toApply->sBlue; eBlue = toApply->eBlue;

	paintStartIts = toApply->paintStartIts;
	//paintStartIts = min(sRed, min(sGreen, sBlue));

	mutStr = toApply->mutStr;

	fourDims = toApply->fourDims;

	paintMirror = toApply->paintMirror;

	skipIsOutsideMandel = toApply->skipIsOutsideMandel;

	paintStartMap = toApply->paintStartMap;

	xAxis = toApply->xAxis;
	yAxis = toApply->yAxis;

	xScale = toApply->xScale;
	yScale = toApply->yScale;
	xOffset = toApply->xOffset;
	yOffset = toApply->yOffset;

	if (toApply->doCustomOrbitStart){

		doCustomOrbitStart = true;

		orbitXScale = toApply->orbitXScale;
		orbitYScale = toApply->orbitYScale;
		orbitXOffset = toApply->orbitXOffset;
		orbitYOffset = toApply->orbitYOffset;
	} else {
		doCustomOrbitStart = false;

		orbitXScale = mandelbrotXScale;
		orbitYScale = mandelbrotYScale;
		orbitXOffset = mandelbrotXOffset;
		orbitYOffset = mandelbrotXOffset;
	}

	sR = toApply->sR;
	sI = toApply->sI;

	init();

}

void Buddhabrot::init(){

	init_genrand((unsigned)time(NULL));

	halfHeight = height / 2;

	//define a blank image
	delete[] image;
	image = new Pixel[width*height];

	for (int x = 0; x < width; x++)
		for (int y = 0; y < height; y++)
			image[x + width * y].clear();


	//delete start map memory
	delete[] orbitStartMap;
	orbitStartMap = nullptr;  //set to nullptr again because this memory is optionally allocated

	//define the startMap, if enabled
	if (paintStartMap){
		startMapBrightest = 0;
		orbitStartMap = new unsigned long[width*height];
		memset(orbitStartMap, 0, width*height*sizeof(unsigned long));
	}

	//define the texture data
	delete[] texData;
	texData = new GLubyte[width*height * 3];

	memset(debugStr, 0, sizeof(debugStr));
	memset(texData, 0, sizeof(GLubyte) * width*height * 3);

	//define the orbit history
	delete[] orbitHistory;
	orbitHistory = new OrbitLoc[iterations];


	mostHits = 1;
	orbitsPainted = orbitsChecked = boxesDone = totalAvgBoxOrbitLengths = 0;
	totalSamples = totalR = totalG = totalB = 1;
	bRed = bGreen = bBlue = 1;

	noMoves = 0;

	numScores = 0;
	nextScore = 0;


	//delete[] scores;
	//scores = new MutationScore[numScores + 1];

	avgDepth = 1;

	delete[] nullZones;

	nullZonesC = 6;
	if (nullZonesC > 0){
		nullZones = new NullCircle[nullZonesC];
		nullZones[0].setTo(-1.0, 0.0, 0.2475); //2nd bulb (big bit at left of mandelbrot)
		nullZones[1].setTo(-0.125, -0.745, 0.0915); //other bulbs above and below main bulb
		nullZones[2].setTo(-0.125, 0.745, 0.0915);

		//farthest into negative real axis bulb
		nullZones[3].setTo(-1.3075, 0.0, 0.0535);

		//other smaller side bulbs
		nullZones[4].setTo(-0.502525, -0.56252, 0.037);
		nullZones[5].setTo(-0.502525, 0.56252, 0.037);

		//TODO: profile performance to determine roughly how many nullZones gain a performance payoff
		//      could also allow them to be specified in settings
	}


	bInitialized = true;

}

bool Buddhabrot::isOutsideMandel(double r, double i){

	/*
	  this can be passed values other than Zr Zi.

	  However, for different planes they would have to be shifted relative to the orbitStart region.
	  This would increase performance in many cases but profiling would have to be done to determine when this is necessary (and that could vary depending on settings and the system >_<).

	*/

	double zRSq = r*r, zISq = i*i;

	//check if location is within the main bulb
	double zxminquart = r - 0.25;
	double q = zxminquart*zxminquart + zISq;
	if (q*(q + zxminquart) < zISq*0.25)
		return false;

	if (nullZonesC > 0)
		for (int j = 0; j < nullZonesC; j++)
			if (nullZones[j].test(r, i)) return false;


	return true;

}

void Buddhabrot::wipeImage(){
	int mX = width, mY = height;
	for (int aS = 0; aS < mX; aS++){
		for (int aY = 0; aY < mY; aY++){
			if (aS >= 0 && aS < width && aY > 0 && aY < height){
				image[aS + width * aY].r = 0.0;
				image[aS + width * aY].g = 0.0;
				image[aS + width * aY].b = 0.0;
				image[aS + width * aY].hits = 0.0;
			}
		}
	}

	if(paintStartMap){
		startMapBrightest = 0;
		memset(orbitStartMap, 0, width*height*sizeof(unsigned long));
	}

	mostHits = 1;
	totalSamples = totalR = totalG = totalB = 1;
	bRed = bGreen = bBlue = 1;

}

void Buddhabrot::fillRegion(int sX, int sY, int w, int h, float val){
	int maxX = sX + w, maxY = sY + h;
	for (int aS = sX; aS < maxX; aS++){
		for (int aY = sY; aY < maxY; aY++){
			if (aS >= 0 && aS < width && aY > 0 && aY < height){
				double o = (val < 0) ? totalSamples : int(totalSamples*val);
				image[aS + width * aY].r = o;
				image[aS + width * aY].g = o;
				image[aS + width * aY].b = o;
				image[aS + width * aY].hits = o;
			}
		}
	}
}

void Buddhabrot::brightenRegion(int sX, int sY, int w, int h, float val){
	int mX = sX + w, mY = sY + h;
	for (int aS = sX; aS < mX; aS++){
		for (int aY = sY; aY < mY; aY++){
			if (aS >= 0 && aS < width && aY > 0 && aY < height){
				unsigned int o = (val < 0) ? totalSamples : int(totalSamples*val);
				image[aS + width * aY].r += o;
				image[aS + width * aY].g += o;
				image[aS + width * aY].b += o;
				image[aS + width * aY].hits += o;
			}
		}
	}
}

void Buddhabrot::saveToFile(const char* fileName){

	char toSave[512];

	std::ofstream out("buddhaLog.txt", std::ios::out | std::ios::trunc);

/*
	out.write(toSave, sprintf_s(toSave, " -- stats -- \n"));
	out.write(toSave, sprintf_s(toSave, "noMoves: %d\n", noMoves));
	out.write(toSave, sprintf_s(toSave, "Orbits:\n%d - painted, %d - checked\nsamples painted: %d\n", orbitsPainted, orbitsChecked, totalSamples));
	out.write(toSave, sprintf_s(toSave, "Visits:\nmaxRed: %f\nmaxGreen: %f\nmaxBlue: %f\n", bRed, bGreen, bBlue));
*/
	out.write(toSave, sprintf(toSave, " -- stats -- \n"));
	out.write(toSave, sprintf(toSave, "noMoves: %d\n", noMoves));
	out.write(toSave, sprintf(toSave, "Orbits:\n%lu - painted, %lu - checked\nsamples painted: %lu\n", orbitsPainted, orbitsChecked, totalSamples));
	out.write(toSave, sprintf(toSave, "Visits:\nmaxRed: %f\nmaxGreen: %f\nmaxBlue: %f\n", bRed, bGreen, bBlue));


	//out.write(toSave, sprintf_s(toSave, "Average orbit depth: %d\n", avgDepth));

	//unsigned long averageBoxOrbitLength = totalAvgBoxOrbitLengths / max(1, boxesDone);
	//out.write(toSave, sprintf_s(toSave, "Average orbit depth: %d\n", averageBoxOrbitLength));

/*
	out.write(toSave, sprintf_s(toSave, " -- settings -- \n"));
	out.write(toSave, sprintf_s(toSave, "min/max its: %d/%d\n", minIts, iterations));
	out.write(toSave, sprintf_s(toSave, "brightness: %f\n", brightness));
	out.write(toSave, sprintf_s(toSave, "red min/max: %d/%d (%d)\n", sRed, eRed, totalR));
	out.write(toSave, sprintf_s(toSave, "green min/max: %d/%d (%d)\n", sGreen, eGreen, totalG));
	out.write(toSave, sprintf_s(toSave, "blue min/max: %d/%d (%d)\n", sBlue, eBlue, totalB));
*/
	out.write(toSave, sprintf(toSave, " -- settings -- \n"));
	out.write(toSave, sprintf(toSave, "min/max its: %d/%d\n", minIts, iterations));
	out.write(toSave, sprintf(toSave, "brightness: %f\n", brightness));
	out.write(toSave, sprintf(toSave, "red min/max: %d/%d (%d)\n", sRed, eRed, totalR));
	out.write(toSave, sprintf(toSave, "green min/max: %d/%d (%d)\n", sGreen, eGreen, totalG));
	out.write(toSave, sprintf(toSave, "blue min/max: %d/%d (%d)\n", sBlue, eBlue, totalB));

	//out.write(toSave, sprintf_s(toSave, "Julia seed: %f:%f\n", sR, sI));
	out.write(toSave, sprintf(toSave, "Julia seed: %f:%f\n", sR, sI));


	unsigned long scoreAvg = 0;
	unsigned long scoreTotal = 0;
	unsigned long smallestScore = ULONG_MAX, biggestScore = 0;
	unsigned int mostOrbits = 0;

	/*
	for (int i = 0; i < nextScore; i++){
	unsigned long thisScore = scores[i].score;

	scoreTotal += thisScore;

	if (thisScore < smallestScore)
	smallestScore = thisScore;
	else if (thisScore > biggestScore)
	biggestScore = thisScore;

	else if (scores[i].numOrbits > mostOrbits)
	mostOrbits = scores[i].numOrbits;

	}

	scoreAvg = scoreTotal / max(nextScore, 1);

	out.write(toSave, sprintf_s(toSave, "number of box scores: %d\n", nextScore));
	out.write(toSave, sprintf_s(toSave, "total box score: %d\n", scoreTotal));
	out.write(toSave, sprintf_s(toSave, "average box score: %d\n", scoreAvg));
	out.write(toSave, sprintf_s(toSave, "min-max box scores: %d-%d\n", smallestScore, biggestScore));
	out.write(toSave, sprintf_s(toSave, "most orbits in a box: %d\n", mostOrbits));


	//order scores
	MutationScore hold;
	for (int i = 0; i<nextScore; i++)
	{
	for (int j = 0; j<nextScore; j++)
	{
	if (scores[j].numOrbits > scores[j + 1].numOrbits)
	{
	hold = scores[j];
	scores[j] = scores[j + 1];
	scores[j + 1] = hold;
	}
	}
	}

	int numChanged = 0;
	int j = nextScore;
	for (int i = 0; i < nextScore; i++){
	//out.write(toSave, sprintf_s(toSave, "%d,", scores[i].score));
	//if ((i + 1) % 15 == 0) out.write(toSave, sprintf_s(toSave, "\n"));
	Buddhabrot::MutationScore* item = &scores[i];
	int imgX = int(width * ((item->x - orbitXOffset) / orbitXScale + 0.5));
	int imgY = int(height * ((item->y - orbitYOffset) / orbitYScale + 0.5));
	int halfBoxWidth2 = static_cast<int>(width * item->halfBoxWidth * xScale);
	int halfBoxHeight2 = static_cast<int>(height * item->halfBoxWidth * yScale);
	//brightenRegion(imgX - halfBoxWidth, imgY - halfBoxHeight, halfBoxWidth<<1, halfBoxHeight<<1, item->numOrbits/float(mostOrbits));
	fillRegion(imgX - halfBoxWidth2, imgY - halfBoxHeight2, 2*halfBoxWidth2, 2*halfBoxHeight2, item->numOrbits / float(mostOrbits));
	numChanged++;

	}

	out.write(toSave, sprintf_s(toSave, "high score regions painted: %d\n", numChanged));
	*/



	//delete [] toSave;



	//hellz yeah, ascii mode
	/*
	double magO = -1;
	for(int x = 0; x < width; x ++){
	for(int y = 0; y < height; y ++){
	//out.write(toSave, sprintf(toSave, "%d ", image[x][y]));

	int val = image[x][y]->hits;
	if(val == 0){
	out.write(toSave, sprintf(toSave, "  "));
	continue;
	}
	double mag = (static_cast<double>(val)/totalSamples)*0.8;
	if(mag < 0.05){
	out.write(toSave, sprintf(toSave, "  "));
	magO = mag;
	} else if(mag < 0.1){
	out.write(toSave, sprintf(toSave, ". "));
	} else if(mag < 0.3){
	out.write(toSave, sprintf(toSave, "' "));
	} else if(mag < 0.4){
	out.write(toSave, sprintf(toSave, "~ "));
	} else if(mag < 0.5){
	out.write(toSave, sprintf(toSave, "v "));
	} else if(mag < 0.6){
	out.write(toSave, sprintf(toSave, "z "));
	} else if(mag < 0.71){
	out.write(toSave, sprintf(toSave, "x "));
	} else if(mag < 0.82){
	out.write(toSave, sprintf(toSave, "Z "));
	} else if(mag < 0.9){
	out.write(toSave, sprintf(toSave, "X "));
	} else if(mag < 0.97){
	out.write(toSave, sprintf(toSave, "# "));
	} else if(mag == 1){
	out.write(toSave, sprintf(toSave, "1 "));
	} else {
	out.write(toSave, sprintf(toSave, "@ "));
	}

	}

	}
	*/

}

//generate an RGB_32 GLUint texture in texData
void Buddhabrot::genTexData(){

	unsigned int loc = 0; //position in texData

	for (int y = 0; y < height; y++){
		for (int x = 0; x < width; x++){

			Pixel &pRead = image[x + width * y];

			if (colMethod == 1){ //sqrt colouring

				GLubyte

				//imgMag = static_cast<GLubyte>(std::trunc(255 * std::sqrt(brightness * pRead.r) / max(1, std::sqrt(gamma*bRed))));
				imgMag = static_cast<GLubyte>(std::trunc(255 * std::sqrt(brightness * pRead.r) / std::max(1., std::sqrt(gamma*bRed))));
				texData[loc] = imgMag;
				loc++;

				//imgMag = static_cast<GLubyte>(std::trunc(255 * std::sqrt(brightness * pRead.g) / max(1, std::sqrt(gamma*bGreen))));
				imgMag = static_cast<GLubyte>(std::trunc(255 * std::sqrt(brightness * pRead.g) / std::max(1., std::sqrt(gamma*bGreen))));
				texData[loc] = imgMag;
				loc++;

				//imgMag = static_cast<GLubyte>(std::trunc(255 * std::sqrt(brightness * pRead.b) / max(1, std::sqrt(gamma*bBlue))));
				imgMag = static_cast<GLubyte>(std::trunc(255 * std::sqrt(brightness * pRead.b) / std::max(1., std::sqrt(gamma*bBlue))));
				texData[loc] = imgMag;
				loc++;

			}
			else { //linear colouring

				GLubyte

					imgMag = static_cast<GLubyte>(255 * std::min(1., (brightness * pRead.r) / std::max(1., gamma*bRed)));
				texData[loc] = imgMag;
				loc++;

				imgMag = static_cast<GLubyte>(255 * std::min(1., (brightness * pRead.g) / std::max(1., gamma*bGreen)));
				texData[loc] = imgMag;

				loc++;
				imgMag = static_cast<GLubyte>(255 * std::min(1., (brightness * pRead.b) / std::max(1., gamma*bBlue)));
				texData[loc] = imgMag;
				loc++;
			}


		}
	}
}


void Buddhabrot::saveStartMapToBMP(const char* fileName){
/*
	HANDLE file;
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER fileInfo;
	RGBTRIPLE *bitmapdata;
	DWORD write = 0;

	file = CreateFile(L"buddhaStartMap.bmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	bitmapdata = new RGBTRIPLE[width*height];

	fileHeader.bfType = 19778;
	fileHeader.bfSize = sizeof(fileHeader.bfOffBits) + sizeof(RGBTRIPLE);
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	fileInfo.biSize = sizeof(BITMAPINFOHEADER);
	fileInfo.biWidth = width;
	fileInfo.biHeight = height;
	fileInfo.biPlanes = 1;
	fileInfo.biBitCount = 24;
	fileInfo.biCompression = BI_RGB;
	fileInfo.biSizeImage = width * height * (24 / 8);
	fileInfo.biXPelsPerMeter = 2400;
	fileInfo.biYPelsPerMeter = 2400;
	fileInfo.biClrImportant = 0;
	fileInfo.biClrUsed = 0;

	WriteFile(file, &fileHeader, sizeof(fileHeader), &write, NULL);
	WriteFile(file, &fileInfo, sizeof(fileInfo), &write, NULL);


	int oi = 0;
	for (int oy = 0; oy < height; oy++){
		for (int ox = 0; ox < width; ox++){

			bitmapdata[oi].rgbtRed = bitmapdata[oi].rgbtGreen = bitmapdata[oi].rgbtBlue =
				static_cast<BYTE>(min(255, 255 * ((std::sqrt(orbitStartMap[ox + width * oy])) / max(1, std::sqrt(startMapBrightest)))));

			
			//if (colMethod == 1){
			//bitmapdata[oi].rgbtRed = static_cast<BYTE>(min(255, 255 * (rColScale*(std::sqrt(brightness * image[ox + width * oy].r)) / max(1, std::sqrt(gamma*bRed)))));
			//bitmapdata[oi].rgbtGreen = static_cast<BYTE>(min(255, 255 * (gColScale*(std::sqrt(brightness * image[ox + width * oy].g)) / max(1, std::sqrt(gamma*bGreen)))));
			//bitmapdata[oi].rgbtBlue = static_cast<BYTE>(min(255, 255 * (bColScale*(std::sqrt(brightness * image[ox + width * oy].b)) / max(1, std::sqrt(gamma*bBlue)))));
			//}
			//else {
			//bitmapdata[oi].rgbtRed = static_cast<BYTE>(min(255, rColScale*(image[ox + width * oy].r * 255 * brightness) / max(1, gamma*bRed)));
			//bitmapdata[oi].rgbtGreen = static_cast<BYTE>(min(255, gColScale*(image[ox + width * oy].g * 255 * brightness) / max(1, gamma*bGreen)));
			//bitmapdata[oi].rgbtBlue = static_cast<BYTE>(min(255, bColScale*(image[ox + width * oy].b * 255 * brightness) / max(1, gamma*bBlue)));
			//}
			

			oi += 1;


		}
	}

	WriteFile(file, bitmapdata, fileInfo.biSizeImage, &write, NULL);


	CloseHandle(file);

	delete[] bitmapdata;
*/
}

void Buddhabrot::saveToBMP(const char* fileName, float rColScale, float gColScale, float bColScale){
/*
	HANDLE file;
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER fileInfo;
	RGBTRIPLE *bitmapdata;
	DWORD write = 0;

	file = CreateFile(L"buddhaCPU.bmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	bitmapdata = new RGBTRIPLE[width*height];

	fileHeader.bfType = 19778;
	fileHeader.bfSize = sizeof(fileHeader.bfOffBits) + sizeof(RGBTRIPLE);
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	fileInfo.biSize = sizeof(BITMAPINFOHEADER);
	fileInfo.biWidth = width;
	fileInfo.biHeight = height;
	fileInfo.biPlanes = 1;
	fileInfo.biBitCount = 24;
	fileInfo.biCompression = BI_RGB;
	fileInfo.biSizeImage = width * height * (24 / 8);
	fileInfo.biXPelsPerMeter = 2400;
	fileInfo.biYPelsPerMeter = 2400;
	fileInfo.biClrImportant = 0;
	fileInfo.biClrUsed = 0;

	WriteFile(file, &fileHeader, sizeof(fileHeader), &write, NULL);
	WriteFile(file, &fileInfo, sizeof(fileInfo), &write, NULL);


	int oi = 0;
	for (int oy = 0; oy < height; oy++){
		for (int ox = 0; ox < width; ox++){
			if (colMethod == 1){
				bitmapdata[oi].rgbtRed = static_cast<BYTE>(min(255, 255 * (rColScale*(std::sqrt(brightness * image[ox + width * oy].r)) / max(1, std::sqrt(gamma * bRed)))));
				bitmapdata[oi].rgbtGreen = static_cast<BYTE>(min(255, 255 * (gColScale*(std::sqrt(brightness * image[ox + width * oy].g)) / max(1, std::sqrt(gamma * bGreen)))));
				bitmapdata[oi].rgbtBlue = static_cast<BYTE>(min(255, 255 * (bColScale*(std::sqrt(brightness * image[ox + width * oy].b)) / max(1, std::sqrt(gamma * bBlue)))));
			}
			else {
				bitmapdata[oi].rgbtRed = static_cast<BYTE>(min(255, rColScale*(image[ox + width * oy].r * 255 * brightness) / max(1, gamma * bRed)));
				bitmapdata[oi].rgbtGreen = static_cast<BYTE>(min(255, gColScale*(image[ox + width * oy].g * 255 * brightness) / max(1, gamma * bGreen)));
				bitmapdata[oi].rgbtBlue = static_cast<BYTE>(min(255, bColScale*(image[ox + width * oy].b * 255 * brightness) / max(1, gamma * bBlue)));
			}


			oi += 1;


		}
	}

	WriteFile(file, bitmapdata, fileInfo.biSizeImage, &write, NULL);


	CloseHandle(file);

	delete[] bitmapdata;
*/
}


void Buddhabrot::doSamplesNaive(int numOrbits){

	double cR, cI, r, i;


	//choose complex plane to render

	double *xPlaneVal, *yPlaneVal;
	switch (xAxis){
	case 0:
		xPlaneVal = &r;
		break;
	case 1:
		xPlaneVal = &i;
		break;
	case 2:
		xPlaneVal = &cR;
		break;
	default:
		xPlaneVal = &r;
	}

	switch (yAxis){
	case 0:
		yPlaneVal = &i;
		break;
	case 1:
		yPlaneVal = &cR;
		break;
	case 2:
		yPlaneVal = &cI;
		break;
	default:
		yPlaneVal = &i;
	}


	//compute orbits
	for (int orbitNum = 0; orbitNum < numOrbits; orbitNum++){

		//find a random location to begin an orbit from

		if (fourDims){
			cR = (genrand_real2() - 0.5)*orbitXScale + orbitXOffset;
			cI = (genrand_real2() - 0.5)*orbitYScale + orbitYOffset;

			r = (genrand_real2() - 0.5)*orbitXScale + orbitXOffset + sR;
			i = (genrand_real2() - 0.5)*orbitYScale + orbitYOffset + sI;
		}
		else {
			cR = (genrand_real2() - 0.5)*orbitXScale + orbitXOffset;
			cI = (genrand_real2() - 0.5)*orbitYScale + orbitYOffset;

			r = cR + sR;
			i = cI + sI;
		}

		
		if (skipIsOutsideMandel || isOutsideMandel(r, i)){

			double orbitStartR = *xPlaneVal;
			double orbitStartI = *yPlaneVal;

			bool doPaint = true;
			int j;
			double zRSq = r*r, zISq = i*i;
			for (j = 0; j < iterations; j++){

				i = r * i;
				i += i;
				i += cI + sI;

				r = zRSq - zISq + cR + sR;

				zRSq = r*r;
				zISq = i*i;


				//if escaped orbit
				if (zRSq + zISq > 4){

					//cancel paint if orbit escape was discovered in too few iterations
					if (j < minIts)
						doPaint = false;

					break;
				}

				//save to trajectory history
				orbitHistory[j].x = *xPlaneVal;
				orbitHistory[j].y = *yPlaneVal;
			}


			if (j >= iterations)
				continue;
			else if (doPaint == false)
				continue;


			//int lastImgX = -2, lastImgY = -2;
			//int noMove = 0;
			unsigned long lastTotalSamples = totalSamples;
			unsigned int jj = 0;
			//for each position in the trajectory history
			for (int o = j; o >= 0; o--){

				//map to image co-ordinates
				int imgX = int(((orbitHistory[jj].x - xOffset) / xScale + 0.5) * width);
				int imgY = int(((orbitHistory[jj].y - yOffset) / yScale + 0.5) * height);

				//if not out of bounds of the image view area
				if (!(imgY >= height || imgY < 0 || imgX >= width || imgX < 0)){

					Pixel *pOut = &image[imgX + width * imgY];

					double pixelVal = 1.0;

					//assess red range
					if (jj <= eRed && jj >= sRed){

						//float smooth = static_cast<float>((eRed - jj) / (jj - sRed + 1))
						//	+ quick
						//	- logf(logf(float(zRSq + zISq))) / logf(float(yScale));

						//float rMag = static_cast<float>((eRed - jj) / (jj - sRed + 1));
						//pixelVal = (cos(rMag) + 1)*rMag;//(cos(rMag) + 1);
						//pixelVal = cos(rMag)*rMag;

						if (pOut->r > bRed){ //record if brightest in this channel
							bRed = pOut->r;
						}
						else // if (pixelVal > 0)
							pOut->r++;
						//pOut->r += pixelVal;
						//pOut->r += smooth;

						//totalR += pixelVal;
					}

					if (jj <= eGreen && jj >= sGreen){

						//float smooth = static_cast<float>((eGreen - jj) / (jj - sGreen + 1))
						//	+ quick
						//	- logf(logf(float(zRSq + zISq))) / logf(float(yScale));

						//float gMag = static_cast<float>((eGreen - jj) / (jj - sGreen + 1));
						//pixelVal = (cos(gMag) + 1)*gMag; //(cos(gMag + 0.05) + 1);
						//pixelVal = cos(gMag + 0.05)*gMag;

						if (pOut->g > bGreen){
							bGreen = pOut->g;
						}
						else // if (pixelVal > 0)
							pOut->g++;
						//pOut->g += pixelVal;
						//pOut->g += smooth;

						//totalG += pixelVal;
					}

					if (jj <= eBlue && jj >= sBlue){

						//float smooth = static_cast<float>((eBlue - jj) / (jj - sBlue + 1))
						//	+ quick
						//	- logf(logf(float(zRSq + zISq))) / logf(float(yScale));

						//float bMag = static_cast<float>((eBlue - jj) / (jj - sBlue + 1));
						//pixelVal = (cos(bMag) + 1)*bMag; //(cos(bMag * 0.15) + 1);
						//pixelVal = cos(bMag * 0.15)*bMag;

						if (pOut->b > bBlue){
							bBlue = pOut->b;
						}
						else // if (pixelVal > 0)
							pOut->b++;
						//pOut->b += pixelVal;
						//pOut->b += smooth;

						//totalB += pixelVal;
					}

					//pOut->hits ++;
					totalSamples++;
					/*
					if(pOut->hits > mostHits){
					mostHits = pOut->hits;
					}
					*/
				}

				jj++;

			}

			orbitsPainted++; //just stats


			
			if (paintStartMap){

				//number of hits in the view area
				long totalSamplesDiff = totalSamples - lastTotalSamples;

				//if this orbit went through the view area enough
				if (totalSamplesDiff > 0){

					//brighten the start map pixel in which this orbit began

					int imgX = int(((orbitStartR - orbitXOffset) / orbitXScale + 0.5) * width);
					int imgY = int(((orbitStartI - orbitYOffset) / orbitYScale + 0.5) * height);

					//if not out of bounds of the image view area
					if (!(imgY >= height || imgY < 0 || imgX >= width || imgX < 0)){

						orbitStartMap[imgX + width * imgY] += totalSamplesDiff; //brighten by number of hits in the view

						//record if it's the brightest
						if (orbitStartMap[imgX + width * imgY] > startMapBrightest)
							startMapBrightest = orbitStartMap[imgX + width * imgY];

					}

				}
			}

		}


		orbitsChecked++;
		
			

	}


	

}

void Buddhabrot::doSamplesAperture(int maxMutations){

	if (!bInitialized) return;


	//int extraMutations = maxMutations;
	int extraMutations = maxMutations >> 1;


	double cR, cI, r, i;

	double halfMutDiff = mutStr;
	double mutDiff = halfMutDiff + halfMutDiff;


	//find a random location to begin orbits from
	double
		startR = (genrand_real2() - 0.5)*orbitXScale + orbitXOffset,
		startI = (genrand_real2() - 0.5)*orbitYScale + orbitYOffset;

	/*
	cR = startR;
	cI = startI;
	r = cR + sR;
	i = cI + sI;
	double zRSq_t = r*r, zISq_t = i*i;
	i = r * i;
	i += i;
	i += cI + sI;
	r = zRSq_t - zISq_t + cR + sR;
	zRSq_t = r*r; zISq_t = i*i;
	//if escaped orbit
	if (zRSq_t + zISq_t > 4){
	return;
	}
	*/

	//mutation box quality score
	unsigned long score = 0;

	//to calculate average depth of orbits in this box
	unsigned long totalBoxOrbitLength = 0;
	unsigned long numBoxOrbits = 0;

	int deepOrbitVal = int((iterations - minIts) * 0.65);

	//quick simple optimization (4 is escape distance, 1 is scale)
	//float quick = logf(logf(4.0)) / logf(float(xScale));

	//choose complex plane to render
	double *xPlaneVal, *yPlaneVal;
	switch (xAxis){
	case 0:
		xPlaneVal = &r;
		break;
	case 1:
		xPlaneVal = &i;
		break;
	case 2:
		xPlaneVal = &cR;
		break;
	default:
		xPlaneVal = &r;
	}

	switch (yAxis){
	case 0:
		yPlaneVal = &i;
		break;
	case 1:
		yPlaneVal = &cR;
		break;
	case 2:
		yPlaneVal = &cI;
		break;
	default:
		yPlaneVal = &i;
	}

	//varied batches (orbit casts) around the initial random location
	for (int mutation = 1; mutation <= maxMutations; mutation++){

		numBoxOrbits++;

		if (fourDims){
			double
			rmh = startR - halfMutDiff, //real mutation area half
			imh = startI - halfMutDiff; //imaginary mutation area half

			cR = rmh + genrand_real2()*mutDiff; // + sR;  //does c need julia seed when in 4d? shall check, < -- seems not, must also apply sR and sI elsewhere
			cI = imh + genrand_real2()*mutDiff; // + sI;

			r = rmh + genrand_real2()*mutDiff + sR;
			i = imh + genrand_real2()*mutDiff + sI;
		}
		else {
			cR = startR + genrand_real2()*mutDiff - halfMutDiff;
			cI = startI + genrand_real2()*mutDiff - halfMutDiff;

			r = cR + sR;
			i = cI + sI;

			//r = std::sin(cR);
			//i = std::sin(cI);
		}
		
		double orbitStartR = *xPlaneVal;
		double orbitStartI = *yPlaneVal;
		
		if (skipIsOutsideMandel || isOutsideMandel(r, i)){

			bool doPaint = true;
			int j;
			double zRSq = r*r, zISq = i*i;
			for (j = 0; j < iterations; j++){

				i = r * i;
				i += i;
				i += cI + sI;

				r = zRSq - zISq + cR + sR;

				zRSq = r*r;
				zISq = i*i;


				//if escaped orbit
				if (zRSq + zISq > 4){

					//cancel paint if orbit escape was discovered in too few iterations
					if (j < minIts){

						mutation += 5; //skip some orbits
						doPaint = false;
					}

					break;
				}

				//save to trajectory history
				orbitHistory[j].x = *xPlaneVal;
				orbitHistory[j].y = *yPlaneVal;
			}


			totalBoxOrbitLength += j;

			if (j >= iterations){

				continue;

			}
			else
				if (doPaint == false){
					continue;
				}

			score += j;


			//a deep orbit
			if (j > deepOrbitVal){


				if (extraMutations > 0){
					extraMutations -= 1;
					mutation -= 5; //do more orbits here
				}

			}


			//int lastImgX = -2, lastImgY = -2;
			//int noMove = 0;
			unsigned long lastTotalSamples = totalSamples;
			unsigned int jj = 0;
			//for each position in the trajectory history
			for (int o = j; o >= 0; o--){

				//map to image co-ordinates
				int imgX = int(((orbitHistory[jj].x - xOffset) / xScale + 0.5) * width);
				int imgY = int(((orbitHistory[jj].y - yOffset) / yScale + 0.5) * height);

				//count how long there seems to have been no movement
				//noMove = (imgX == lastImgX && imgY == lastImgY) ? noMove + 1 : max(0, noMove - 1);
				//if ((imgX == lastImgX && imgY == lastImgY)) noMove++;

				//record image location
				//lastImgX = imgX;
				//lastImgY = imgY;


				//if there has been no movement for too long
				//if (noMove >= 2){
				//	noMoves++;
				//	//continue; //don't paint this pixel in the orbit's trajactory history
				//	break; //don't paint the rest of this orbit
				//}


				//if not out of bounds of the image view area
				if (!(imgY >= height || imgY < 0 || imgX >= width || imgX < 0)){

					Pixel *pOut = &image[imgX + width * imgY];

					/*
					unsigned int mu = max(0, jj);

					float smooth = float(mu)
						+ quick
						- logf(logf(float(zRSq + zISq))) / logf(float(yScale));

					double pixelVal = cos(smooth);
					if (pOut->r > bRed){
					bRed = pOut->r;
					}
					else if (pixelVal > 0)
					pOut->r += pixelVal;

					pixelVal = cos(smooth + 0.05);
					if (pOut->g > bGreen){
					bGreen = pOut->g;
					}
					else if (pixelVal > 0)
					pOut->g += pixelVal;

					pixelVal = cos(smooth + 0.15);
					if (pOut->b > bBlue){
					bBlue = pOut->b;
					}
					else if (pixelVal > 0)
					pOut->b += pixelVal;
					*/

					
					double pixelVal = 1.0;

					//assess red range
					if (jj <= eRed && jj >= sRed){

						//float smooth = static_cast<float>((eRed - jj) / (jj - sRed + 1))
						//	+ quick
						//	- logf(logf(float(zRSq + zISq))) / logf(float(yScale));  //this can lead to over saturated pixels, oops

						float rMag = static_cast<float>((eRed - jj) / (jj - sRed + 1));
						//pixelVal = (cos(rMag) + 1)*rMag;//(cos(rMag) + 1);
						//pixelVal = cos(rMag)*rMag;

						if (pOut->r > bRed){ //record if brightest in this channel
							bRed = pOut->r;
						}
						else // if (pixelVal > 0)
							//pOut->r++;// pixelVal;
							//pOut->r += pixelVal;
							pOut->r += rMag;
							//pOut->r += smooth;

						//totalR += pixelVal;
					}

					if (jj <= eGreen && jj >= sGreen){

						//float smooth = static_cast<float>((eGreen - jj) / (jj - sGreen + 1))
						//	+ quick
						//	- logf(logf(float(zRSq + zISq))) / logf(float(yScale));

						float gMag = static_cast<float>((eGreen - jj) / (jj - sGreen + 1));
						//pixelVal = (cos(gMag) + 1)*gMag; //(cos(gMag + 0.05) + 1);
						//pixelVal = cos(gMag + 0.05)*gMag;

						if (pOut->g > bGreen){
							bGreen = pOut->g;
						}
						else // if (pixelVal > 0)
							//pOut->g++;
							//pOut->g += pixelVal;
							pOut->g += gMag;
							//pOut->g += smooth;

						//totalG += pixelVal;
					}

					if (jj <= eBlue && jj >= sBlue){

						//float smooth = static_cast<float>((eBlue - jj) / (jj - sBlue + 1))
						//	+ quick
						//	- logf(logf(float(zRSq + zISq))) / logf(float(yScale));

						float bMag = static_cast<float>((eBlue - jj) / (jj - sBlue + 1));
						//pixelVal = (cos(bMag) + 1)*bMag; //(cos(bMag * 0.15) + 1);
						//pixelVal = cos(bMag * 0.15)*bMag;

						if (pOut->b > bBlue){
							bBlue = pOut->b;
						}
						else // if (pixelVal > 0)
							//pOut->b++;
							//pOut->b += pixelVal;
							pOut->b += bMag;
							//pOut->b += smooth;

						//totalB += pixelVal;
					}
					

					//pOut->hits ++;
					totalSamples++;

					

					/*
					if(pOut->hits > mostHits){
					mostHits = pOut->hits;
					}
					*/
				}

				jj++;

			}

			orbitsPainted++; //just stats

			
			if (paintStartMap){

				//number of hits in the view area
				long totalSamplesDiff = totalSamples - lastTotalSamples;

				//if this orbit went through the view area enough
				if (totalSamplesDiff > 0){

					//brighten the start map pixel in which this orbit began

					int imgX = int(((orbitStartR - orbitXOffset) / orbitXScale + 0.5) * width);
					int imgY = int(((orbitStartI - orbitYOffset) / orbitYScale + 0.5) * height);

					//if not out of bounds of the image view area
					if (!(imgY >= height || imgY < 0 || imgX >= width || imgX < 0)){
						
						orbitStartMap[imgX + width * imgY] += totalSamplesDiff; //brighten by number of hits in the view

						//record if it's the brightest
						if (orbitStartMap[imgX + width * imgY] > startMapBrightest)
							startMapBrightest = orbitStartMap[imgX + width * imgY];

					}

				}
			}


		} // else mutation += 10;

	}

	orbitsChecked += numBoxOrbits;
	//unsigned long avgOrbitLength = totalBoxOrbitLength / max(1, numBoxOrbits);

	//totalAvgBoxOrbitLengths += avgOrbitLength;
	//boxesDone ++;

	/*
	if (nextScore < numScores){
	MutationScore* newScore = &scores[nextScore];
	nextScore++;
	newScore->score = score;
	newScore->x = startR;
	newScore->y = startI;
	newScore->halfBoxWidth = halfMutDiff;
	newScore->avgOrbitLength = avgOrbitLength;
	newScore->numOrbits = numBoxOrbits;
	}
	*/

}

void Buddhabrot::doSamplesMH(int numOrbits){

	long double cR = 0, cI = 0, r, i;

	//choose complex plane to render
	long double *xPlaneVal, *yPlaneVal;
	switch (xAxis){
	case 0:
		xPlaneVal = &r;
		break;
	case 1:
		xPlaneVal = &i;
		break;
	case 2:
		xPlaneVal = &cR;
		break;
	default:
		xPlaneVal = &r;
	}

	switch (yAxis){
	case 0:
		yPlaneVal = &i;
		break;
	case 1:
		yPlaneVal = &cR;
		break;
	case 2:
		yPlaneVal = &cI;
		break;
	default:
		yPlaneVal = &i;
	}

	//part of my attempts to maximise the metropolis hastings effect (of course it doesn't improve anything because "maths people" knew what they were doing)
	//bool sticky = false;


	//calculate a 20th of RAND_MAX
	//int twentiethRandMax = int(RAND_MAX / 20);
	//calculate a 5th of RAND_MAX
	int fithRandMax = int(RAND_MAX / 5); //to not re-calculate regularly, the compiler probably does this anyway though

	//long double r1 = 1.f / 1.f / float(yScale) * 0.0001;
	//long double r2 = 1.f / 1.f / float(yScale) * 0.1;
	long double r1 = 1.f / 1.f / float(yScale) * orbitYScale * 0.0001;
	long double r2 = 1.f / 1.f / float(yScale) * orbitYScale * 0.1;

	float logXscale = logf(float(xScale));
	//float quick = logf(logf(4.0)) / logXscale;


	//start with a random location
	cR = (genrand_real2() - 0.5)*orbitXScale + orbitXOffset;
	cI = (genrand_real2() - 0.5)*orbitYScale + orbitYOffset;
	r = cR + sR;
	i = cI + sI;

	long double prevR = r;
	long double prevI = i;
	

	//compute orbits
	for (int orbitNum = 0; orbitNum < numOrbits; orbitNum++){
		
		orbitsChecked++;

		//find a random location to begin an orbit from
		if (rand() < fithRandMax){

			cR = (genrand_real2() - 0.5)*orbitXScale + orbitXOffset;
			cI = (genrand_real2() - 0.5)*orbitYScale + orbitYOffset;
			
			r = cR + sR;
			i = cI + sI;
		}
		else { //mutate with metropolis hastings

			//double phi = (rand() / RAND_MAX) * TAU;
			//double dr = r2 * exp(-log(r2 / r1) * (rand() / RAND_MAX));

			long double phi = genrand_real2() * TAU;
			long double dr = r2 * exp(-log(r2 / r1) * genrand_real2());

			//double phi = random(0, 1) * TAU;
			//double dr = r2 * exp(-log(r2 / r1) * random(0, 1));

			r = prevR + dr * cos(phi);
			i = prevI + dr * sin(phi);

		}

		//record orbit start location on the complex plane
		prevR = r;
		prevI = i;
		
		double orbitStartR = *xPlaneVal;
		double orbitStartI = *yPlaneVal;

		//skip any start locations that are certainly inside the mandelbrot set
		if (skipIsOutsideMandel || isOutsideMandel(r, i)){

			bool doPaint = true;

			int j;
			long double zRSq = r*r, zISq = i*i; //no point calculating this every iteration (the compiler probably helps with this anyway)
			for (j = 0; j < iterations; j++){

				//this has been optimised (I don't think it made any difference because the compiler should help anyway)

				i = r * i;
				i += i;
				i += cI + sI;

				r = zRSq - zISq + cR + sR;

				zRSq = r*r;
				zISq = i*i;


				//if escaped orbit
				if (zRSq + zISq > 4){

					//cancel paint if orbit escape was discovered in too few iterations
					if (j < minIts)
						doPaint = false;

					break;
				}

				//save to trajectory history
				orbitHistory[j].x = *xPlaneVal;
				orbitHistory[j].y = *yPlaneVal;
			}


			if (j >= iterations) //cancel paint if the orbit "never escapes"
				continue;
			else if (doPaint == false)
				continue;


			//still here, so paint this orbit


			unsigned long lastTotalSamples = totalSamples;
			unsigned int jj = 0;
			//for each position in the trajectory history
			for (int o = j; o >= 0; o--){

				//map to image co-ordinates
				int imgX = int(((orbitHistory[jj].x - xOffset) / xScale + 0.5) * width);
				int imgY = int(((orbitHistory[jj].y - yOffset) / yScale + 0.5) * height);

				
				// -- experimental "fancy" orbit painting that I haven't seen anyone online discuss the possibilities of 
				//in other words, the channel intensity to is based on the depth of the orbit. There are many different ways to do this that could make different "pretty pictures"
				/*
				//if not out of bounds of the image view area
				if (!(imgY >= height || imgY < 0 || imgX >= width || imgX < 0)){

					float mag;
					//float smooth;

					Pixel *pOut = &image[imgX + width * imgY];
					
					//calculate mirrored y image co-ordinate
					//int imgYm = int(((orbitHistory[jj].y - yOffset) / -yScale + 0.5) * height);
					int imgYm = (int)((((halfHeight - orbitHistory[jj].y - halfHeight) - yOffset) / yScale + 0.5) * height );

					//only attempt to paint the mirrored pixel if it is within the canvas
					bool mirror = !(imgYm >= height || imgYm < 0);

					Pixel *pOutM = &image[imgX + width * imgYm];

					if (jj <= eRed && jj >= sRed){

						//smooth = static_cast<float>((eRed - jj) / (jj - sRed + 1))
						//	+ quick
						//	- logf(logf(float(zRSq + zISq))) / logXscale;

						mag = cos(static_cast<float>((eRed - jj) / (jj - sRed + 1)));

						pOut->r += cos(mag);
						//pOut->r += mag;

						if (mirror) pOutM->r = pOut->r;


						if (pOut->r > bRed)
							bRed = pOut->r;

					}


					if (jj <= eGreen && jj >= sGreen){

						//smooth = static_cast<float>((eGreen - jj) / (jj - sGreen + 1))
						//	+ quick
						//	- logf(logf(float(zRSq + zISq))) / logXscale;

						mag = cos(static_cast<float>((eGreen - jj) / (jj - sGreen + 1)) + 0.05f);

						pOut->g += cos(mag + 0.05f);
						//pOut->g += mag;

						if (mirror) pOutM->g = pOut->g;


						if (pOut->g > bGreen)
							bGreen = pOut->g;

					}


					if (jj <= eBlue && jj >= sBlue){
						
						//smooth = static_cast<float>((eBlue - jj) / (jj - sBlue + 1))
						//	+ quick
						//	- logf(logf(float(zRSq + zISq))) / logXscale;

						mag = cos(static_cast<float>((eBlue - jj) / (jj - sBlue + 1)) + 0.15f);

						pOut->b += cos(mag + 0.15f);
						//pOut->b += mag;

						if (mirror) pOutM->b = pOut->b;


						if (pOut->b > bBlue)
							bBlue = pOut->b;

					}

					totalSamples++;
				}
				*/

				
				// -- standard flat painting (+1 r/g/b for a hit on a pixel in that colour's range)
				
				//remember if a channel was painted so it's not calculated again for the mirrored pixel
				bool r = false;
				bool g = false;
				bool b = false;

				//if not out of bounds of the image view area
				if (!(imgY >= height || imgY < 0 || imgX >= width || imgX < 0)){
					Pixel *pOut = &image[imgX + width * imgY];

					if (jj <= eRed && jj >= sRed){
						r = true;

						pOut->r++;

						if (pOut->r > bRed)
							bRed = pOut->r;
					}

					if (jj <= eGreen && jj >= sGreen){
						g = true;

						pOut->g++;

						if (pOut->g > bGreen)
							bGreen = pOut->g;
					}

					if (jj <= eBlue && jj >= sBlue){
						b = true;

						pOut->b++;

						if (pOut->b > bBlue)
							bBlue = pOut->b;
					}

					totalSamples++;
				}

				if(paintMirror){
					//calculate mirrored y image co-ordinate
					//int imgMy = int(((orbitHistory[jj].y - yOffset) / -yScale + 0.5) * height);
					int imgMy = int((((halfHeight - orbitHistory[jj].y - halfHeight) - yOffset) / yScale + 0.5) * height - 1);
					//(halfHeight - imgY - halfHeight)

					//if not out of bounds of the image view area
					if (!(imgMy >= height || imgMy < 0 || imgX >= width || imgX < 0)){
						Pixel *pOut = &image[imgX + width * imgMy];

						if (r || (jj <= eRed && jj >= sRed)){

							pOut->r++;

							if (pOut->r > bRed)
								bRed = pOut->r;
						}

						if (g || (jj <= eGreen && jj >= sGreen)){

							pOut->g++;

							if (pOut->g > bGreen)
								bGreen = pOut->g;
						}

						if (b || (jj <= eBlue && jj >= sBlue)){

							pOut->b++;

							if (pOut->b > bBlue)
								bBlue = pOut->b;
						}
					}

					totalSamples++;
				}


				jj++;

			}

			orbitsPainted++; //just stats


			if (paintStartMap){

				//number of hits in the view area
				long totalSamplesDiff = totalSamples - lastTotalSamples;

				//if this orbit went through the view area enough
				if (totalSamplesDiff > 0){

					//brighten the start map pixel in which this orbit began

					int imgX = int(((orbitStartR - orbitXOffset) / orbitXScale + 0.5) * width);
					int imgY = int(((orbitStartI - orbitYOffset) / orbitYScale + 0.5) * height);

					//if not out of bounds of the image view area
					if (!(imgY >= height || imgY < 0 || imgX >= width || imgX < 0)){

						orbitStartMap[imgX + width * imgY] += totalSamplesDiff; //brighten by number of hits in the view

						//record if it's the brightest
						if (orbitStartMap[imgX + width * imgY] > startMapBrightest)
							startMapBrightest = orbitStartMap[imgX + width * imgY];

					}

				}
			}
			

			/*
			long totalSamplesDiff = totalSamples - lastTotalSamples;
			//more mutations for deep orbits
			if (totalSamplesDiff > 0 && jj > (unsigned int)round(iterations / 2.f)){// && rand() > twentiethRandMax){
				sticky = true;
			}
			else {
				//sticky = false; //ensure sticky is false 
			}
			*/
			

		}

	}

}
