#pragma once
#include "BehaviorBase.h"
#include "SonarPointCloud.h"
#include "DataVolume.h"
#include "TrackedDeviceManager.h"
#include "DataLogger.h"

#include <queue>

class StudyTrialFishtankBehavior :
	public InitializableEventBehavior
{
public:
	StudyTrialFishtankBehavior(TrackedDeviceManager* pTDM, std::string fileName, std::string category, DataVolume* dataVolume);
	~StudyTrialFishtankBehavior();

	void init();

	void processEvent(SDL_Event &ev);

	void update();

	void draw();

	void finish();

private:
	TrackedDeviceManager *m_pTDM;
	ColorScaler* m_pColorScaler;
	SonarPointCloud* m_pPointCloud;
	DataVolume* m_pDataVolume;

	std::string m_strFileName;
	std::string m_strCategory;

	unsigned int m_nPointsLeft;
	unsigned int m_nPointsCleaned;
	unsigned int m_nCleanedGoodPoints;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastUpdate;

	bool m_bPointsCleaned;
};

