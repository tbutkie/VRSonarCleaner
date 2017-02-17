#pragma once

#include <shared/glm/glm.hpp>

struct Particle
{
	glm::vec3 m_vec3Pos;            // Position
	glm::vec3 m_vec3Vel;            // Velocity
	glm::vec4 m_vec4Col;            // Color
	float m_fSize;                  // Size
	unsigned long long m_ullEnergy; // Energy / Time till death
};

class ParticleSystem
{
public:
	ParticleSystem(int numParticles, glm::vec3 pos,	int type);
	virtual ~ParticleSystem();

	virtual bool update(float time) = 0;

	virtual void setParticleDefaults(Particle &p);

protected:
	int m_nParticles;
	int type;
	glm::vec3 m_vec3Pos;

	unsigned long long m_ullLastUpdateTime; 
};

