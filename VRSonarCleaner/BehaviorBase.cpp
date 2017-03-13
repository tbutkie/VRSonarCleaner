#include "BehaviorBase.h"



BehaviorBase::BehaviorBase(TrackedDevice *trackedDevice)
	: m_pTrackedDevice(trackedDevice)
{
	trackedDevice->attach(this);
}


BehaviorBase::~BehaviorBase()
{
	if (m_pTrackedDevice)
		m_pTrackedDevice->detach(this);
}
