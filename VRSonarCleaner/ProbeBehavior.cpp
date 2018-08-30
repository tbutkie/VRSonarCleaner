#include "ProbeBehavior.h"

#include <gtc/matrix_transform.hpp> // for translate()

#include "Renderer.h"
#include "DataLogger.h"

ProbeBehavior::ProbeBehavior(ViveController* pController, DataVolume* dataVolume)
	: m_pController(pController)
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
	, m_vec4ProbeColorDiff(glm::vec4(0.125f, 0.125f, 0.125f, 1.f))
	, m_vec4ProbeColorSpec(glm::vec4(1.f))
	, m_vec4ProbeActivateColorDiff(glm::vec4(0.502f, 0.125f, 0.125f, 1.f))
	, m_vec4ProbeActivateColorSpec(glm::vec4(0.f, 0.f, 1.f, 1.f))
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
	return glm::vec3((m_pController->getDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset))[3]);
}

glm::vec3 ProbeBehavior::getLastPosition()
{
	return glm::vec3((m_pController->getLastDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset))[3]);
}

glm::mat4 ProbeBehavior::getTransformProbeToWorld()
{
	return m_pController->getDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset);
}

glm::mat4 ProbeBehavior::getTransformProbeToWorld_Last()
{
	return m_pController->getLastDeviceToWorldTransform() * glm::translate(glm::mat4(), m_vec3ProbeOffsetDirection * m_fProbeOffset);
}

void ProbeBehavior::update()
{
	if (!m_pController || !m_pController->valid())
		return;

	if (m_pController->isTouchpadTouched())
	{
		glm::vec2 delta = m_pController->getCurrentTouchpadTouchPoint() - m_pController->getInitialTouchpadTouchPoint();

		if (!(m_bVerticalSwipeMode || m_bHorizontalSwipeMode) &&
			glm::length(delta) > c_fTouchDeltaThreshold)
		{
			m_vec2InitialMeasurementPoint = m_pController->getCurrentTouchpadTouchPoint();

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

			//if (DataLogger::getInstance().logging())
			//{
			//	glm::vec3 hmdPos = m_pTDM->getHMDToWorldTransform()[3];
			//	glm::quat hmdQuat = glm::quat_cast(m_pTDM->getHMDToWorldTransform());
			//
			//	std::stringstream ss;
			//
			//	ss << (m_bHorizontalSwipeMode ? "Probe Length Begin" : "Probe Radius Begin");
			//	ss << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
			//	ss << "\t";
			//	ss << "hmd-pos:\"" << hmdPos.x << "," << hmdPos.y << "," << hmdPos.z << "\"";
			//	ss << ";";
			//	ss << "hmd-quat:\"" << hmdQuat.x << "," << hmdQuat.y << "," << hmdQuat.z << "," << hmdQuat.w << "\"";
			//
			//	if (m_pController)
			//	{
			//		glm::vec3 primCtrlrPos = m_pController->getDeviceToWorldTransform()[3];
			//		glm::quat primCtrlrQuat = glm::quat_cast(m_pController->getDeviceToWorldTransform());
			//
			//		glm::mat4 probeTrans(getTransformProbeToWorld());
			//
			//		ss << ";";
			//		ss << "probe-pos:\"" << probeTrans[3].x << "," << probeTrans[3].y << "," << probeTrans[3].z << "\"";
			//		ss << ";";
			//		ss << "probe-radius:\"" << getProbeRadius() << "\"";
			//		ss << ";";
			//		ss << "primary-controller-pos:\"" << primCtrlrPos.x << "," << primCtrlrPos.y << "," << primCtrlrPos.z << "\"";
			//		ss << ";";
			//		ss << "primary-controller-quat:\"" << primCtrlrQuat.x << "," << primCtrlrQuat.y << "," << primCtrlrQuat.z << "," << primCtrlrQuat.w << "\"";
			//	}
			//
			//	if (m_pTDM->getSecondaryController())
			//	{
			//		glm::vec3 secCtrlrPos = m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3];
			//		glm::quat secCtrlrQuat = glm::quat_cast(m_pTDM->getSecondaryController()->getDeviceToWorldTransform());
			//
			//		ss << ";";
			//		ss << "secondary-controller-pos:\"" << secCtrlrPos.x << "," << secCtrlrPos.y << "," << secCtrlrPos.z << "\"";
			//		ss << ";";
			//		ss << "secondary-controller-quat:\"" << secCtrlrQuat.x << "," << secCtrlrQuat.y << "," << secCtrlrQuat.z << "," << secCtrlrQuat.w << "\"";
			//	}
			//
			//	DataLogger::getInstance().logMessage(ss.str());
			//}
		}

		assert(!(m_bVerticalSwipeMode && m_bHorizontalSwipeMode));

		glm::vec2 measuredOffset = m_pController->getCurrentTouchpadTouchPoint() - m_vec2InitialMeasurementPoint;

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
				m_vec2InitialMeasurementPoint.y = m_pController->getCurrentTouchpadTouchPoint().y;
			}
			else if (m_fProbeOffset < m_fProbeOffsetMin)
			{
				m_fProbeOffset = m_fProbeOffsetMin;
				m_fProbeInitialOffset = m_fProbeOffsetMin;
				m_vec2InitialMeasurementPoint.y = m_pController->getCurrentTouchpadTouchPoint().y;
			}
		}

		if (m_bHorizontalSwipeMode)
		{
			float dx = m_pController->getCurrentTouchpadTouchPoint().x - m_vec2InitialMeasurementPoint.x;

			float range = m_fProbeRadiusMax - m_fProbeRadiusMin;

			m_fProbeRadius = m_fProbeRadiusInitial + dx * range;

			if (m_fProbeRadius > m_fProbeRadiusMax)
			{
				m_fProbeRadius = m_fProbeRadiusMax;
				m_fProbeRadiusInitial = m_fProbeRadiusMax;
				m_vec2InitialMeasurementPoint.x = m_pController->getCurrentTouchpadTouchPoint().x;
			}
			else if (m_fProbeRadius < m_fProbeRadiusMin)
			{
				m_fProbeRadius = m_fProbeRadiusMin;
				m_fProbeRadiusInitial = m_fProbeRadiusMin;
				m_vec2InitialMeasurementPoint.x = m_pController->getCurrentTouchpadTouchPoint().x;
			}
		}
	}

	if (m_pController->justUntouchedTouchpad())
	{
		//if (DataLogger::getInstance().logging())
		//{
		//	glm::vec3 hmdPos = m_pTDM->getHMDToWorldTransform()[3];
		//	glm::quat hmdQuat = glm::quat_cast(m_pTDM->getHMDToWorldTransform());
		//
		//	std::stringstream ss;
		//
		//	ss << (m_bHorizontalSwipeMode ? "Probe Length End" : "Probe Radius End");
		//	ss << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
		//	ss << "\t";
		//	ss << "hmd-pos:\"" << hmdPos.x << "," << hmdPos.y << "," << hmdPos.z << "\"";
		//	ss << ";";
		//	ss << "hmd-quat:\"" << hmdQuat.x << "," << hmdQuat.y << "," << hmdQuat.z << "," << hmdQuat.w << "\"";
		//
		//	if (m_pController)
		//	{
		//		glm::vec3 primCtrlrPos = m_pController->getDeviceToWorldTransform()[3];
		//		glm::quat primCtrlrQuat = glm::quat_cast(m_pController->getDeviceToWorldTransform());
		//
		//		glm::mat4 probeTrans(getTransformProbeToWorld());
		//
		//		ss << ";";
		//		ss << "probe-pos:\"" << probeTrans[3].x << "," << probeTrans[3].y << "," << probeTrans[3].z << "\"";
		//		ss << ";";
		//		ss << "probe-radius:\"" << getProbeRadius() << "\"";
		//		ss << ";";
		//		ss << "primary-controller-pos:\"" << primCtrlrPos.x << "," << primCtrlrPos.y << "," << primCtrlrPos.z << "\"";
		//		ss << ";";
		//		ss << "primary-controller-quat:\"" << primCtrlrQuat.x << "," << primCtrlrQuat.y << "," << primCtrlrQuat.z << "," << primCtrlrQuat.w << "\"";
		//	}
		//
		//	if (m_pTDM->getSecondaryController())
		//	{
		//		glm::vec3 secCtrlrPos = m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3];
		//		glm::quat secCtrlrQuat = glm::quat_cast(m_pTDM->getSecondaryController()->getDeviceToWorldTransform());
		//
		//		ss << ";";
		//		ss << "secondary-controller-pos:\"" << secCtrlrPos.x << "," << secCtrlrPos.y << "," << secCtrlrPos.z << "\"";
		//		ss << ";";
		//		ss << "secondary-controller-quat:\"" << secCtrlrQuat.x << "," << secCtrlrQuat.y << "," << secCtrlrQuat.z << "," << secCtrlrQuat.w << "\"";
		//	}
		//
		//	DataLogger::getInstance().logMessage(ss.str());
		//}

		m_bVerticalSwipeMode = m_bHorizontalSwipeMode = false;
		m_pController->setScrollWheelVisibility(false);
	}

	if (m_pController->justClickedTrigger())
	{
		activateProbe();
	}

	if (m_pController->justUnclickedTrigger() && !m_pController->isTriggerClicked())
	{
		deactivateProbe();
	}
}

void ProbeBehavior::drawProbe(float length)
{
	if (!m_pController || !m_pController->valid())
		return;

	if (m_bShowProbe)
	{	
		// Set color
		glm::vec4 diffColor, specColor;
		if (m_pController->isTriggerClicked())
		{
			diffColor = m_vec4ProbeActivateColorDiff;
			specColor = m_vec4ProbeActivateColorSpec;
		}
		else
		{
			diffColor = m_vec4ProbeColorDiff;
			specColor = m_vec4ProbeColorSpec;
		}
		float specExp(30.f);

		glm::mat4 matCyl = m_pController->getDeviceToWorldTransform() * glm::rotate(glm::mat4(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(0.0025f, 0.0025f, length));

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
