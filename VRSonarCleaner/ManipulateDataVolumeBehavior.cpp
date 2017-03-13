#include "ManipulateDataVolumeBehavior.h"



ManipulateDataVolumeBehavior::ManipulateDataVolumeBehavior(ViveController* gripController, ViveController* scaleController, DataVolume* dataVolume)
	: DualControllerBehavior(gripController, scaleController)
	, m_pGripController(gripController)
	, m_pScaleController(scaleController)
	, m_pDataVolume(dataVolume)
	, m_bGripping(false)
	, m_bScaling(false)
{
	scaleController->attach(this);
}


ManipulateDataVolumeBehavior::~ManipulateDataVolumeBehavior()
{
	if (m_pScaleController)
		m_pScaleController->detach(this);
}

void ManipulateDataVolumeBehavior::update()
{
	if (m_bScaling)
	{
		float currentDist = controllerDistance();
		float delta = currentDist - m_fInitialDistance;
		m_pDataVolume->setSize(glm::vec3(exp(delta * 10.f) * m_vec3InitialScale));
	}
	else if (m_bGripping)
		m_pDataVolume->continueRotation(m_pGripController->getDeviceToWorldTransform());
}

void ManipulateDataVolumeBehavior::receiveEvent(const int event, void * payloadData)
{
	switch (event)
	{
	case BroadcastSystem::EVENT::VIVE_TRIGGER_DOWN:
	{
		BroadcastSystem::Payload::Trigger payload;
		memcpy(&payload, payloadData, sizeof(BroadcastSystem::Payload::Trigger));
		if (payload.m_pSelf == m_pGripController)
		{
			m_pDataVolume->startRotation(m_pGripController->getDeviceToWorldTransform());
			m_bGripping = true;
		}
		else
		{
			if (m_bGripping)
			{
				m_bGripping = false;
				m_bScaling = true;

				m_fInitialDistance = controllerDistance();
				m_vec3InitialScale = m_pDataVolume->getSize();
			}
		}
		break;
	}
	case BroadcastSystem::EVENT::VIVE_TRIGGER_UP:
	{
		BroadcastSystem::Payload::Trigger payload;
		memcpy(&payload, payloadData, sizeof(BroadcastSystem::Payload::Trigger));
		if (payload.m_pSelf == m_pGripController)
		{
			m_pDataVolume->endRotation();
			m_bGripping = false;
			m_bScaling = false;
		}
		else
		{
			if (m_bScaling)
			{
				m_bScaling = false;
				m_pDataVolume->startRotation(m_pGripController->getDeviceToWorldTransform());
				m_bGripping = true;
			}
		}
		break;
	}
	default:
		break;
	}
}

float ManipulateDataVolumeBehavior::controllerDistance()
{
	return glm::length(m_pGripController->getDeviceToWorldTransform()[3] - m_pScaleController->getDeviceToWorldTransform()[3]);
}
