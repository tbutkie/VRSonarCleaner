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

class CosmoStudyTrialDesktopScene :
	public Scene
{
public:
	CosmoStudyTrialDesktopScene();
	~CosmoStudyTrialDesktopScene();

	void init();

	void processSDLEvent(SDL_Event &ev);

	void update();

	void draw();

private:
	CosmoVolume* m_pCosmoVolume;

	HairySlice* m_pHairySlice;
	
	Renderer::RendererSubmission m_rsPlane;

	bool m_bShowPlane;

	std::vector<glm::vec3> m_vvec3Zeros;
	std::vector<std::vector<glm::vec3>> m_vvvec3ZeroLines;

private:
	void setupParameters();
	void setupViews();
	void buildScalarPlane();

	float m_fOscAmpDeg;
	float m_fOscTime;

	float m_fScreenDiagonalInches;

	struct StudyParam {
		std::string desc;
		std::string buf;
		uint16_t format;
	};

	std::vector<StudyParam> m_vParams;
	StudyParam* m_pEditParam;
};

