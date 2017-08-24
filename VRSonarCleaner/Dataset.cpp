#include "Dataset.h"

Dataset::Dataset(bool coord_sys_right_handed)
	: m_bRHCoordSys_input(coord_sys_right_handed)
	, m_bRHCoordSys_output(true)
{
}

Dataset::~Dataset()
{
}

glm::dvec3 Dataset::getMinBounds()
{
	return m_dvec3MinBounds;
}

double Dataset::getXMin()
{
	return m_dvec3MinBounds.x;
}

double Dataset::getYMin()
{
	return m_dvec3MinBounds.y;
}

double Dataset::getZMin()
{
	if (m_bRHCoordSys_input != m_bRHCoordSys_output)
		return -m_dvec3MaxBounds.z;
	else
		return m_dvec3MinBounds.z;
}

glm::dvec3 Dataset::getMaxBounds()
{
	return m_dvec3MaxBounds;
}

double Dataset::getXMax()
{
	return m_dvec3MaxBounds.x;
}

double Dataset::getYMax()
{
	return m_dvec3MaxBounds.y;
}

double Dataset::getZMax()
{
	if (m_bRHCoordSys_input != m_bRHCoordSys_output)
		return -m_dvec3MinBounds.z;
	else
		return m_dvec3MaxBounds.z;
}

glm::dvec3 Dataset::getDimensions()
{
	return m_dvec3Dimensions;
}

double Dataset::getXDimension()
{
	return m_dvec3Dimensions.x;
}

double Dataset::getYDimension()
{
	return m_dvec3Dimensions.y;
}

double Dataset::getZDimension()
{
	return m_dvec3Dimensions.z;
}

bool Dataset::isDataRightHanded()
{
	return m_bRHCoordSys_input;
}

bool Dataset::isOutputRightHanded()
{
	return m_bRHCoordSys_output;
}
