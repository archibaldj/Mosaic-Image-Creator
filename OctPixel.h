#pragma once
#include "pixel.h"
#include <vector>
using namespace std;

struct OctPixel {
	pixel* pix;
	pixel center;
	
	//dark, r, g, b, rg, rb, gb, rgb
	OctPixel* children[8];

	//left = lower bound, right = upper bound
	pair<float, float> bounds[3];

	bool leaf;

	OctPixel(int index, pixel pix)
		:leaf(true), pix(nullptr)
	{
		this->pix = new pixel(pix.red, pix.green, pix.blue, index);
	}

	OctPixel(pair<float, float> red, pair<float, float> green, pair<float, float> blue) {
		bounds[0] = red;
		bounds[1] = green;
		bounds[2] = blue;
		pix = nullptr;
	}

	void Center()
	{
		center.red = (bounds[0].first + bounds[0].second)/2;
		center.green = (bounds[1].first + bounds[1].second) / 2;
		center.blue = (bounds[2].first + bounds[2].second) / 2;
	}
};