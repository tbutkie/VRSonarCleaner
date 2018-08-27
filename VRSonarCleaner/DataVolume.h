#pragma once

#include <GL/glew.h>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <map>
#include "Dataset.h"
#include "Object3D.h"
#include <gtc/quaternion.hpp>
#include <gtx/quaternion.hpp>
#include <gtc/type_ptr.hpp>
//#include <string>
//#include <cstdlib>

class DataVolume : public Object3D
{
public:
	DataVolume(glm::vec3 pos, glm::quat orientation, glm::vec3 dimensions);
	virtual ~DataVolume();

	void add(Dataset* data);
	void remove(Dataset* data);

	std::vector<Dataset*> getDatasets();

	void setFrameColor(glm::vec4 color);
	glm::vec4 getFrameColor();
	void drawBBox(float padPct);
	void setBackingColor(glm::vec4 color);
	glm::vec4 getBackingColor();
	void drawVolumeBacking(glm::mat4 worldToHMDTransform, float padPct);
	void drawEllipsoidBacking(glm::vec4 color, float padPct);

	glm::dvec3 convertToRawDomainCoords(glm::vec3 worldPos);
	glm::vec3 convertToAdjustedDomainCoords(glm::vec3 worldPos);
	glm::vec3 convertToDataCoords(Dataset* dataset, glm::vec3 worldPos);
	glm::vec3 convertToWorldCoords(glm::dvec3 rawDataPos);
	bool isWorldCoordPointInDomainBounds(glm::vec3 worldPt, bool checkZ = true);
	
	glm::mat4 getTransformDataset(Dataset* dataset);
	glm::mat4 getTransformDataset_Last(Dataset* dataset);
	glm::dmat4 getTransformRawDomainToVolume();
	glm::dmat4 getTransformRawDomainToVolume_Last();
	glm::mat4 getTransformVolume();
	glm::mat4 getTransformVolume_Last();

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

	virtual void update();

	bool isVisible();
	void setVisible(bool yesno);

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

	glm::vec4 m_vec4BackingColor;
	glm::vec4 m_vec4FrameColor;
	
	bool m_bVisible;
	bool m_bFirstRun;                        // Flag for First Runthrough
	bool m_bUseCustomBounds;
	bool m_bUsePivot;
};