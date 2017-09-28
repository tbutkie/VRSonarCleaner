#include "SelectAreaBehavior.h"

#include "Renderer.h"
#include "SonarPointCloud.h"
#include <limits>

SelectAreaBehavior::SelectAreaBehavior(ViveController* primaryController, ViveController* secondaryController, DataVolume* selectionVolume, DataVolume* displayVolume)
	: DualControllerBehavior(primaryController, secondaryController)
	, m_pDataVolumeSelection(selectionVolume)
	, m_pDataVolumeDisplay(displayVolume)
	, m_bShowCursor(false)
	, m_bSelectingArea(false)
	, m_vec3CursorSize(glm::vec3(0.01f, 0.01f, 0.001f))
{
}


SelectAreaBehavior::~SelectAreaBehavior()
{
}

void SelectAreaBehavior::update()
{
	glm::vec3 planeOrigin = m_pDataVolumeSelection->getPosition() + glm::rotate(m_pDataVolumeSelection->getOrientation(), glm::vec3(0.f, 0.f, 0.5f)) * m_pDataVolumeSelection->getDimensions().z;
	glm::vec3 planeNormal = glm::rotate(m_pDataVolumeSelection->getOrientation(), glm::vec3(0.f, 0.f, 1.f));
	glm::vec3 rayOrigin = glm::vec3(m_pPrimaryController->getPose()[3]);
	glm::vec3 rayDirection = glm::normalize(glm::vec3(-m_pPrimaryController->getPose()[2]));

	bool rayHitDataVolume = castRay(rayOrigin, rayDirection, planeOrigin, planeNormal, &m_vec3CurrentLocationOnPlane);
	bool rayHitInDomain = rayHitDataVolume ? m_pDataVolumeSelection->isWorldCoordPointInBounds(m_vec3CurrentLocationOnPlane, false) : false;

	if (rayHitInDomain)
		m_bShowCursor = true;
	else
		m_bShowCursor = false;

	if (m_bSelectingArea && rayHitInDomain)
	{

		glm::dvec3 tmpStart = m_pDataVolumeSelection->convertToRawDomainCoords(m_vec3StartLocationOnPlane);
		glm::dvec3 tmpNow = m_pDataVolumeSelection->convertToRawDomainCoords(m_vec3CurrentLocationOnPlane);

		m_dvec3MinBound.x = std::min(tmpStart.x, tmpNow.x);
		m_dvec3MinBound.y = std::min(tmpStart.y, tmpNow.y);
		m_dvec3MinBound.z = std::numeric_limits<double>::max();

		m_dvec3MaxBound.x = std::max(tmpStart.x, tmpNow.x);
		m_dvec3MaxBound.y = std::max(tmpStart.y, tmpNow.y);
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
	}
}

void SelectAreaBehavior::draw()
{
	if (m_bSelectingArea)
	{
		glm::mat4 trans(glm::translate(glm::mat4(), m_vec3StartLocationOnPlane) * glm::scale(glm::mat4(), m_vec3CursorSize));

		Renderer::getInstance().drawPrimitive("icosphere", trans, glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec4(1.f, 1.f, 1.f, 1.f), 10.f);
	}

	if (m_bShowCursor)
	{
		glm::mat4 trans(glm::translate(glm::mat4(), m_vec3CurrentLocationOnPlane) * glm::scale(glm::mat4(), m_vec3CursorSize));

		Renderer::getInstance().drawPrimitive("icosphere", trans, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 1.f, 1.f, 1.f), 10.f);
	}

	if (m_pDataVolumeDisplay->getUseCustomBounds())
	{

		//Renderer::getInstance().drawFlatPrimitive("bbox", m_pDataVolumeSelection->);
	}
}

void SelectAreaBehavior::receiveEvent(const int event, void * payloadData)
{
	switch (event)
	{
	case BroadcastSystem::EVENT::VIVE_TRIGGER_DOWN:
	{
		BroadcastSystem::Payload::Trigger* payload;
		memcpy(&payload, &payloadData, sizeof(BroadcastSystem::Payload::Trigger*));
		if (payload->m_pSelf == m_pPrimaryController)
		{
			glm::vec3 planeOrigin = m_pDataVolumeSelection->getPosition() + glm::rotate(m_pDataVolumeSelection->getOrientation(), glm::vec3(0.f, 0.f, 0.5f)) * m_pDataVolumeSelection->getDimensions().z;
			glm::vec3 planeNormal = glm::rotate(m_pDataVolumeSelection->getOrientation(), glm::vec3(0.f, 0.f, 1.f));
			glm::vec3 rayOrigin = glm::vec3(m_pPrimaryController->getPose()[3]);
			glm::vec3 rayDirection = glm::normalize(glm::vec3(-m_pPrimaryController->getPose()[2]));

			if (castRay(rayOrigin, rayDirection, planeOrigin, planeNormal, &m_vec3StartLocationOnPlane) && m_pDataVolumeSelection->isWorldCoordPointInBounds(m_vec3StartLocationOnPlane, false))
			{
				m_dvec3MinBound = m_pDataVolumeSelection->convertToRawDomainCoords(m_vec3StartLocationOnPlane);
				std::cout << "(" << m_dvec3MinBound.x << ", " << m_dvec3MinBound.y << ", " << m_dvec3MinBound.z << ")" << std::endl;
				m_bSelectingArea = true;
			}
		}

		break;
	}
	case BroadcastSystem::EVENT::VIVE_TRIGGER_UP:
	{
		BroadcastSystem::Payload::Trigger* payload;
		memcpy(&payload, &payloadData, sizeof(BroadcastSystem::Payload::Trigger*));
		if (payload->m_pSelf == m_pPrimaryController)
		{
			m_bSelectingArea = false;
		}

		break;
	}
	case BroadcastSystem::EVENT::VIVE_TOUCHPAD_DOWN:
	{
		BroadcastSystem::Payload::Touchpad* payload;
		memcpy(&payload, &payloadData, sizeof(BroadcastSystem::Payload::Touchpad*));
		if (payload->m_pSelf == m_pPrimaryController)
		{
			m_pDataVolumeDisplay->useCustomBounds(false);
		}

		break;
	}
	default:
		break;
	}
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
