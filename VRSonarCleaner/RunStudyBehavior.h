#pragma once
#include "BehaviorBase.h"
#include "StudyTrialStandingBehavior.h"
#include "TrackedDeviceManager.h"
#include "DataLogger.h"

#include <vector>
#include <queue>
#include <filesystem>
#include <future>
#include <fstream>

class RunStudyBehavior :
	public InitializableBehavior
{
public:
	RunStudyBehavior(TrackedDeviceManager* pTDM);
	~RunStudyBehavior();

	void init();

	void update();

	void draw();

private:
	TrackedDeviceManager *m_pTDM;
	std::vector<std::pair<std::experimental::filesystem::v1::path, std::string>> m_vStudyDatasets;
	std::queue<InitializableBehavior*> m_qTrials;

	std::future<void> m_Future;

	bool m_bTrialsLoaded;
};

