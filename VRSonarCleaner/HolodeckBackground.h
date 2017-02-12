#pragma once

#include <GL/glew.h>
#include <math.h>
#include <stdio.h>

#include <vector>

class HolodeckBackground
{
public:
	HolodeckBackground(float SizeX, float SizeY, float SizeZ, float Spacing);
	virtual ~HolodeckBackground();

	void draw();
	void drawSolid();

	void drawGrids(float r, float g, float b, float spacingFactor);

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