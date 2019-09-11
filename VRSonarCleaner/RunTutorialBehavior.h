#pragma once
#include "BehaviorBase.h"
#include "TrackedDeviceManager.h"
#include "DataLogger.h"
#include "DataVolume.h"
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
	enum EStudyType {
		VR,
		FISHTANK,
		DESKTOP
	};

	RunStudyBehavior(TrackedDeviceManager* pTDM, EStudyType mode, DataVolume* fishtankVolume);
	~RunStudyBehavior();

	void init();

	void processSDLEvent(SDL_Event &ev);

	void update();

	void draw();
	    
	void next();

private:
	EStudyType m_eStudyType;

	std::vector<std::pair<std::experimental::filesystem::v1::path, std::string>> m_vStudyDatasets;
	std::queue<InitializableEventBehavior*> m_qTrials;

	bool m_bTrialsLoaded;

	// VR Vars
	TrackedDeviceManager *m_pTDM;

	// Fishtank Vars
	DataVolume* m_pFishtankVolume;
};

