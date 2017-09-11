#include "Dataset.h"

#include <limits>

Dataset::Dataset(std::string name, bool coord_sys_right_handed)
	: m_strName(name)
	, m_bRHCoordSys_input(coord_sys_right_handed)
	, m_bRHCoordSys_output(true)
	, m_bNeedsUpdate(true)
	, m_dvec3RawMinBounds(glm::dvec3(std::numeric_limits<double>::max()))
	, m_dvec3RawMaxBounds(glm::dvec3(-std::numeric_limits<double>::max()))
	, m_vec3AdjustedMinBounds(glm::vec3(std::numeric_limits<float>::max()))
	, m_vec3AdjustedMaxBounds(glm::vec3(-std::numeric_limits<float>::max()))
{
}

Dataset::~Dataset()
{
}

std::string Dataset::getName()
{
	return m_strName;
}

void Dataset::setRawMinBounds(glm::dvec3 minBounds)
{
	m_dvec3RawMinBounds = minBounds;
	m_bNeedsUpdate = true;
}

void Dataset::setRawXMin(double xMin)
{
	m_dvec3RawMinBounds.x = xMin;
	m_bNeedsUpdate = true;
}

void Dataset::setRawYMin(double yMin)
{
	m_dvec3RawMinBounds.y = yMin;
	m_bNeedsUpdate = true;
}

void Dataset::setRawZMin(double zMin)
{
	m_dvec3RawMinBounds.z = zMin;
	m_bNeedsUpdate = true;
}

glm::dvec3 Dataset::getRawMinBounds()
{
	update();
	return m_dvec3RawMinBounds;
}

double Dataset::getRawXMin()
{
	update();
	return m_dvec3RawMinBounds.x;
}

double Dataset::getRawYMin()
{
	update();
	return m_dvec3RawMinBounds.y;
}

double Dataset::getRawZMin()
{
	update();
	//if (m_bRHCoordSys_input != m_bRHCoordSys_output)
	//	return -m_dvec3RawMaxBounds.z;
	//else
		return m_dvec3RawMinBounds.z;
}

glm::vec3 Dataset::getAdjustedMinBounds()
{
	update();
	return m_vec3AdjustedMinBounds;
}

float Dataset::getAdjustedXMin()
{
	update();
	return m_vec3AdjustedMinBounds.x;
}

float Dataset::getAdjustedYMin()
{
	update();
	return m_vec3AdjustedMinBounds.y;
}

float Dataset::getAdjustedZMin()
{
	update();
	return m_vec3AdjustedMinBounds.z;
}

void Dataset::setRawMaxBounds(glm::dvec3 maxBounds)
{
	m_dvec3RawMaxBounds = maxBounds;
	m_bNeedsUpdate = true;
}

void Dataset::setRawXMax(double xMax)
{
	m_dvec3RawMaxBounds.x = xMax;
	m_bNeedsUpdate = true;
}

void Dataset::setRawYMax(double yMax)
{
	m_dvec3RawMaxBounds.y = yMax;
	m_bNeedsUpdate = true;
}

void Dataset::setRawZMax(double zMax)
{
	m_dvec3RawMaxBounds.z = zMax;
	m_bNeedsUpdate = true;
}

glm::dvec3 Dataset::getRawMaxBounds()
{
	update();
	return m_dvec3RawMaxBounds;
}

double Dataset::getRawXMax()
{
	update();
	return m_dvec3RawMaxBounds.x;
}

double Dataset::getRawYMax()
{
	update();
	return m_dvec3RawMaxBounds.y;
}

double Dataset::getRawZMax()
{
	update();
	//if (m_bRHCoordSys_input != m_bRHCoordSys_output)
	//	return -m_dvec3RawMinBounds.z;
	//else
		return m_dvec3RawMaxBounds.z;
}

glm::vec3 Dataset::getAdjustedMaxBounds()
{
	update();
	return m_vec3AdjustedMaxBounds;
}

float Dataset::getAdjustedXMax()
{
	update();
	return m_vec3AdjustedMaxBounds.x;
}

float Dataset::getAdjustedYMax()
{
	update();
	return m_vec3AdjustedMaxBounds.y;
}

float Dataset::getAdjustedZMax()
{
	update();
	return m_vec3AdjustedMaxBounds.z;
}

glm::dvec3 Dataset::getRawDimensions()
{
	update();
	return m_dvec3RawDimensions;
}

double Dataset::getRawXDimension()
{
	update();
	return m_dvec3RawDimensions.x;
}

double Dataset::getRawYDimension()
{
	update();
	return m_dvec3RawDimensions.y;
}

double Dataset::getRawZDimension()
{
	update();
	return m_dvec3RawDimensions.z;
}

glm::vec3 Dataset::getAdjustedDimensions()
{
	update();
	return m_vec3AdjustedDimensions;
}

float Dataset::getAdjustedXDimension()
{
	update();
	return m_vec3AdjustedDimensions.x;
}

float Dataset::getAdjustedYDimension()
{
	update();
	return m_vec3AdjustedDimensions.y;
}

float Dataset::getAdjustedZDimension()
{
	update();
	return m_vec3AdjustedDimensions.z;
}

void Dataset::checkNewRawPosition(glm::dvec3 rawPos)
{
	if (rawPos.x < m_dvec3RawMinBounds.x)
		setRawXMin(rawPos.x);
	if (rawPos.x > m_dvec3RawMaxBounds.x)
		setRawXMax(rawPos.x);

	if (rawPos.y < m_dvec3RawMinBounds.y)
		setRawYMin(rawPos.y);
	if (rawPos.y > m_dvec3RawMaxBounds.y)
		setRawYMax(rawPos.y);

	if (rawPos.z < m_dvec3RawMinBounds.z)
		setRawZMin(rawPos.z);
	if (rawPos.z > m_dvec3RawMaxBounds.z)
		setRawZMax(rawPos.z);
}

glm::dvec3 Dataset::getDataCenteringAdjustments()
{
	update();
	return m_dvec3Adjustments;
}

bool Dataset::isDataRightHanded()
{
	update();
	return m_bRHCoordSys_input;
}

bool Dataset::isOutputRightHanded()
{
	update();
	return m_bRHCoordSys_output;
}

void Dataset::update()
{
	if (m_bNeedsUpdate)
	{
		m_dvec3RawDimensions = m_dvec3RawMaxBounds - m_dvec3RawMinBounds;

		m_dvec3Adjustments = -(m_dvec3RawMinBounds + m_dvec3RawDimensions * 0.5); // origin at center
		//m_dvec3Adjustments = -m_dvec3RawMinBounds; // origin at min bound

		m_vec3AdjustedMaxBounds = m_dvec3RawMaxBounds + m_dvec3Adjustments;
		m_vec3AdjustedMinBounds = m_dvec3RawMinBounds + m_dvec3Adjustments;
		m_vec3AdjustedDimensions = m_vec3AdjustedMaxBounds - m_vec3AdjustedMinBounds;

		m_bNeedsUpdate = false;
	}
}
