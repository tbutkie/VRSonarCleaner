#pragma once
#include "BehaviorBase.h"
#include "SonarPointCloud.h"
#include "DataVolume.h"
#include "TrackedDeviceManager.h"
#include "DataLogger.h"

#include <queue>

class StudyTutorialFishtankBehavior :
	public InitializableEventBehavior
{
public:
	StudyTutorialFishtankBehavior(TrackedDeviceManager* pTDM, DataVolume* dataVolume);
	~StudyTutorialFishtankBehavior();

	void init();

	void processEvent(SDL_Event &ev);

	void update();

	void draw();

	void reset();

	void finish();

private:
	TrackedDeviceManager *m_pTDM;
	ColorScaler* m_pColorScaler;
	SonarPointCloud* m_pPointCloud;
	DataVolume* m_pDataVolume;

	unsigned int m_nPointsLeft;
	unsigned int m_nPointsCleaned;
	unsigned int m_nCleanedGoodPoints;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastUpdate;

	bool m_bPointsCleaned;
};

