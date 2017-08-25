#pragma once

#include <GL/glew.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include "Node.h"
#include "Dataset.h"
#include "../shared/glm/gtc/quaternion.hpp"
#include "../shared/glm/gtx/quaternion.hpp"
#include "../shared//glm/gtc/type_ptr.hpp"
//#include <string>
//#include <cstdlib>

class DataVolume
	: public Node
{
public:
	DataVolume(Dataset* data, glm::vec3 pos, int startingOrientation, glm::vec3 dimensions);
	virtual ~DataVolume();

	void drawBBox(glm::vec4 color, float padPct);
	void drawVolumeBacking(glm::mat4 worldToHMDTransform, glm::vec4 color, float padPct);
	void drawEllipsoidBacking(glm::vec4 color, float padPct);

	glm::vec3 getOriginalPosition();
	glm::quat getOriginalOrientation();
	
	glm::vec3 convertToDataCoords(glm::vec3 worldPos);
	glm::vec3 convertToWorldCoords(glm::vec3 dataPos);
	
	glm::mat4 getCurrentDataTransform();
	glm::mat4 getLastDataTransform();
	glm::mat4 getCurrentVolumeTransform();
	glm::mat4 getLastVolumeTransform();

	void resetPositionAndOrientation();

protected:
	void updateTransforms();

	Dataset* m_pDataset;

	glm::vec3 m_vec3OriginalPosition; // Original Data Volume Position	
	glm::quat m_qOriginalOrientation; // Original Data Volume Orientation
	glm::vec3 m_vec3OriginalScale; // Original Data Volume Orientation

	glm::vec3 m_vec3ScalingFactors; // Volume Dimensions

	glm::mat4 m_mat4VolumeTransform; // Volume Position and Orientation Transform
	glm::mat4 m_mat4VolumeTransformPrevious; // Previous Volume Position and Orientation Transform
	glm::mat4 m_mat4DataTransform; // Data to World Transform
	glm::mat4 m_mat4DataTransformPrevious; // Previous Data to World Transform
	
	bool m_bFirstRun; // Flag for First Runthrough
};