#pragma once
#include "BehaviorBase.h"

#include "TrackedDeviceManager.h"
#include "DataVolume.h"

#include <queue>



class TutorialBehavior :
	public BehaviorBase
{

public:
	TutorialBehavior(TrackedDeviceManager* m_pTDM, DataVolume* TableVolume, DataVolume* WallVolume);
	~TutorialBehavior();

	void update();

	void draw();

private:
	TrackedDeviceManager *m_pTDM;
	DataVolume* m_pTableVolume;
	DataVolume* m_pWallVolume;

	std::queue<InitializableBehavior*> m_qTutorialQueue;

private:
	void createTutorialQueue();
};

