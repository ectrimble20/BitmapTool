
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
	unsigned int c;
	void Set(unsigned char r, unsigned char g, unsigned char b)
	{
		c = ((r << 16u) | (g << 8u) | b );
	}
	constexpr unsigned char GetR() const
	{
		return (c >> 16u) & 0xFFu;
	}
	constexpr unsigned char GetG() const
	{
		return (c >> 8u) & 0xFFu;
	}
	constexpr unsigned char GetB() const
	{
		return c & 0xFFu;
	}
	//add an ==, != operator
	bool operator==(const Color& rhs) const
	{
		return c == rhs.c;
	}
	bool operator!=(const Color& rhs) const
	{
		return !(*this == rhs);
	}
};

struct ColorS
{
	ColorS(unsigned char r, unsigned char g, unsigned char b) : r(r), g(g), b(b) {};
	void replaceColors(unsigned char rr, unsigned char rg, unsigned char rb) { r = rr; g = rg; b = rb; };
	unsigned char r;
	unsigned char g;
	unsigned char b;
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
			c.Set(bitmapFile.get(), bitmapFile.get(), bitmapFile.get());
			pColors[(yStart * width) + x] = c;
			if (spitOutDebug) {
				std::string m = "Debug, first color raw: " + std::to_string(c.GetR()) + " " + std::to_string(c.GetG()) + " " + std::to_string(c.GetB());
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
	//Think I'm doing this wrong
	//bitmapFile.seekg(std::ios::beg);
	//for (int i = 0; i < bitmapInfoHeader.biBitCount; i++)
	//{
	//	bitmapOutFile.write((char*)&bitmapFile.seekg(i),sizeof(char));
	//}

	//Lets write the shit we read in first right back into the new file rather than trying to be stupid and read it in byte by byte
	bitmapOutFile.write((char*)&bitmapFileHeader, sizeof(BITMAPFILEHEADER));
	bitmapOutFile.write((char*)&bitmapInfoHeader, sizeof(BITMAPINFOHEADER));


	//alright, should have our header written... does a BMP need an end byte or something?
	//beats me, I don't see anything about it on the interwebs so I'll assume not until it all fucks up
	bitmapFile.close(); //close the handle, we're done with it.

	//Okay, so now what we're going to do is loop through our Colors array and start spitting out our colors
	//BUT what we want to do is Ignore the background color (e.g write it as is) and replace anything else with
	//the replacement color to normalize the color.

	//So what did I forget?!?
	//derp derp, if we read the image in reversed and we write it back out guess what?  The image is upside down.
	//lets try to resolve this

	const char paddingByte = ' ';
	//reset our variables - this is kinda dumb but I'm just working through some issues
	height = bitmapInfoHeader.biHeight;
	yStart = height - 1;
	yEnd = 0;
	deltaY = -1;
	if (height < 0)
	{
		height = -height; //convert back to a positive number
		yStart = 0;
		yEnd = height;
		deltaY = 1;
	}

	for (yStart; yStart != yEnd; yStart += deltaY)
	{
		for (int x = 0; x < width; x++)
		{
			int i = (yStart * width) + x;
			ColorS insertColor(pColors[i].GetR(), pColors[i].GetG(), pColors[i].GetB());
			if (pColors[i] != backgroundColor)
			{
				insertColor.replaceColors(replaceWith.GetR(), replaceWith.GetG(), replaceWith.GetB());
			}
			bitmapOutFile.write((char*)&insertColor, sizeof(ColorS));
			if (i % width == 0)
			{
				if (bitmapPadding > 0)
				{
					bitmapOutFile.write((char*)&paddingByte, sizeof(char));
				}
			}
		}
	}
	//that should be it, we can clean up now
	bitmapOutFile.close();
	delete[] pColors;
	//delete[] headerInfo;
	log("Finished... maybe... idk");
}








int main()
{
	//test code before I even attempt to do anything with user input
	std::string testIn = "inputimage.bmp";
	std::string testOut = "outputimage.bmp";
	Color bgColor;
	bgColor.Set(255, 0, 255);
	Color normalizeColor;
	normalizeColor.Set(255, 255, 255);
	//oh boy, this is either gona be sweet or it's gonna goto shit real quick lol
	NormalizeBMP(testIn, testOut, bgColor, normalizeColor);

	//so I can see if there's any output
	std::cout << "Press Enter to Continue";
	std::cin.ignore();

	return 0;
}