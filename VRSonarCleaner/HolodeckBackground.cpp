#include "HolodeckBackground.h"

#include <shared/glm/glm.hpp>

#include "DebugDrawer.h"

HolodeckBackground::HolodeckBackground(float SizeX, float SizeY, float SizeZ, float Spacing)
{
	sizeX = SizeX;
	sizeY = SizeY;
	sizeZ = SizeZ;

	spacing = Spacing;

	spacesX = floor(sizeX/spacing);
	spacesY = floor(sizeY/spacing);
	spacesZ = floor(sizeZ/spacing);

	spacingX = sizeX / spacesX;
	spacingY = sizeY / spacesY;
	spacingZ = sizeZ / spacesZ;

	minX = -sizeX/2;
	minY = 0 ;
	minZ = -sizeZ / 2;

	maxX = (sizeX / 2);
	maxY = sizeY;
	maxZ = (sizeZ / 2);

}

HolodeckBackground::~HolodeckBackground()
{

}

void HolodeckBackground::draw()
{
	glm::vec4 floorOpacity(1.f, 1.f, 0.f, 0.3f);
	glm::vec4 ceilingOpacity(1.f, 1.f, 0.f, 0.03f);
	glm::vec4 inBetweenOpacity(1.f, 1.f, 0.f, 0.f);
	
	for (float x = minX; x <= maxX ; x += spacingX)
	{
		DebugDrawer::getInstance().drawLine(glm::vec3(x, minY, minZ), glm::vec3(x, maxY, minZ), floorOpacity, ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(x, minY, maxZ), glm::vec3(x, maxY, maxZ), floorOpacity, ceilingOpacity);		
		DebugDrawer::getInstance().drawLine(glm::vec3(x, minY, minZ), glm::vec3(x, minY, maxZ), floorOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(x, maxY, minZ), glm::vec3(x, maxY, maxZ), ceilingOpacity);
	}

	for (float y = minY; y <= maxY; y += spacingY)
	{
		inBetweenOpacity.a = ((1-(y / maxY))*(floorOpacity.a - ceilingOpacity.a)) + ceilingOpacity.a;
		DebugDrawer::getInstance().drawLine(glm::vec3(minX, y, minZ), glm::vec3(maxX, y, minZ), inBetweenOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(minX, y, maxZ), glm::vec3(maxX, y, maxZ), inBetweenOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(minX, y, minZ), glm::vec3(minX, y, maxZ), inBetweenOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(maxX, y, minZ), glm::vec3(maxX, y, maxZ), inBetweenOpacity);		
	}

	for (float z = minZ; z <= maxZ; z += spacingZ)
	{
		DebugDrawer::getInstance().drawLine(glm::vec3(minX, minY, z), glm::vec3(maxX, minY, z), floorOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(minX, maxY, z), glm::vec3(maxX, maxY, z), ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(minX, minY, z), glm::vec3(minX, maxY, z), floorOpacity, ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(maxX, minY, z), glm::vec3(maxX, maxY, z), floorOpacity, ceilingOpacity);
	}
}


void HolodeckBackground::drawSolid()
{
	DebugDrawer::getInstance().setTransformDefault();

	drawGrids(0.15, 0.21, 0.31, 1);
	drawGrids(0.23, 0.29, 0.39, 0.25);	
}

void HolodeckBackground::drawGrids(float r, float g, float b, float spacingFactor)
{
	glm::vec4 floorOpacity(r, g, b, 0.3f);
	glm::vec4 ceilingOpacity(r, g, b, 0.03f);
	glm::vec4 inBetweenOpacity(r, g, b, 0.f);

	for (float x = minX; x <= maxX; x += spacingX*spacingFactor)
	{
		DebugDrawer::getInstance().drawLine(glm::vec3(x, minY, minZ), glm::vec3(x, maxY, minZ), floorOpacity, ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(x, minY, maxZ), glm::vec3(x, maxY, maxZ), floorOpacity, ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(x, minY, minZ), glm::vec3(x, minY, maxZ), floorOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(x, maxY, minZ), glm::vec3(x, maxY, maxZ), ceilingOpacity);
	}

	for (float y = minY; y <= maxY; y += spacingY*spacingFactor)
	{
		inBetweenOpacity.a = ((1 - (y / maxY))*(floorOpacity.a - ceilingOpacity.a)) + ceilingOpacity.a;

		DebugDrawer::getInstance().drawLine(glm::vec3(minX, y, minZ), glm::vec3(maxX, y, minZ), inBetweenOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(minX, y, maxZ), glm::vec3(maxX, y, maxZ), inBetweenOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(minX, y, minZ), glm::vec3(minX, y, maxZ), inBetweenOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(maxX, y, minZ), glm::vec3(maxX, y, maxZ), inBetweenOpacity);
	}

	for (float z = minZ; z <= maxZ; z += spacingZ*spacingFactor)
	{
		DebugDrawer::getInstance().drawLine(glm::vec3(minX, minY, z), glm::vec3(maxX, minY, z), floorOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(minX, maxY, z), glm::vec3(maxX, maxY, z), ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(minX, minY, z), glm::vec3(minX, maxY, z), floorOpacity, ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(maxX, minY, z), glm::vec3(maxX, maxY, z), floorOpacity, ceilingOpacity);
	}
}