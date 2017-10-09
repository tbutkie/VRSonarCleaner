#pragma once
#include "BehaviorBase.h"

#include <vector>

#include "TrackedDeviceManager.h"
#include "DataVolume.h"

#include "SonarPointCloud.h"
#include "ColorScaler.h"

class GrabTutorial :
	public InitializableBehavior
{
public:
	GrabTutorial(TrackedDeviceManager* pTDM);
	virtual ~GrabTutorial();

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

