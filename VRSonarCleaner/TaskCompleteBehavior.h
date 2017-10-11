#pragma once
#include "BehaviorBase.h"

#include <vector>

#include "TrackedDeviceManager.h"
#include "DataVolume.h"

#include "SonarPointCloud.h"
#include "ColorScaler.h"

class TaskCompleteBehavior :
	public InitializableBehavior
{
public:
	TaskCompleteBehavior(TrackedDeviceManager* pTDM);
	virtual ~TaskCompleteBehavior();

	void init();
	
	void update();

	void draw();

	bool restartRequested();

private:
	TrackedDeviceManager *m_pTDM;

	bool m_bTriggersReady;
	bool m_bTouchpadReady;
	bool m_bRestart;
};

