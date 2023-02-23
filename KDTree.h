#pragma once
#include "kdPixel.h"
#include "pixel.h"
#include <vector>
using namespace std;

class KDTree
{
private:
	kdPixel* root;
	kdPixel* queryHelper(kdPixel* root, unsigned char target[3], int level);
	int dist(kdPixel* pix, unsigned char target[3]);
	void deletionHelper(kdPixel* root);

public:
	KDTree(vector<pixel> means);
	int query(pixel);
	~KDTree();
};

