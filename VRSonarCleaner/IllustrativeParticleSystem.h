#ifndef __IllustrativeParticleSystem_h__
#define __IllustrativeParticleSystem_h__

#include <GL/glew.h>
#include <vector>
#include <chrono>
#include <functional>
#include "FlowGrid.h"
#include "IllustrativeParticle.h"
#include "IllustrativeDyePole.h"
#include "IllustrativeParticleEmitter.h"
#include "Renderer.h"

#define PARTICLE_SYSTEM_MIN_UPDATE_INTERVAL 20ms

#define MAX_PARTICLES 100000

class IllustrativeParticleSystem
{
public:
	IllustrativeParticleSystem(std::vector<FlowGrid*> FlowGridCollection);
	virtual ~IllustrativeParticleSystem();

	void resetParticles();
			
	int addDyePole(double x, double y, float minZ, float maxZ);
	std::vector<IllustrativeDyePole*> m_vpDyePoles;
	std::vector<IllustrativeParticleEmitter*> m_vpDyePots;
	void drawDyePoles();
	void drawDyePots();
	void deleteAllDyePoles();
	void deleteDyePole(int index);

	void addDyeParticle(double x, double y, double z, float r, float g, float b, std::chrono::milliseconds lifetime);

	IllustrativeDyePole* getDyePoleClosestTo(double x, double y);
	IllustrativeParticleEmitter* getDyePotClosestTo(float x, float y, float z);

	void update(float time);
	void pause();
	void unPause();
	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastParticleUpdate;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpPauseTime;
	
	int m_nMaxParticles;
	std::vector<IllustrativeParticle*> m_vpParticles;

	std::vector<FlowGrid*> m_vpFlowGridCollection;

	bool m_bUseEuler;
	
	//int getNumParticles();
	int getNumLiveParticles();
	int getNumLiveSeeds();
	int getNumDyePoles();
	int getNumDyeEmitters();
	int getNumDyeParticles();
	int m_nLastCountLiveParticles;
	int m_nLastCountLiveSeeds;
	GLsizei m_nIndexCount;

	bool prepareForRender();
	GLuint getVAO();
	GLsizei getIndexCount();

	void setEulerIntegration();
	void setRK4Integration();

	int m_nStreakSegments;

private:
	// Holds all particle positions and their trails in one array and colors in another
	glm::vec3 m_arrvec3PositionsBuffer[MAX_PARTICLES * MAX_NUM_TRAIL_POSITIONS];
	glm::vec4 m_arrvec4ColorBuffer[MAX_PARTICLES * MAX_NUM_TRAIL_POSITIONS];
	GLuint m_arruiIndices[MAX_PARTICLES * MAX_NUM_TRAIL_POSITIONS];

	GLuint m_glVAO, m_glVBO, m_glEBO;
	void initGL();

	bool m_bReadyToTransferData;

	glm::vec3 eulerForward(IllustrativeParticle* particle, float time, float delta);
	glm::vec3 rk4(IllustrativeParticle* particle, float time, float delta);
};

#endif