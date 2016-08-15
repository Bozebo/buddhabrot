#include "TextRenderer.h"


using namespace tinyxml2;

void TextRenderer::TestTextLine(float x, float y, float r, float g, float b, float size, const char* text){
	

	size_t textLength = strlen(text);
	if(textLength > 1024) textLength = 1024; //cap string length for safety (draw call is unsigned short indices, so there can never be more than 65535 indices)
	
	//int visibleChars = textLength;

	int buffPos = 0;
	int indexPos = 0;
	GLushort curIndex = 0;

	GLfloat baseX = x;
	GLfloat baseY = y;
	
	
	//allocate at least enough memory for the indices (if every character is visible)
	indices = new GLushort[textLength*4 + textLength*2 - 2]; //4 indices for each quad, plus degenerates, -2 because no degenerates at the end

	//allocate at least enough memory for the character quads (if every character is visible)
	characterVertices = new CharacterVertexData[textLength*4]; //4 vertices for each quad

	
	//iterate over each character to populate the vertices

	Chr* thisChar = nullptr;
	Chr* prevChar = nullptr;

	std::map<char, Chr*>::iterator thisCharIterator;
	std::map<char, GLfloat>::iterator kerningsIterator;

	for(size_t i = 0; i < textLength; i ++){

		//find the character in the map
		thisCharIterator = chars.find(text[i]);

		if(thisCharIterator == chars.end()){

			thisChar = chars.at('?'); //resort to ? if the character wasn't found
			//possibility for an exception here, maybe later check for that

		} else {
			thisChar = thisCharIterator->second;
		}
		
		
		//find the kerning relative to the previous character
		if(prevChar != nullptr){
			kerningsIterator = prevChar->kernings.find(thisChar->id);
			if(kerningsIterator != prevChar->kernings.end())
				baseX += kerningsIterator->second; //adjust baseX by the kerning amount
		}

		prevChar = thisChar; //remember this character to check for kerning on the next character


		//the correct location to draw this character (top left corner of quad) can now be determined
		GLfloat thisX = baseX + thisChar->xOffset * size;
		GLfloat thisY = baseY - thisChar->yOffset * size;


		//handle control characters

		if(text[i] == 32){ //space should be the only one

			//just advance baseX by the size
			baseX += thisChar->xAdvance * size;

			//visibleChars --; //one less character that is actually rendered (leaving unused memory in characterVertices)

			continue;
		}


		//CCW winding order

		//top left vertex
		characterVertices[buffPos].pos.x = thisX;
		characterVertices[buffPos].pos.y = thisY;
		
		characterVertices[buffPos].atlasCoords.x = thisChar->atlasLeft;
		characterVertices[buffPos].atlasCoords.y = thisChar->atlasTop;

		characterVertices[buffPos].col.x = r;
		characterVertices[buffPos].col.y = g;
		characterVertices[buffPos].col.z = b;
		characterVertices[buffPos].col.w = 1.f;

		buffPos ++;

		indices[indexPos] = curIndex;
		indexPos ++;
		curIndex ++;
		
		//bottom left vertex
		characterVertices[buffPos].pos.x = thisX;
		characterVertices[buffPos].pos.y = thisY - thisChar->height * size;

		characterVertices[buffPos].atlasCoords.x = thisChar->atlasLeft;
		characterVertices[buffPos].atlasCoords.y = thisChar->atlasBottom;
		
		characterVertices[buffPos].col.x = r;
		characterVertices[buffPos].col.y = g;
		characterVertices[buffPos].col.z = b;
		characterVertices[buffPos].col.w = 1.f;

		buffPos ++;
		
		indices[indexPos] = curIndex;
		indexPos ++;
		curIndex ++;

		//top right vertex
		characterVertices[buffPos].pos.x = thisX + thisChar->width * size;
		characterVertices[buffPos].pos.y = thisY;

		characterVertices[buffPos].atlasCoords.x = thisChar->atlasRight;
		characterVertices[buffPos].atlasCoords.y = thisChar->atlasTop;
		
		characterVertices[buffPos].col.x = r;
		characterVertices[buffPos].col.y = g;
		characterVertices[buffPos].col.z = b;
		characterVertices[buffPos].col.w = 1.f;

		buffPos ++;
		
		indices[indexPos] = curIndex;
		indexPos ++;
		curIndex ++;

		//bottom right vertex
		characterVertices[buffPos].pos.x = characterVertices[buffPos - 1].pos.x; //thisX + thisChar->width * size;
		characterVertices[buffPos].pos.y = characterVertices[buffPos - 2].pos.y; //thisY - thisChar->height * size;

		characterVertices[buffPos].atlasCoords.x = thisChar->atlasRight;
		characterVertices[buffPos].atlasCoords.y = thisChar->atlasBottom;
		
		characterVertices[buffPos].col.x = r;
		characterVertices[buffPos].col.y = g;
		characterVertices[buffPos].col.z = b;
		characterVertices[buffPos].col.w = 1.f;

		buffPos ++;
		
		indices[indexPos] = curIndex;
		indexPos ++;
		curIndex ++;


		if(i != textLength - 1){

			//degenerates
			indices[indexPos] = curIndex - 1;
			indexPos ++;
			indices[indexPos] = curIndex;
			indexPos ++;

			//shift x position over
			baseX += thisChar->xAdvance * size;

		}
		
	}

	//actual number of indices to be sent to the draw call
	numIndices = indexPos;
	

	//set buffer data

		
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	/*
	glBindVertexArray(mVAO);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	*/

		//glDeleteBuffers(1, &characterBuffer);
		//glDeleteBuffers(1, &indexBuffer);
		
		//glGenBuffers(1, &characterBuffer);
		//glGenBuffers(1, &indexBuffer);
		
	//glBindVertexArray(mVAO);
	
		glBindBuffer(GL_ARRAY_BUFFER, characterBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		glBufferData(GL_ARRAY_BUFFER, sizeof(CharacterVertexData)*buffPos, characterVertices, GL_STREAM_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLushort), indices, GL_STREAM_DRAW);

	//glBindVertexArray(0);

}

void TextRenderer::ConsumeTextLines(){

	TextLine* ptr;
	bool update = false;
	size_t textLength = 0;

	//check every text line, to determine if the buffers need rebuilt, also count the total number of characters
	//there is probably a clean way to avoid double iteration, or maybe determining the total data sizes beforehand make it worthwhile anyway
	for(std::vector<TextLine*>::iterator textLinesIterator = textLines->begin(); textLinesIterator != textLines->end(); textLinesIterator ++){
		ptr = *textLinesIterator;


		if(ptr->toConsume){
			update = true;
			ptr->toConsume = false; //about to be consumed now, so set to false
		}

		if(ptr->enabled)
			textLength += ptr->displayLen;

	}

	if(!update)
		return; //text has not changed, so don't rebuild buffers

	//if text has no length
	if(textLength == 0){
		//set numIndices to 0 and return, no need to do processing (some memory is left behind but it's not a permenant leak)
		numIndices = 0;
		return;
	}

	//soooooo, if the text gets short then there's a memory leak for extra vertex data...
	//options are: free and allocate each change, or use vector member functions to populate buffer data.

	//allocate at least enough memory for the indices (if every character is visible)
	//delete[] indices;
	//indices = new GLushort[textLength*4 + textLength*2 - 2]; 
	indexData.reserve(textLength*4 + textLength*2 - 2); //4 indices for each quad, plus degenerates, -2 because no degenerates at the end

	//allocate at least enough memory for the character quads (if every character is visible)
	//delete[] characterVertices;
	//characterVertices = new CharacterVertexData[textLength*4]; 
	vertexData.reserve(textLength*4); //4 vertices for each quad

	int buffPos = 0;
	int indexPos = 0;
	GLushort* indexPtr = indexData.data(); //&indexData[0];
	CharacterVertexData* vertexPtr = vertexData.data(); //&vertexData[0];
	//GLushort* indexPtr = indices; //&indexData[0];
	//CharacterVertexData* vertexPtr = characterVertices; //&vertexData[0];

	GLushort curIndex = 0;

	//prepare vertices and indices for each line of text

	std::map<char, Chr*>::iterator thisCharIt;
	std::map<char, GLfloat>::iterator kerningsIt;
	GLfloat baseX, baseY;
	
	for(std::vector<TextLine*>::iterator textLinesIterator = textLines->begin(); textLinesIterator != textLines->end(); textLinesIterator ++){
		ptr = *textLinesIterator;

		if(!ptr->enabled)
			continue;
		
		baseX = ptr->pos.x;
		baseY = ptr->pos.y;
		
		Chr* thisChar = nullptr;
		Chr* prevChar = nullptr;
		
		//iterate over each character in the text line to populate the vertices
		for(size_t i = 0; i < ptr->displayLen; i ++){

			//find the character in the map
			thisCharIt = chars.find(ptr->text[i]);

			if(thisCharIt == chars.end()){

				thisChar = chars.at('?'); //resort to ? if the character wasn't found
				//possibility for an exception here, maybe later check for that

			} else {
				thisChar = thisCharIt->second;
			}
		
		
			//find the kerning relative to the previous character
			if(prevChar != nullptr){
				kerningsIt = prevChar->kernings.find(thisChar->id);
				if(kerningsIt != prevChar->kernings.end())
					baseX += kerningsIt->second; //adjust baseX by the kerning amount
			}

			prevChar = thisChar; //remember this character to check for kerning on the next character


			//the correct location to draw this character (top left corner of quad) can now be determined
			GLfloat thisX = baseX + thisChar->xOffset * ptr->size;
			GLfloat thisY = baseY - thisChar->yOffset * ptr->size;


			//handle control characters

			if(ptr->text[i] == 32){ //space should be the only one

				//just advance baseX
				baseX += thisChar->xAdvance * ptr->size;

				continue;
			}


			//CCW winding order

			//top left vertex
			vertexPtr[buffPos].pos.x = thisX;
			vertexPtr[buffPos].pos.y = thisY;
		
			vertexPtr[buffPos].atlasCoords.x = thisChar->atlasLeft;
			vertexPtr[buffPos].atlasCoords.y = thisChar->atlasTop;

			vertexPtr[buffPos].col.x = ptr->col.x;
			vertexPtr[buffPos].col.y = ptr->col.y;
			vertexPtr[buffPos].col.z = ptr->col.z;
			vertexPtr[buffPos].col.w = ptr->col.w;

			buffPos ++;

			indexPtr[indexPos] = curIndex;
			indexPos ++;
			curIndex ++;
		
			//bottom left vertex
			vertexPtr[buffPos].pos.x = thisX;
			vertexPtr[buffPos].pos.y = thisY - thisChar->height * ptr->size;

			vertexPtr[buffPos].atlasCoords.x = thisChar->atlasLeft;
			vertexPtr[buffPos].atlasCoords.y = thisChar->atlasBottom;
		
			vertexPtr[buffPos].col.x = ptr->col.x;
			vertexPtr[buffPos].col.y = ptr->col.y;
			vertexPtr[buffPos].col.z = ptr->col.z;
			vertexPtr[buffPos].col.w = ptr->col.w;

			buffPos ++;
		
			indexPtr[indexPos] = curIndex;
			indexPos ++;
			curIndex ++;

			//top right vertex
			vertexPtr[buffPos].pos.x = thisX + thisChar->width * ptr->size;
			vertexPtr[buffPos].pos.y = thisY;

			vertexPtr[buffPos].atlasCoords.x = thisChar->atlasRight;
			vertexPtr[buffPos].atlasCoords.y = thisChar->atlasTop;
		
			vertexPtr[buffPos].col.x = ptr->col.x;
			vertexPtr[buffPos].col.y = ptr->col.y;
			vertexPtr[buffPos].col.z = ptr->col.z;
			vertexPtr[buffPos].col.w = ptr->col.w;

			buffPos ++;
		
			indexPtr[indexPos] = curIndex;
			indexPos ++;
			curIndex ++;

			//bottom right vertex
			vertexPtr[buffPos].pos.x = vertexPtr[buffPos - 1].pos.x; //thisX + thisChar->width * ptr->size;
			vertexPtr[buffPos].pos.y = vertexPtr[buffPos - 2].pos.y; //thisY - thisChar->height * ptr->size;

			vertexPtr[buffPos].atlasCoords.x = thisChar->atlasRight;
			vertexPtr[buffPos].atlasCoords.y = thisChar->atlasBottom;
		
			vertexPtr[buffPos].col.x = ptr->col.x;
			vertexPtr[buffPos].col.y = ptr->col.y;
			vertexPtr[buffPos].col.z = ptr->col.z;
			vertexPtr[buffPos].col.w = ptr->col.w;

			buffPos ++;
		
			indexPtr[indexPos] = curIndex;
			indexPos ++;
			curIndex ++;


			//all but the very last character have degenerates
			if(i != textLength - 1){

				indexPtr[indexPos] = curIndex - 1;
				indexPos ++;
				indexPtr[indexPos] = curIndex;
				indexPos ++;

				//shift x position over
				baseX += thisChar->xAdvance * ptr->size;

			}
		
		}

	}

	numIndices = indexPos;

	//upload buffer data
	
		glBindBuffer(GL_ARRAY_BUFFER, characterBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		//glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(CharacterVertexData), vertexData.data(), GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, buffPos * sizeof(CharacterVertexData), vertexData.data(), GL_STATIC_DRAW);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(CharacterVertexData)*buffPos, characterVertices, GL_STREAM_DRAW);
		
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLushort), indexData.data(), GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexPos * sizeof(GLushort), indexData.data(), GL_STATIC_DRAW);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLushort), indices, GL_STREAM_DRAW);
		
}

bool TextRenderer::Draw(){

	ConsumeTextLines();

	if(numIndices > 0){
		glUseProgram(progDraw);
		
		//glUniform2f(uiScaleUniform, uiScale.x, uiScale.y);
		//glUniform2f(uiScaleUniform, uiScale.x, uiScale.y);
		glUniformMatrix4fv(uiTransformUniform, 1, false, transformation.elements);
		
		glBindTexture(GL_TEXTURE_2D, fontTexture);

		glBindVertexArray(mVAO);
		
			glDrawElements(GL_TRIANGLE_STRIP, numIndices, GL_UNSIGNED_SHORT, (void*) 0);
			//glDrawElements(GL_LINE_STRIP, numIndices, GL_UNSIGNED_SHORT, (void*) 0);
		
		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_2D, 0);

		glUseProgram(0);

		return true; //drawing happened (renderer will change state machine back)
	}
	

	return false; //drawing did not happen

}

int TextRenderer::LoadFont(const char* path){
	
    tinyxml2::XMLDocument* settingsFile;
	
	settingsFile = new XMLDocument ();
    
	XMLError xmlErr = settingsFile->LoadFile(path);
	
	if (xmlErr != XML_SUCCESS){

		FILE* xmlErrFile;
		//fopen_s(&xmlErrFile, "xmlErr.txt", "w");
		//fprintf_s(xmlErrFile, "font: %d\n", xmlErr);
		xmlErrFile = fopen("xmlErr.txt", "w");
		fprintf(xmlErrFile, "font: %d\n", xmlErr);
		fclose(xmlErrFile);

		if(xmlErr != XML_ERROR_FILE_NOT_FOUND)
			return -1;

		return 0;
	}

	
	XMLElement* font = settingsFile->FirstChildElement("font");
	if(font == NULL)
		return -2;
	
	
	int intVal; //temporarily holds an int read from the file

	// -- get the font attributes
	
	//font->QueryIntAttribute("padding", &padding);
	//font->QueryIntAttribute("spacing", &spacing);

	//font->QueryIntAttribute("lineHeight", &lineHeight);
	//font->QueryIntAttribute("base", &base);

	font->QueryIntAttribute("scaleW", &scaleW);
	font->QueryIntAttribute("scaleH", &scaleH);

	GLfloat xScale = 1 / GLfloat(scaleW);
	GLfloat yScale = 1 / GLfloat(scaleH);


	// -- get the page file

	XMLElement* pages = font->FirstChildElement("pages");
	if(pages == NULL)
		return -3;

	XMLElement* page = pages->FirstChildElement("page");
	if(page == NULL)
		return -4;

	if(page->Attribute("file")){
		pageFile = page->Attribute("file");
	} else
		return -5;

	if(page->NextSiblingElement("page") != NULL)
		return -6; //only support 1 page


	int i;
	// -- get the characters

	XMLElement* charsElem = font->FirstChildElement("chars");
	if(charsElem == NULL)
		return -7;

	int numChars;
	charsElem->QueryIntAttribute("count", &numChars);
	if(numChars == 0)
		return -8;

	if(numChars > 65535)
		return -9;
	
	XMLElement* characterElement = charsElem->FirstChildElement("char");
	for(i = 1; i < numChars; i ++){

		//make a new character
		Chr* newCharacter = new Chr();

		//todo: more input verification

		
		//load values

		characterElement->QueryIntAttribute("id", &intVal);

		//range safety check
		if(intVal < 0 || intVal > 127) 
			return -18;

		newCharacter->id = char(intVal);

		characterElement->QueryIntAttribute("x", &intVal);
		newCharacter->atlasLeft = intVal * xScale;
		characterElement->QueryIntAttribute("y", &intVal);
		newCharacter->atlasTop = intVal * yScale;

		characterElement->QueryIntAttribute("width", &intVal);
		newCharacter->width = intVal * xScale;
		characterElement->QueryIntAttribute("height", &intVal);
		newCharacter->height = intVal * yScale;

		characterElement->QueryIntAttribute("xoffset", &intVal);
		newCharacter->xOffset = intVal * xScale;
		characterElement->QueryIntAttribute("yoffset", &intVal);
		newCharacter->yOffset = intVal * yScale;
		
		characterElement->QueryIntAttribute("xadvance", &intVal);
		newCharacter->xAdvance = intVal * xScale;
		
		newCharacter->atlasRight = newCharacter->atlasLeft + newCharacter->width;
		newCharacter->atlasBottom = newCharacter->atlasTop + newCharacter->height;

		//insert into map
		chars.insert(std::pair<char, Chr*>(newCharacter->id, newCharacter));

		//get the next element
		characterElement = characterElement->NextSiblingElement("char");
	}


	// -- get the kernings
	
	XMLElement* kerningsElem = font->FirstChildElement("kernings");
	if(kerningsElem == NULL) //kernings are mandatory, for no particular reason
		return -10; 
	
	int numKernings;
	kerningsElem->QueryIntAttribute("count", &numKernings);
	if(intVal > 65535)
		return -11;

	std::map<char, Chr*>::iterator firstCharIterator;

	XMLElement* kerningElement = kerningsElem->FirstChildElement("kerning");
	for(i = 1; i < numKernings; i ++){

		//load values

		kerningElement->QueryIntAttribute("first", &intVal);
		if(intVal < 0 || intVal > 127)
			return -12;

		char first = char(intVal);


		kerningElement->QueryIntAttribute("second", &intVal);
		if(intVal < 0 || intVal > 127)
			return -19;

		int second = char(intVal);


		kerningElement->QueryIntAttribute("amount", &intVal);
		GLfloat amount = intVal * xScale;


		//find the first character and add the kerning value in the map, in the first character, keyed by the second character's ID
		firstCharIterator = chars.find(first);
		if(firstCharIterator != chars.end()){
			firstCharIterator->second->kernings.insert(std::pair<char, GLfloat>(second, amount));
		}
		

		//get the next element
		kerningElement = kerningElement->NextSiblingElement("kerning");
	}
	

	// -- load the image data
	std::ifstream file(pageFile, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	
	pageData = new std::vector<char>((unsigned int)(size));
	if (!file.read(pageData->data(), size))
		return -14;


	// -- OpenGL calls
	

	//upload font atlas texture

	glGenTextures(1, &fontTexture);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 32);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, scaleW, scaleH, 0, GL_RED, GL_UNSIGNED_BYTE, pageData->data());
	glGenerateMipmap(GL_TEXTURE_2D);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1);

	glBindTexture(GL_TEXTURE_2D, 0);
	

	//create the text shader program

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vShader, 1, &VertShader, NULL);
	glShaderSource(fShader, 1, &FragShader, NULL);
		
	glCompileShader(vShader);
	glCompileShader(fShader);
	
	//check for fragment shader compilation errors
	GLint compileStatus = 0;
	glGetShaderiv(fShader, GL_COMPILE_STATUS, &compileStatus);
	if(!compileStatus){
		GLchar errLog[1024];
		GLint maxLen = 1024;
		glGetShaderInfoLog(fShader, maxLen, &maxLen, errLog);

		std::ofstream out("UI_Frag_Err.txt", std::ios::out | std::ios::trunc);
		out.write(errLog, maxLen);
		out.close();

		return -15;
	}

	//check for vertex shader compilation errors
	GLint compileStatus2 = 0;
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &compileStatus2);
	if(!compileStatus2){
		GLchar errLog[1024];
		GLint maxLen = 1024;
		glGetShaderInfoLog(vShader, maxLen, &maxLen, errLog);

		std::ofstream out("UI_Vert_Err.txt", std::ios::out | std::ios::trunc);
		out.write(errLog, maxLen);
		out.close();

		return -16;
	}


	progDraw = glCreateProgram();
	glAttachShader(progDraw, vShader);
	glAttachShader(progDraw, fShader);
	glLinkProgram(progDraw);

	//check link status
	GLint linkStatus = GL_FALSE;
	glGetProgramiv(progDraw, GL_LINK_STATUS, &linkStatus);
	if(linkStatus != GL_TRUE){
		GLchar infoLog[2048];
		GLint maxLen = 2040;

		glGetProgramInfoLog(progDraw, maxLen, &maxLen, infoLog);

		std::ofstream out("linkLogUI.txt", std::ios::out | std::ios::trunc);
		out.write(infoLog, maxLen);
		out.close();

		return -17;
	}

	glUseProgram(progDraw);
	uiTransformUniform = glGetUniformLocation(progDraw, "uiScale");
	glUseProgram(0);

	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &characterBuffer);
	glGenBuffers(1, &indexBuffer);

	glBindVertexArray(mVAO);
	
		glBindBuffer(GL_ARRAY_BUFFER, characterBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 32, 0); //vertex location
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, (void*) 8); //texture co-ords
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 32, (void*) 16); //colour

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		
	glBindVertexArray(0);

	// -- still here? Success!
	init = true;
	return 1;


}

