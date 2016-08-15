#ifndef HEADER_TEXTLINE
#define HEADER_TEXTLINE

#include "maths.h"

class TextRenderer; //forward declaration

class TextLine{
public:
	
	TextLine();
	TextLine(const maths::vec2 &sPos, const maths::vec3 &sCol, const float &sSize, const char* sText);
	TextLine(const maths::vec2 &sPos, const maths::vec4 &sCol, const float &sSize, const char* sText);

	TextLine(const maths::vec2 &sPos, const maths::vec3 &sCol, const char* sText);
	TextLine(const maths::vec2 &sPos, const maths::vec4 &sCol, const char* sText);

	TextLine(const maths::vec2 &sPos, const float &sSize, const char* sText);
	TextLine(const maths::vec2 &sPos, const char* sText);

	~TextLine();

	void SetPos(const maths::vec2 &sPos);
	void SetCol(const maths::vec3 &sCol);
	void SetCol(const maths::vec4 &sCol);
	void SetAlpha(const float &sAlpha);

	void SetSize(const float &sSize);

	void SetText(const char* sText, bool preserveDisplayLength = false);

	void Clear();

	void SetDisplayLength(const size_t &setDisplayLength); //could be used to cleanly draw characters in an animation

	//void Enable();
	//void Disable();
	void SetEnabled(bool to);


	const bool IsEnabled();
	const size_t GetLength();
	const size_t GetDisplayLength();
	const float GetSize();
	const float GetAlpha();

private:

	bool enabled;

	float size;

	maths::vec2 pos;
	maths::vec4 col;

	char* text; //will not be null terminated!
	size_t textLen; //to not do sizeof() regularly (maybe unnecessary)
	size_t displayLen; //# of characters to draw


	friend class TextRenderer;
	
	bool toConsume; //set to true any time a change is made

	//things get complicated:
	//Could store the vertex offset where the data for this line began so the text renderer could skip to updating vertices at the earliest location for which a changed line's vertex data sat
	//a completely different system of some kind is probably a better idea
};

#endif
