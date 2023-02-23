#pragma once
struct pixel
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	int index;
	pixel(unsigned char blue, unsigned char green, unsigned char red)
		:blue(blue), green(green), red(red), index(0)
	{}
	pixel() {};
	pixel(unsigned char blue, unsigned char green, unsigned char red, int index)
		:blue(blue), green(green), red(red), index(index)
	{}
	pixel(pixel pix, int index)
	{
		this->blue = pix.blue;
		this->red = pix.red;
		this->green = pix.green;
		this->index = index;
	}
};