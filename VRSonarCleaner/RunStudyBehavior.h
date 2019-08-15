#pragma once
#include "BehaviorBase.h"
#include "TrackedDeviceManager.h"
#include "DataLogger.h"
#include "Renderer.h"

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
	RunStudyBehavior();
	~RunStudyBehavior();

	void init();

	void update();

	void draw();

	void next();

	enum STUDY_TYPE {
		VR,
		FISHTANK,
		DESKTOP
	};

private:
	STUDY_TYPE m_eStudyType;

	std::vector<std::pair<std::experimental::filesystem::v1::path, std::string>> m_vStudyDatasets;
	std::queue<InitializableBehavior*> m_qTrials;

	bool m_bTrialsLoaded;

	// VR Vars
	TrackedDeviceManager *m_pTDM;
};

