#pragma once

#include "BroadcastSystem.h"

class BehaviorBase : public BroadcastSystem::Listener
{
public:
	BehaviorBase() {};
	virtual ~BehaviorBase() {};

	virtual void update() = 0;

protected:
	virtual void receiveEvent(const int event, void* payloadData) = 0;
};

