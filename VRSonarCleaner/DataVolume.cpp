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

glm::vec3 DataVolume::convertToDataCoords(Dataset* dataset, glm::vec3 worldPos)
{
	return glm::vec3(glm::inverse(getCurrentDataTransform(dataset)) * glm::vec4(worldPos, 1.f));
}

glm::vec3 DataVolume::convertToWorldCoords(Dataset* dataset, glm::vec3 dataPos)
{
	return glm::vec3(getCurrentDataTransform(dataset) * glm::vec4(dataPos, 1.f));
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
	return m_mapDataTransforms[dataset];
}

glm::mat4 DataVolume::getLastDataTransform(Dataset* dataset)
{
	return m_mapDataTransformsPrevious[dataset];
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

void DataVolume::updateTransforms()
{
	if (m_bDirty)
	{
		m_mat4VolumeTransformPrevious = m_mat4VolumeTransform;

		// M = T * R * S
		m_mat4VolumeTransform = glm::translate(glm::mat4(), m_vec3Position) * glm::mat4(m_qOrientation) * glm::scale(glm::mat4(), m_vec3Dimensions);

		if (m_vpDatasets.size() == 0u)
			return;

		auto minXFn = [](Dataset* &lhs, Dataset* &rhs) { return lhs->getRawXMin() < rhs->getRawXMin(); };
		auto minXCloud = *std::min_element(m_vpDatasets.begin(), m_vpDatasets.end(), minXFn);
		auto maxXCloud = *std::max_element(m_vpDatasets.begin(), m_vpDatasets.end(), minXFn);

		auto minYFn = [](Dataset* &lhs, Dataset* &rhs) { return lhs->getRawYMin() < rhs->getRawYMin(); };
		auto minYCloud = *std::min_element(m_vpDatasets.begin(), m_vpDatasets.end(), minYFn);
		auto maxYCloud = *std::max_element(m_vpDatasets.begin(), m_vpDatasets.end(), minYFn);

		auto minZFn = [](Dataset* &lhs, Dataset* &rhs) { return lhs->getRawZMin() < rhs->getRawZMin(); };
		auto minZCloud = *std::min_element(m_vpDatasets.begin(), m_vpDatasets.end(), minZFn);
		auto maxZCloud = *std::max_element(m_vpDatasets.begin(), m_vpDatasets.end(), minZFn);

		glm::dvec3 minBound(minXCloud->getRawXMin(), minYCloud->getRawYMin(), minZCloud->getRawZMin());
		glm::dvec3 maxBound(maxXCloud->getRawXMax(), maxYCloud->getRawYMax(), maxZCloud->getRawZMax());
		glm::dvec3 dims(maxBound - minBound);

		glm::dvec3 combinedDataCenter = minBound + dims * 0.5;

		m_mapDataTransformsPrevious = m_mapDataTransforms;

		for (auto &dataset : m_vpDatasets)
		{
			glm::dvec3 dataCenterRaw = dataset->getRawMinBounds() + dataset->getRawDimensions() * 0.5;

			glm::vec3 dataPositionOffsetInVolume = combinedDataCenter - dataCenterRaw;

			glm::vec3 dataCenteringOffset = -(dataset->getAdjustedMinBounds() + dataset->getAdjustedDimensions() * 0.5f);

			glm::mat4 handednessConversion;
			if (dataset->isDataRightHanded() != dataset->isOutputRightHanded())
				handednessConversion[2][2] = -1.f;

			// Scale x and y (lon and lat) while maintaining aspect ratio
			float XYscale = std::min(1.f / dataset->getAdjustedXDimension(), 1.f / dataset->getAdjustedYDimension());
			float depthScale = 1.f / dataset->getAdjustedZDimension();

			glm::vec3 scalingFactors = glm::vec3(XYscale, XYscale, depthScale);

			glm::mat4 dataTransform = m_mat4VolumeTransform; // 4. Now apply the data volume transform
			dataTransform *= glm::scale(scalingFactors); // 3. Scaling factors so data to fits in the data volume
			dataTransform *= handednessConversion; // 2. Inverts the z-coordinate to change handedness, if needed
			dataTransform *= glm::translate(glm::mat4(), dataCenteringOffset); // 1. Move origin to center of dataset

			m_mapDataTransforms[dataset] = dataTransform;
		}

		if (m_bFirstRun)
		{
			m_mapDataTransformsPrevious = m_mapDataTransforms;
			m_mat4VolumeTransformPrevious = m_mat4VolumeTransform;
			m_bFirstRun = false;
		}

		m_bDirty = false;
	}
}
