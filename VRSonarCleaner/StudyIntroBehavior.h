#pragma once
#include "BehaviorBase.h"

#include "TrackedDeviceManager.h"

class StudyIntroBehavior :
	public InitializableBehavior
{
public:
	StudyIntroBehavior(TrackedDeviceManager* pTDM);
	virtual ~StudyIntroBehavior();

	void init();

	void update();

	void draw();

private:
	TrackedDeviceManager *m_pTDM;

	bool m_bWaitForTriggerRelease;
};

