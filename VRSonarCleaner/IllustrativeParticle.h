#ifndef __IllustrativeParticle_h__
#define __IllustrativeParticle_h__

#include <windows.h>
#include "FlowGrid.h"
#include <vector>
#include "Vec3.h"

#define MAX_NUM_POSITIONS 100

class IllustrativeParticle
{
public:
	IllustrativeParticle(float x, float y, float z, float TimeToLive, float TrailTime, ULONGLONG currentTime);
	virtual ~IllustrativeParticle();

	void updatePosition(ULONGLONG currentTime, float newX, float newY, float newZ); //true if still in bounds, false if needs to be deleted

	void reset();
	void reset(float x, float y, float z);
	void kill();
	//bool updated;
	ULONGLONG birthTime;
	float timeToLive;
	ULONGLONG timeToStartDying;
	float trailTime;

	float getLastSpeed();
	float getOldLastSpeed();
	float lastSpeed;
	float oldLastSpeed;

	float latestPosition[3];
	
	float getCurrentX();
	float getCurrentY();
	float getCurrentZ();
	void getCurrentXYZ(float *x, float *y, float *z);

	
	float getDyingOpacity();

	float getFadeInFadeOutOpacity();

	bool dead;
	bool dying;
	ULONGLONG timeOfDeath;

	bool userCreated;
	//int color;

	float color[3];
	void getColor(float *r, float *g, float *b);

	float gravity;

	float speedFactor; //100 = normal

	float startingPosition[3];
	float positions[3*MAX_NUM_POSITIONS];
	ULONGLONG times[MAX_NUM_POSITIONS];

	int liveStartIndex;
	int liveEndIndex;
	ULONGLONG liveTimeElapsed;
	ULONGLONG lastUpdateTimestamp;
	
	int getNumLivePositions();
	int getLivePosition(int index);

	float temperatureColor[3];
	//void draw();

	//Index of which flowGrid in the flowGridCollection it is within
	int flowGridIndex;
	void setFlowGridIndex(int index);
	int getFlowGridIndex();

private:

	//update() vars, put them here so no alloc needed each frame
	ULONGLONG timeSinceLast;
	float U, V, T, S;
	float rand1, rand2, rand3;
	bool inWater;
	float currentPos[3];
	float newPos[3];
	ULONGLONG timeSince;
	bool foundValid;
};

#endif