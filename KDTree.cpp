#include "KDTree.h"

KDTree::KDTree(vector<pixel> means)
{
	//takes in a vector of the means of the tiles stored as pixels and constructs the kd tree

	root = new kdPixel(means[0], 0);

	for (int i = 1; i < means.size(); i++)
	{
		kdPixel* pix = new kdPixel(means[i], i);
		kdPixel* target = root;

		//finds the correct place in the tree and inserts the node
		while (target != nullptr)
		{
			for (int j = 0; j < 3; j++)
			{
				if (pix->colors[j] < target->colors[j])
				{
					if (target->left != nullptr)
						target = target->left;
					else
					{
						target->left = pix;
						target = nullptr;
						break;
					}
				}
				else
				{
					if (target->right != nullptr)
						target = target->right;
					else
					{
						target->right = pix;
						target = nullptr;
						break;
					}
				}
			}
		}
	}
}

int KDTree::query(pixel pix)
{
	unsigned char target[3] = { pix.red, pix.green, pix.blue };
	return queryHelper(root, target, 0)->index;
}

kdPixel* KDTree::queryHelper(kdPixel* root, unsigned char target[3], int level)
{
	kdPixel* taken;
	kdPixel* untaken;
	kdPixel* nearest;

	//check where to go and set branches
	if (target[level] < root->colors[level])
	{
		if (root->left)
		{
			taken = root->left;
			untaken = root->right;
		}
		else
			return root;
	}
	else
	{
		if (root->right)
		{
			taken = root->right;
			untaken = root->left;
		}
		else
			return root;
	}

	//recursively call and make sure the result is closer than the current node
	kdPixel* result = queryHelper(taken, target, (level + 1) % 3);
	if (dist(result, target) < dist(root, target))
		nearest = result;
	else
		nearest = root;

	//if there is an untaken path, makes sure it can't have a closer node
	if (untaken != nullptr)
	{
		//calculate the distance between the target and untaken axis
		int distanceToPlane = 0;
		for (int i = 0; i < 3; i++)
			if (i != level)
				distanceToPlane += (untaken->colors[i] - target[i]) * (untaken->colors[i] - target[i]);

		//checks other subtree for a closer point if possible
		if (distanceToPlane < dist(nearest, target))
		{
			result = queryHelper(untaken, target, (level + 1) % 3);
			if (dist(result, target) < dist(nearest, target))
				nearest = result;
		}
	}

	return nearest;
}

int KDTree::dist(kdPixel* pix, unsigned char target[3])
{
	int sum = 0;
	for (int i = 0; i < 3; i++)
		sum += (pix->colors[i] - target[i]) * (pix->colors[i] - target[i]);
	return sum;
}

KDTree::~KDTree()
{
	deletionHelper(root);
}

void KDTree::deletionHelper(kdPixel* root)
{
	if (root == nullptr)
		return;

	if (root->left)
		deletionHelper(root->left);
	if (root->right)
		deletionHelper(root->right);

	delete root;
}