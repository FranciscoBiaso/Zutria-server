#pragma once

class Color
{
public:
	Color()
	{
		r = g = b = a = 255;
	}
	Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}
	~Color(){}
	unsigned char r, g, b, a;
};

