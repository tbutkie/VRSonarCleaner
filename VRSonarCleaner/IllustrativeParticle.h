#ifndef __IllustrativeParticle_h__
#define __IllustrativeParticle_h__

#include <windows.h>                                                                         
#include <vector>

#include <shared/glm/glm.hpp>

#include "FlowGrid.h"

// number of particle positions to store for things like trails, etc.
#define MAX_NUM_TRAIL_POSITIONS 100

class IllustrativeParticle
{
public:
	IllustrativeParticle(float x, float y, float z, float TimeToLive, float TrailTime, ULONGLONG currentTime);
	virtual ~IllustrativeParticle();

	void updatePosition(ULONGLONG currentTime, float newX, float newY, float newZ);
	void updateBufferIndices(ULONGLONG currentTime);

	void reset();
	void reset(float x, float y, float z);
	void kill();

	ULONGLONG m_ullBirthTime;
	float m_fTimeToLive;
	ULONGLONG m_ullTimeToStartDying;
	float m_fTrailTime;
		
	float getCurrentX();
	float getCurrentY();
	float getCurrentZ();
	glm::vec3 getCurrentXYZ();
	 
	float getFadeInFadeOutOpacity();

	bool m_bDead;
	bool m_bDying;
	ULONGLONG m_ullTimeDeathBegan;

	bool m_bUserCreated;
	//int color;

	glm::vec3 m_vec3Color;
	void getColor(float *r, float *g, float *b);

	float m_fGravity;

	glm::vec3 m_vec3StartingPosition;
	std::vector<glm::vec3> m_vvec3Positions;
	std::vector<ULONGLONG> m_vullTimes;

	int m_iBufferTail;
	int m_iBufferHead; // Index of the next FREE buffer slot
	ULONGLONG m_ullLiveTimeElapsed;
	ULONGLONG m_ullLastUpdateTimestamp;
	
	int getNumLivePositions();
	int getLivePosition(int index);
	
	//Index of which flowGrid in the flowGridCollection it is within
	FlowGrid* m_pFlowGrid;

private:
	int getWrappedIndex(int index);

	//update() vars, put them here so no alloc needed each frame
	ULONGLONG m_ullTimeSince;
	bool foundValid;
};

#endif