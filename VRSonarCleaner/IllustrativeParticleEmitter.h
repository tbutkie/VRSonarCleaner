#ifndef __IllustrativeParticleEmitter_h__
#define __IllustrativeParticleEmitter_h__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <chrono>
#include <GL/glew.h>
#include "ColorsAndSizes.h"

#include <glm.hpp>

class IllustrativeParticleEmitter
{
public:
	IllustrativeParticleEmitter(float xLoc, float yLoc, float zLoc);
	virtual ~IllustrativeParticleEmitter();

	void changeColor(int Color);
	void incrementColor();
	void decrementColor();
	void changeSpread(float Radius);
	void setRate(float ParticlesPerSecond);
	float getRate();
	void setLifetime(std::chrono::milliseconds time);
	std::chrono::milliseconds getLifetime();
	void setTrailTime(std::chrono::milliseconds time);
	std::chrono::milliseconds getTrailTime();
	float getRadius();
	void setRadius(float rad);

	int getNumParticlesToEmit(std::chrono::time_point<std::chrono::high_resolution_clock> tick);
	std::vector<glm::vec3> getParticlesToEmit(int number);

	
	glm::vec3 getColor();
	glm::vec3 getMutedColor();

	float getGravity();
	void setGravity(float Gravity);
	float x, y, z;	

	int color;
	float particlesPerSecond;
	std::chrono::milliseconds m_msLifetime;
	std::chrono::milliseconds m_msTrailTime;

	float gravity;


	float radius;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastEmission; //tick count of last emission

};

#endif