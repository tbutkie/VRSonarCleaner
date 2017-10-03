#pragma once
#include "BehaviorBase.h"

#include "TrackedDeviceManager.h"

class WelcomeBehavior :
	public InitializableBehavior
{
public:
	WelcomeBehavior(TrackedDeviceManager* pTDM);
	virtual ~WelcomeBehavior();

	void init();

	void update();

	void draw();

private:
	TrackedDeviceManager *m_pTDM;
};

