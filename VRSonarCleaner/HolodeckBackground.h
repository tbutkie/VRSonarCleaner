#pragma once

#include <GL/glew.h>
#include <math.h>
#include <stdio.h>

#include <shared/glm/glm.hpp>

class HolodeckBackground
{
public:
	HolodeckBackground(glm::vec3 roomSizeMeters, float gridSpacingMeters);
	virtual ~HolodeckBackground();

	void draw();
	void drawSolid();
	void drawGrids(glm::vec3 color, float spacingFactor);

private:
	glm::vec3 m_vec3RoomSize;
	float m_fGridSpacing;
	glm::vec3 m_vec3RoomSpacings;
	glm::vec3 m_vec3Spaces;
	glm::vec3 m_vec3RoomMin;
	glm::vec3 m_vec3RoomMax;
};