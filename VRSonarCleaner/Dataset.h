#pragma once

#include <GL/glew.h>

#include <glm.hpp>
#include <gtc/quaternion.hpp>
#include <gtc/matrix_transform.hpp>

#include <string>

class Dataset
{
public:
	Dataset(std::string name, bool coord_sys_right_handed = true);
	virtual ~Dataset();

	// Get the dataset filename
	std::string getName();

	// Returns true after 
	bool isLoaded();

	glm::dvec3 getRawMinBounds();
	double getRawXMin();
	double getRawYMin();
	double getRawZMin();

	glm::vec3 getCenteredMinBounds();
	float getCenteredXMin();
	float getCenteredYMin();
	float getCenteredZMin();

	glm::dvec3 getRawMaxBounds();
	double getRawXMax();
	double getRawYMax();
	double getRawZMax();

	glm::vec3 getCenteredMaxBounds();
	float getCenteredXMax();
	float getCenteredYMax();
	float getCenteredZMax();

	glm::dvec3 getRawDimensions();
	double getRawXDimension();
	double getRawYDimension();
	double getRawZDimension();

	glm::vec3 getCenteredDimensions();
	float getCenteredXDimension();
	float getCenteredYDimension();
	float getCenteredZDimension();

	void checkNewRawPosition(glm::dvec3 rawPos);

	glm::dvec3 getRawToCenteredOffsets();

	bool isDataRightHanded();
	bool isOutputRightHanded();

protected:
	bool m_bLoaded;

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
	std::string m_strName;

	glm::dvec3 m_dvec3RawMinBounds;
	glm::dvec3 m_dvec3RawMaxBounds;
	glm::dvec3 m_dvec3RawDimensions;

	glm::dvec3 m_dvec3CenteringOffsets;

	glm::vec3 m_vec3CenteredMinBounds;
	glm::vec3 m_vec3CenteredMaxBounds;
	glm::vec3 m_vec3CenteredDimensions;
		
	bool m_bRHCoordSys_input; // flag indicating whether input coordinate system is left- or right-handed
	bool m_bRHCoordSys_output; // flag indicating whether output coordinate system should be left- or right-handed

	bool m_bNeedsUpdate;
};

