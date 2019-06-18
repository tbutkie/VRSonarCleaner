#pragma once
#include "SceneBase.h"

#include "DataVolume.h"
#include "CosmoVolume.h"
#include "DataLogger.h"
#include "Renderer.h"
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
	
	Renderer::RendererSubmission m_rs, m_rsHalo, m_rsPlane;

	bool m_bShowHalos;
	bool m_bShowStreamtubes;
	bool m_bShowPlane;

	std::vector<glm::vec3> m_vvec3Zeros;
	std::vector<std::vector<glm::vec3>> m_vvvec3ZeroLines;

private:
	void buildScalarPlane();
	glm::quat getSegmentOrientationMatrixNormalized(glm::vec3 segmentDirection, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));

	float m_fOscAmpDeg;
	float m_fOscTime;

	std::mt19937 m_RNG; // Mersenne Twister
	std::uniform_real_distribution<float> m_Distribution;

	glm::vec4 m_vec4HaloColor, m_vec4VelColorMin, m_vec4VelColorMax;
	float m_fHaloRadiusFactor;

	unsigned int m_uiCuttingPlaneGridRes, m_uiNumTubeSegments;
	float m_fCuttingPlaneWidth, m_fCuttingPlaneHeight;
	float m_fTubeRadius;

	float m_fRK4StepSize, m_fRK4StopVelocity;
	unsigned int m_uiRK4MaxPropagation_OneWay;

	bool m_bCuttingPlaneJitter;
	bool m_bCuttingPlaneSet;

	glm::vec3 m_vec3PlacedFrameDomain_x0y0;
	glm::vec3 m_vec3PlacedFrameDomain_x0y1;
	glm::vec3 m_vec3PlacedFrameDomain_x1y0;
	glm::vec3 m_vec3PlacedFrameDomain_x1y1;
	glm::mat4 m_mat4PlacedFrameWorldPose;

	glm::vec3 m_vec3ActiveFrameWorld_x0y0;
	glm::vec3 m_vec3ActiveFrameWorld_x0y1;
	glm::vec3 m_vec3ActiveFrameWorld_x1y0;
	glm::vec3 m_vec3ActiveFrameWorld_x1y1;


	struct StudyParam {
		std::string desc;
		std::string buf;
		uint16_t format;
	};

	std::vector<StudyParam> m_vParams;
	StudyParam* m_pEditParam;
};

