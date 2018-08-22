#include "Object3D.h"

#include <vector>

Object3D::Object3D(glm::vec3 pos, glm::quat orientation, glm::vec3 dimensions)
	: m_vec3Position(pos)
	, m_qOrientation(orientation)
	, m_vec3Dimensions(dimensions)
	, m_vec3OriginalPosition(pos)
	, m_qOriginalOrientation(orientation)
	, m_vec3OriginalDimensions(dimensions)
	, m_bDirty(true)
{
	calculateBoundingRadius();
}

Object3D::~Object3D()
{
}



glm::vec3 Object3D::getOriginalPosition()
{
	return m_vec3OriginalPosition;
}

glm::quat Object3D::getOriginalOrientation()
{
	return m_qOriginalOrientation;
}

glm::vec3 Object3D::getOriginalDimensions()
{
	return m_vec3OriginalDimensions;
}

void Object3D::resetPositionAndOrientation()
{
	setPosition(m_vec3OriginalPosition);
	setOrientation(m_qOriginalOrientation);
	setDimensions(m_vec3OriginalDimensions);
}

void Object3D::setPosition(glm::vec3 newPos)
{
	m_vec3Position = newPos;
	m_bDirty = true;
}

glm::vec3 Object3D::getPosition()
{
	return m_vec3Position;
}

void Object3D::setOrientation(glm::quat newOrientation)
{
	m_qOrientation = newOrientation;
	m_bDirty = true;
}

glm::quat Object3D::getOrientation()
{
	return m_qOrientation;
}

void Object3D::setDimensions(glm::vec3 newScale)
{
	m_vec3Dimensions = newScale;
	calculateBoundingRadius();
	m_bDirty = true;
}

glm::vec3 Object3D::getDimensions()
{
	return m_vec3Dimensions;
}

void Object3D::calculateBoundingRadius()
{
	std::vector<float> vTableDims = { m_vec3Dimensions.x, m_vec3Dimensions.y, m_vec3Dimensions.z };
	m_fBoundingRadius = std::sqrt(vTableDims[0] * vTableDims[0] + vTableDims[1] * vTableDims[1] + vTableDims[2] * vTableDims[2]) * 0.5f;
}

float Object3D::getBoundingRadius()
{
	return m_fBoundingRadius;
}