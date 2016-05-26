#pragma once

//#include <SDL.h>
#include <GL/glew.h>
#include <math.h>
//#include <SDL_opengl.h>
//#include <gl/glu.h>
#include <stdio.h>
//#include <string>
//#include <cstdlib>

#include <vector>

//#include <openvr.h>

//#include "../shared/lodepng.h"
//#include "../shared/Matrices.h"
//#include "../shared/pathtools.h"

class HolodeckBackground
{
public:
	HolodeckBackground(float SizeX, float SizeY, float SizeZ, float Spacing);
	virtual ~HolodeckBackground();

	void draw();

private:

	GLuint m_unControllerVAO;
	GLuint m_glControllerVertBuffer;

	float sizeX, sizeY, sizeZ;
	float spacing;
	float spacingX, spacingY, spacingZ;
	int spacesX, spacesY, spacesZ;
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

};