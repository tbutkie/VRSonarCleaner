#pragma once

#include <GL/glew.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include "../shared/glm/gtc/quaternion.hpp"
#include "../shared/glm/gtx/quaternion.hpp"
#include "../shared//glm/gtc/type_ptr.hpp"
//#include <string>
//#include <cstdlib>

class DataVolume
{
public:
	DataVolume(glm::vec3 pos, int startingOrientation, glm::vec3 size, glm::vec3 innerCoordsMin, glm::vec3 innerCoordsMax);
	virtual ~DataVolume();

	void drawBBox();
	void drawBacking();
	void drawAxes();

	void setSize(glm::vec3 size);
	glm::vec3 getSize();
	void setPosition(glm::vec3 pos);
	void setOrientation(glm::mat4 orientation);
	void setInnerCoords(glm::vec3 minCoords, glm::vec3 maxCoords);
	
	void recalcScaling();

	glm::vec3 convertToInnerCoords(glm::vec3 worldPos);
	glm::vec3 convertToWorldCoords(glm::vec3 innerPos);
	
	glm::mat4 getCurrentDataTransform();
	glm::mat4 getLastDataTransform();
	glm::mat4 getCurrentVolumeTransform();
	glm::mat4 getLastVolumeTransform();

	void resetPositionAndOrientation();

private:
	void updateTransforms();

	glm::vec3 m_vec3Pos; // Data Volume Position
	glm::vec3 m_vec3OriginalPosition; // Original Data Volume Position
	
	glm::quat m_qOrientation; // Data Volume Orientation
	glm::quat m_qOriginalOrientation; // Original Data Volume Orientation

	glm::vec3 m_vec3Size; // Data Volume Dimensions
	glm::vec3 m_vec3InnerMin; // Minimum Data Dimensions
	glm::vec3 m_vec3InnerMax; // Maximum Data Dimensions
	glm::vec3 m_vec3InnerRange; // Range of Data Dimensions

	glm::vec3 m_vec3Scale; // Data -> Volume Scaling Factors 

	glm::vec3 m_vec3DataCenteringOffset; // Data Origin -> Volume Origin Offset

	glm::mat4 m_mat4VolumeTransform; // Volume Position and Orientation Transform
	glm::mat4 m_mat4VolumeTransformPrevious; // Previous Volume Position and Orientation Transform
	glm::mat4 m_mat4DataTransform; // Data to World Transform
	glm::mat4 m_mat4DataTransformPrevious; // Previous Data to World Transform
	
	bool m_bFirstRun; // Flag for First Runthrough
	bool m_bNeedsUpdate; // Transforms Update Flag
};