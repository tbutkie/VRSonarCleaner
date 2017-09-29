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

	glm::dvec3 convertToRawDomainCoords(glm::vec3 worldPos);
	glm::vec3 convertToAdjustedDomainCoords(glm::vec3 worldPos);
	glm::vec3 convertToDataCoords(Dataset* dataset, glm::vec3 worldPos);
	glm::vec3 convertToWorldCoords(glm::vec3 dataPos);
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

	void setCustomBounds(glm::dvec3 minBound, glm::dvec3 maxBound);
	glm::dvec3 getCustomMinBound();
	glm::dvec3 getCustomMaxBound();
	glm::dvec3 getCustomDomainDimensions();

	static glm::vec3 calcAspectAdjustedDimensions(glm::vec3 fromDims, glm::vec3 toDims);

	void useCustomBounds(bool yesno);
	bool getUseCustomBounds();

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

	glm::vec3 m_vec3OriginalPosition;        // Original Data Volume Position	
	glm::vec3 m_vec3Position;
	glm::quat m_qOriginalOrientation;        // Original Data Volume Orientation
	glm::quat m_qOrientation;
	glm::vec3 m_vec3OriginalDimensions;           // Original Data Volume Orientation
	glm::vec3 m_vec3Dimensions;

	std::map<Dataset*, glm::mat4> m_mapDataTransforms;
	std::map<Dataset*, glm::mat4> m_mapDataTransformsPrevious;

	glm::mat4 m_mat4VolumeTransform;         // Volume Position and Orientation Transform
	glm::mat4 m_mat4VolumeTransformPrevious; // Previous Volume Position and Orientation Transform

	glm::dmat4 m_dmat4RawDomainToVolumeTransform;		 // The transform from the entire raw data domain to the data volume
	glm::dmat4 m_dmat4RawDomainToVolumeTransformPrevious; // Previous transform from the entire raw data domain to the data volume

	glm::mat4 m_mat4AdjustedDomainToVolumeTransform;		 // The transform from the entire adjusted data domain to the data volume
	glm::mat4 m_mat4AdjustedDomainToVolumeTransformPrevious; // Previous transform from the entire adjusted data domain to the data volume

	glm::dvec3 m_dvec3DomainMinBound;
	glm::dvec3 m_dvec3DomainMaxBound;
	glm::dvec3 m_dvec3DomainDims;

	glm::dvec3 m_dvec3CustomDomainMinBound;
	glm::dvec3 m_dvec3CustomDomainMaxBound;
	glm::dvec3 m_dvec3CustomDomainDims;
	std::map<Dataset*, glm::mat4> m_mapCustomDataTransforms;
	std::map<Dataset*, glm::mat4> m_mapCustomDataTransformsPrevious;
	
	bool m_bFirstRun;                        // Flag for First Runthrough
	bool m_bDirty;                           // a user flag to tell whether or not the transform has changed
	bool m_bUseCustomBounds;
};