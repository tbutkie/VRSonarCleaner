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

#define PARTICLE_SYSTEM_MIN_UPDATE_INTERVAL 20

#define MAX_PARTICLES 10000

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
	void drawStreakVBOs();

private:
	// Holds all particles and their trails as moving coordinate frames
	// with the forward vector (mat4()[2]) of the frame scaled to match the
	// magnitude of the velocity at that particle position
	glm::mat4 m_rmat4ParticleBuffer[MAX_PARTICLES * MAX_NUM_TRAIL_POSITIONS];
	glm::vec3 m_rvec34ColorBuffer[MAX_PARTICLES * MAX_NUM_TRAIL_POSITIONS];
};

#endif