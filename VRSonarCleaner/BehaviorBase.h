#pragma once

#include "BroadcastSystem.h"

#include "TrackedDevice.h"

class BehaviorBase : public BroadcastSystem::Listener
{
public:
	BehaviorBase(TrackedDevice *trackedDevice);
	virtual ~BehaviorBase();

	virtual void update() = 0;

protected:
	TrackedDevice* m_pTrackedDevice;

	virtual void receiveEvent(const int event, void* payloadData) = 0;
};

