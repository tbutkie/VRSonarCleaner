#include "SelectAreaBehavior.h"

#include "DebugDrawer.h"

SelectAreaBehavior::SelectAreaBehavior(ViveController* primaryController, ViveController* secondaryController, DataVolume* dataVolume)
	: DualControllerBehavior(primaryController, secondaryController)
	, m_pDataVolume(dataVolume)
	, m_bSelectingArea(false)
	, m_vec3CursorSize(glm::vec3(0.01f, 0.01f, 0.001f))
{
}


SelectAreaBehavior::~SelectAreaBehavior()
{
}

void SelectAreaBehavior::update()
{
	glm::vec3 planeOrigin = m_pDataVolume->getPosition() + glm::rotate(m_pDataVolume->getOrientation(), glm::vec3(0.f, 0.f, 0.5f)) * m_pDataVolume->getDimensions().z;
	glm::vec3 planeNormal = glm::rotate(m_pDataVolume->getOrientation(), glm::vec3(0.f, 0.f, 1.f));
	glm::vec3 rayOrigin = glm::vec3(m_pPrimaryController->getPose()[3]);
	glm::vec3 rayDirection = glm::normalize(glm::vec3(-m_pPrimaryController->getPose()[2]));

	m_fDistanceToPlane = glm::dot(planeNormal, (planeOrigin - rayOrigin)) / glm::dot(planeNormal, rayDirection);
	m_vec3LocationOnPlane = rayOrigin + (rayDirection * m_fDistanceToPlane);
}

void SelectAreaBehavior::draw()
{
	if (m_fDistanceToPlane >= 0.f)
	{
		if (!m_pDataVolume->isWorldCoordPointInBounds(m_vec3LocationOnPlane, false))
			return;
		
		glm::mat4 trans(glm::translate(glm::mat4(), m_vec3LocationOnPlane) * glm::scale(glm::mat4(), m_vec3CursorSize));

		Renderer::getInstance().drawPrimitive("icosphere", trans, glm::vec4(1.f, 1.f, 1.f, 0.5f), glm::vec4(1.f, 1.f, 1.f, 0.5f), 10.f);
	}
}

void SelectAreaBehavior::receiveEvent(const int event, void * payloadData)
{
	switch (event)
	{
	case BroadcastSystem::EVENT::VIVE_TRIGGER_DOWN:
	{
		m_bSelectingArea = true;
		break;
	}
	case BroadcastSystem::EVENT::VIVE_TRIGGER_UP:
	{
		m_bSelectingArea = false;
		break;
	}
	default:
		break;
	}
}