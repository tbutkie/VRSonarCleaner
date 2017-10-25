#include "ProbeBehavior.h"

#include <gtc/matrix_transform.hpp> // for translate()

#include "Renderer.h"

ProbeBehavior::ProbeBehavior(TrackedDeviceManager* pTDM, DataVolume* dataVolume)
	: m_pTDM(pTDM)
	, m_pDataVolume(dataVolume)
	, c_fTouchDeltaThreshold(0.2f)
	, m_bEnabled(true)
	, m_bShowProbe(true)
	, m_bVerticalSwipeMode(false)
	, m_bHorizontalSwipeMode(false)
	, m_vec3ProbeOffsetDirection(glm::vec3(0.f, 0.f, -1.f))
	, m_fProbeOffset(0.1f)
	, m_fProbeOffsetMin(0.1f)
	, m_fProbeOffsetMax(2.f)
	, m_fProbeRadius(0.05f)
	, m_fProbeRadiusMin(0.005f)
	, m_fProbeRadiusMax(0.25f)
{
}


ProbeBehavior::~ProbeBehavior()
{
}

void ProbeBehavior::lockProbeLength()
{
	m_fProbeOffsetMin = m_fProbeOffsetMax = m_fProbeOffset;
}

void ProbeBehavior::unlockProbeLength()
{
	m_fProbeOffsetMin = 0.1f;
	m_fProbeOffsetMax = 2.f;
}

void ProbeBehavior::lockProbeSize()
{
	m_fProbeRadiusMin = m_fProbeRadiusMax = m_fProbeRadius;
}

void ProbeBehavior::unlockProbeSize()
{
	m_fProbeRadiusMin = 0.005f;
	m_fProbeRadiusMax = 0.25f;
}

void ProbeBehavior::disable()
{
	m_bEnabled = false;
}

void ProbeBehavior::enable()
{
	m_bEnabled = true;
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
	m_fProbeRadiusMin = 0.005f;
	m_fProbeRadiusMax = 0.25f;
}

glm::vec3 ProbeBehavior::getPosition()
{
	return glm::vec3((m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset))[3]);
}

glm::vec3 ProbeBehavior::getLastPosition()
{
	return glm::vec3((m_pTDM->getPrimaryController()->getLastDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset))[3]);
}

glm::mat4 ProbeBehavior::getProbeToWorldTransform()
{
	return m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset);
}

glm::mat4 ProbeBehavior::getLastProbeToWorldTransform()
{
	return m_pTDM->getPrimaryController()->getLastDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset);
}

void ProbeBehavior::update()
{
	if (!m_pTDM->getPrimaryController())
		return;

	if (m_pTDM->getPrimaryController()->isTouchpadTouched())
	{
		glm::vec2 delta = m_pTDM->getPrimaryController()->getCurrentTouchpadTouchPoint() - m_pTDM->getPrimaryController()->getInitialTouchpadTouchPoint();

		if (!(m_bVerticalSwipeMode || m_bHorizontalSwipeMode) &&
			glm::length(delta) > c_fTouchDeltaThreshold)
		{
			m_vec2InitialMeasurementPoint = m_pTDM->getPrimaryController()->getCurrentTouchpadTouchPoint();

			if (abs(delta.x) > abs(delta.y))
			{
				m_bHorizontalSwipeMode = true;
				m_fProbeRadiusInitial = m_fProbeRadius;
			}
			else
			{
				m_bVerticalSwipeMode = true;
				m_pTDM->getPrimaryController()->setScrollWheelVisibility(true);
				m_fProbeInitialOffset = m_fProbeOffset;
			}
		}

		assert(!(m_bVerticalSwipeMode && m_bHorizontalSwipeMode));

		glm::vec2 measuredOffset = m_pTDM->getPrimaryController()->getCurrentTouchpadTouchPoint() - m_vec2InitialMeasurementPoint;

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
				m_vec2InitialMeasurementPoint.y = m_pTDM->getPrimaryController()->getCurrentTouchpadTouchPoint().y;
			}
			else if (m_fProbeOffset < m_fProbeOffsetMin)
			{
				m_fProbeOffset = m_fProbeOffsetMin;
				m_fProbeInitialOffset = m_fProbeOffsetMin;
				m_vec2InitialMeasurementPoint.y = m_pTDM->getPrimaryController()->getCurrentTouchpadTouchPoint().y;
			}
		}

		if (m_bHorizontalSwipeMode)
		{
			float dx = m_pTDM->getPrimaryController()->getCurrentTouchpadTouchPoint().x - m_vec2InitialMeasurementPoint.x;

			float range = m_fProbeRadiusMax - m_fProbeRadiusMin;

			m_fProbeRadius = m_fProbeRadiusInitial + dx * range;

			if (m_fProbeRadius > m_fProbeRadiusMax)
			{
				m_fProbeRadius = m_fProbeRadiusMax;
				m_fProbeRadiusInitial = m_fProbeRadiusMax;
				m_vec2InitialMeasurementPoint.x = m_pTDM->getPrimaryController()->getCurrentTouchpadTouchPoint().x;
			}
			else if (m_fProbeRadius < m_fProbeRadiusMin)
			{
				m_fProbeRadius = m_fProbeRadiusMin;
				m_fProbeRadiusInitial = m_fProbeRadiusMin;
				m_vec2InitialMeasurementPoint.x = m_pTDM->getPrimaryController()->getCurrentTouchpadTouchPoint().x;
			}
		}
	}

	if (m_pTDM->getPrimaryController()->justUntouchedTouchpad())
	{
		m_bVerticalSwipeMode = m_bHorizontalSwipeMode = false;
		m_pTDM->getPrimaryController()->setScrollWheelVisibility(false);
	}

	if (m_pTDM->getPrimaryController()->justClickedTrigger())
	{
		activateProbe();
	}

	if (!m_pTDM->getPrimaryController()->isTriggerClicked())
	{
		deactivateProbe();
	}
}

void ProbeBehavior::drawProbe(float length)
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getPrimaryController()->readyToRender())
		return;

	if (m_bShowProbe)
	{	
		// Set color
		glm::vec4 diffColor, specColor;
		if (m_pTDM->getPrimaryController()->isTriggerClicked())
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

		glm::mat4 matCyl = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(0.0025f, 0.0025f, length));

		Renderer::getInstance().drawPrimitive("cylinder", matCyl, diffColor, specColor, specExp);
	}
}

float ProbeBehavior::getProbeOffset()
{
	return m_fProbeOffset;
}

float ProbeBehavior::getProbeOffsetMax()
{
	return m_fProbeOffsetMax;
}

float ProbeBehavior::getProbeOffsetMin()
{
	return m_fProbeOffsetMin;
}

glm::vec3 ProbeBehavior::getProbeOffsetDirection()
{
	return m_vec3ProbeOffsetDirection;
}

float ProbeBehavior::getProbeRadius()
{
	return m_fProbeRadius;
}

float ProbeBehavior::getProbeRadiusMax()
{
	return m_fProbeRadiusMax;
}

float ProbeBehavior::getProbeRadiusMin()
{
	return m_fProbeRadiusMin;
}
