#pragma once
#include "BehaviorBase.h"

#include "TrackedDeviceManager.h"
#include "DataVolume.h"

#include <queue>



class DemoBehavior :
	public BehaviorBase
{

public:
	DemoBehavior(TrackedDeviceManager* m_pTDM, DataVolume* TableVolume, DataVolume* WallVolume);
	virtual ~DemoBehavior();

	void update();

	void draw();

private:
	TrackedDeviceManager *m_pTDM;
	DataVolume* m_pTableVolume;
	DataVolume* m_pWallVolume;

	std::queue<std::pair<std::string, InitializableBehavior*>> m_qTutorialQueue;

private:
	void createDemoQueue();
};

