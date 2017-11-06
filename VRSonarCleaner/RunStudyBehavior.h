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
	RunStudyBehavior(TrackedDeviceManager* pTDM, bool sitting);
	RunStudyBehavior(Renderer::SceneViewInfo *pSceneInfo, glm::ivec4 &viewport, Renderer::Camera *pCamera);
	~RunStudyBehavior();

	void init();

	void update();

	void draw();

	enum STUDY_TYPE {
		VR_STANDING,
		VR_SITTING,
		DESKTOP
	};

private:
	STUDY_TYPE m_eStudyType;

	std::vector<std::pair<std::experimental::filesystem::v1::path, std::string>> m_vStudyDatasets;
	std::queue<InitializableBehavior*> m_qTrials;

	std::future<void> m_Future;

	bool m_bTrialsLoaded;

	// VR Vars
	TrackedDeviceManager *m_pTDM;

	// Desktop vars
	Renderer::SceneViewInfo *m_pDesktop3DViewInfo;
	glm::ivec4 m_ivec4Viewport;
	Renderer::Camera *m_pCamera;
};

