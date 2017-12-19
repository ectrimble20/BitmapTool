
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

struct Color
{
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;
};

//need to keep our struct simple, so no overloads
bool ColorEqualsColor(const Color& lhs, const Color& rhs)
{
	return (lhs.r == rhs.r && lhs.g == lhs.g && lhs.b == rhs.b);
}

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

	//We're not handling the alpha channel (or w/e the other 8 bytes are in a 32 bit image
	if (bitmapInfoHeader.biBitCount != 24)
	{
		log("This application is only designed to work on 24-bit .bmp images, this image is not 24-bit");
		return;
	}
	//setup our looping variables but check that the height isn't less than 0, if it is, we need to handle reversed situation
	log("Normalizing " + infile + " to " + outfile);
	int width = bitmapInfoHeader.biWidth;
	int height = bitmapInfoHeader.biHeight;
	log("File Dimensions: " + std::to_string(width) + " " + std::to_string(height));
	if (height < 0) {
		height = height * -1; //convert to positive number if it's reversed
	}
	Color * pColors = new Color[width * height];

	//point to the first "image" bits
	bitmapFile.seekg(bitmapFileHeader.bfOffBits);
	
	//check if padding is required, I know 32 bit never needs padding, but 24 might if we have uneven btyes per row.
	//this little guy here will let us know if that's the case
	const int bitmapPadding = (4 - (width * 3) % 4) % 4;
	const char paddingByte = ' '; //empty byte that we can pad with if needed
	
	//begin our loop and grab each bit
	for(int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			Color c;
			c.r = bitmapFile.get();
			c.g = bitmapFile.get();
			c.b = bitmapFile.get();
			pColors[(y * width) + x] = c;
		}
		//handle padding
		if (bitmapPadding > 0) {
			bitmapFile.seekg(bitmapPadding, std::ios::cur);
		}
	}
	//close the input file handle, we don't need it anymore
	bitmapFile.close();

	//alright, we have our bits, open up the outfile so we can copy the header information
	std::ofstream bitmapOutFile(outfile, std::ios::binary);
	if (!bitmapOutFile)
	{
		log("Failed to open outfile :(");
		delete[] pColors;
		return;
	}

	//Writing the header information back out is as easy as writing the header structs back.
	//write the header
	bitmapOutFile.write((char*)&bitmapFileHeader, sizeof(BITMAPFILEHEADER));
	//write the info header
	bitmapOutFile.write((char*)&bitmapInfoHeader, sizeof(BITMAPINFOHEADER));
	
	//with our file header information written, we can now run through out colors and replace those that need to be replaced.
	for (int i = 0; i < (width*height); i++)
	{
		if (! ColorEqualsColor(pColors[i], backgroundColor))
		{
			Color c;
			c.r = replaceWith.r;
			c.g = replaceWith.g;
			c.b = replaceWith.b;
			pColors[i] = c; //replace the struct with the replace struct.
		}
		//write out the color to the file
		bitmapOutFile.write((char*)&pColors[i], sizeof(Color));
	}

	bitmapOutFile.close();
	delete[] pColors;
	log("Finished... maybe... idk");
}

int main()
{
	//test code before I even attempt to do anything with user input
	std::string testIn = "inputimage.bmp";
	std::string testOut = "outputimage.bmp";
	//idk if this is a good idea to cast them with { } :P
	Color bgColor{ 255, 0, 255 };
	Color normalizeColor{ 255, 255, 255 };
	//oh boy, this is either gona be sweet or it's gonna goto shit real quick lol
	NormalizeBMP(testIn, testOut, bgColor, normalizeColor);

	//so I can see if there's any output
	std::cout << "Press Enter to Continue";
	std::cin.ignore();

	return 0;
}