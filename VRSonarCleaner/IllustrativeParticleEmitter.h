#ifndef __IllustrativeParticleEmitter_h__
#define __IllustrativeParticleEmitter_h__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <GL/glew.h>
#include "CoordinateScaler.h"
#include "ColorsAndSizes.h"

#include <shared/glm/glm.hpp>

class IllustrativeParticleEmitter
{
public:
	IllustrativeParticleEmitter(float xLoc, float yLoc, float zLoc, CoordinateScaler *Scaler);
	virtual ~IllustrativeParticleEmitter();

	void drawSmall3D();

	void changeColor(int Color);
	void incrementColor();
	void decrementColor();
	void changeSpread(float Radius);
	void setRate(float ParticlesPerSecond);
	float getRate();
	void setLifetime(float time);
	void setTrailTime(float time);
	float getTrailTime();
	float getRadius();
	void setRadius(float rad);

	int getNumParticlesToEmit(float tickCount);
	std::vector<glm::vec3> getParticlesToEmit(int number);

	float getLifetime();

	float trailTime;

	glm::vec3 getColor();
	glm::vec3 getMutedColor();

	float x, y, z;
	
	CoordinateScaler *scaler;

	int color;
	float particlesPerSecond;
	float lifetime;

	float gravity;

	float getGravity();
	void setGravity(float Gravity);

	float radius;

	float lastEmission; //tick count of last emission

};

#endif