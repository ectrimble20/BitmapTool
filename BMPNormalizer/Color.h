#pragma once
struct Color
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

//Might remove this
struct Color32
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

class Colors
{
public:
	static Color GetColor(int red, int green, int blue)
	{
		Color c;
		c.r = red;
		c.g = green;
		c.b = blue;
		return c;
	};
	static bool ColorsMatch(const Color& lhs, const Color& rhs)
	{
		return (lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b);
	};
private:

};
