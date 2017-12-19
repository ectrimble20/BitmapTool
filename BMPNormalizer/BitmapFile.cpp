#include "BitmapFile.h"



BitmapFile::BitmapFile(std::string FilePath)
{
	/*  This worked, this means theres something going on below that is screwing up our file handle.
	std::ifstream inputFileT(FilePath, std::ios::binary);
	inputFileT.close();
	*/
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

	//we're just going to read everything in as is
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

	/*
	I'm commenting this whole section out.  I'm not sure if the problem lies in here or not
	but something is causing my heap to go to hell and since pPixel is the only thing I'm
	putting on the heap, it stands to reason that something is wrong with how the below
	code is writting (e.g maybe it's going out of bounds???)

	//setup our start points
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
			//If I had to take a guess, I think this is where it was going sideway, I think this was going out of bounds for the arrays size
			//and because it's a pointer it was writing to a random place on the heap.
			pPixels[(y * width) + x] = Colors::GetColor(inputFile.get(), inputFile.get(), inputFile.get());
		}
		//check for padding and skip a byte if we need to
		if (padding > 0) {
			inputFile.seekg(padding, std::ios::cur); //seek forward by the amount of padding (probably just 1 if any)
		}
	}
	*/
	//okay, should have the BMP loaded
	inputFile.close(); //doesn't seem to like this, get an exception: Critical error detected c0000374
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
	//does nothing right now
}
