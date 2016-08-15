#include "TextLine.h"


// -- constructors

TextLine::TextLine(){
	size = 1.f;
	textLen = 0;
	displayLen = 0;
	enabled = false;
	
	col = maths::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	pos = maths::vec2(1.0f, 1.0f);

	text = nullptr;

	toConsume = false;
}

TextLine::TextLine(const maths::vec2 &sPos, const maths::vec4 &sCol, const float &sSize, const char* sText){
	pos = sPos;
	col = sCol;
	size = sSize;

	textLen = strlen(sText);
	text = new char[textLen];
	memcpy(text, sText, textLen); //no longer null terminated!

	displayLen = textLen;

	enabled = true;

	toConsume = true;
}

TextLine::TextLine(const maths::vec2 &sPos, const maths::vec3 &sCol, const float &sSize, const char* sText)
	: TextLine(sPos, maths::vec4(sCol.x, sCol.y, sCol.z, 1.0f), sSize, sText){};

TextLine::TextLine(const maths::vec2 &sPos, const maths::vec3 &sCol, const char* sText)
	: TextLine(sPos, sCol, 1.f, sText){};

TextLine::TextLine(const maths::vec2 &sPos, const maths::vec4 &sCol, const char* sText)
	: TextLine(sPos, sCol, 1.f, sText){};

TextLine::TextLine(const maths::vec2 &sPos, const float &sSize, const char* sText)
	: TextLine(sPos, maths::vec4(1.f, 1.f, 1.f, 1.f), sSize, sText){}

TextLine::TextLine(const maths::vec2 &sPos, const char* sText)
	: TextLine(sPos, maths::vec4(1.f, 1.f, 1.f, 1.f), sText){};


TextLine::~TextLine(){
	delete[] text;
}


// -- setters

void TextLine::SetPos(const maths::vec2 &sPos){
	pos = sPos;

	toConsume = true;
}

void TextLine::SetCol(const maths::vec4 &sCol){
	col = sCol;

	toConsume = true;
}
void TextLine::SetCol(const maths::vec3 &sCol){
	SetCol(maths::vec4(sCol.x, sCol.y, sCol.z, 1.f));
}

void TextLine::SetAlpha(const float &sAlpha){
	col.w = sAlpha;

	toConsume = true;
}

void TextLine::SetSize(const float &sSize){
	size = sSize;

	toConsume = true;
}

void TextLine::SetText(const char* sText, bool preserveDisplayLength){
	delete[] text;

	textLen = strlen(sText);
	text = new char[textLen];
	memcpy(text, sText, textLen);

	if(preserveDisplayLength){
		//ensure displayLen is not longer than textLen, as preserveDisplayLength would be intended for when it was shorter than the new texts' length
		displayLen = (displayLen > textLen)? textLen: displayLen;
	} else {
		displayLen = textLen;
	}

	toConsume = true;
}

void TextLine::Clear(){
	delete[] text;
	textLen = 0;
	displayLen = 0;
	text = nullptr;

	toConsume = true;
}

void TextLine::SetDisplayLength(const size_t &setDisplayLength){
	//if(setDisplayLength != displayLen) //don't actually need this check because the calling code shouldn't be calling this if it's not needed, so optimise there instead of here
		toConsume = true;

	if(setDisplayLength > textLen){
		displayLen = textLen;
	} else {
		displayLen = setDisplayLength;
	}
}

void TextLine::SetEnabled(bool to){
	enabled = to;
	toConsume = true;
}


// -- getters

const bool TextLine::IsEnabled(){
	return enabled;
}

const size_t TextLine::GetLength(){
	return textLen;
}

const size_t TextLine::GetDisplayLength(){
	return displayLen;
}

const float TextLine::GetSize(){
	return size;
}

const float TextLine::GetAlpha(){
	return col.w;
}