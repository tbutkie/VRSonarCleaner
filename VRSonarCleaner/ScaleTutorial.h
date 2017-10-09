#pragma once
#include "BehaviorBase.h"

#include <vector>

#include "TrackedDeviceManager.h"
#include "DataVolume.h"

class ScaleTutorial :
	public InitializableBehavior
{
public:
	ScaleTutorial(TrackedDeviceManager* pTDM);
	virtual ~ScaleTutorial();

	void init();
	
	void update();

	void draw();

private:
	TrackedDeviceManager *m_pTDM;
	DataVolume *m_pDemoVolume;
	DataVolume *m_pGoalVolume;

	bool m_bWaitForTriggerRelease;

	bool checkVolBounds();
};

