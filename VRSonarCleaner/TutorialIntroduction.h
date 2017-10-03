#pragma once
#include "BehaviorBase.h"

#include "TrackedDeviceManager.h"
#include "DataVolume.h"

class TutorialIntroduction :
	public InitializableBehavior
{
public:
	TutorialIntroduction(TrackedDeviceManager* pTDM, DataVolume* tableVolume);
	virtual ~TutorialIntroduction();

	void init();
	
	void update();

	void draw();

private:
	TrackedDeviceManager *m_pTDM;
	DataVolume *m_pTableVolume;
};

