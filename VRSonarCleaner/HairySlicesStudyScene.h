#pragma once
#include "SceneBase.h"

#include "DataVolume.h"
#include "CosmoVolume.h"
#include "DataLogger.h"
#include "Renderer.h"
#include "HairySlice.h"
#include "HairyVolume.h"
#include "TrackedDeviceManager.h"
#include <random>
#include <chrono>

#define STUDYPARAM_DECIMAL	1 << 0
#define STUDYPARAM_POSNEG	1 << 1
#define STUDYPARAM_ALPHA	1 << 2
#define STUDYPARAM_NUMERIC	1 << 3
#define STUDYPARAM_IP		1 << 4
#define STUDYPARAM_RGB		1 << 5
#define STUDYPARAM_RGBA		1 << 6

class HairySlicesStudyScene :
	public Scene
{
	struct StudyCondition {
		std::string geometry; // CONE, TUBE, LINE
		std::string texture; // STATIC, ANIMATED
		bool stereo;
		bool motion;
	};

public:
	HairySlicesStudyScene(float displayDiagonalInches, TrackedDeviceManager* pTDM);
	~HairySlicesStudyScene();

	void init();

	void processSDLEvent(SDL_Event &ev);

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;

	CosmoVolume* m_pCosmoVolume;

	HairySlice* m_pHairySlice;
	HairyVolume* m_pHairyVolume;

	bool m_bShowCosmoBBox;
	bool m_bShowProbe;
	bool m_bStudyActive;
	bool m_bTraining;
	bool m_bShowCondition;
	bool m_bShowError;
	bool m_bPaused;
	bool m_bStudyComplete;

	bool m_bCalibrated;

	float m_fScreenDiagonalInches;
	float m_fEyeSeparationCentimeters;

	bool m_bStereo;

	int m_nCurrentTrial;
	int m_nReplicatesPerCondition;
	int m_nCurrentReplicate;

	std::string m_strParticipantName;

	glm::mat4 m_mat4TrackingToScreen;
	glm::vec3 m_vec3ProbeDirection;

	struct StudyParam {
		std::string desc;
		std::string buf;
		uint16_t format;
	};

	std::vector<StudyParam> m_vParams;
	StudyParam* m_pEditParam;

	std::vector<StudyCondition> m_vStudyConditions;
	StudyCondition* m_pCurrentCondition;

private:
	void setupParameters();
	void setupViews();

	void startTraining();

	void makeStudyConditions();
	void startStudy();
	void loadStudyCondition();

	void startBreak();
	void endBreak();

	void recordResponse();

	void randomData();

	bool calibrateTracker();
};

