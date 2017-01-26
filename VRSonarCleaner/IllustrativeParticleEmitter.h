#ifndef __IllustrativeParticleEmitter_h__
#define __IllustrativeParticleEmitter_h__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include "CoordinateScaler.h"
#include "ColorsAndSizes.h"

class IllustrativeParticleEmitter
{
public:
	IllustrativeParticleEmitter(float xLoc, float yLoc, float DepthBottom, float DepthTop, CoordinateScaler *Scaler);
	virtual ~IllustrativeParticleEmitter();

	void drawSmall3D();

	void changeColor(int Color);
	void incrementColor();
	void decrementColor();
	int getColor();
	void changeSpread(float Radius);
	void setRate(float ParticlesPerSecond);
	float getRate();
	void setLifetime(float time);
	void setTrailTime(float time);
	float getTrailTime();
	float getRadius();
	void setRadius(float rad);

	int getNumParticlesToEmit(float tickCount);
	float* getParticlesToEmit(int number);

	bool isPointInGlyph(float X, float Y);

	float getLifetime();

	float trailTime;

	void drawGlyph(float X, float Y, float Width, float Height);
	void drawRoundedGlyph(float X, float Y, float Width, float Height, float roundingHeight, bool selected);
	void setColor();
	void setMutedColor();
	float lastGlyphX, lastGlyphY, lastGlyphWidth, lastGlyphHeight;

	void setBottom(float DepthBottom);
	void setTop(float DepthTop);

	float getBottom();
	float getTop();

	float x, y, depthBottom, depthTop;
	
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