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

	glm::dvec3 getMinBounds();
	double getXMin();
	double getYMin();
	double getZMin();

	glm::vec3 getCenteredMinBounds();
	float getCenteredXMin();
	float getCenteredYMin();
	float getCenteredZMin();

	glm::dvec3 getMaxBounds();
	double getXMax();
	double getYMax();
	double getZMax();

	glm::vec3 getCenteredMaxBounds();
	float getCenteredXMax();
	float getCenteredYMax();
	float getCenteredZMax();

	glm::dvec3 getRange();
	double getXRange();
	double getYRange();
	double getZRange();

	void checkNewPosition(glm::dvec3 pos);

	glm::dvec3 getCenteringOffsets();

	bool isDataRightHanded();
	bool isOutputRightHanded();

protected:
	bool m_bLoaded;

private:
	void setMinBounds(glm::dvec3 minBounds);
	void setXMin(double xMin);
	void setYMin(double yMin);
	void setZMin(double zMin);

	void setMaxBounds(glm::dvec3 maxBounds);
	void setXMax(double xMax);
	void setYMax(double yMax);
	void setZMax(double zMax);

	void update();

private:
	std::string m_strName;

	glm::dvec3 m_dvec3MinBounds;
	glm::dvec3 m_dvec3MaxBounds;
	glm::dvec3 m_dvec3Range;

	glm::dvec3 m_dvec3CenteringOffsets;

	glm::vec3 m_vec3CenteredMinBounds;
	glm::vec3 m_vec3CenteredMaxBounds;
		
	bool m_bRHCoordSys_input; // flag indicating whether input coordinate system is left- or right-handed
	bool m_bRHCoordSys_output; // flag indicating whether output coordinate system should be left- or right-handed

	bool m_bNeedsUpdate;
};

