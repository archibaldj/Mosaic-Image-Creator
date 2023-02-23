Hello! Welcome to the image mosaic generator!

To use this program, first set it up in an environemnt where it can be run on C++ 17. Then add the image you want to recreate to the baseImage folder, and place the images you want to compose it of into the tileImages folder. All images must be tga images and the tile images must be square. We recommend tile images between 21x21 and 32x32 pixels, as significantly larger images won't allow large base images. With 21x21 tiles, base iamges of up to 2.25mb can be used. Some example images and their output are provided, and the exampleTiles folders contain sets of tiles you can use.

When running the program, you will be presented with three options. The first one replaces each pixel in the original image with the most similar base image. When using a small number of base images or base images too similar in color, the output may not be similar to the original image. In this case you can use the second option, which shifts the mean color of the tile images to match their corresponding pixel from the base image, allowing decent results with as little as one tile. The third option simply compares the efficiency of an octree vs a kd-tree in recreating the original image from randomly generated pixels, and returns an image similar to the original.

Common errors and solutions:

Any error involving filesystem: The C++ version is likely not set to C++ 17

Vector subscript out of range: The base images are either not square or are not the same size

Final image generates but will not open: Some of the base images were incorrectly converted to tga files

Debug Assertion Failed! vector subscript out of range: The product of the size of the base image and the size of the tiles is too large. This program is only designed to create images smaller than 1gb.
