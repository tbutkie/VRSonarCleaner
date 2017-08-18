#include "ProbeBehavior.h"

#include "shared/glm/gtc/matrix_transform.hpp" // for translate()

#include "Renderer.h"

ProbeBehavior::ProbeBehavior(ViveController* controller, DataVolume* dataVolume)
	: SingleControllerBehavior(controller)
	, m_pDataVolume(dataVolume)
	, c_fTouchDeltaThreshold(0.2f)
	, m_bShowProbe(true)
	, m_bVerticalSwipeMode(false)
	, m_bHorizontalSwipeMode(false)
	, m_vec3ProbeOffsetDirection(glm::vec3(0.f, 0.f, -1.f))
	, m_fProbeOffset(0.1f)
	, m_fProbeOffsetMin(0.1f)
	, m_fProbeOffsetMax(2.f)
	, m_fProbeRadius(0.05f)
	, m_fProbeRadiusMin(0.01f)
	, m_fProbeRadiusMax(0.5f)
{
}


ProbeBehavior::~ProbeBehavior()
{
}

void ProbeBehavior::activateDemoMode()
{
	m_fProbeOffset = m_fProbeOffsetMin = m_fProbeOffsetMax = 0.1f;
	m_fProbeRadius = m_fProbeRadiusMin = m_fProbeRadiusMax = 0.05f;
}

void ProbeBehavior::deactivateDemoMode()
{
	m_fProbeOffset = 0.1f;
	m_fProbeOffsetMin = 0.1f;
	m_fProbeOffsetMax = 2.f;
	m_fProbeRadius = 0.05f;
	m_fProbeRadiusMin = 0.01f;
	m_fProbeRadiusMax = 0.5f;
}

glm::vec3 ProbeBehavior::getPosition()
{
	return glm::vec3((m_pController->getDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset))[3]);
}

glm::vec3 ProbeBehavior::getLastPosition()
{
	return glm::vec3((m_pController->getLastDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset))[3]);
}

glm::mat4 ProbeBehavior::getProbeToWorldTransform()
{
	return m_pController->getDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset);
}

glm::mat4 ProbeBehavior::getLastProbeToWorldTransform()
{
	return m_pController->getLastDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset);
}

void ProbeBehavior::update()
{
}

void ProbeBehavior::drawProbe(float length)
{
	if (!m_pController->readyToRender())
		return;

	if (m_bShowProbe)
	{	
		// Set color
		glm::vec4 diffColor, specColor;
		if (m_pController->isTriggerClicked())
		{
			diffColor = glm::vec4(0.502f, 0.125f, 0.125f, 1.f);
			specColor = glm::vec4(0.f, 0.f, 1.f, 1.f);
		}
		else
		{
			diffColor = glm::vec4(0.125f, 0.125f, 0.125f, 1.f);
			specColor = glm::vec4(1.f);
		}
		float specExp(30.f);

		glm::mat4 matCyl = m_pController->getDeviceToWorldTransform() * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(0.0025f, 0.0025f, length));

		Renderer::getInstance().drawPrimitive("cylinder", matCyl, diffColor, specColor, specExp);
	}
}

void ProbeBehavior::receiveEvent(const int event, void * payloadData)
{
	switch (event)
	{
	case BroadcastSystem::EVENT::VIVE_TOUCHPAD_ENGAGE:
	{
		BroadcastSystem::Payload::Touchpad* payload;
		memcpy(&payload, &payloadData, sizeof(payload));
		
		break;
	}
	case BroadcastSystem::EVENT::VIVE_TOUCHPAD_TOUCH:
	{
		BroadcastSystem::Payload::Touchpad* payload;
		memcpy(&payload, &payloadData, sizeof(payload));
		
		glm::vec2 delta = payload->m_vec2CurrentTouch - payload->m_vec2InitialTouch;

		if (!(m_bVerticalSwipeMode || m_bHorizontalSwipeMode) &&
			glm::length(delta) > c_fTouchDeltaThreshold)
		{
			m_vec2InitialMeasurementPoint = payload->m_vec2CurrentTouch;

			if (abs(delta.x) > abs(delta.y))
			{
				m_bHorizontalSwipeMode = true;
				m_fProbeRadiusInitial = m_fProbeRadius;
			}
			else
			{
				m_bVerticalSwipeMode = true;
				m_pController->setScrollWheelVisibility(true);
				m_fProbeInitialOffset = m_fProbeOffset;
			}
		}

		assert(!(m_bVerticalSwipeMode && m_bHorizontalSwipeMode));

		glm::vec2 measuredOffset = payload->m_vec2CurrentTouch - m_vec2InitialMeasurementPoint;

		if (m_bVerticalSwipeMode)
		{
			m_fProbeOffset = measuredOffset.y;

			float dy = measuredOffset.y;

			float range = m_fProbeOffsetMax - m_fProbeOffsetMin;

			m_fProbeOffset = m_fProbeInitialOffset + dy * range * 0.5f;
			
			if (m_fProbeOffset > m_fProbeOffsetMax)
			{
				m_fProbeOffset = m_fProbeOffsetMax;
				m_fProbeInitialOffset = m_fProbeOffsetMax;
				m_vec2InitialMeasurementPoint.y = payload->m_vec2CurrentTouch.y;
			}
			else if (m_fProbeOffset < m_fProbeOffsetMin)
			{
				m_fProbeOffset = m_fProbeOffsetMin;
				m_fProbeInitialOffset = m_fProbeOffsetMin;
				m_vec2InitialMeasurementPoint.y = payload->m_vec2CurrentTouch.y;
			}
		}

		if (m_bHorizontalSwipeMode)
		{
			float dx = payload->m_vec2CurrentTouch.x - m_vec2InitialMeasurementPoint.x;

			float range = m_fProbeRadiusMax - m_fProbeRadiusMin;

			m_fProbeRadius = m_fProbeRadiusInitial + dx * range;

			if (m_fProbeRadius > m_fProbeRadiusMax)
			{
				m_fProbeRadius = m_fProbeRadiusMax;
				m_fProbeRadiusInitial = m_fProbeRadiusMax;
				m_vec2InitialMeasurementPoint.x = payload->m_vec2CurrentTouch.x;
			}
			else if (m_fProbeRadius < m_fProbeRadiusMin)
			{ 
				m_fProbeRadius = m_fProbeRadiusMin;
				m_fProbeRadiusInitial = m_fProbeRadiusMin;
				m_vec2InitialMeasurementPoint.x = payload->m_vec2CurrentTouch.x;
			}
		}

		break;
	}
	case BroadcastSystem::EVENT::VIVE_TOUCHPAD_DISENGAGE:
	{
		m_bVerticalSwipeMode = m_bHorizontalSwipeMode = false;
		m_pController->setScrollWheelVisibility(false);
		break;
	}
	case BroadcastSystem::EVENT::VIVE_TRIGGER_DOWN:
	{
		activateProbe();
		break;
	}
	case BroadcastSystem::EVENT::VIVE_TRIGGER_UP:
	{
		deactivateProbe();
		break;
	}
	default:
		break;
	}
}
