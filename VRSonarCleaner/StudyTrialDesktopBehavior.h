#pragma once
#include "BehaviorBase.h"
#include "SonarPointCloud.h"
#include "DataVolume.h"
#include "DataLogger.h"
#include "Renderer.h"

#include <queue>

class StudyTrialDesktopBehavior :
	public InitializableBehavior
{
public:
	StudyTrialDesktopBehavior(std::string fileName, std::string category);
	~StudyTrialDesktopBehavior();

	void init();

	void update();

	void draw();

	void finish();

private:
	ColorScaler* m_pColorScaler;
	SonarPointCloud* m_pPointCloud;
	DataVolume* m_pDataVolume;

	std::string m_strFileName;
	std::string m_strCategory;

	unsigned int m_nPointsLeft;
	unsigned int m_nPointsCleaned;
	unsigned int m_nCleanedGoodPoints;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastUpdate;

	std::vector<std::pair<unsigned int, std::chrono::time_point<std::chrono::high_resolution_clock>>> m_vPointUpdateAnimations;

	bool m_bPointsCleaned;
};

