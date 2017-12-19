#include "BitmapFile.h"



BitmapFile::BitmapFile(std::string FilePath)
{
	std::ifstream inputFile(FilePath, std::ios::binary);
	if (!inputFile)
	{
		//exception of some sort?
		Good = false;
		lastErr = "Unable to open input file " + FilePath + "\n";
		return;
	}
	//read in the headers
	inputFile.read(reinterpret_cast<char*>(&bitmapFileHeader), sizeof(bitmapFileHeader));
	inputFile.read(reinterpret_cast<char*>(&bitmapInfoHeader), sizeof(bitmapInfoHeader));
	//setup our Color's array
	width = bitmapInfoHeader.biWidth;
	height = bitmapInfoHeader.biHeight;
	if (height < 0)
	{
		height = std::abs(height);
		heightReversed = true; //so we know the bytes are reversed if we do anything to it
	}
	iter = width * height;
	offset = bitmapFileHeader.bfOffBits;
	//seek to the image bytes
	inputFile.seekg(offset);
	//setup padding if we need it
	padding = (4 - (width * 3) % 4) % 4; //extra % 4 at the end in case width*3 % 4 is 0 and we get 4 - 0 = 4 even though in that case it should be 0 padding
	
	//Finished intializing our size and stuff
	pPixels = new Color[iter];

	//lets read in the raw info
	unsigned int rawIndex = 0;
	//setup our start points
	unsigned int startX = 0;
	unsigned int startY = height;
	unsigned int endY = 0;
	unsigned int inc = -1;
	if (heightReversed) {
		//adjust our Y if things are reversed
		startY = 0;
		inc = 1;
		endY = height;
	}
	//start our main loop
	for (int y = startY; y != endY; y += inc)
	{
		for (int x = 0; x < width; x++)
		{
			Color c;
			c.r = inputFile.get();
			c.g = inputFile.get();
			c.b = inputFile.get();
			pPixels[(y * width) + x] = c;
		}
		//check for padding and skip a byte if we need to
		if (padding > 0) {
			inputFile.seekg(padding, std::ios::cur); //seek forward by the amount of padding (probably just 1 if any)
		}
	}
	//okay, should have the BMP loaded
	inputFile.close(); //doesn't seem to like this, get an exception: Critical error detected c0000374
	//debug, adding this so it has something to do after closing the file, this will tell me if it's after the constructor closes (this class is the problem)
	//of if it's the file closing, which would mean the ifstream is causing the problem
	int garbage = 0;
	garbage += 1;
}


BitmapFile::~BitmapFile()
{
	delete[] pPixels;
}

void BitmapFile::WriteNewBMP(std::string FilePath)
{
	std::ofstream outputFile(FilePath, std::ios::binary);
	if (!outputFile)
	{
		Good = false;
		lastErr = "Unable to open output file " + FilePath + "\n";
		return;
	}
	//Write out headers
	outputFile.write((char*)&bitmapFileHeader, sizeof(BITMAPFILEHEADER));
	outputFile.write((char*)&bitmapInfoHeader, sizeof(BITMAPINFOHEADER));
	//write out bytes

	//setup our start points
	unsigned int startX = 0;
	unsigned int startY = height;
	unsigned int endY = 0;
	unsigned int inc = -1;
	if (heightReversed) {
		//adjust our Y if things are reversed
		startY = 0;
		inc = 1;
		endY = height;
	}
	//start our main loop
	for (int y = startY; y != endY; y += inc)
	{
		for (int x = 0; x < width; x++)
		{
			Color c = pPixels[(y * width) + x];
			outputFile.write((char *)&c, sizeof(Color));
		}
		//check for padding and add a byte if we need one
		if (padding > 0) {
			//in case we have more than one byte of padding, SHOULDN'T really be an issue but just in case
			for (int x = 0; x < padding; x++) {
				outputFile.write((char *)&paddingByte, sizeof(char));
			}
		}
	}
	// outputFile.close(); removing this for now.
}

void BitmapFile::ReplaceColor(const Color & colorToReplace, const Color & replaceWithColor)
{
	//does nothing right now
}
