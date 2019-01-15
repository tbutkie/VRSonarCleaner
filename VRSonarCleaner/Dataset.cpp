#include "Dataset.h"

#include <limits>

Dataset::Dataset(std::string name, bool coord_sys_right_handed)
	: m_strName(name)
	, m_bLoaded(false)
	, m_bRHCoordSys_input(coord_sys_right_handed)
	, m_bRHCoordSys_output(true)
	, m_bNeedsUpdate(true)
	, m_dvec3MinBounds(glm::dvec3(std::numeric_limits<double>::max()))
	, m_dvec3MaxBounds(glm::dvec3(-std::numeric_limits<double>::max()))
	, m_vec3CenteredMinBounds(glm::vec3(std::numeric_limits<float>::max()))
	, m_vec3CenteredMaxBounds(glm::vec3(-std::numeric_limits<float>::max()))
{
}

Dataset::~Dataset()
{
}

std::string Dataset::getName()
{
	return m_strName;
}

bool Dataset::isLoaded()
{
	return m_bLoaded;
}

void Dataset::setMinBounds(glm::dvec3 minBounds)
{
	m_dvec3MinBounds = minBounds;
	m_bNeedsUpdate = true;
}

void Dataset::setXMin(double xMin)
{
	m_dvec3MinBounds.x = xMin;
	m_bNeedsUpdate = true;
}

void Dataset::setYMin(double yMin)
{
	m_dvec3MinBounds.y = yMin;
	m_bNeedsUpdate = true;
}

void Dataset::setZMin(double zMin)
{
	m_dvec3MinBounds.z = zMin;
	m_bNeedsUpdate = true;
}

glm::dvec3 Dataset::getMinBounds()
{
	update();
	return m_dvec3MinBounds;
}

double Dataset::getXMin()
{
	update();
	return m_dvec3MinBounds.x;
}

double Dataset::getYMin()
{
	update();
	return m_dvec3MinBounds.y;
}

double Dataset::getZMin()
{
	update();
	//if (m_bRHCoordSys_input != m_bRHCoordSys_output)
	//	return -m_dvec3MaxBounds.z;
	//else
		return m_dvec3MinBounds.z;
}

glm::vec3 Dataset::getCenteredMinBounds()
{
	update();
	return m_vec3CenteredMinBounds;
}

float Dataset::getCenteredXMin()
{
	update();
	return m_vec3CenteredMinBounds.x;
}

float Dataset::getCenteredYMin()
{
	update();
	return m_vec3CenteredMinBounds.y;
}

float Dataset::getCenteredZMin()
{
	update();
	return m_vec3CenteredMinBounds.z;
}

void Dataset::setMaxBounds(glm::dvec3 maxBounds)
{
	m_dvec3MaxBounds = maxBounds;
	m_bNeedsUpdate = true;
}

void Dataset::setXMax(double xMax)
{
	m_dvec3MaxBounds.x = xMax;
	m_bNeedsUpdate = true;
}

void Dataset::setYMax(double yMax)
{
	m_dvec3MaxBounds.y = yMax;
	m_bNeedsUpdate = true;
}

void Dataset::setZMax(double zMax)
{
	m_dvec3MaxBounds.z = zMax;
	m_bNeedsUpdate = true;
}

glm::dvec3 Dataset::getMaxBounds()
{
	update();
	return m_dvec3MaxBounds;
}

double Dataset::getXMax()
{
	update();
	return m_dvec3MaxBounds.x;
}

double Dataset::getYMax()
{
	update();
	return m_dvec3MaxBounds.y;
}

double Dataset::getZMax()
{
	update();
	//if (m_bRHCoordSys_input != m_bRHCoordSys_output)
	//	return -m_dvec3MinBounds.z;
	//else
		return m_dvec3MaxBounds.z;
}

glm::vec3 Dataset::getCenteredMaxBounds()
{
	update();
	return m_vec3CenteredMaxBounds;
}

float Dataset::getCenteredXMax()
{
	update();
	return m_vec3CenteredMaxBounds.x;
}

float Dataset::getCenteredYMax()
{
	update();
	return m_vec3CenteredMaxBounds.y;
}

float Dataset::getCenteredZMax()
{
	update();
	return m_vec3CenteredMaxBounds.z;
}

glm::dvec3 Dataset::getRange()
{
	update();
	return m_dvec3Range;
}

double Dataset::getXRange()
{
	update();
	return m_dvec3Range.x;
}

double Dataset::getYRange()
{
	update();
	return m_dvec3Range.y;
}

double Dataset::getZRange()
{
	update();
	return m_dvec3Range.z;
}

void Dataset::checkNewPosition(glm::dvec3 Pos)
{
	if (Pos.x < m_dvec3MinBounds.x)
		setXMin(Pos.x);
	if (Pos.x > m_dvec3MaxBounds.x)
		setXMax(Pos.x);

	if (Pos.y < m_dvec3MinBounds.y)
		setYMin(Pos.y);
	if (Pos.y > m_dvec3MaxBounds.y)
		setYMax(Pos.y);

	if (Pos.z < m_dvec3MinBounds.z)
		setZMin(Pos.z);
	if (Pos.z > m_dvec3MaxBounds.z)
		setZMax(Pos.z);
}

glm::dvec3 Dataset::getCenteringOffsets()
{
	update();
	return m_dvec3CenteringOffsets;
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
		m_dvec3Range = m_dvec3MaxBounds - m_dvec3MinBounds;

		m_dvec3CenteringOffsets = -(m_dvec3MinBounds + m_dvec3Range * 0.5); // origin at center

		m_vec3CenteredMaxBounds = m_dvec3MaxBounds + m_dvec3CenteringOffsets;
		m_vec3CenteredMinBounds = m_dvec3MinBounds + m_dvec3CenteringOffsets;

		m_bNeedsUpdate = false;
	}
}
