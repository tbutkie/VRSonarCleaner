#pragma once

#include <GL/glew.h>

#include <shared/glm/glm.hpp>
#include <shared/glm/gtc/quaternion.hpp>
#include <shared/glm/gtc/matrix_transform.hpp>

#include <string>

class Dataset
{
public:
	Dataset(bool coord_sys_right_handed = true);
	virtual ~Dataset();

	glm::dvec3 getMinBounds();
	double getXMin();
	double getYMin();
	double getZMin();

	glm::dvec3 getMaxBounds();
	double getXMax();
	double getYMax();
	double getZMax();

	glm::dvec3 getDimensions();
	double getXDimension();
	double getYDimension();
	double getZDimension();

	bool isDataRightHanded();
	bool isOutputRightHanded();

protected:
	glm::dvec3 m_dvec3MinBounds;
	glm::dvec3 m_dvec3MaxBounds;
	glm::dvec3 m_dvec3Dimensions;
		
	bool m_bRHCoordSys_input; // flag indicating whether input coordinate system is left- or right-handed
	bool m_bRHCoordSys_output; // flag indicating whether output coordinate system should be left- or right-handed
};

