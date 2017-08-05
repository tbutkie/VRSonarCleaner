#include "DataVolume.h"
#include "DebugDrawer.h"
#include "Renderer.h"

#include "shared/glm/gtx/transform.hpp"

#include <iostream>
#include <math.h>

DataVolume::DataVolume(glm::vec3 pos, int startingOrientation, glm::vec3 dimensions, glm::vec3 innerCoordsMin, glm::vec3 innerCoordsMax)
	: Node(pos)
	, m_vec3OriginalPosition(pos)
	, m_vec3Dimensions(dimensions)
	, m_bFirstRun(true)
{
	if (startingOrientation == 0)
		setOrientation(glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)));
	else
		setOrientation(glm::angleAxis(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)));

	m_qOriginalOrientation = getOrientation();			
	
	setInnerCoords(innerCoordsMin, innerCoordsMax);

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
}

void DataVolume::setDimensions(glm::vec3 dimensions)
{
	m_vec3Dimensions = dimensions;

	recalcScaling();
}

glm::vec3 DataVolume::getDimensions()
{
	return m_vec3Dimensions;
}

void DataVolume::setInnerCoords(glm::vec3 minCoords, glm::vec3  maxCoords)
{
	m_vec3InnerMin = minCoords;
	m_vec3InnerMax = maxCoords;
	m_vec3InnerRange = m_vec3InnerMax - m_vec3InnerMin;

	m_vec3DataCenteringOffset = (-m_vec3InnerRange * 0.5f) - m_vec3InnerMin;

	recalcScaling();
}

void DataVolume::recalcScaling()
{
	float XYscale = std::min(m_vec3Dimensions.x / m_vec3InnerRange.x, m_vec3Dimensions.y / m_vec3InnerRange.y);
	float depthScale = m_vec3Dimensions.z / m_vec3InnerRange.z;

	setScale(glm::vec3(XYscale, XYscale, depthScale));

	updateTransforms();
}

glm::vec3 DataVolume::convertToInnerCoords(glm::vec3 worldPos)
{
	return glm::vec3(glm::inverse(getCurrentDataTransform()) * glm::vec4(worldPos, 1.f));
}

glm::vec3 DataVolume::convertToWorldCoords(glm::vec3 innerPos)
{
	updateTransforms();

	return glm::vec3(getCurrentDataTransform() * glm::vec4(innerPos, 1.f));
}

void DataVolume::drawBBox(glm::vec4 color, float padPct)
{	
	glm::vec3 bbMin(-0.5f);
	glm::vec3 bbMax(0.5f);

	DebugDrawer::getInstance().setTransform(
		glm::translate(glm::mat4(), getPosition()) * glm::mat4(getOrientation()) * glm::scale(m_vec3Dimensions * (1.f + 0.01f * padPct))
	);

	DebugDrawer::getInstance().drawBox(bbMin, bbMax, color);
}

void DataVolume::drawAdaptiveBacking(glm::mat4 worldToHMDTransform, glm::vec4 color, float padPct)
{
	glm::vec3 bbMin(-0.5f);
	glm::vec3 bbMax(0.5f);

	glm::vec3 viewPos = glm::vec3(worldToHMDTransform[3]);
	glm::vec3 viewDir = -glm::normalize(glm::vec3(worldToHMDTransform[2]));

	glm::mat4 volTransform = glm::translate(glm::mat4(), getPosition()) * glm::mat4(getOrientation()) * glm::scale(m_vec3Dimensions * (1.f + 0.01f * padPct));
	
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
	
		float angleCutoff = 80.f;
		float cosCutoff = cos(glm::radians(angleCutoff));
		float angleFade = 70.f; // degrees viewing angle to plane normal
		float cosFade = cos(glm::radians(angleFade));

		//if (glm::dot(norm, viewDir) < 0.f)
		//	continue;
		
		if (dpPlaneHMD > cosCutoff)
		{
			// calculate transparency fade
			float range = cosFade - cosCutoff;
			color.a *= (dpPlaneHMD - cosCutoff) / range;

			// now position the planes
			float eps = 0.001f;
			glm::mat4 planeTransform;
			planeTransform[2] = glm::normalize(volumeCtr - midPt); // z
			planeTransform[3] = midPt; // pos

			if (abs(glm::dot(v3PlaneNorm, glm::normalize(glm::vec3(volTransform[0]))) - (-1.f)) < eps)
			{ // right
				planeTransform[0] = volTransform[2];
				planeTransform[1] = volTransform[1];
				color = glm::vec4(glm::vec3(1.f, 0.f, 0.f), color.a);
			}
			else if (abs(glm::dot(v3PlaneNorm, glm::normalize(glm::vec3(volTransform[0]))) - 1.f) < eps)
			{ // left
				planeTransform[0] = -volTransform[2];
				planeTransform[1] = volTransform[1];
				color = glm::vec4(glm::vec3(0.f, 1.f, 0.f), color.a);
			}
			else if (abs(glm::dot(v3PlaneNorm, glm::normalize(glm::vec3(volTransform[1]))) - (-1.f)) < eps)
			{ // top
				planeTransform[0] = volTransform[0];
				planeTransform[1] = volTransform[2];
				color = glm::vec4(glm::vec3(0.f, 0.f, 1.f), color.a);
			}
			else if (abs(glm::dot(v3PlaneNorm, glm::normalize(glm::vec3(volTransform[1]))) - 1.f) < eps)
			{ // bottom
				planeTransform[0] = volTransform[0];
				planeTransform[1] = -volTransform[2];
				color = glm::vec4(glm::vec3(1.f, 1.f, 0.f), color.a);
			}
			else if (abs(glm::dot(v3PlaneNorm, glm::normalize(glm::vec3(volTransform[2]))) - (-1.f)) < eps)
			{ // front
				planeTransform[0] = -volTransform[0];
				planeTransform[1] = volTransform[1];
				color = glm::vec4(glm::vec3(0.f, 1.f, 1.f), color.a);
			}
			else if (abs(glm::dot(v3PlaneNorm, glm::normalize(glm::vec3(volTransform[2]))) - 1.f) < eps)
			{ // back
				planeTransform[0] = volTransform[0];
				planeTransform[1] = volTransform[1];
				color = glm::vec4(glm::vec3(1.f, 0.f, 1.f), color.a);
			}

			Renderer::getInstance().drawPrimitive("plane", planeTransform, color, color, 10.f);
		}
	}
}

glm::mat4 DataVolume::getCurrentDataTransform()
{
	updateTransforms();

	return m_mat4DataTransform;
}

glm::mat4 DataVolume::getLastDataTransform()
{
	updateTransforms();

	return m_mat4DataTransformPrevious;
}

glm::mat4 DataVolume::getCurrentVolumeTransform()
{
	updateTransforms();

	return m_mat4VolumeTransform;
}

glm::mat4 DataVolume::getLastVolumeTransform()
{
	updateTransforms();

	return m_mat4VolumeTransformPrevious;
}

void DataVolume::updateTransforms()
{
	if (isDirty())
	{
		m_mat4DataTransformPrevious = m_mat4DataTransform;
		m_mat4VolumeTransformPrevious = m_mat4VolumeTransform;

		m_mat4DataTransform = getModelToWorldTransform() * glm::translate(glm::mat4(), m_vec3DataCenteringOffset);
		m_mat4VolumeTransform = getModelToWorldTransform() * glm::scale(1.f / getScale()); // undo the scaling

		if (m_bFirstRun)
		{
			m_mat4DataTransformPrevious = m_mat4DataTransform;
			m_mat4VolumeTransformPrevious = m_mat4VolumeTransform;
			m_bFirstRun = false;
		}
	}
}
