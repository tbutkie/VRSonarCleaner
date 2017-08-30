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

	glm::dvec3 getRawMinBounds();
	double getRawXMin();
	double getRawYMin();
	double getRawZMin();

	glm::vec3 getAdjustedMinBounds();
	float getAdjustedXMin();
	float getAdjustedYMin();
	float getAdjustedZMin();

	glm::dvec3 getRawMaxBounds();
	double getRawXMax();
	double getRawYMax();
	double getRawZMax();

	glm::vec3 getAdjustedMaxBounds();
	float getAdjustedXMax();
	float getAdjustedYMax();
	float getAdjustedZMax();

	glm::dvec3 getRawDimensions();
	double getRawXDimension();
	double getRawYDimension();
	double getRawZDimension();

	glm::vec3 getAdjustedDimensions();
	float getAdjustedXDimension();
	float getAdjustedYDimension();
	float getAdjustedZDimension();

	void checkNewRawPosition(glm::dvec3 rawPos);

	glm::dvec3 getDataCenteringAdjustments();

	bool isDataRightHanded();
	bool isOutputRightHanded();

private:
	void setRawMinBounds(glm::dvec3 minBounds);
	void setRawXMin(double xMin);
	void setRawYMin(double yMin);
	void setRawZMin(double zMin);

	void setRawMaxBounds(glm::dvec3 maxBounds);
	void setRawXMax(double xMax);
	void setRawYMax(double yMax);
	void setRawZMax(double zMax);

	void update();

private:
	glm::dvec3 m_dvec3RawMinBounds;
	glm::dvec3 m_dvec3RawMaxBounds;
	glm::dvec3 m_dvec3RawDimensions;

	glm::dvec3 m_dvec3Adjustments;

	glm::vec3 m_vec3AdjustedMinBounds;
	glm::vec3 m_vec3AdjustedMaxBounds;
	glm::vec3 m_vec3AdjustedDimensions;
		
	bool m_bRHCoordSys_input; // flag indicating whether input coordinate system is left- or right-handed
	bool m_bRHCoordSys_output; // flag indicating whether output coordinate system should be left- or right-handed

	bool m_bNeedsUpdate;
};

