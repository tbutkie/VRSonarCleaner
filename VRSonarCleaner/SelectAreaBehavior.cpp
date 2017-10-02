#include "SelectAreaBehavior.h"

#include "Renderer.h"
#include "SonarPointCloud.h"
#include <limits>

SelectAreaBehavior::SelectAreaBehavior(ViveController* primaryController, ViveController* secondaryController, DataVolume* selectionVolume, DataVolume* displayVolume)
	: m_pPrimaryController(primaryController)
	, m_pSecondaryController(secondaryController)
	, m_pDataVolumeSelection(selectionVolume)
	, m_pDataVolumeDisplay(displayVolume)
	, m_bActive(false)
	, m_bShowCursor(false)
	, m_bSelectingArea(false)
	, m_bMovingArea(false)
	, m_bCustomAreaSet(false)
	, m_bRayHitPlane(false)
	, m_bRayHitDomain(false)
	, m_vec3CursorSize(glm::vec3(0.01f, 0.01f, 0.001f))
{
}


SelectAreaBehavior::~SelectAreaBehavior()
{
}

void SelectAreaBehavior::update()
{
	updateState();

	glm::vec3 hitPt = calcHits();

	if (m_bRayHitPlane)
		m_vec3CurrentLocationOnPlane = hitPt;

	if (m_bRayHitDomain)
	{		
		m_bShowCursor = true;
	}
	else
	{
		m_bShowCursor = false;
	}


	if (m_bActive)
	{
		if (m_bRayHitDomain)
		{
			if (!m_bSelectingArea && !m_bMovingArea)
			{
				if (m_bRayHitCustomDomain)
				{
					m_vec3BeginDragOnPlane = m_vec3CurrentLocationOnPlane;
					m_dvec3MinBoundAtDragStart = m_dvec3MinBound;
					m_dvec3MaxBoundAtDragStart = m_dvec3MaxBound;
					m_bMovingArea = true;
				}
				else
				{
					m_vec3BeginBoundOnPlane = m_vec3CurrentLocationOnPlane;
					m_bSelectingArea = true;
				}			
			}

			if (m_bSelectingArea)
			{
				m_vec3EndBoundOnPlane = m_vec3CurrentLocationOnPlane;
				
				glm::dvec3 domainCoordsStart = m_pDataVolumeSelection->convertToRawDomainCoords(m_vec3BeginBoundOnPlane);
				glm::dvec3 domainCoordsNow = m_pDataVolumeSelection->convertToRawDomainCoords(m_vec3EndBoundOnPlane);

				m_dvec3MinBound.x = std::min(domainCoordsStart.x, domainCoordsNow.x);
				m_dvec3MinBound.y = std::min(domainCoordsStart.y, domainCoordsNow.y);

				m_dvec3MaxBound.x = std::max(domainCoordsStart.x, domainCoordsNow.x);
				m_dvec3MaxBound.y = std::max(domainCoordsStart.y, domainCoordsNow.y);

				m_bCustomAreaSet = true;
			}

			if (m_bMovingArea)
			{
				glm::dvec3 domainCoordsStart = m_pDataVolumeSelection->convertToRawDomainCoords(m_vec3BeginDragOnPlane);
				glm::dvec3 domainCoordsNow = m_pDataVolumeSelection->convertToRawDomainCoords(m_vec3CurrentLocationOnPlane);
				glm::dvec3 offset = domainCoordsNow - domainCoordsStart;

				m_dvec3MinBound.x = m_dvec3MinBoundAtDragStart.x + offset.x;
				m_dvec3MinBound.y = m_dvec3MinBoundAtDragStart.y + offset.y;

				m_dvec3MaxBound.x = m_dvec3MaxBoundAtDragStart.x + offset.x;
				m_dvec3MaxBound.y = m_dvec3MaxBoundAtDragStart.y + offset.y;

				glm::vec3 markerOffsets = m_vec3CurrentLocationOnPlane - m_vec3BeginDragOnPlane;
				m_pDataVolumeSelection->convertToRawDomainCoords(m_dvec3MinBoundAtDragStart);
				m_pDataVolumeSelection->convertToRawDomainCoords(m_dvec3MaxBoundAtDragStart);
			}	

			m_dvec3MinBound.z = std::numeric_limits<double>::max();
			m_dvec3MaxBound.z = -std::numeric_limits<double>::max();

			for (auto & ds : m_pDataVolumeDisplay->getDatasets())
			{
				SonarPointCloud* pc = static_cast<SonarPointCloud*>(ds);
				for (unsigned int i = 0; i < pc->getPointCount(); ++i)
				{
					glm::dvec3 thisRawPt = pc->getRawPointPosition(i);

					if (thisRawPt.x < m_dvec3MinBound.x || thisRawPt.x > m_dvec3MaxBound.x ||
						thisRawPt.y < m_dvec3MinBound.y || thisRawPt.y > m_dvec3MaxBound.y)
					{
						pc->markPoint(i, 1);
					}
					else
					{
						pc->markPoint(i, 0);
						if (pc->getRawPointPosition(i).z < m_dvec3MinBound.z)
							m_dvec3MinBound.z = pc->getRawPointPosition(i).z;
						else if (pc->getRawPointPosition(i).z > m_dvec3MaxBound.z)
							m_dvec3MaxBound.z = pc->getRawPointPosition(i).z;
					}
				}
			}

			m_pDataVolumeDisplay->setCustomBounds(m_dvec3MinBound, m_dvec3MaxBound);
			m_pDataVolumeDisplay->useCustomBounds(true);					

			m_vec3LastSelectedLocationOnPlaneWithinDomain = m_vec3CurrentLocationOnPlane;
		}
	}
	else
	{
		m_bSelectingArea = false;
		m_bMovingArea = false;
	}
}

void SelectAreaBehavior::draw()
{
	if (m_bShowCursor)
	{
		glm::vec4 pointerColor;
		
		if (m_bActive)
			pointerColor = glm::vec4(1.f, 0.f, 0.f, 0.15f);
		else
		{
			if (m_bRayHitCustomDomain)
				pointerColor = glm::vec4(0.f, 1.f, 0.f, 0.25f);
			else
				pointerColor = glm::vec4(1.f, 1.f, 1.f, 0.25f);
		}

		// cursor dot
		glm::mat4 trans(glm::translate(glm::mat4(), m_vec3CurrentLocationOnPlane) * glm::scale(glm::mat4(), m_vec3CursorSize));

		Renderer::getInstance().drawPrimitive("icosphere", trans, pointerColor, glm::vec4(1.f), 10.f);

		// connector
		float connectorRadius = 0.005f;

		glm::vec3 controllerToCursorVec = m_vec3CurrentLocationOnPlane - glm::vec3(m_pPrimaryController->getPose()[3]);

		glm::quat rot = glm::rotation(glm::vec3(0.f, 0.f, 1.f), glm::normalize(controllerToCursorVec));

		glm::mat4 rotMat = glm::mat4_cast(rot);

		trans = glm::translate(glm::mat4(), glm::vec3(m_pPrimaryController->getPose()[3]));
		trans *= rotMat;
		trans *= glm::scale(glm::mat4(), glm::vec3(connectorRadius, connectorRadius, glm::length(controllerToCursorVec)));

		Renderer::getInstance().drawPrimitive("cylinder", trans, pointerColor, glm::vec4(1.f), 10.f);
	}

	if (m_bSelectingArea)
	{
		// MARKER DOTS
		glm::vec4 markerColor = m_bSelectingArea ? glm::vec4(1.f, 0.f, 0.f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f);

		glm::mat4 trans = glm::translate(glm::mat4(), m_vec3BeginBoundOnPlane) * glm::scale(glm::mat4(), m_vec3CursorSize);
		Renderer::getInstance().drawPrimitive("icosphere", trans, markerColor, glm::vec4(1.f), 10.f);

		trans = glm::translate(glm::mat4(), m_vec3EndBoundOnPlane) * glm::scale(glm::mat4(), m_vec3CursorSize);
		Renderer::getInstance().drawPrimitive("icosphere", trans, markerColor, glm::vec4(1.f), 10.f);
	}

	if (m_bCustomAreaSet)
	{
		// CUSTOM AREA BBOX
		glm::vec4 bboxColor;
		
		if (m_bSelectingArea)
			bboxColor = glm::vec4(0.f, 1.f, 1.f, 1.f);
		else if (m_bRayHitCustomDomain)
			bboxColor = glm::vec4(0.f, 1.f, 0.f, 1.f);
		else
			bboxColor = glm::vec4(1.f, 1.f, 0.f, 1.f);

		glm::dvec3 midPt = m_pDataVolumeSelection->convertToWorldCoords(m_pDataVolumeDisplay->getCustomMinBound() + m_pDataVolumeDisplay->getCustomDomainDimensions() * 0.5);

		glm::vec3 tmp = m_pDataVolumeDisplay->getCustomDomainDimensions() / m_pDataVolumeSelection->getDataDimensions();

		glm::vec3 domainCustomAdjustedVolumeDims = DataVolume::calcAspectAdjustedDimensions(m_pDataVolumeSelection->getDataDimensions(), m_pDataVolumeSelection->getDimensions());
		glm::vec3 customScalingFactors = domainCustomAdjustedVolumeDims * tmp;

		glm::mat4 trans = glm::translate(glm::mat4(), glm::vec3(midPt)) * glm::mat4_cast(m_pDataVolumeSelection->getOrientation()) * glm::scale(glm::mat4(), customScalingFactors);
		Renderer::getInstance().drawFlatPrimitive("bbox_lines", trans, bboxColor);
	}
}

void SelectAreaBehavior::reset()
{
	m_dvec3MinBound = glm::dvec3(std::numeric_limits<double>::max());
	m_dvec3MaxBound = glm::dvec3(-std::numeric_limits<double>::max());

	for (auto & ds : m_pDataVolumeDisplay->getDatasets())
	{
		SonarPointCloud* pc = static_cast<SonarPointCloud*>(ds);
		for (unsigned int i = 0; i < pc->getPointCount(); ++i)
			pc->markPoint(i, 0);
	}

	m_pDataVolumeDisplay->setCustomBounds(m_dvec3MinBound, m_dvec3MaxBound);
	m_pDataVolumeDisplay->useCustomBounds(false);
	m_bCustomAreaSet = false;
	m_bSelectingArea = false;
	m_bMovingArea = false;
}

void SelectAreaBehavior::updateState()
{
	if (m_pPrimaryController->justClickedTrigger())
	{
		m_bActive = true;
	}
	if (m_pPrimaryController->justUnclickedTrigger())
	{
		m_bActive = false;
	}

	if (m_pPrimaryController->justPressedTouchpad())
	{
		reset();
	}
}

glm::vec3 SelectAreaBehavior::calcHits()
{
	glm::vec3 planeOrigin = m_pDataVolumeSelection->getPosition() + glm::rotate(m_pDataVolumeSelection->getOrientation(), glm::vec3(0.f, 0.f, 1.f)) * m_pDataVolumeSelection->getDimensions().z * 0.5f;
	glm::vec3 planeNormal = glm::rotate(m_pDataVolumeSelection->getOrientation(), glm::vec3(0.f, 0.f, 1.f));
	glm::vec3 rayOrigin = glm::vec3(m_pPrimaryController->getPose()[3]);
	glm::vec3 rayDirection = glm::normalize(glm::vec3(-m_pPrimaryController->getPose()[2]));
	glm::vec3 ptOnPlane;

	m_bRayHitPlane = castRay(rayOrigin, rayDirection, planeOrigin, planeNormal, &ptOnPlane);
	m_bRayHitDomain = m_bRayHitPlane ? m_pDataVolumeSelection->isWorldCoordPointInBounds(ptOnPlane, false) : false;

	if (m_bRayHitDomain)
	{
		if (m_bCustomAreaSet)
		{
			glm::dvec3 hitPtDataCoords = m_pDataVolumeSelection->convertToRawDomainCoords(ptOnPlane);

			if (hitPtDataCoords.x > m_dvec3MinBound.x && hitPtDataCoords.x < m_dvec3MaxBound.x &&
				hitPtDataCoords.y > m_dvec3MinBound.y && hitPtDataCoords.y < m_dvec3MaxBound.y)
				m_bRayHitCustomDomain = true;
			else
				m_bRayHitCustomDomain = false;
		}
		else
			m_bRayHitCustomDomain = false;
	}
	else
	{
		m_bRayHitCustomDomain = false;
	}

	return ptOnPlane;
}

bool SelectAreaBehavior::castRay(glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec3 planeOrigin, glm::vec3 planeNormal, glm::vec3 * locationOnPlane)
{
	float denom = glm::dot(planeNormal, rayDirection);

	if (denom == 0.f)
		return false;

	float dist = glm::dot(planeNormal, (planeOrigin - rayOrigin)) / denom;

	if (dist >= 0)
	{
		*locationOnPlane = rayOrigin + (rayDirection * dist);
		return true;
	}
	else
		return false;
}
