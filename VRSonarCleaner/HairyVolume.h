#pragma once
#include "CosmoVolume.h"
#include <random>
#include <queue>

class HairyVolume
{
	friend class HairySlicesStudyScene;
	
public:
	HairyVolume(CosmoVolume* cosmoVolume);
	~HairyVolume();

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

	bool m_bShowGeometry;
	bool m_bShowParticles;
	bool m_bShowSeeds;

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

	Renderer::RendererSubmission m_rs;
	
	std::mt19937 m_RNG; // Mersenne Twister
	std::uniform_real_distribution<float> m_Distribution;

	glm::vec4 m_vec4VelColorMin, m_vec4VelColorMax;

	unsigned int m_uiSamplingResolution;

	unsigned int m_uiNumTubeSegments;
	float m_fTubeRadius;
	
	glm::quat m_qVolumeOrientation;
	float m_fOscAmpDeg;
	float m_fOscTime;

	float m_fRK4StepSize, m_fRK4StopVelocity;
	unsigned int m_uiRK4MaxPropagation_OneWay;

	std::vector<std::string> m_vstrShaderNames;
	int m_iCurrentShader;
	
	std::vector<std::string> m_vstrGeomStyleNames;
	int m_iCurrentGeomStyle;

private:
	void reseed();
	void sampleVolume();
	void rebuildGeometry();
	void buildStreamLines();
	void buildStreamTubes(float radius);
	void buildStreamCones(float radius);

	void destroyGeometry();

	void initParticles();
	void updateParticles(float elapsedTime);
	void drawParticleHeads(float radius);
	void buildParticleTails();

	glm::quat getSegmentOrientationMatrixNormalized(glm::vec3 segmentDirection, glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));
};
