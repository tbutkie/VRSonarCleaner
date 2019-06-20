#pragma once
#include "CosmoVolume.h"
#include <random>

class HairySlice
{
public:
	enum SEEDTYPE {
		STREAMLET_ANIMATED,
		STREAMLET_STATIC,
		STREAMTUBE_STATIC,
		STREAMTUBE_ANIMATED,
		STREAMTUBE_HALO,
		STREAMCONE
	};

public:
	HairySlice(CosmoVolume* cosmoVolume);
	~HairySlice();

	void update();

	void draw();

	void set();

	void nextShader();
	void prevShader();
	std::string getShaderName();

private:
	bool m_bProbeActive;

	bool m_bShowHalos;
	bool m_bShowStreamtubes;
	bool m_bCuttingPlaneJitter;
	bool m_bCuttingPlaneSet;

	CosmoVolume* m_pCosmoVolume;

	SEEDTYPE m_eSeedType;

	std::vector<std::vector<glm::vec3>> m_vvvec3RawStreamlines;
	std::vector<glm::vec3> m_vvec3StreamlineSeedsDomain;

	GLuint m_glVBO, m_glEBO, m_glVAO;
	GLuint m_glHaloVBO, m_glHaloVAO;

	Renderer::RendererSubmission m_rs, m_rsHalo;
	
	std::mt19937 m_RNG; // Mersenne Twister
	std::uniform_real_distribution<float> m_Distribution;

	glm::vec4 m_vec4HaloColor, m_vec4VelColorMin, m_vec4VelColorMax;
	float m_fHaloRadiusFactor;

	unsigned int m_uiCuttingPlaneGridRes, m_uiNumTubeSegments;
	float m_fCuttingPlaneWidth, m_fCuttingPlaneHeight;
	float m_fTubeRadius;

	glm::vec3 m_vec3PlacedFrameDomain_x0y0;
	glm::vec3 m_vec3PlacedFrameDomain_x0y1;
	glm::vec3 m_vec3PlacedFrameDomain_x1y0;
	glm::vec3 m_vec3PlacedFrameDomain_x1y1;
	glm::mat4 m_mat4PlacedFrameWorldPose;

	glm::vec3 m_vec3ActiveFrameWorld_x0y0;
	glm::vec3 m_vec3ActiveFrameWorld_x0y1;
	glm::vec3 m_vec3ActiveFrameWorld_x1y0;
	glm::vec3 m_vec3ActiveFrameWorld_x1y1;

	float m_fRK4StepSize, m_fRK4StopVelocity;
	unsigned int m_uiRK4MaxPropagation_OneWay;

	std::vector<std::string> m_vstrShaderNames;
	int m_iCurrentShader;

private:
	void reseed();
	void sampleCuttingPlane();
	void sampleVolume(unsigned int gridRes = 10u);
	void buildStreamTubes();
	void buildStreamCones(float coneEnlargementFactor = 5.f);
	void buildStreamlets();

	glm::quat getSegmentOrientationMatrixNormalized(glm::vec3 segmentDirection, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));
};
