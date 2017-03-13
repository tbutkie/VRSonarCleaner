#include "DataVolume.h"
#include "DebugDrawer.h"

#include "shared/glm/gtx/transform.hpp"

#include <iostream>
#include <math.h>

DataVolume::DataVolume(glm::vec3 pos, int startingOrientation, glm::vec3 size, glm::vec3 innerCoordsMin, glm::vec3 innerCoordsMax)
	: m_vec3Pos(pos)
	, m_vec3OriginalPosition(pos)
	, m_vec3Size(size)
	, m_vec3Scale(glm::vec3(1.f))
	, m_bFirstRun(true)
	, m_bNeedsUpdate(true)
{
	if (startingOrientation == 0)
		m_qOrientation = glm::angleAxis(0.f, glm::vec3(0, 0, 0));
	else
		m_qOrientation = glm::angleAxis(-90.f, glm::vec3(1, 0, 0));

	m_qOriginalOrientation = m_qOrientation;			
	
	setInnerCoords(innerCoordsMin, innerCoordsMax);

	updateTransforms();
}

DataVolume::~DataVolume()
{

}

void DataVolume::resetPositionAndOrientation()
{
	m_vec3Pos = m_vec3OriginalPosition;
	m_qOrientation = m_qOriginalOrientation;

	m_bNeedsUpdate = true;
}

void DataVolume::setSize(glm::vec3 size)
{
	m_vec3Size = size;

	recalcScaling();
}

glm::vec3 DataVolume::getSize()
{
	return m_vec3Size;
}

void DataVolume::setPosition(glm::vec3 pos)
{
	m_vec3Pos = pos;

	m_bNeedsUpdate = true;
}

void DataVolume::setOrientation(glm::mat4 orientation)
{
	m_qOrientation = glm::quat_cast(orientation);
}

void DataVolume::setInnerCoords(glm::vec3 minCoords, glm::vec3  maxCoords)
{
	m_vec3InnerMin = minCoords;
	m_vec3InnerMax = maxCoords;
	m_vec3InnerRange = m_vec3InnerMax - m_vec3InnerMin;

	m_vec3DataCenteringOffset = (-m_vec3InnerRange * 0.5f) - m_vec3InnerMin;

	recalcScaling();
}

void DataVolume::recalcScaling()
{
	float XZscale = std::min(m_vec3Size.x / m_vec3InnerRange.x, m_vec3Size.z / m_vec3InnerRange.z);
	float depthScale = m_vec3Size.y / m_vec3InnerRange.y;

	m_vec3Scale = glm::vec3(XZscale, depthScale, XZscale);

	m_bNeedsUpdate = true;
}//end recalc scaling

glm::vec3 DataVolume::convertToInnerCoords(glm::vec3 worldPos)
{
	return glm::vec3(glm::inverse(m_mat4DataTransform) * glm::vec4(worldPos, 1.f));
}

glm::vec3 DataVolume::convertToWorldCoords(glm::vec3 innerPos)
{	
	return glm::vec3(m_mat4DataTransform * glm::vec4(innerPos, 1.f));
}

void DataVolume::drawBBox()
{	
	glm::vec3 bbMin(-1.f);
	glm::vec3 bbMax(1.f);
	glm::vec4 color(0.22f, 0.25f, 0.34f, 1.f);

	DebugDrawer::getInstance().setTransform(
		glm::translate(glm::mat4(), m_vec3Pos) * glm::mat4(m_qOrientation) * glm::scale(m_vec3Size * 0.5f)
	);

	DebugDrawer::getInstance().drawBox(bbMin, bbMax, color);
}

void DataVolume::drawBacking()
{
	glm::vec3 bbMin(-1.f);
	glm::vec3 bbMax(1.f);
	glm::vec4 color(0.22f, 0.25f, 0.34f, 1.f);

	DebugDrawer::getInstance().setTransform(
		glm::translate(glm::mat4(), m_vec3Pos) * glm::mat4(m_qOrientation) * glm::scale(m_vec3Size * 0.5f)
	);
	
	DebugDrawer::getInstance().drawSolidTriangle(
		glm::vec3(bbMin.x, bbMin.y, bbMin.z),
		glm::vec3(bbMax.x, bbMin.y, bbMin.z),
		glm::vec3(bbMax.x, bbMin.y, bbMax.z),
		color
	);

	DebugDrawer::getInstance().drawSolidTriangle(
		glm::vec3(bbMin.x, bbMin.y, bbMin.z),
		glm::vec3(bbMax.x, bbMin.y, bbMax.z),
		glm::vec3(bbMin.x, bbMin.y, bbMax.z),
		color
	);	
}

void DataVolume::drawAxes()
{
	DebugDrawer::getInstance().setTransform(
		glm::translate(glm::mat4(), m_vec3Pos) * glm::mat4(m_qOrientation)
	);

	DebugDrawer::getInstance().drawTransform(0.1f);
}

glm::mat4 DataVolume::getCurrentDataTransform()
{
	if (m_bNeedsUpdate)
		updateTransforms();

	return m_mat4DataTransform;
}

glm::mat4 DataVolume::getLastDataTransform()
{
	return m_mat4DataTransformPrevious;
}

glm::mat4 DataVolume::getCurrentVolumeTransform()
{
	if (m_bNeedsUpdate)
		updateTransforms();

	return m_mat4VolumeTransform;
}

glm::mat4 DataVolume::getLastVolumeTransform()
{
	return m_mat4VolumeTransformPrevious;
}

void DataVolume::updateTransforms()
{
	m_mat4DataTransformPrevious = m_mat4DataTransform;
	m_mat4VolumeTransformPrevious = m_mat4VolumeTransform;

	glm::mat4 trans = glm::translate(glm::mat4(), m_vec3Pos);

	glm::mat4 rot = glm::mat4_cast(m_qOrientation);

	glm::mat4 scl = glm::scale(m_vec3Scale);

	glm::mat4 dataCenterTrans = glm::translate(glm::mat4(), m_vec3DataCenteringOffset);

	m_mat4DataTransform = trans * rot * scl * dataCenterTrans;
	m_mat4VolumeTransform = trans * rot;

	if (m_bFirstRun)
	{
		m_mat4DataTransformPrevious = m_mat4DataTransform;
		m_mat4VolumeTransformPrevious = m_mat4VolumeTransform;
		m_bFirstRun = false;
	}

	m_bNeedsUpdate = false;
}
