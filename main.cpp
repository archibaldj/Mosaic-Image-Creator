#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string>
#include "pixel.h"
#include "Header.h"
#include "KDTree.h"
#include <sstream>
#include "Octree.h"
#include <stdlib.h>
#include <unordered_set>
using namespace std;
namespace fs = filesystem;

//Functions for reading and writing images
vector<vector<pixel>> ReadTilesFromTGA(Header &headerObject);
vector<pixel> ReadBaseFromTGA(Header& base);
string CleanFilename(string file);
void writeFile(string filename, Header& headerObject, vector<pixel> image);

//Helper functions for forming final mosaic
vector<pixel> CalculateMeans(vector<vector<pixel>>& tiles);
void Compose(vector<vector<pixel>>& tiles, vector <int>&indicies, Header baseHeader, int dims);
void ComposeAndShift(vector<vector<pixel>>& tiles, vector <int>& indicies, Header baseHeader, int dims, vector<pixel>& means, vector<pixel>& base);

//Creating and querying the data structures
vector<int> GetIndiciesWithKDTree(vector<pixel>& base, vector<pixel>& means);
vector <int> GetIndiciesWithOctree(vector<pixel>& base, vector<pixel>& means);

//Timed test to compare the two data structures
void LargeTest(vector<pixel> base, Header& baseHeader, int n);

int main()
{
	vector<vector<pixel>> tiles;
	vector<pixel> base;
	Header baseHeader;
    Header tileHeader;
    int tileSize = 0;
    int option = 0;

    cout << "Welcome to the mosaic image creator! Please select a menu option:" << endl;
    cout << "1. Create mosaic from original images" << endl << "2. Create mosaic and match colors" << endl << "3. Compare octree and k-d tree" << endl;
    cin >> option;

	//read from tga files
	string filetype = "tga";
	if (filetype == "tga")
	{
		tiles = ReadTilesFromTGA(tileHeader);
		base = ReadBaseFromTGA(baseHeader);
        tileSize = tileHeader.height;
	}

    if (option == 1)
    {
        //Calculate the means and make the final image
        vector<pixel> means = CalculateMeans(tiles);
        vector<int> indicies = GetIndiciesWithKDTree(base, means);
        vector<int> octIndicies = GetIndiciesWithOctree(base, means);
        Compose(tiles, octIndicies, baseHeader, tileSize);
    }
    //100,000 touple comparison
    else if (option == 2)
    {
        //Calculate the means and make the final image
        vector<pixel> means = CalculateMeans(tiles);
        vector<int> indicies = GetIndiciesWithKDTree(base, means);
        vector<int> octIndicies = GetIndiciesWithOctree(base, means);
        ComposeAndShift(tiles, octIndicies, baseHeader, tileSize, means, base);
    }
    else if (option == 3)
    {
        int n;
        cout << "Enter the number of unique randomly generated pixels to use (1-13,000,000): ";
        cin >> n;
        if (n < 1 || n > 13000000)
        {
            cout << "n not in range" << endl;
            return -1;
        }
        LargeTest(base, baseHeader, n);
    }
    else
        cout << "Not a menu option";
}

//Functions for reading images

vector<vector<pixel>> ReadTilesFromTGA(Header &headerObject)
{
    //reads all of the files from "tileImages", stores them in a vector of a vector of pixels, 
    //and adds the image data to the header
    vector<vector<pixel>> tiles;
	string path = "tileImages";
	for (const auto& entry : fs::directory_iterator(path))
	{
        ostringstream ss;
        ss << entry.path();
        string filename = ss.str();
        filename = CleanFilename(filename);
        ifstream file(filename, ios_base::binary);

        //read the header info
        file.read((char*)&headerObject.idLength, sizeof(headerObject.idLength));
        file.read((char*)&headerObject.colorMapType, sizeof(headerObject.colorMapType));
        file.read((char*)&headerObject.dataTypeCode, sizeof(headerObject.dataTypeCode));
        file.read((char*)&headerObject.colorMapOrigin, sizeof(headerObject.colorMapOrigin));
        file.read((char*)&headerObject.colorMapLength, sizeof(headerObject.colorMapLength));
        file.read((char*)&headerObject.colorMapDepth, sizeof(headerObject.colorMapDepth));
        file.read((char*)&headerObject.xOrigin, sizeof(headerObject.xOrigin));
        file.read((char*)&headerObject.yOrigin, sizeof(headerObject.yOrigin));
        file.read((char*)&headerObject.width, sizeof(headerObject.width));
        file.read((char*)&headerObject.height, sizeof(headerObject.height));
        file.read((char*)&headerObject.bitsPerPixel, sizeof(headerObject.bitsPerPixel));
        file.read((char*)&headerObject.imageDescriptor, sizeof(headerObject.imageDescriptor));

        //forms the actual image
        vector<pixel> image;
        for (int i = 0; i < headerObject.width * headerObject.height; i++)
        {
            pixel pix;

            file.read((char*)&pix.blue, sizeof(unsigned char));
            file.read((char*)&pix.green, sizeof(unsigned char));
            file.read((char*)&pix.red, sizeof(unsigned char));

            image.push_back(pix);
        }
        file.close();

        tiles.push_back(image);
	}

    return tiles;
}

vector<pixel> ReadBaseFromTGA(Header& baseHeader)
{
    int iterations = 0;
    vector<pixel> image;
    string path = "baseImage";
    for (const auto& entry : fs::directory_iterator(path))
    {
        if (iterations == 1)
        {
            cout << "Too many base images";
            abort();
        }
        ostringstream ss;
        ss << entry.path();
        string filename = ss.str();
        filename = CleanFilename(filename);
        ifstream file(filename, ios_base::binary);

        file.read((char*)&baseHeader.idLength, sizeof(baseHeader.idLength));
        file.read((char*)&baseHeader.colorMapType, sizeof(baseHeader.colorMapType));
        file.read((char*)&baseHeader.dataTypeCode, sizeof(baseHeader.dataTypeCode));
        file.read((char*)&baseHeader.colorMapOrigin, sizeof(baseHeader.colorMapOrigin));
        file.read((char*)&baseHeader.colorMapLength, sizeof(baseHeader.colorMapLength));
        file.read((char*)&baseHeader.colorMapDepth, sizeof(baseHeader.colorMapDepth));
        file.read((char*)&baseHeader.xOrigin, sizeof(baseHeader.xOrigin));
        file.read((char*)&baseHeader.yOrigin, sizeof(baseHeader.yOrigin));
        file.read((char*)&baseHeader.width, sizeof(baseHeader.width));
        file.read((char*)&baseHeader.height, sizeof(baseHeader.height));
        file.read((char*)&baseHeader.bitsPerPixel, sizeof(baseHeader.bitsPerPixel));
        file.read((char*)&baseHeader.imageDescriptor, sizeof(baseHeader.imageDescriptor));

        for (int i = 0; i < baseHeader.width * baseHeader.height; i++)
        {
            pixel pix;

            file.read((char*)&pix.blue, sizeof(unsigned char));
            file.read((char*)&pix.green, sizeof(unsigned char));
            file.read((char*)&pix.red, sizeof(unsigned char));

            image.push_back(pix);
        }
        file.close();
        iterations++;
    }

    return image;
}

string CleanFilename(string file)
{
    //tales a filename read from entry(), and transforms it into one usable by fstream
    file = file.substr(1, file.length() - 2);
    for (int i = 0; i < file.length() - 1; i++)
        if (file.at(i) == '\\' && file.at(i + 1) == '\\')
        {
            file = file.substr(0, i) + '/' + file.substr(i + 2, file.length() - i - 2);
            break;
        }
    return file;
}

void writeFile(string filename, Header& headerObject, vector<pixel> image)
{
    ofstream outfile(filename, ios_base::binary);

    outfile.write((char*)&headerObject.idLength, sizeof(headerObject.idLength));
    outfile.write((char*)&headerObject.colorMapType, sizeof(headerObject.colorMapType));
    outfile.write((char*)&headerObject.dataTypeCode, sizeof(headerObject.dataTypeCode));
    outfile.write((char*)&headerObject.colorMapOrigin, sizeof(headerObject.colorMapOrigin));
    outfile.write((char*)&headerObject.colorMapLength, sizeof(headerObject.colorMapLength));
    outfile.write((char*)&headerObject.colorMapDepth, sizeof(headerObject.colorMapDepth));
    outfile.write((char*)&headerObject.xOrigin, sizeof(headerObject.xOrigin));
    outfile.write((char*)&headerObject.yOrigin, sizeof(headerObject.yOrigin));
    outfile.write((char*)&headerObject.width, sizeof(headerObject.width));
    outfile.write((char*)&headerObject.height, sizeof(headerObject.height));
    outfile.write((char*)&headerObject.bitsPerPixel, sizeof(headerObject.bitsPerPixel));
    outfile.write((char*)&headerObject.imageDescriptor, sizeof(headerObject.imageDescriptor));

    for (int i = 0; i < headerObject.width * headerObject.height; i++)
    {
        outfile.write((char*)&image[i].blue, sizeof(unsigned char));
        outfile.write((char*)&image[i].green, sizeof(unsigned char));
        outfile.write((char*)&image[i].red, sizeof(unsigned char));
    }

    outfile.close();
}

//Helper functions for forming the final mosaic

vector<pixel> CalculateMeans(vector<vector<pixel>>& tiles)
{
    //calculates the mean of each color for each tile, and stores it in means as a pixel
    vector<pixel> means;

    //for each image
    for (unsigned int i = 0; i < tiles.size(); i++)
    {
        float red = 0;
        float green = 0;
        float blue = 0;

        //calculates the total of each color and pushes back a pixel storing the means
        for (unsigned int j = 0; j < tiles[i].size(); j++)
        {
            red += tiles[i][j].red;
            green += tiles[i][j].green;
            blue += tiles[i][j].blue;
        }
        means.push_back(pixel(blue / tiles[i].size(), green / tiles[i].size(), red / tiles[i].size()));
    }

    return means;
}

void Compose(vector<vector<pixel>>& tiles, vector<int>& indicies, Header baseHeader, int dims)
{
    //makes and saves the new image from the base image. Tiles is the original vector of pixels representing the tiles,
    //indicies is the order in which we apply the tiles, and dims is the height = width of the tile images

    //sets the height and width and creates a blank pixel vector
    Header mosaicHeader = baseHeader;
    mosaicHeader.width *= dims;
    mosaicHeader.height *= dims;
    vector<pixel> mosaic((int)mosaicHeader.height * mosaicHeader.width, pixel(0, 0, 0));
    for (int x = 0; x < baseHeader.width; x++)
        for (int y = 0; y < baseHeader.height; y++)
            for (int minix = 0; minix < dims; minix++)
                for (int miniy = 0; miniy < dims; miniy++)
                {
                    mosaic[dims * x + minix + mosaicHeader.width * y * dims + miniy*mosaicHeader.width].red = tiles[indicies[x + y * baseHeader.width]][minix + dims * miniy].red;
                    mosaic[dims * x + minix + mosaicHeader.width * y * dims + miniy * mosaicHeader.width].green = tiles[indicies[x + y * baseHeader.width]][minix + dims * miniy].green;
                    mosaic[dims * x + minix + mosaicHeader.width * y * dims + miniy * mosaicHeader.width].blue = tiles[indicies[x + y * baseHeader.width]][minix + dims * miniy].blue;
                }

    writeFile("output/mosaic.tga", mosaicHeader, mosaic);
    return;
}

void ComposeAndShift(vector<vector<pixel>>& tiles, vector <int>& indicies, Header baseHeader, int dims, vector<pixel>& means, vector<pixel>& base)
{
    //makes and saves the new image from the base image. Tiles is the original vector of pixels representing the tiles,
    //indicies is the order in which we apply the tiles, and dims is the height = width of the tile images

    //sets the height and width and creates a blank pixel vector
    Header mosaicHeader = baseHeader;
    mosaicHeader.width *= dims;
    mosaicHeader.height *= dims;
    vector<pixel> mosaic((int)mosaicHeader.height * mosaicHeader.width, pixel(0, 0, 0));
    for (int x = 0; x < baseHeader.width; x++)
        for (int y = 0; y < baseHeader.height; y++)
            for (int minix = 0; minix < dims; minix++)
                for (int miniy = 0; miniy < dims; miniy++)
                {
                    float red = tiles[indicies[x + y * baseHeader.width]][minix + dims * miniy].red / means[indicies[x + y * baseHeader.width]].red * base[x + y * baseHeader.width].red;
                    float green = tiles[indicies[x + y * baseHeader.width]][minix + dims * miniy].green / means[indicies[x + y * baseHeader.width]].green * base[x + y * baseHeader.width].green;
                    float blue = tiles[indicies[x + y * baseHeader.width]][minix + dims * miniy].blue / means[indicies[x + y * baseHeader.width]].blue * base[x + y * baseHeader.width].blue;
                    if (red > 255)
                        red = 255;
                    if (blue > 255)
                        blue = 255;
                    if (green > 255)
                        green = 255;
                    mosaic[dims * x + minix + mosaicHeader.width * y * dims + miniy * mosaicHeader.width].red = (unsigned char) red;
                    mosaic[dims * x + minix + mosaicHeader.width * y * dims + miniy * mosaicHeader.width].green = (unsigned char) green;
                    mosaic[dims * x + minix + mosaicHeader.width * y * dims + miniy * mosaicHeader.width].blue = (unsigned char) blue;
                }

    writeFile("output/mosaic.tga", mosaicHeader, mosaic);
    return;
}


//Creating and querying the data structures

vector<int> GetIndiciesWithKDTree(vector<pixel>& base, vector<pixel>& means)
{
    KDTree kdtree = KDTree(means);
    vector<int> indicies;

    for (int i = 0; i < base.size(); i++)
    {
        indicies.push_back(kdtree.query(base[i]));
    }

    return indicies;
}

vector<int> GetIndiciesWithOctree(vector<pixel>& base, vector<pixel>& means)
{
    Octree octree = Octree(means);
    vector<int> indicies;

    for (int i = 0; i < base.size(); i++)
    {
        if (i == 37)
            i = 37;
        indicies.push_back(octree.Query(base[i]));
    }

    return indicies;
}

//Timed test to compare the two data structures

void LargeTest(vector<pixel> base, Header &baseHeader, int n)
{
    //create random subimages
    vector<pixel> tiles;
    unordered_set<int> repeats;
    for (int i = 0; i < n; i++)
    {
        pixel pix = pixel(rand() % 255, rand() % 255, rand() % 255);
        while (repeats.find(pix.red + pix.green * 255 + pix.blue * 255 * 255) != repeats.end())
        {
            pix.red = rand() % 255;
            pix.green = rand() % 255;
            pix.blue = rand() % 255;
        }
        tiles.push_back(pix);
        repeats.insert(pix.red + pix.green * 255 + pix.blue * 255 * 255);
    }

    clock_t startTime = clock();
    vector<int> indicies = GetIndiciesWithKDTree(base, tiles);
    double time = double(clock() - startTime) / (double)CLOCKS_PER_SEC;
    cout << "Time to create and query KD-tree: " << time << "s" << endl;
    indicies.clear();

    startTime = clock();
    vector<int> octindicies = GetIndiciesWithOctree(base, tiles);
    time = double(clock() - startTime) / (double)CLOCKS_PER_SEC;
    cout << "Time to create and query Octree:  " << time << "s" << endl;

    vector<vector<pixel>> tiles2;
    for (int i = 0; i < tiles.size(); i++)
    {
        vector<pixel> temp;
        temp.push_back(tiles[i]);
        tiles2.push_back(temp);
    }

    startTime = clock();
    Compose(tiles2, octindicies, baseHeader, 1);
    time = double(clock() - startTime) / (double)CLOCKS_PER_SEC;
    cout << "Time to compose the final image:  " << time << "s" << endl;
}
