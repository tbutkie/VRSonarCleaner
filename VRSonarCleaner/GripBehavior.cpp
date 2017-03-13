#include "GripBehavior.h"



GripBehavior::GripBehavior(ViveController* controller, DataVolume* dataVolume)
	: BehaviorBase(controller)
	, m_pController(controller)
	, m_pDataVolume(dataVolume)
	, m_bGripping(false)
{
}


GripBehavior::~GripBehavior()
{
}

void GripBehavior::update()
{
	if (m_bGripping)
		m_pDataVolume->continueRotation(m_pController->getDeviceToWorldTransform());
}

void GripBehavior::receiveEvent(const int event, void * payloadData)
{
	switch (event)
	{
	case BroadcastSystem::EVENT::VIVE_TRIGGER_DOWN:
		BroadcastSystem::Payload::Trigger payload;
		memcpy(&payload, payloadData, sizeof(BroadcastSystem::Payload::Trigger));
		m_pDataVolume->startRotation(m_pController->getDeviceToWorldTransform());
		m_bGripping = true;
		break;
	case BroadcastSystem::EVENT::VIVE_TRIGGER_UP:
		m_pDataVolume->endRotation();
		m_bGripping = false;
		break;
	default:
		break;
	}
}
