#include <iostream>
#include <string>
#include "BitmapFile.h"
#include "Color.h"

int main()
{
	//test code before I even attempt to do anything with user input
	std::string testIn = "inputtesttwo.bmp";
	//std::string testIn = "inputimage.bmp";
	std::string testOut = "outputimage.bmp";

	Color bgColor{ 255, 0, 255 };
	Color normalizeColor{ 255, 255, 255 };
	BitmapFile f(testIn);
	if (!f.Good) {
		std::cout << f.GetLastError();
		return 1; //exit with fail
	}
	//lets test color replacement, we'll replace the BG color with white
	f.ReplaceColor(bgColor, normalizeColor);

	f.WriteNewBMP(testOut);
	if (!f.Good) {
		std::cout << f.GetLastError();
		return 1;
	}

	//so I can see if there's any output
	std::cout << "Press Enter to Continue";
	std::cin.ignore();

	return 0;
}