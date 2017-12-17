
// target Windows 7 or later
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>
// The following #defines disable a bunch of unused windows stuff. If you 
// get weird errors when trying to do some windows stuff, try removing some
// (or all) of these defines (it will increase build time though).
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

#define STRICT

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <ostream> //not sure if I need this or just fstream
#include <string>

typedef unsigned char BYTE;

//0-255 color codes
struct Color
{

	int r;
	int g;
	int b;
	//add an ==, != operator
	bool operator==(const Color& rhs) const
	{
		return (r == rhs.r && g == rhs.g && b == rhs.b);
	}
	bool operator!=(const Color& rhs) const
	{
		return !(*this == rhs);
	}
};

void log(std::string msg)
{
	std::cout << msg + "\n";
}


void NormalizeBMP(std::string infile, std::string outfile, Color backgroundColor, Color replaceWith)
{
	std::ifstream bitmapFile(infile, std::ios::binary);
	//exit if we failed to open - TODO log this error
	if (!bitmapFile)
	{
		log("Can't open the in-file :(");
		return;
	}
	BITMAPFILEHEADER bitmapFileHeader;
	bitmapFile.read(reinterpret_cast<char*>(&bitmapFileHeader), sizeof(bitmapFileHeader));

	BITMAPINFOHEADER bitmapInfoHeader;
	bitmapFile.read(reinterpret_cast<char*>(&bitmapInfoHeader), sizeof(bitmapInfoHeader));

	if (bitmapInfoHeader.biBitCount != 24)
	{
		log("This application is only designed to work on 24-bit .bmp images, this image is not 24-bit");
	}
	//setup our looping variables but check that the height isn't less than 0, if it is, we need to handle reversed situation
	log("Normalizing " + infile + " to " + outfile);
	int width = bitmapInfoHeader.biWidth;
	int height = bitmapInfoHeader.biHeight;
	log("File Dimensions: " + std::to_string(width) + " " + std::to_string(height));
	int yStart = height - 1;
	int yEnd = 0;
	int deltaY = -1;
	bool rowOrderRevered = false;
	if (height < 0)
	{
		log("Notice: file is row reversed, compensating for this");
		rowOrderRevered = true;
		height = -height; //convert back to a positive number
		yStart = 0;
		yEnd = height;
		deltaY = 1;
	}
	Color * pColors = new Color[width * height];

	//point to the first "image" bits
	bitmapFile.seekg(bitmapFileHeader.bfOffBits);
	//also handle padding if we need to
	const int bitmapPadding = (4 - (width * 3) % 4) % 4;
	//begin our loop and grab each bit
	bool spitOutDebug = true;
	for (yStart; yStart != yEnd; yStart += deltaY)
	{
		for (int x = 0; x < width; x++)
		{
			Color c;
			c.r = bitmapFile.get();
			c.g = bitmapFile.get();
			c.b = bitmapFile.get();
			pColors[(yStart * width) + x] = c;
			if (spitOutDebug) {
				std::string m = "Debug, first color raw: " + std::to_string(c.r) + " " + std::to_string(c.g) + " " + std::to_string(c.b);
				log(m);
				spitOutDebug = false; //so we don't spam a million messages
			}
		}
		//handle padding
		bitmapFile.seekg(bitmapPadding, std::ios::cur);	
	}
	//alright, we have our bits, open up the outfile so we can copy the header information
	std::ofstream bitmapOutFile(outfile, std::ios::binary);
	if (!bitmapOutFile)
	{
		log("Failed to open outfile :(");
		delete[] pColors;
		return;
	}
	bitmapFile.seekg(std::ios::beg);
	BYTE * headerInfo = new BYTE[bitmapInfoHeader.biBitCount];
	for (int i = 0; i < bitmapInfoHeader.biBitCount; i++)
	{
		bitmapFile.seekg(i);
		headerInfo[i] = bitmapFile.get();
	}
	bitmapOutFile.write((char*)&headerInfo[0], bitmapInfoHeader.biBitCount);
	//alright, should have our header written... does a BMP need an end byte or something?
	//beats me, I don't see anything about it on the interwebs so I'll assume not until it all fucks up
	bitmapFile.close(); //close the handle, we're done with it.

	//Okay, so now what we're going to do is loop through our Colors array and start spitting out our colors
	//BUT what we want to do is Ignore the background color (e.g write it as is) and replace anything else with
	//the replacement color to normalize the color.
	const int endPoint = width * height;
	const char paddingByte = ' ';
	BYTE * fileInformation = new BYTE[bitmapInfoHeader.biSizeImage];
	int atIndex = 0;
	bool spitOutMoreDebug = true;
	for (int i = 0; i < endPoint; i++)
	{
		if (pColors[i] == backgroundColor)
		{
			char r = (char)pColors->r;
			fileInformation[atIndex++] = r;
			char g = (char)pColors->g;
			fileInformation[atIndex++] = g;
			char b = (char)pColors->b;
			fileInformation[atIndex++] = b;
			if (spitOutMoreDebug)
			{
				std::string m = std::to_string(r) + std::to_string(g) + std::to_string(b);
				log("Debug image info, what we pulled from color: " + m);
				spitOutMoreDebug = false;
			}
		}
		else
		{
			char r = (char)replaceWith.r;
			fileInformation[atIndex++] = r;
			char g = (char)replaceWith.g;
			fileInformation[atIndex++] = g;
			char b = (char)replaceWith.b;
			fileInformation[atIndex++] = b;
			if (spitOutMoreDebug)
			{
				std::string m = std::to_string(r) + std::to_string(g) + std::to_string(b);
				log("Debug image info, what we pulled from color: " + m);
				spitOutMoreDebug = false;
			}
		}
		if (i % width == 0)
		{
			if (bitmapPadding > 0)
			{
				fileInformation[atIndex++] = paddingByte;
			}
		}
	}
	//alright, now we just gotta write our file information and it'll be all hunky dory, work the first time with no bugs ! ! !
	bitmapOutFile.write((char*)&fileInformation[0], bitmapInfoHeader.biSizeImage);

	//that should be it, we can clean up now
	bitmapOutFile.close();
	delete[] pColors;
	delete[] headerInfo;
	log("Finished... maybe... idk");
}








int main()
{
	//test code before I even attempt to do anything with user input
	std::string testIn = "inputimage.bmp";
	std::string testOut = "outputimage.bmp";
	Color bgColor;
	bgColor.r = 255;
	bgColor.g = 0;
	bgColor.b = 255;
	Color normalizeColor;
	normalizeColor.r = 255;
	normalizeColor.g = 255;
	normalizeColor.b = 255;
	//oh boy, this is either gona be sweet or it's gonna goto shit real quick lol
	NormalizeBMP(testIn, testOut, bgColor, normalizeColor);

	//so I can see if there's any output
	std::cout << "Press Enter to Continue";
	std::cin.ignore();

	return 0;
}