#pragma once
#include "pixel.h"

struct kdPixel
{
	unsigned char colors[3];
	int index;

	kdPixel* left;
	kdPixel* right;
	kdPixel(pixel pix, int index)
		:index(index), left(nullptr), right(nullptr)
	{
		colors[0] = pix.red;
		colors[1] = pix.green;
		colors[2] = pix.blue;
	}
};