#include "DataVolume.h"
#include "Renderer.h"

#include "shared/glm/gtx/transform.hpp"

#include <iostream>
#include <math.h>

DataVolume::DataVolume(glm::vec3 pos, glm::quat orientation, glm::vec3 dimensions)
	: m_vec3Position(pos)
	, m_qOrientation(orientation)
	, m_vec3Dimensions(dimensions)
	, m_vec3OriginalPosition(pos)
	, m_qOriginalOrientation(orientation)
	, m_vec3OriginalDimensions(dimensions)
	, m_bFirstRun(true)
	, m_bUseCustomBounds(false)
{
	updateTransforms();
}

DataVolume::~DataVolume()
{
}

void DataVolume::add(Dataset * data)
{
	m_vpDatasets.push_back(data);
	m_bDirty = true;
	updateTransforms();
}

std::vector<Dataset*> DataVolume::getDatasets()
{
	return m_vpDatasets;
}

glm::vec3 DataVolume::getOriginalPosition()
{
	return m_vec3OriginalPosition;
}

glm::quat DataVolume::getOriginalOrientation()
{
	return m_qOriginalOrientation;
}

void DataVolume::resetPositionAndOrientation()
{
	setPosition(m_vec3OriginalPosition);
	setOrientation(m_qOriginalOrientation);
	setDimensions(m_vec3OriginalDimensions);
}

glm::dvec3 DataVolume::convertToRawDomainCoords(glm::vec3 worldPos)
{
	if (m_vpDatasets.size() == 0u)
		return glm::dvec3();
	else
		return glm::dvec3(glm::inverse(m_dmat4RawDomainToVolumeTransform) * glm::dvec4(worldPos, 1.));
}

glm::vec3 DataVolume::convertToAdjustedDomainCoords(glm::vec3 worldPos)
{
	if (m_vpDatasets.size() == 0u)
		return glm::vec3();
	else
		return glm::vec3(glm::inverse(m_mat4AdjustedDomainToVolumeTransform) * glm::vec4(worldPos, 1.f));
}

glm::vec3 DataVolume::convertToDataCoords(Dataset* dataset, glm::vec3 worldPos)
{
	if (m_vpDatasets.size() == 0u)
		return glm::vec3();
	else
		return glm::vec3(glm::inverse(getCurrentDataTransform(dataset)) * glm::vec4(worldPos, 1.f));
}

glm::vec3 DataVolume::convertToWorldCoords(Dataset* dataset, glm::vec3 dataPos)
{
	if (m_vpDatasets.size() == 0u)
		return glm::vec3();
	else
		return glm::vec3(getCurrentDataTransform(dataset) * glm::vec4(dataPos, 1.f));
}

bool DataVolume::isWorldCoordPointInBounds(glm::vec3 worldPt, bool checkZ)
{
	if (m_vpDatasets.size() == 0u)
		return false;
	
	glm::dvec3 ptXform(glm::inverse(m_dmat4RawDomainToVolumeTransform) * glm::dvec4(worldPt, 1.));

	if (ptXform.x >= getMinXDataBound() && ptXform.x <= getMaxXDataBound() &&
		ptXform.y >= getMinYDataBound() && ptXform.y <= getMaxYDataBound())
	{
		if (checkZ)
		{
			if (ptXform.z >= getMinZDataBound() && ptXform.z <= getMaxZDataBound())
				return true;
		}
		else
		{
			return true;
		}
	}

	return false;
}

void DataVolume::drawBBox(glm::vec4 color, float padPct)
{
	glm::mat4 transform = glm::translate(glm::mat4(), getPosition()) * glm::mat4(getOrientation()) * glm::scale(getDimensions() * (1.f + 0.01f * padPct));

	Renderer::getInstance().drawFlatPrimitive("bbox_lines", transform, color);
}

void DataVolume::drawEllipsoidBacking(glm::vec4 color, float padPct)
{
	glm::mat4 volTransform = glm::translate(glm::mat4(), getPosition()) * glm::mat4(getOrientation()) * glm::scale(getDimensions() * (1.f + 0.01f * padPct));

	Renderer::getInstance().drawPrimitive("inverse_icosphere", volTransform, color, color, 0.f);
}

void DataVolume::drawVolumeBacking(glm::mat4 worldToHMDTransform, glm::vec4 color, float padPct)
{
	glm::vec3 bbMin(-0.5f);
	glm::vec3 bbMax(0.5f);

	glm::vec3 viewPos = glm::vec3(worldToHMDTransform[3]);
	glm::vec3 viewDir = -glm::normalize(glm::vec3(worldToHMDTransform[2]));

	glm::mat4 volTransform = glm::translate(glm::mat4(), getPosition()) * glm::mat4(getOrientation()) * glm::scale(getDimensions() * (1.f + 0.01f * padPct));

	std::vector<glm::vec4> vv4cubeSideCtrs;
	
	vv4cubeSideCtrs.push_back(volTransform * glm::vec4(bbMin.x, bbMax.y - (bbMax.y - bbMin.y) / 2.f, bbMax.z - (bbMax.z - bbMin.z) / 2.f, 1.f)); // left
	vv4cubeSideCtrs.push_back(volTransform * glm::vec4(bbMax.x, bbMax.y - (bbMax.y - bbMin.y) / 2.f, bbMax.z - (bbMax.z - bbMin.z) / 2.f, 1.f)); // right
	
	vv4cubeSideCtrs.push_back(volTransform * glm::vec4(bbMax.x - (bbMax.x - bbMin.x) / 2.f, bbMin.y, bbMax.z - (bbMax.z - bbMin.z) / 2.f, 1.f)); // bottom
	vv4cubeSideCtrs.push_back(volTransform * glm::vec4(bbMax.x - (bbMax.x - bbMin.x) / 2.f, bbMax.y, bbMax.z - (bbMax.z - bbMin.z) / 2.f, 1.f)); // top
	
	vv4cubeSideCtrs.push_back(volTransform * glm::vec4(bbMax.x - (bbMax.x - bbMin.x) / 2.f, bbMax.y - (bbMax.y - bbMin.y) / 2.f, bbMax.z, 1.f)); // front
	vv4cubeSideCtrs.push_back(volTransform * glm::vec4(bbMax.x - (bbMax.x - bbMin.x) / 2.f, bbMax.y - (bbMax.y - bbMin.y) / 2.f, bbMin.z, 1.f)); // back
	
	glm::vec4 volumeCtr = volTransform[3];
	
	for (auto const &midPt : vv4cubeSideCtrs)
	{
		glm::vec3 v3PlaneNorm(glm::normalize(volumeCtr - midPt));
	
		glm::vec3 v3PlaneToHMD(glm::normalize(viewPos - glm::vec3(midPt)));
	
		float dpPlaneHMD = glm::dot(v3PlaneToHMD, v3PlaneNorm);
		float dpPlaneView = glm::dot(viewDir, v3PlaneNorm);
	
		if (dpPlaneHMD < 0.f)
			continue;
	
		float angleCutoff = 0.f;
		float cosCutoff = glm::cos(glm::radians(angleCutoff));
		float angleFade = 70.f; // degrees viewing angle to plane normal
		float cosFade = glm::cos(glm::radians(angleFade));
	
		if (dpPlaneView < cosCutoff)
		{
			// calculate transparency fade
			float range = cosFade - cosCutoff;
			//color.a *= (dpPlaneView - cosCutoff) / range;
			//color.a = 0.5f;
	
			// now position the planes
			float eps = 0.001f;
			glm::mat4 planeTransform;
			planeTransform[2] = glm::normalize(volumeCtr - midPt); // z
			planeTransform[3] = midPt; // pos
	
			if (abs(glm::dot(v3PlaneNorm, glm::normalize(glm::vec3(volTransform[0]))) - (-1.f)) < eps)
			{ // right
				planeTransform[0] = volTransform[2];
				planeTransform[1] = volTransform[1];
				//color = glm::vec4(glm::vec3(1.f, 0.f, 0.f), color.a);
			}
			else if (abs(glm::dot(v3PlaneNorm, glm::normalize(glm::vec3(volTransform[0]))) - 1.f) < eps)
			{ // left
				planeTransform[0] = -volTransform[2];
				planeTransform[1] = volTransform[1];
				//color = glm::vec4(glm::vec3(0.f, 1.f, 0.f), color.a);
			}
			else if (abs(glm::dot(v3PlaneNorm, glm::normalize(glm::vec3(volTransform[1]))) - (-1.f)) < eps)
			{ // top
				planeTransform[0] = volTransform[0];
				planeTransform[1] = volTransform[2];
				//color = glm::vec4(glm::vec3(0.f, 0.f, 1.f), color.a);
			}
			else if (abs(glm::dot(v3PlaneNorm, glm::normalize(glm::vec3(volTransform[1]))) - 1.f) < eps)
			{ // bottom
				planeTransform[0] = volTransform[0];
				planeTransform[1] = -volTransform[2];
				//color = glm::vec4(glm::vec3(1.f, 1.f, 0.f), color.a);
			}
			else if (abs(glm::dot(v3PlaneNorm, glm::normalize(glm::vec3(volTransform[2]))) - (-1.f)) < eps)
			{ // front
				planeTransform[0] = -volTransform[0];
				planeTransform[1] = volTransform[1];
				//color = glm::vec4(glm::vec3(0.f, 1.f, 1.f), color.a);
			}
			else if (abs(glm::dot(v3PlaneNorm, glm::normalize(glm::vec3(volTransform[2]))) - 1.f) < eps)
			{ // back
				planeTransform[0] = volTransform[0];
				planeTransform[1] = volTransform[1];
				//color = glm::vec4(glm::vec3(1.f, 0.f, 1.f), color.a);
			}
	
			Renderer::getInstance().drawFlatPrimitive("plane", planeTransform, color);
		}
	}
}

glm::mat4 DataVolume::getCurrentDataTransform(Dataset* dataset)
{
	return m_bUseCustomBounds ? m_mapCustomDataTransforms[dataset] : m_mapDataTransforms[dataset];
}

glm::mat4 DataVolume::getLastDataTransform(Dataset* dataset)
{
	return m_bUseCustomBounds ? m_mapCustomDataTransformsPrevious[dataset] : m_mapDataTransformsPrevious[dataset];
}

glm::mat4 DataVolume::getCurrentVolumeTransform()
{
	return m_mat4VolumeTransform;
}

glm::mat4 DataVolume::getLastVolumeTransform()
{
	return m_mat4VolumeTransformPrevious;
}

void DataVolume::setPosition(glm::vec3 newPos)
{
	m_vec3Position = newPos;
	m_bDirty = true;
}

glm::vec3 DataVolume::getPosition()
{
	return m_vec3Position;
}

void DataVolume::setOrientation(glm::quat newOrientation)
{
	m_qOrientation = newOrientation;
	m_bDirty = true;
}

glm::quat DataVolume::getOrientation()
{
	return m_qOrientation;
}

void DataVolume::setDimensions(glm::vec3 newScale)
{
	m_vec3Dimensions = newScale;
	m_bDirty = true;
}

glm::vec3 DataVolume::getDimensions()
{
	return m_vec3Dimensions;
}

glm::dvec3 DataVolume::getMinDataBound()
{
	if (m_vpDatasets.size() == 0u)
		return glm::dvec3(std::numeric_limits<double>::max());
	else
		return m_dvec3DomainMinBound;
}

double DataVolume::getMinXDataBound()
{
	auto minXFn = [](Dataset* &lhs, Dataset* &rhs) { return lhs->getRawXMin() < rhs->getRawXMin(); };
	auto minXDS = std::min_element(m_vpDatasets.begin(), m_vpDatasets.end(), minXFn);
	if (minXDS == m_vpDatasets.end())
		return std::numeric_limits<double>::max();
	else
		return (*minXDS)->getRawXMin();
}

double DataVolume::getMinYDataBound()
{
	auto minYFn = [](Dataset* &lhs, Dataset* &rhs) { return lhs->getRawYMin() < rhs->getRawYMin(); };
	auto minYDS = std::min_element(m_vpDatasets.begin(), m_vpDatasets.end(), minYFn);
	if (minYDS == m_vpDatasets.end())
		return std::numeric_limits<double>::max();
	else
		return (*minYDS)->getRawYMin();
}

double DataVolume::getMinZDataBound()
{
	auto minZFn = [](Dataset* &lhs, Dataset* &rhs) { return lhs->getRawZMin() < rhs->getRawZMin(); };
	auto minZDS = std::min_element(m_vpDatasets.begin(), m_vpDatasets.end(), minZFn);
	if (minZDS == m_vpDatasets.end())
		return std::numeric_limits<double>::max();
	else
		return (*minZDS)->getRawZMin();
}

glm::dvec3 DataVolume::getMaxDataBound()
{
	if (m_vpDatasets.size() == 0u)
		return glm::dvec3(-std::numeric_limits<double>::max());
	else
		return m_dvec3DomainMaxBound;
}

double DataVolume::getMaxXDataBound()
{
	auto maxXFn = [](Dataset* &lhs, Dataset* &rhs) { return lhs->getRawXMax() < rhs->getRawXMax(); };
	auto maxXDS = std::max_element(m_vpDatasets.begin(), m_vpDatasets.end(), maxXFn);
	if (maxXDS == m_vpDatasets.end())
		return -std::numeric_limits<double>::max();
	else
		return (*maxXDS)->getRawXMax();
}

double DataVolume::getMaxYDataBound()
{
	auto maxYFn = [](Dataset* &lhs, Dataset* &rhs) { return lhs->getRawYMax() < rhs->getRawYMax(); };
	auto maxYDS = std::max_element(m_vpDatasets.begin(), m_vpDatasets.end(), maxYFn);
	if (maxYDS == m_vpDatasets.end())
		return -std::numeric_limits<double>::max();
	else
		return (*maxYDS)->getRawYMax();
}

double DataVolume::getMaxZDataBound()
{
	auto maxZFn = [](Dataset* &lhs, Dataset* &rhs) { return lhs->getRawZMax() < rhs->getRawZMax(); };
	auto maxZDS = std::max_element(m_vpDatasets.begin(), m_vpDatasets.end(), maxZFn);
	if (maxZDS == m_vpDatasets.end())
		return -std::numeric_limits<double>::max();
	else
		return (*maxZDS)->getRawZMax();
}

glm::dvec3 DataVolume::getDataDimensions()
{
	if (m_vpDatasets.size() == 0u)
		return glm::dvec3(0.);
	else
		return m_dvec3DomainDims;
}

void DataVolume::setCustomBounds(glm::dvec3 minBound, glm::dvec3 maxBound)
{
	m_dvec3CustomDomainMinBound = minBound;
	m_dvec3CustomDomainMaxBound = maxBound;
	m_dvec3CustomDomainDims = maxBound - minBound;
	m_bDirty = true;
}

glm::dvec3 DataVolume::getCustomMinBound()
{
	return m_dvec3CustomDomainMinBound;
}

glm::dvec3 DataVolume::getCustomMaxBound()
{
	return m_dvec3CustomDomainMaxBound;
}

void DataVolume::useCustomBounds(bool yesno)
{
	m_bUseCustomBounds = yesno;
}

bool DataVolume::getUseCustomBounds()
{
	return m_bUseCustomBounds;
}

void DataVolume::drawAxes(float size)
{
	Renderer::getInstance().drawPrimitive("cylinder", glm::scale(glm::rotate(m_mat4VolumeTransform, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)), glm::vec3(0.1f, 0.1f, 1.f) * size), glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec4(1.f, 0.f, 0.f, 1.f), 1.f);
	Renderer::getInstance().drawPrimitive("cylinder", glm::scale(glm::rotate(m_mat4VolumeTransform, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)), glm::vec3(0.1f, 0.1f, 1.f) * size), glm::vec4(0.f, 1.f, 0.f, 1.f), glm::vec4(0.f, 1.f, 0.f, 1.f), 1.f);
	Renderer::getInstance().drawPrimitive("cylinder", glm::scale(m_mat4VolumeTransform, glm::vec3(0.1f, 0.1f, 1.f) * size), glm::vec4(0.f, 0.f, 1.f, 1.f), glm::vec4(0.f, 0.f, 1.f, 1.f), 1.f);
}

void DataVolume::update()
{
	updateTransforms();
}

glm::vec3 getAspectAdjustedDimensions(glm::vec3 fromDims, glm::vec3 toDims)
{
	float fromAR = fromDims.x / fromDims.y; // for figuring out how to maintain correct scale in the data volume
	float toAR = toDims.x / toDims.y;

	glm::vec3 adjustedDims;

	if (toAR > fromAR)
	{
		adjustedDims.x = fromDims.x * (toDims.y / fromDims.y);
		adjustedDims.y = toDims.y;
	}
	else
	{
		adjustedDims.x = toDims.x;
		adjustedDims.y = fromDims.y * (toDims.x / fromDims.x);
	}

	adjustedDims.z = toDims.z;

	return adjustedDims;
}

void DataVolume::updateTransforms()
{
	if (m_bDirty)
	{
		m_mat4VolumeTransformPrevious = m_mat4VolumeTransform;

		// M = T * R * S
		m_mat4VolumeTransform = glm::translate(glm::mat4(), m_vec3Position) * glm::mat4(m_qOrientation) * glm::scale(glm::mat4(), m_vec3Dimensions);

		if (m_vpDatasets.size() == 0u)
			return;

		m_dvec3DomainMinBound = glm::dvec3(getMinXDataBound(), getMinYDataBound(), getMinZDataBound());
		m_dvec3DomainMaxBound = glm::dvec3(getMaxXDataBound(), getMaxYDataBound(), getMaxZDataBound());
		m_dvec3DomainDims = glm::dvec3(m_dvec3DomainMaxBound - m_dvec3DomainMinBound);

		glm::vec3 domainAdjustedVolumeDims = getAspectAdjustedDimensions(m_dvec3DomainDims, m_vec3Dimensions);
		glm::dvec3 combinedDataCenter = m_dvec3DomainMinBound + m_dvec3DomainDims * 0.5;
		m_mapDataTransformsPrevious = m_mapDataTransforms;
		glm::vec3 scalingFactors = domainAdjustedVolumeDims / glm::vec3(m_dvec3DomainDims);
		// Apply data volume transform and aspect-corrected scaling so data fits inside volume
		m_mat4AdjustedDomainToVolumeTransformPrevious = m_mat4AdjustedDomainToVolumeTransform;
		m_mat4AdjustedDomainToVolumeTransform = glm::translate(glm::mat4(), m_vec3Position) * glm::mat4(m_qOrientation) * glm::scale(scalingFactors);


		glm::vec3 domainCustomAdjustedVolumeDims = getAspectAdjustedDimensions(m_dvec3CustomDomainDims, m_vec3Dimensions);
		glm::dvec3 combinedCustomDataCenter = m_dvec3CustomDomainMinBound + m_dvec3CustomDomainDims * 0.5;
		m_mapCustomDataTransformsPrevious = m_mapCustomDataTransforms;
		glm::vec3 customScalingFactors = domainCustomAdjustedVolumeDims / glm::vec3(m_dvec3CustomDomainDims);
		glm::mat4 customDomainToVolumeTransform = glm::translate(glm::mat4(), m_vec3Position) * glm::mat4(m_qOrientation) * glm::scale(customScalingFactors);


		// calculate the transform from the entire raw domain to the world space
		m_dmat4RawDomainToVolumeTransformPrevious = m_dmat4RawDomainToVolumeTransform;
		m_dmat4RawDomainToVolumeTransform = glm::translate(glm::dmat4(), glm::dvec3(m_vec3Position)) * glm::dmat4(glm::mat4(m_qOrientation)) * glm::scale(glm::dvec3(scalingFactors)) * glm::translate(glm::dmat4(), -combinedDataCenter);
		

		for (auto &dataset : m_vpDatasets)
		{
			glm::dvec3 dataCenterRaw = dataset->getRawMinBounds() + dataset->getRawDimensions() * 0.5;

			glm::vec3 dataPositionOffsetInVolume = dataCenterRaw - combinedDataCenter;
			glm::vec3 dataPositionOffsetInCustomVolume = dataCenterRaw - combinedCustomDataCenter;

			glm::vec3 dataCenteringOffset = -(dataset->getAdjustedMinBounds() + dataset->getAdjustedDimensions() * 0.5f);

			glm::mat4 handednessConversion;
			if (dataset->isDataRightHanded() != dataset->isOutputRightHanded())
				handednessConversion[2][2] = -1.f;

			glm::mat4 trans = m_mat4AdjustedDomainToVolumeTransform;
			trans *= handednessConversion; // 3. Inverts the z-coordinate to change handedness, if needed
			trans *= glm::translate(glm::mat4(), dataPositionOffsetInVolume); // 2. Positions center of dataset within data volume
			trans *= glm::translate(glm::mat4(), dataCenteringOffset); // 1. Move origin to center of dataset

			m_mapDataTransforms[dataset] = trans;

			// custom domain calcs
			glm::mat4 customTrans = customDomainToVolumeTransform;
			customTrans *= handednessConversion;
			customTrans *= glm::translate(glm::mat4(), dataPositionOffsetInCustomVolume);
			customTrans *= glm::translate(glm::mat4(), dataCenteringOffset);

			m_mapCustomDataTransforms[dataset] = customTrans;
		}

		if (m_bFirstRun)
		{
			m_mapDataTransformsPrevious = m_mapDataTransforms;
			m_mapCustomDataTransformsPrevious = m_mapCustomDataTransforms;
			m_mat4VolumeTransformPrevious = m_mat4VolumeTransform;
			m_dmat4RawDomainToVolumeTransformPrevious = m_dmat4RawDomainToVolumeTransform;
			m_mat4AdjustedDomainToVolumeTransformPrevious = m_mat4AdjustedDomainToVolumeTransform;
			m_bFirstRun = false;
		}

		m_bDirty = false;
	}
}
