#pragma once

#include <GL/glew.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <map>
#include "Dataset.h"
#include "../shared/glm/gtc/quaternion.hpp"
#include "../shared/glm/gtx/quaternion.hpp"
#include "../shared//glm/gtc/type_ptr.hpp"
//#include <string>
//#include <cstdlib>

class DataVolume
{
public:
	DataVolume(glm::vec3 pos, glm::quat orientation, glm::vec3 dimensions);
	virtual ~DataVolume();

	void add(Dataset* data);

	std::vector<Dataset*> getDatasets();

	void drawBBox(glm::vec4 color, float padPct);
	void drawVolumeBacking(glm::mat4 worldToHMDTransform, glm::vec4 color, float padPct);
	void drawEllipsoidBacking(glm::vec4 color, float padPct);

	glm::vec3 getOriginalPosition();
	glm::quat getOriginalOrientation();
	
	glm::vec3 convertToDataCoords(Dataset* dataset, glm::vec3 worldPos);
	glm::vec3 convertToWorldCoords(Dataset* dataset, glm::vec3 dataPos);
	bool isWorldCoordPointInBounds(glm::vec3 worldPt, bool checkZ = true);
	
	glm::mat4 getCurrentDataTransform(Dataset* dataset);
	glm::mat4 getLastDataTransform(Dataset* dataset);
	glm::mat4 getCurrentVolumeTransform();
	glm::mat4 getLastVolumeTransform();

	void resetPositionAndOrientation();

	void setPosition(glm::vec3 newPos);
	glm::vec3 getPosition();
	void setOrientation(glm::quat newOrientation);
	glm::quat getOrientation();
	void setDimensions(glm::vec3 newScale);
	glm::vec3 getDimensions();

	glm::dvec3 getMinDataBound();
	glm::dvec3 getMaxDataBound();
	glm::dvec3 getDataDimensions();

	void drawAxes(float size = 1.f);

	void update();

protected:
	void updateTransforms();
	
	double getMinXDataBound();
	double getMinYDataBound();
	double getMinZDataBound();
	double getMaxXDataBound();
	double getMaxYDataBound();
	double getMaxZDataBound();

	std::vector<Dataset*> m_vpDatasets;
	std::map<Dataset*, glm::mat4> m_mapDataTransforms;
	std::map<Dataset*, glm::mat4> m_mapDataTransformsPrevious;

	glm::vec3 m_vec3OriginalPosition;        // Original Data Volume Position	
	glm::vec3 m_vec3Position;
	glm::quat m_qOriginalOrientation;        // Original Data Volume Orientation
	glm::quat m_qOrientation;
	glm::vec3 m_vec3OriginalDimensions;           // Original Data Volume Orientation
	glm::vec3 m_vec3Dimensions;

	glm::mat4 m_mat4VolumeTransform;         // Volume Position and Orientation Transform
	glm::mat4 m_mat4VolumeTransformPrevious; // Previous Volume Position and Orientation Transform

	glm::dvec3 m_dvec3DomainMinBound;
	glm::dvec3 m_dvec3DomainMaxBound;
	glm::dvec3 m_dvec3DomainDims;
	
	bool m_bFirstRun;                        // Flag for First Runthrough
	bool m_bDirty;                           // a user flag to tell whether or not the transform has changed
};