#ifndef __IllustrativeDyePole_h__
#define __IllustrativeDyePole_h__

#include <windows.h>
#include <GL/glew.h>
#include <vector>
#include <chrono>
#include "IllustrativeParticleEmitter.h"
#include "CoordinateScaler.h"
#include "ColorsAndSizes.h"

class IllustrativeDyePole
{
public:
	IllustrativeDyePole(float xLoc, float yLoc, float DepthBottom, float DepthTop, CoordinateScaler *Scaler);
	virtual ~IllustrativeDyePole();

	void addEmitter(float DepthBottom, float DepthTop);
	void addDefaultEmitter();

	void deleteEmitter(int index);
	void removeAllEmitters();
	void changeEmitterColor(int emitterIndex, int color);
	void changeEmitterSpread(int emitterIndex, float radius);
	void changeEmitterRate(int emitterIndex, float msBetweenParticles);
	void changeEmitterLifetime(int emitterIndex, std::chrono::milliseconds lifetime);
	void changeEmitterTrailtime(int emitterIndex, std::chrono::milliseconds trailTime);
	int getNumEmitters();

	float getTopActualDepth();
	float getBottomActualDepth();
	float getActualDepthOf(float Z);

	//auto color in series vs categorical

	void kill();
	bool shouldBeDeleted();

	bool deleteMe;

	float y;
	float x;
	float depthBottom; //bottom: meters depth below surface
	float depthTop; //top: 0 if surface

	std::vector <IllustrativeParticleEmitter*> emitters;

	CoordinateScaler *scaler;
};

#endif