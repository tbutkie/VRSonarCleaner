#pragma once
#include "CosmoVolume.h"
#include <random>
#include <queue>

class HairySlice
{
	friend class HairySlicesStudyScene;
	
public:
	HairySlice(CosmoVolume* cosmoVolume);
	~HairySlice();

	void update();

	void draw();

	void set();

	void nextShader();
	void prevShader();
	std::string getShaderName();
	bool setShader(std::string shaderName);

	void nextGeomStyle();
	void prevGeomStyle();
	std::string getGeomStyle();
	bool setGeomStyle(std::string geomType);
	
private:
	bool m_bProbeActive;

	bool m_bShowHalos;
	bool m_bShowGeometry;
	bool m_bShowParticles;
	bool m_bShowSeeds;
	bool m_bShowFrame;
	bool m_bShowGrid;
	bool m_bShowReticule;
	bool m_bShowTarget;

	bool m_bSpinReticule;
	bool m_bOscillate;
	bool m_bJitterSeeds;

	float m_fLastUpdate;

	CosmoVolume* m_pCosmoVolume;

	std::vector<std::vector<glm::vec3>> m_vvvec3RawStreamlines;
	std::vector<glm::vec3> m_vvec3StreamlineSeeds;

	GLuint m_glParticleVBO, m_glParticleEBO, m_glParticleVAO;
	Renderer::RendererSubmission m_rsParticle;
	unsigned int m_nParticleCount;
	float m_fParticleTail;
	float m_fParticleLifetime;
	float m_fParticleBirthTime;
	float m_fParticleDeathTime;
	float* m_arrfParticleLives;
	std::deque<std::pair<int, float>> m_qpBirthingParticles;
	std::deque<std::pair<int, float>> m_qpDyingParticles;
	std::queue<int> m_qiDeadParticles;
	glm::vec4 m_vec4ParticleHeadColor;
	glm::vec4 m_vec4ParticleTailColor;

	GLuint m_glVBO, m_glEBO, m_glVAO;
	GLuint m_glHaloVBO, m_glHaloVAO;
	GLuint m_glReticuleVBO, m_glReticuleEBO, m_glReticuleVAO;

	Renderer::RendererSubmission m_rs, m_rsHalo, m_rsReticule;
	
	std::mt19937 m_RNG; // Mersenne Twister
	std::uniform_real_distribution<float> m_Distribution;

	glm::vec4 m_vec4HaloColor, m_vec4VelColorMin, m_vec4VelColorMax;
	float m_fHaloRadiusFactor;

	glm::quat m_qPlaneOrientation;
	unsigned int m_uiCuttingPlaneGridRes, m_uiNumTubeSegments;
	glm::vec2 m_vec2CuttingPlaneSize;
	float m_fTubeRadius;
	
	float m_fOscAmpDeg;
	float m_fOscTime;

	glm::vec3 m_vec3Reticule;
	glm::vec3 m_vec3FlowAtReticule;

	glm::vec3 m_vec3PlacedFrame_x0y0;
	glm::vec3 m_vec3PlacedFrame_x0y1;
	glm::vec3 m_vec3PlacedFrame_x1y0;
	glm::vec3 m_vec3PlacedFrame_x1y1;
	glm::mat4 m_mat4PlacedFrameWorldPose;

	glm::vec3 m_vec3ActiveFrameWorld_x0y0;
	glm::vec3 m_vec3ActiveFrameWorld_x0y1;
	glm::vec3 m_vec3ActiveFrameWorld_x1y0;
	glm::vec3 m_vec3ActiveFrameWorld_x1y1;

	float m_fRK4StepSize, m_fRK4StopVelocity;
	unsigned int m_uiRK4MaxPropagation_OneWay;

	std::vector<std::string> m_vstrShaderNames;
	int m_iCurrentShader;
	
	std::vector<std::string> m_vstrGeomStyleNames;
	int m_iCurrentGeomStyle;

private:
	void reseed();
	glm::vec3 randomSeedOnPlane();
	glm::vec3 randomPointOnPlane();
	void sampleCuttingPlane();
	void rebuildGeometry();
	void buildStreamLines();
	void buildStreamTubes(float radius);
	void buildStreamCones(float radius);
	void buildReticule();

	void destroyGeometry();

	void initParticles();
	void updateParticles(float elapsedTime);
	void drawParticleHeads(float radius);
	void buildParticleTails();

	glm::quat getSegmentOrientationMatrixNormalized(glm::vec3 segmentDirection, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));
};
