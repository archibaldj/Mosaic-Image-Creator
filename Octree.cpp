#include "Octree.h"
#include <iostream>

Octree::Octree(vector<pixel>& means) {
	//make the root bounded over the entire space
	root = new OctPixel(0, means[0]);
	for (int i = 0; i < 3; i++)
		root->bounds[i] = make_pair(0, 255);
	root->Center();

	//insert each mean into the tree
	for (int i = 1; i < means.size(); i++)
	{
		pixel* pix = new pixel(means[i], i);
		Insert(pix);
	}
}

void Octree::Insert(pixel* pix)
{
	OctPixel* target = root;

	//finds the leaf node in which the point would be placed
	while (!target->leaf)
	{
		if (target->center.red < pix->red && target->center.green < pix->green && target->center.blue < pix->blue)
			target = target->children[7];
		else if (target->center.red < pix->red && target->center.green < pix->green)
			target = target->children[4];
		else if (target->center.red < pix->red && target->center.blue < pix->blue)
			target = target->children[5];
		else if (target->center.green < pix->green && target->center.blue < pix->blue)
			target = target->children[6];
		else if (target->center.red < pix->red)
			target = target->children[1];
		else if (target->center.green < pix->green)
			target = target->children[2];
		else if (target->center.blue < pix->blue)
			target = target->children[3];
		else
			target = target->children[0];
	}

	//checks if the OctPixel already has a node
	if (target->pix == nullptr)
		target->pix = pix;
	else
	{
		pixel *temp = target->pix;
		Break(target);
		Insert(temp);
		Insert(pix);
	}
}

void Octree::Break(OctPixel* octpix)
{
	octpix->leaf = false;
	octpix->pix = nullptr;

	octpix->children[0] = new OctPixel(make_pair(octpix->bounds[0].first, (octpix->bounds[0].second + octpix->bounds[0].first) / 2), make_pair(octpix->bounds[1].first, (octpix->bounds[1].second+octpix->bounds[1].first) / 2), make_pair(octpix->bounds[2].first, (octpix->bounds[2].second+octpix->bounds[2].first) / 2));
	octpix->children[1] = new OctPixel(make_pair(octpix->bounds[0].first + (octpix->bounds[0].second - octpix->bounds[0].first) / 2, octpix->bounds[0].second), make_pair(octpix->bounds[1].first, (octpix->bounds[1].second + octpix->bounds[1].first) / 2), make_pair(octpix->bounds[2].first, (octpix->bounds[2].second+octpix->bounds[2].first) / 2));
	octpix->children[2] = new OctPixel(make_pair(octpix->bounds[0].first, (octpix->bounds[0].second+octpix->bounds[0].first) / 2), make_pair(octpix->bounds[1].first + (octpix->bounds[1].second - octpix->bounds[1].first) / 2, octpix->bounds[1].second), make_pair(octpix->bounds[2].first, (octpix->bounds[2].second+octpix->bounds[2].first) / 2));
	octpix->children[3] = new OctPixel(make_pair(octpix->bounds[0].first, (octpix->bounds[0].second+octpix->bounds[0].first) / 2), make_pair(octpix->bounds[1].first, (octpix->bounds[1].second+octpix->bounds[1].first)  / 2), make_pair(octpix->bounds[2].first + (octpix->bounds[2].second - octpix->bounds[2].first) / 2, octpix->bounds[2].second));
	octpix->children[4] = new OctPixel(make_pair(octpix->bounds[0].first+ (octpix->bounds[0].second-octpix->bounds[0].first) / 2, octpix->bounds[0].second), make_pair(octpix->bounds[1].first + (octpix->bounds[1].second-octpix->bounds[1].first) / 2, octpix->bounds[1].second), make_pair(octpix->bounds[2].first, (octpix->bounds[2].second+octpix->bounds[2].first) / 2));
	octpix->children[5] = new OctPixel(make_pair(octpix->bounds[0].first+ (octpix->bounds[0].second - octpix->bounds[0].first) / 2, octpix->bounds[0].second), make_pair(octpix->bounds[1].first, (octpix->bounds[1].second+octpix->bounds[1].first) / 2), make_pair(octpix->bounds[2].first + (octpix->bounds[2].second - octpix->bounds[2].first) / 2, octpix->bounds[2].second));
	octpix->children[6] = new OctPixel(make_pair(octpix->bounds[0].first, (octpix->bounds[0].second+octpix->bounds[0].first) / 2), make_pair(octpix->bounds[1].first + (octpix->bounds[1].second - octpix->bounds[1].first) / 2, octpix->bounds[1].second), make_pair(octpix->bounds[2].first + (octpix->bounds[2].second - octpix->bounds[2].first) / 2, octpix->bounds[2].second));
	octpix->children[7] = new OctPixel(make_pair(octpix->bounds[0].first+ (octpix->bounds[0].second - octpix->bounds[0].first) / 2, octpix->bounds[0].second), make_pair(octpix->bounds[1].first + (octpix->bounds[1].second - octpix->bounds[1].first) / 2, octpix->bounds[1].second), make_pair(octpix->bounds[2].first + (octpix->bounds[2].second - octpix->bounds[2].first) / 2, octpix->bounds[2].second));
	for (int i = 0; i < 8; i++)
		octpix->children[i]->Center();
}

int Octree::Query(pixel pix)
{
	unsigned char target[3] = { pix.red, pix.green, pix.blue };
	return QueryHelper(root, target)->index;
}

pixel* Octree::QueryHelper(OctPixel* octant, unsigned char (&target)[3])
{
	pixel* result;

	//recurse down until a leaf is reached
	if (!octant->leaf)
	{
		int child = FindOctant(octant, target);
		result = QueryHelper(octant->children[child], target);

		//if the leaf had no nodes, check all children
		if (result == nullptr)
		{
			result = Nearest(target, octant->children);
		}
	}
	else {
		if (octant->pix)
			return octant->pix;
		else
			return nullptr;
	}
	if (result == nullptr)
		return result;
	int dist = (target[0] - result->red) * (target[0] - result->red) + (target[1] - result->green) * (target[1] - result->green) + (target[2] - result->blue) * (target[2] - result->blue);
	//implement edge checking 
	/*
	for (int i = 0; i < 6; i++)
	{

	}
	*/
	return result;
}

pixel* Octree::Nearest(unsigned char (&target)[3], OctPixel* children[8]) {
	int dist = 255 * 255 * 255;
	pixel* nearest = nullptr;
	pixel* pix = nullptr;

	for (unsigned int i = 0; i < 8; i++) {
		if (children[i]->pix != nullptr)
			pix = children[i]->pix;
		else if (!children[i]->leaf)
			pix = QueryHelper(children[i], target);
		if (pix != nullptr)
		{
			if ((target[0] - pix->red) * (target[0] - pix->red) + (target[1] - pix->green) * (target[1] - pix->green) + (target[2] - pix->blue) * (target[2] - pix->blue) < dist)
			{
				dist = (target[0] - pix->red) * (target[0] - pix->red) + (target[1] - pix->green) * (target[1] - pix->green) + (target[2] - pix->blue) * (target[2] - pix->blue);
				nearest = pix;
			}
		}
	}

	if (nearest != nullptr)
		return nearest;
	else
		return nullptr;
}

int Octree::FindOctant(OctPixel* octpix, unsigned char target[3])
{
	//0 = darker, 1 = red, 2 = green, 3 = blue, 4 = rg, 5 = rb, 6 = gb, 7 = brighter
	if (octpix->center.red < target[0] && octpix->center.green < target[1] && octpix->center.blue < target[2])
		return 7;
	if (octpix->center.red < target[0] && octpix->center.green < target[1])
		return 4;
	if (octpix->center.green < target[1] && octpix->center.blue < target[2])
		return 6;
	if (octpix->center.red < target[0] && octpix->center.blue < target[2])
		return 5;
	if (octpix->center.red < target[0])
		return 1;
	if (octpix->center.blue < target[2])
		return 3;
	if (octpix->center.green < target[1])
		return 2;
	return 0;
}
