#pragma once
#include "SceneBase.h"

#include "DataVolume.h"
#include "CosmoVolume.h"
#include "DataLogger.h"
#include "Renderer.h"
#include "HairySlice.h"
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
		std::string geometry; // CONE, TUBE, STREAMLET
		std::string texture; // STATIC, ANIMATED
		bool stereo;
		bool motion;
	};

public:
	HairySlicesStudyScene(float displayDiagonalInches);
	~HairySlicesStudyScene();

	void init();

	void processSDLEvent(SDL_Event &ev);

	void update();

	void draw();

private:
	CosmoVolume* m_pCosmoVolume;

	HairySlice* m_pHairySlice;
	
	Renderer::RendererSubmission m_rsPlane;

	bool m_bShowCosmoBBox;
	bool m_bShowPlane;
	bool m_bStudyActive;

	float m_fScreenDiagonalInches;
	float m_fEyeSeparationMeters;

	bool m_bStereo;

	int m_nReplicatesPerCondition;
	int m_nCurrentReplicate;

	std::string m_strParticipantName;

	struct StudyParam {
		std::string desc;
		std::string buf;
		uint16_t format;
	};

	std::vector<StudyParam> m_vParams;
	StudyParam* m_pEditParam;

	std::vector<StudyCondition> m_vStudyConditions;

private:
	void setupParameters();
	void setupViews();
	void buildScalarPlane();
	
	void makeStudyConditions();
	void loadStudyCondition();
	void startStudy();

	void randomData();
};

