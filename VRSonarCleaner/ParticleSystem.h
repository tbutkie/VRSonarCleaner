#pragma once

#include <vector>
#include <shared/glm/glm.hpp>

#define PARTICLE_SYSTEM_MIN_UPDATE_INTERVAL 20

struct Particle
{
	int m_iID;                      // Particle ID
	glm::vec3 m_vec3Pos;            // Position
	glm::vec3 m_vec3Vel;            // Velocity
	glm::vec4 m_vec4Col;            // Color
	float m_fSize;                  // Size
	unsigned long long m_ullDeathTime; // Energy / Time till death
	bool m_bDead;                  // Is this particle currently dead?
	bool m_bDying;                  // Is this particle currently dieing?
};

enum ParticleSystemType {
	CUSTOM = 0,
	STREAKLET
};

class ParticleSystem
{
	friend class ParticleManager;

public:
	ParticleSystem(int numParticles, glm::vec3 pos, ParticleSystemType type);
	virtual ~ParticleSystem();

	virtual bool update(float time) = 0;

	virtual void setParticleDefaults(Particle &p);
	virtual std::vector<int> releaseParticles();

protected:
	std::vector<Particle*> m_vpParticles;
	int m_nParticles;
	glm::vec3 m_vec3Pos;
	ParticleSystemType m_iType;

	unsigned long long m_ullLastUpdateTime; 
};

