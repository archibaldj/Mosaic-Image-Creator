#pragma once
#include "OctPixel.h"
#include "pixel.h"
#include <vector>
using namespace std;

class Octree {
	OctPixel* root;
	void Insert(pixel* pix);
	void Break(OctPixel* octpix);
	int FindOctant(OctPixel* octpix, unsigned char target[3]);
	pixel* QueryHelper(OctPixel* root, unsigned char (&target)[3]);
	pixel* Nearest(unsigned char (&target)[3], OctPixel* children[8]);

public:
	Octree(vector<pixel>& means);
	int Query(pixel pix);

};