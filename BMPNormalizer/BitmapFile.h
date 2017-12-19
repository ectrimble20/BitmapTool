#pragma once
#include "WinInclude.h"
#include "Color.h"
#include <fstream>
#include <ostream>
#include <string>
#include <math.h> //need abs

class BitmapFile
{
public:
	BitmapFile() = delete; //no default constructor for you!
	BitmapFile(std::string FilePath);
	bool Good = true;
	~BitmapFile();
	bool Is32Bit() {
		return bitmapInfoHeader.biBitCount == 32;
	};
	bool Is24Bit() {
		return bitmapInfoHeader.biBitCount == 24;
	};
	std::string GetLastError() {
		return lastErr;
	};
	//Fun time functions
	void WriteNewBMP(std::string FilePath);
	void ReplaceColor(const Color& colorToReplace, const Color& replaceWithColor);
private:
	std::string lastErr = "";
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	Color * pPixels; //raw data converted to our Color struct
	//this gets set on load, if it's > 0, padding is required
	int padding = 0;
	unsigned char paddingByte = ' '; //empty byte to pad with if needed
	bool heightReversed = false;
	int width;
	int height;
	int offset;
	unsigned int iter; //this is width * height
};

