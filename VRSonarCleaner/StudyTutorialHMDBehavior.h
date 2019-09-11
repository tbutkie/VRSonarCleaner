#pragma once
#include "BehaviorBase.h"

#include "TrackedDeviceManager.h"
#include "DataVolume.h"

#include <queue>



class StudyTutorialHMDBehavior :
	public BehaviorBase
{

public:
	StudyTutorialHMDBehavior(TrackedDeviceManager* m_pTDM);
	virtual ~StudyTutorialHMDBehavior();

	void update();

	void draw();

private:
	TrackedDeviceManager *m_pTDM;

	std::queue<std::pair<std::string, InitializableBehavior*>> m_qTutorialQueue;

private:
	void createDemoQueue();
};

