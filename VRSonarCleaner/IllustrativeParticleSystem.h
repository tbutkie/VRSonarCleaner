#ifndef __IllustrativeParticleSystem_h__
#define __IllustrativeParticleSystem_h__

#include <windows.h>
#include <GL/glew.h>
#include "FlowGrid.h"
#include <vector>
#include "IllustrativeParticle.h"
#include "IllustrativeDyePole.h"
#include "IllustrativeParticleEmitter.h"
#include "CoordinateScaler.h"
#include "Renderer.h"

#define PARTICLE_SYSTEM_MIN_UPDATE_INTERVAL 20

#define MAX_PARTICLES 20000

class IllustrativeParticleSystem
{
public:
	IllustrativeParticleSystem(CoordinateScaler *Scaler, std::vector<FlowGrid*> FlowGridCollection);
	virtual ~IllustrativeParticleSystem();
			
	int addDyePole(double x, double y, float minZ, float maxZ);
	std::vector<IllustrativeDyePole*> m_vpDyePoles;
	std::vector<IllustrativeParticleEmitter*> m_vpDyePots;
	void drawDyePoles();
	void drawDyePots();
	void deleteAllDyePoles();
	void deleteDyePole(int index);

	void addDyeParticleWorldCoords(double x, double y, double z, float r, float g, float b, float lifetime);
	void addDyeParticle(double x, double y, double z, float r, float g, float b, float lifetime);

	IllustrativeDyePole* getDyePoleClosestTo(double x, double y);
	IllustrativeParticleEmitter* getDyePotClosestTo(float x, float y, float z);

	void update(float time);
	void pause();
	void unPause();
	ULONGLONG m_ullLastParticleUpdate;
	ULONGLONG m_ullPauseTime;
	
	int m_nMaxParticles;

	//void draw();

	//vector <Particle> particles;
	std::vector<IllustrativeParticle*> m_vpParticles;
	
	CoordinateScaler *m_pScaler;

	std::vector<FlowGrid*> m_vpFlowGridCollection;
		
	
	//int getNumParticles();
	int getNumLiveParticles();
	int getNumLiveSeeds();
	int getNumDyePoles();
	int getNumDyeEmitters();
	int getNumDyeParticles();
	int m_nLastCountLiveParticles;
	int m_nLastCountLiveSeeds;

	int m_nStreakSegments;
	bool prepareForRender(Renderer::RendererSubmission &rs);

private:
	// Holds all particle positions and their trails in one array and colors in another
	std::vector<glm::vec3> m_vvec3PositionsBuffer;
	std::vector<glm::vec4> m_vvec4ColorBuffer;
	std::vector<GLuint> m_vuiIndices;

	GLuint m_glVAO, m_glVBO, m_glEBO;
	void initGL();
};

#endif