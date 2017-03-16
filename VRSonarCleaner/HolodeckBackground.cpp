#include "HolodeckBackground.h"

#include <shared/glm/glm.hpp>

#include "DebugDrawer.h"

HolodeckBackground::HolodeckBackground(glm::vec3 roomSizeMeters, float spacingMeters)
	: m_vec3RoomSize(roomSizeMeters)
	, m_fGridSpacing(spacingMeters)
{
	m_vec3Spaces = floor(m_vec3RoomSize / m_fGridSpacing);
	m_vec3RoomSpacings = m_vec3RoomSize / m_vec3Spaces;

	m_vec3RoomMin = -m_vec3RoomSize * 0.5f;
	m_vec3RoomMin.y = 0.f;

	m_vec3RoomMax = m_vec3RoomSize * 0.5f;
	m_vec3RoomMax.y = m_vec3RoomSize.y;
}

HolodeckBackground::~HolodeckBackground()
{
}

void HolodeckBackground::draw()
{
	glm::vec4 floorOpacity(1.f, 1.f, 0.f, 0.3f);
	glm::vec4 ceilingOpacity(1.f, 1.f, 0.f, 0.03f);
	glm::vec4 inBetweenOpacity(1.f, 1.f, 0.f, 0.f);
	
	for (float x = m_vec3RoomMin.x; x <= m_vec3RoomMax.x; x += m_vec3RoomSpacings.x)
	{
		DebugDrawer::getInstance().drawLine(glm::vec3(x, m_vec3RoomMin.y, m_vec3RoomMin.z), glm::vec3(x, m_vec3RoomMax.y, m_vec3RoomMin.z), floorOpacity, ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(x, m_vec3RoomMin.y, m_vec3RoomMax.z), glm::vec3(x, m_vec3RoomMax.y, m_vec3RoomMax.z), floorOpacity, ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(x, m_vec3RoomMin.y, m_vec3RoomMin.z), glm::vec3(x, m_vec3RoomMin.y, m_vec3RoomMax.z), floorOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(x, m_vec3RoomMax.y, m_vec3RoomMin.z), glm::vec3(x, m_vec3RoomMax.y, m_vec3RoomMax.z), ceilingOpacity);
	}

	for (float y = m_vec3RoomMin.y; y <= m_vec3RoomMax.y; y += m_vec3RoomSpacings.y)
	{
		inBetweenOpacity.a = ((1.f-(y / m_vec3RoomMax.y)) * (floorOpacity.a - ceilingOpacity.a)) + ceilingOpacity.a;
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMin.x, y, m_vec3RoomMin.z), glm::vec3(m_vec3RoomMax.x, y, m_vec3RoomMin.z), inBetweenOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMin.x, y, m_vec3RoomMax.z), glm::vec3(m_vec3RoomMax.x, y, m_vec3RoomMax.z), inBetweenOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMin.x, y, m_vec3RoomMin.z), glm::vec3(m_vec3RoomMin.x, y, m_vec3RoomMax.z), inBetweenOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMax.x, y, m_vec3RoomMin.z), glm::vec3(m_vec3RoomMax.x, y, m_vec3RoomMax.z), inBetweenOpacity);
	}

	for (float z = m_vec3RoomMin.z; z <= m_vec3RoomMax.z; z += m_vec3RoomSpacings.z)
	{
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMin.x, m_vec3RoomMin.y, z), glm::vec3(m_vec3RoomMax.x, m_vec3RoomMin.y, z), floorOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMin.x, m_vec3RoomMax.y, z), glm::vec3(m_vec3RoomMax.x, m_vec3RoomMax.y, z), ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMin.x, m_vec3RoomMin.y, z), glm::vec3(m_vec3RoomMin.x, m_vec3RoomMax.y, z), floorOpacity, ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMax.x, m_vec3RoomMin.y, z), glm::vec3(m_vec3RoomMax.x, m_vec3RoomMax.y, z), floorOpacity, ceilingOpacity);
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

	for (float x = m_vec3RoomMin.x; x <= m_vec3RoomMax.x; x += m_vec3RoomSpacings.x * spacingFactor)
	{
		DebugDrawer::getInstance().drawLine(glm::vec3(x, m_vec3RoomMin.y, m_vec3RoomMin.z), glm::vec3(x, m_vec3RoomMax.y, m_vec3RoomMin.z), floorOpacity, ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(x, m_vec3RoomMin.y, m_vec3RoomMax.z), glm::vec3(x, m_vec3RoomMax.y, m_vec3RoomMax.z), floorOpacity, ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(x, m_vec3RoomMin.y, m_vec3RoomMin.z), glm::vec3(x, m_vec3RoomMin.y, m_vec3RoomMax.z), floorOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(x, m_vec3RoomMax.y, m_vec3RoomMin.z), glm::vec3(x, m_vec3RoomMax.y, m_vec3RoomMax.z), ceilingOpacity);
	}

	for (float y = m_vec3RoomMin.y; y <= m_vec3RoomMax.y; y += m_vec3RoomSpacings.y * spacingFactor)
	{
		inBetweenOpacity.a = ((1.f - (y / m_vec3RoomMax.y)) * (floorOpacity.a - ceilingOpacity.a)) + ceilingOpacity.a;

		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMin.x, y, m_vec3RoomMin.z), glm::vec3(m_vec3RoomMax.x, y, m_vec3RoomMin.z), inBetweenOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMin.x, y, m_vec3RoomMax.z), glm::vec3(m_vec3RoomMax.x, y, m_vec3RoomMax.z), inBetweenOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMin.x, y, m_vec3RoomMin.z), glm::vec3(m_vec3RoomMin.x, y, m_vec3RoomMax.z), inBetweenOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMax.x, y, m_vec3RoomMin.z), glm::vec3(m_vec3RoomMax.x, y, m_vec3RoomMax.z), inBetweenOpacity);
	}

	for (float z = m_vec3RoomMin.z; z <= m_vec3RoomMax.z; z += m_vec3RoomSpacings.z * spacingFactor)
	{
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMin.x, m_vec3RoomMin.y, z), glm::vec3(m_vec3RoomMax.x, m_vec3RoomMin.y, z), floorOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMin.x, m_vec3RoomMax.y, z), glm::vec3(m_vec3RoomMax.x, m_vec3RoomMax.y, z), ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMin.x, m_vec3RoomMin.y, z), glm::vec3(m_vec3RoomMin.x, m_vec3RoomMax.y, z), floorOpacity, ceilingOpacity);
		DebugDrawer::getInstance().drawLine(glm::vec3(m_vec3RoomMax.x, m_vec3RoomMin.y, z), glm::vec3(m_vec3RoomMax.x, m_vec3RoomMax.y, z), floorOpacity, ceilingOpacity);
	}
}