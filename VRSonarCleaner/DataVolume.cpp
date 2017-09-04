#include "DataVolume.h"
#include "Renderer.h"

#include "shared/glm/gtx/transform.hpp"

#include <iostream>
#include <math.h>

DataVolume::DataVolume(Dataset* data, glm::vec3 pos, int startingOrientation, glm::vec3 dimensions)
	: m_vec3Position(pos)
	, m_vec3Dimensions(dimensions)
	, m_pDataset(data)
	, m_vec3OriginalPosition(pos)
	, m_vec3OriginalDimensions(dimensions)
	, m_bFirstRun(true)
{
	if (startingOrientation == 0)
		setOrientation(glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)));
	else
		setOrientation(glm::angleAxis(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)));

	m_qOriginalOrientation = getOrientation();

	updateTransforms();
}

DataVolume::~DataVolume()
{
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

glm::vec3 DataVolume::convertToDataCoords(glm::vec3 worldPos)
{
	return glm::vec3(glm::inverse(getCurrentDataTransform()) * glm::vec4(worldPos, 1.f));
}

glm::vec3 DataVolume::convertToWorldCoords(glm::vec3 dataPos)
{
	return glm::vec3(getCurrentDataTransform() * glm::vec4(dataPos, 1.f));
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

glm::mat4 DataVolume::getCurrentDataTransform()
{
	return m_mat4DataTransform;
}

glm::mat4 DataVolume::getLastDataTransform()
{
	return m_mat4DataTransformPrevious;
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
		m_mat4DataTransformPrevious = m_mat4DataTransform;
		m_mat4VolumeTransformPrevious = m_mat4VolumeTransform;

		glm::mat4 handednessConversion;
		if (m_pDataset->isDataRightHanded() != m_pDataset->isOutputRightHanded())
			handednessConversion[2][2] = -1.f;

		glm::vec3 dataCenteringOffset = -(m_pDataset->getAdjustedMinBounds() + m_pDataset->getAdjustedDimensions() * 0.5f);

		float XYscale = std::min(1.f / m_pDataset->getAdjustedXDimension(), 1.f / m_pDataset->getAdjustedYDimension());
		float depthScale = 1.f / m_pDataset->getAdjustedZDimension();

		m_vec3ScalingFactors = glm::vec3(XYscale, XYscale, depthScale);

		m_mat4VolumeTransform = glm::translate(glm::mat4(), m_vec3Position) * glm::mat4(m_qOrientation) * glm::scale(glm::mat4(), m_vec3Dimensions);
		m_mat4DataTransform = m_mat4VolumeTransform * glm::scale(m_vec3ScalingFactors) * handednessConversion * glm::translate(glm::mat4(), dataCenteringOffset);

		if (m_bFirstRun)
		{
			m_mat4DataTransformPrevious = m_mat4DataTransform;
			m_mat4VolumeTransformPrevious = m_mat4VolumeTransform;
			m_bFirstRun = false;
		}

		m_bDirty = false;
	}
}
