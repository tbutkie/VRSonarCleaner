#include "DataVolume.h"
#include "DebugDrawer.h"

#include "shared/glm/gtx/transform.hpp"

#include <iostream>
#include <math.h>

DataVolume::DataVolume(glm::vec3 pos, int startingOrientation, glm::vec3 dimensions, glm::vec3 innerCoordsMin, glm::vec3 innerCoordsMax)
	: Node(pos)
	, m_vec3OriginalPosition(pos)
	, m_vec3Dimensions(dimensions)
	, m_bFirstRun(true)
{
	if (startingOrientation == 0)
		setOrientation(glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)));
	else
		setOrientation(glm::angleAxis(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)));

	m_qOriginalOrientation = getOrientation();			
	
	setInnerCoords(innerCoordsMin, innerCoordsMax);

	updateTransforms();
}

DataVolume::~DataVolume()
{

}

void DataVolume::resetPositionAndOrientation()
{
	setPosition(m_vec3OriginalPosition);
	setOrientation(m_qOriginalOrientation);
}

void DataVolume::setDimensions(glm::vec3 dimensions)
{
	m_vec3Dimensions = dimensions;

	recalcScaling();
}

glm::vec3 DataVolume::getDimensions()
{
	return m_vec3Dimensions;
}

void DataVolume::setInnerCoords(glm::vec3 minCoords, glm::vec3  maxCoords)
{
	m_vec3InnerMin = minCoords;
	m_vec3InnerMax = maxCoords;
	m_vec3InnerRange = m_vec3InnerMax - m_vec3InnerMin;

	m_vec3DataCenteringOffset = (-m_vec3InnerRange * 0.5f) - m_vec3InnerMin;

	recalcScaling();
}

glm::vec3 DataVolume::getOriginalPosition()
{
	return m_vec3OriginalPosition;
}

glm::quat DataVolume::getOriginalOrientation()
{
	return m_qOriginalOrientation;
}

void DataVolume::recalcScaling()
{
	float XYscale = std::min(m_vec3Dimensions.x / m_vec3InnerRange.x, m_vec3Dimensions.y / m_vec3InnerRange.y);
	float depthScale = m_vec3Dimensions.z / m_vec3InnerRange.z;

	setScale(glm::vec3(XYscale, XYscale, depthScale));

	updateTransforms();
}

glm::vec3 DataVolume::convertToInnerCoords(glm::vec3 worldPos)
{
	return glm::vec3(glm::inverse(getCurrentDataTransform()) * glm::vec4(worldPos, 1.f));
}

glm::vec3 DataVolume::convertToWorldCoords(glm::vec3 innerPos)
{
	updateTransforms();

	return glm::vec3(getCurrentDataTransform() * glm::vec4(innerPos, 1.f));
}

void DataVolume::drawBBox()
{	
	glm::vec3 bbMin(-1.f);
	glm::vec3 bbMax(1.f);
	glm::vec4 color(0.22f, 0.25f, 0.34f, 1.f);

	DebugDrawer::getInstance().setTransform(
		glm::translate(glm::mat4(), getPosition()) * glm::mat4(getOrientation()) * glm::scale(m_vec3Dimensions * 0.5f)
	);

	DebugDrawer::getInstance().drawBox(bbMin, bbMax, color);
}

void DataVolume::drawBacking()
{
	glm::vec3 bbMin(-1.f);
	glm::vec3 bbMax(1.f);
	glm::vec4 color(0.22f, 0.25f, 0.34f, 1.f);

	DebugDrawer::getInstance().setTransform(
		glm::translate(glm::mat4(), getPosition()) * glm::mat4(getOrientation()) * glm::scale(m_vec3Dimensions * 0.5f)
	);
	
	DebugDrawer::getInstance().drawSolidTriangle(
		glm::vec3(bbMin.x, bbMin.y, bbMin.z),
		glm::vec3(bbMax.x, bbMin.y, bbMin.z),
		glm::vec3(bbMax.x, bbMax.y, bbMin.z),
		color
	);

	DebugDrawer::getInstance().drawSolidTriangle(
		glm::vec3(bbMin.x, bbMin.y, bbMin.z),
		glm::vec3(bbMax.x, bbMax.y, bbMin.z),
		glm::vec3(bbMin.x, bbMax.y, bbMin.z),
		color
	);	
}

glm::mat4 DataVolume::getCurrentDataTransform()
{
	updateTransforms();

	return m_mat4DataTransform;
}

glm::mat4 DataVolume::getLastDataTransform()
{
	updateTransforms();

	return m_mat4DataTransformPrevious;
}

glm::mat4 DataVolume::getCurrentVolumeTransform()
{
	updateTransforms();

	return m_mat4VolumeTransform;
}

glm::mat4 DataVolume::getLastVolumeTransform()
{
	updateTransforms();

	return m_mat4VolumeTransformPrevious;
}

void DataVolume::updateTransforms()
{
	if (isDirty())
	{
		m_mat4DataTransformPrevious = m_mat4DataTransform;
		m_mat4VolumeTransformPrevious = m_mat4VolumeTransform;

		m_mat4DataTransform = getModelToWorldTransform() * glm::translate(glm::mat4(), m_vec3DataCenteringOffset);
		m_mat4VolumeTransform = getModelToWorldTransform() * glm::scale(1.f / getScale()); // undo the scaling

		if (m_bFirstRun)
		{
			m_mat4DataTransformPrevious = m_mat4DataTransform;
			m_mat4VolumeTransformPrevious = m_mat4VolumeTransform;
			m_bFirstRun = false;
		}
	}
}
