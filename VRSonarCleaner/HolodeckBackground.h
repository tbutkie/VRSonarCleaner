#pragma once

//#include <SDL.h>
#include <GL/glew.h>
#include <math.h>
//#include <SDL_opengl.h>
//#include <gl/glu.h>
//#include <stdio.h>
//#include <string>
//#include <cstdlib>

//#include <openvr.h>

//#include "../shared/lodepng.h"
//#include "../shared/Matrices.h"
//#include "../shared/pathtools.h"

class HolodeckBackground
{
public:
	HolodeckBackground(float SizeX, float SizeY, float SizeZ);
	virtual ~HolodeckBackground();

	void draw();

private:
	float sizeX, sizeY, sizeZ;
	float spacingX, spacingY, spacingZ;
	int spacesX, spacesY, spacesZ;
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

};