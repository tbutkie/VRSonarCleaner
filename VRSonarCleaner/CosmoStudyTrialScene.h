#pragma once
#include "SceneBase.h"

#include "TrackedDeviceManager.h"
#include "DataVolume.h"
#include "CosmoVolume.h"
#include "DataLogger.h"
#include "Renderer.h"
#include <random>

#define STUDYPARAM_DECIMAL	1 << 0
#define STUDYPARAM_POSNEG	1 << 1
#define STUDYPARAM_ALPHA	1 << 2
#define STUDYPARAM_NUMERIC	1 << 3
#define STUDYPARAM_IP		1 << 4
#define STUDYPARAM_RGB		1 << 5
#define STUDYPARAM_RGBA		1 << 6

class CosmoStudyTrialScene :
	public Scene
{
public:
	CosmoStudyTrialScene(TrackedDeviceManager* pTDM);
	~CosmoStudyTrialScene();

	void init();

	void processSDLEvent(SDL_Event &ev);

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;
	CosmoVolume* m_pCosmoVolume;

	std::vector<std::vector<glm::vec3>> m_vvvec3RawStreamlines;
	std::vector<glm::vec3> m_vvec3StreamlineSeedsDomain;

	GLuint m_glVBO, m_glEBO, m_glVAO;
	GLuint m_glHaloVBO, m_glHaloVAO;
	
	Renderer::RendererSubmission m_rs, m_rsHalo;

	bool m_bShowHalos;

	std::vector<glm::vec3> m_vvec3Zeros;
	std::vector<std::vector<glm::vec3>> m_vvvec3ZeroLines;

private:
	void sampleCuttingPlane(bool reseed);
	void sampleVolume(unsigned int gridRes = 10u);
	void buildStreamTubes();
	glm::vec4 parseRGBText(std::string color);
	std::string colorString(glm::vec4 color);
	glm::quat getSegmentOrientationMatrixNormalized(glm::vec3 segmentDirection, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));

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

