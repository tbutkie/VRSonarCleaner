#include "HolodeckBackground.h"

HolodeckBackground::HolodeckBackground(float SizeX, float SizeY, float SizeZ)
{
	sizeX = SizeX;
	sizeY = SizeY;
	sizeZ = SizeZ;

	spacesX = floor(sizeX);
	spacesY = floor(sizeY);
	spacesZ = floor(sizeZ);

	spacingX = sizeX / spacesX;
	spacingY = sizeY / spacesY;
	spacingZ = sizeZ / spacesZ;

	minX = -sizeX/2;
	minY = -sizeY/2;
	minZ = 0;

	maxX = (sizeX / 2);
	maxY = (sizeY / 2);
	maxZ = sizeZ;

}

HolodeckBackground::~HolodeckBackground()
{

}

void HolodeckBackground::draw()
{
	glColor3f(1.0, 1.0, 0.0);
	glLineWidth(2.0);
	glBegin(GL_LINES);
		glVertex3f(minX, minY, minZ);
		glVertex3f(maxX, minY, minZ);

		glVertex3f(minX, maxY, minZ);
		glVertex3f(maxX, maxY, minZ);

		glVertex3f(minX, minY, maxZ);
		glVertex3f(maxX, minY, maxZ);

		glVertex3f(minX, maxY, maxZ);
		glVertex3f(maxX, maxY, maxZ);

		glVertex3f(minX, minY, minZ);
		glVertex3f(minX, maxY, minZ);

		glVertex3f(minX, minY, minZ);
		glVertex3f(minX, minY, maxZ);

		glVertex3f(maxX, minY, minZ);
		glVertex3f(maxX, maxY, minZ);

		glVertex3f(maxX, minY, minZ);
		glVertex3f(maxX, minY, maxZ);
	glEnd();


}