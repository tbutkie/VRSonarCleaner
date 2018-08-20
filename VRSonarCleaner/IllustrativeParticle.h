#ifndef __IllustrativeParticle_h__
#define __IllustrativeParticle_h__
                                                                   
#include <vector>
#include <chrono>

#include <glm.hpp>

#include "FlowGrid.h"

// number of particle positions to store for things like trails, etc.
#define MAX_NUM_TRAIL_POSITIONS 100

class IllustrativeParticle
{
public:
	IllustrativeParticle();
	virtual ~IllustrativeParticle();

	void init(glm::vec3 pos, glm::vec3 color, float gravity, std::chrono::milliseconds timeToLive, std::chrono::milliseconds trailTime, std::chrono::time_point<std::chrono::high_resolution_clock> currentTime, bool userCreated);

	void updatePosition(std::chrono::time_point<std::chrono::high_resolution_clock> currentTime, float newX, float newY, float newZ);
	void updateBufferIndices(std::chrono::time_point<std::chrono::high_resolution_clock> currentTime);

	void reset();

	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpBirthTime;
	std::chrono::milliseconds m_msTimeToLive;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpTimeToStartDying;
	std::chrono::milliseconds m_msTrailTime;
		
	float getCurrentX();
	float getCurrentY();
	float getCurrentZ();
	glm::vec3 getCurrentXYZ();
	 
	float getFadeInFadeOutOpacity();

	bool m_bDead;
	bool m_bDying;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpTimeDeathBegan;

	bool m_bUserCreated;
	//int color;

	glm::vec3 m_vec3Color;
	void getColor(float *r, float *g, float *b);

	float m_fGravity;

	glm::vec3 m_vec3StartingPosition;
	std::vector<glm::vec3> m_vvec3Positions;
	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> m_vtpTimes;

	int m_iBufferTail;
	int m_iBufferHead; // Index of the next FREE buffer slot
	std::chrono::milliseconds m_msLiveTimeElapsed;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_tpLastUpdateTimestamp;
	
	int getNumLivePositions();
	int getLivePosition(int index);
	
	//Index of which flowGrid in the flowGridCollection it is within
	FlowGrid* m_pFlowGrid;

private:
	//update() vars, put them here so no alloc needed each frame
	std::chrono::milliseconds m_msTimeSince;
	bool foundValid;
};

#endif