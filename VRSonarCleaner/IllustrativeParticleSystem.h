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

#define MAX_PARTICLES 100000

class IllustrativeParticleSystem
{
public:
	IllustrativeParticleSystem(CoordinateScaler *Scaler, std::vector <FlowGrid *> *FlowGridCollection);
	virtual ~IllustrativeParticleSystem();
		
	
	

	int addDyePole(double x, double y, float minZ, float maxZ);
	std::vector <IllustrativeDyePole*> dyePoles;
	void drawDyePoles();
	void deleteAllDyePoles();
	void deleteDyePole(int index);

	void addDyeParticleWorldCoords(double x, double y, double z, float r, float g, float b, float lifetime);
	void addDyeParticle(double x, double y, double z, float r, float g, float b, float lifetime);

	IllustrativeDyePole* getDyePoleClosestTo(double x, double y);

	void update(float time);
	void pause();
	void unPause();
	ULONGLONG lastParticleUpdate;
	ULONGLONG lastSeeding;
	ULONGLONG pauseTime;
	

	int numSeedsToMaintain;
	int maxNumParticles;

	//void draw();

	//vector <Particle> particles;
	IllustrativeParticle* particles[MAX_PARTICLES];
	
	CoordinateScaler *scaler;

	std::vector <FlowGrid *> *flowGridCollection;
		
	
	//int getNumParticles();
	int getNumLiveParticles();
	int getNumLiveSeeds();
	int getNumDyePoles();
	int getNumDyeEmitters();
	int getNumDyeParticles();
	int lastCountLiveParticles;
	int lastCountLiveSeeds;


	GLuint streakletPositionsVBO;
	GLuint streakletColorsVBO;
	bool streakletBuffersGenerated;
	int numStreakSegments;
	void loadStreakVBOs();
	void drawStreakVBOs();

	GLuint particlePositionsVBO;
	GLuint particleColorsVBO;
	bool particleBuffersGenerated;
	int numParticlePoints;
	void loadParticleVBOs();
	void drawParticleVBOs();



};

#endif