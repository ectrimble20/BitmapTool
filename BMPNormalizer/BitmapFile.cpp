#include "BitmapFile.h"



BitmapFile::BitmapFile(std::string FilePath)
{
	std::ifstream inputFile(FilePath, std::ios::binary);

	//doing a quick test above
	if (!inputFile)
	{
		//exception of some sort?
		Good = false;
		lastErr = "Unable to open input file " + FilePath + "\n";
		return;
	}
	//make sure we're at the start of the file.
	inputFile.seekg(0, std::ios::beg);
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
	offset = bitmapFileHeader.bfOffBits;
	//seek to the image bytes from the start of the file
	inputFile.seekg(offset, std::ios::beg);  //I think I did this wrong.
	//setup padding if we need it
	padding = (4 - (width * 3) % 4) % 4; //extra % 4 at the end in case width*3 % 4 is 0 and we get 4 - 0 = 4 even though in that case it should be 0 padding
	
	//Finished intializing our size and stuff
	pPixels = new Color[width * height];

	//this should read 3 bytes at a time with .get(), then check if i+1 % width is 0 meaning we've reached the end of a row
	//at which point we check if padding is greater than 0 and adjust for padding if we need to.
	for (int i = 0; i < (width * height); i++) {
		Color c;
		c.r = inputFile.get();
		c.g = inputFile.get();
		c.b = inputFile.get();
		pPixels[i] = c;
		//handle padding if we need to
		if (width % (i+1) == 0 && padding > 0) {
			inputFile.seekg(padding, std::ios::cur);
		}
	}
	inputFile.close();
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
	//This should write the bytes back out in the same order they were read in UNLESS we flagged for height revered
	//in which case they'll write back at (width*height) - iterator which should give us the reversed index.
	for (int i = 0; i < (width * height); i++) {
		int idx = i;
		if (heightReversed) {
			idx = (width * height) - i;
		}
		outputFile.write((char *)&pPixels[idx], sizeof(Color));
		//handle padding if we need to
		if (width % (i + 1) == 0 && padding > 0) {
			outputFile.write((char*)&paddingByte, sizeof(char));
		}
	}
	outputFile.close();
}

void BitmapFile::ReplaceColor(const Color & colorToReplace, const Color & replaceWithColor)
{
	//this function is realatively simple, it iterates through all the colors and replaces one with another
	for (int i = 0; i < (width * height); i++)
	{
		if (AreTwoColorsEqual(pPixels[i], colorToReplace))
		{
			pPixels[i] = replaceWithColor;
		}
	}
}

bool BitmapFile::AreTwoColorsEqual(const Color& lhs, const Color& rhs) const
{
	return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == lhs.b;
}