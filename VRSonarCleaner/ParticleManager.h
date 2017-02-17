#pragma once

#include <vector>

#include <shared/glm/glm.hpp>

#include "ShaderUtils.h"
#include "ParticleSystem.h"

#define MAX_NUM_PARTICLES 10000

class ParticleManager
{
public:
	// Singleton instance access
	static ParticleManager& getInstance()
	{
		static ParticleManager instance;
		return instance;
	}

	void add(ParticleSystem *ps);
	void remove(ParticleSystem *ps);
	bool exists(ParticleSystem *ps) const;

	void update(float time);

	void render();

	void shutdown();

private:
	ParticleManager(); // Private ctor for singleton
	void _init();

private:
	static ParticleManager *s_instance;

	std::vector<ParticleSystem*> m_vpParticleSystems;
	
	Particle m_rParticles[MAX_NUM_PARTICLES];

	// Array index of the next free particle
	int m_iLiveParticleCount;

	// The dynamic buffer holds particle information in a mat4,
	// where the upper-left 3x3 mat holds the uvw coordinate frame 
	// that encodes the particle orientation, magnitude, and scale;
	// the position is the first three rows of the last column, and
	// rgba color is the entire fourth row of the mat4
	glm::mat4 m_rmat4DynamicBuffer[MAX_NUM_PARTICLES];

// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
public:
	ParticleManager(ParticleManager const&) = delete;
	void operator=(ParticleManager const&) = delete;
};
