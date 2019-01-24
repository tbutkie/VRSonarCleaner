#pragma once
#include "BehaviorBase.h"
#include "SonarPointCloud.h"
#include "DataVolume.h"
#include "TrackedDeviceManager.h"
#include "DataLogger.h"

#include <queue>

class StudyTrialNoMotionCompensation :
	public InitializableBehavior
{
public:
	StudyTrialNoMotionCompensation(TrackedDeviceManager* pTDM, std::string fileName, std::string category);
	~StudyTrialNoMotionCompensation();

	void init();

	void update();

	void draw();

private:
	void refreshColorScale(ColorScaler * colorScaler, std::vector<SonarPointCloud*> clouds);

	TrackedDeviceManager *m_pTDM;
	ColorScaler* m_pColorScaler;
	SonarPointCloud* m_pPointCloud;
	DataVolume* m_pDataVolume;

	bool m_bInitialColorRefresh;

	std::string m_strFileName;
	std::string m_strCategory;

	unsigned int m_nPointsLeft;
	unsigned int m_nPointsCleaned;
	unsigned int m_nCleanedGoodPoints;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastUpdate;

	std::vector<std::pair<unsigned int, std::chrono::time_point<std::chrono::high_resolution_clock>>> m_vPointUpdateAnimations;
};

