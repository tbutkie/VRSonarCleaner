#ifndef __IllustrativeParticle_h__
#define __IllustrativeParticle_h__

#include <windows.h>                                                                         
#include <vector>

#include <shared/glm/glm.hpp>

// number of particle positions to store for things like trails, etc.
#define MAX_NUM_POSITIONS 100

class IllustrativeParticle
{
public:
	IllustrativeParticle(float x, float y, float z, float TimeToLive, float TrailTime, ULONGLONG currentTime);
	virtual ~IllustrativeParticle();

	void updatePosition(ULONGLONG currentTime, float newX, float newY, float newZ);

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
	void getCurrentXYZ(float *x, float *y, float *z);
	 
	float getFadeInFadeOutOpacity();

	bool m_bDead;
	bool m_bDying;
	ULONGLONG m_ullTimeOfDeath;

	bool m_bUserCreated;
	//int color;

	glm::vec3 m_vec3Color;
	void getColor(float *r, float *g, float *b);

	float m_fGravity;

	glm::vec3 m_vec3StartingPosition;
	std::vector<glm::vec3> m_vvec3Positions;
	std::vector<ULONGLONG> m_vullTimes;

	int m_iBufferTail;
	int m_iBufferHead;
	ULONGLONG m_ullLiveTimeElapsed;
	ULONGLONG m_ullLastUpdateTimestamp;
	
	int getNumLivePositions();
	int getLivePosition(int index);
	
	//Index of which flowGrid in the flowGridCollection it is within
	int m_iFlowGridIndex;
	void setFlowGridIndex(int index);
	int getFlowGridIndex();

private:

	//update() vars, put them here so no alloc needed each frame
	ULONGLONG m_ullTimeSince;
	bool foundValid;
};

#endif