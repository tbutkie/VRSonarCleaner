#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(int numParticles, glm::vec3 pos, int type)
	: m_nParticles(numParticles)
	, m_vec3Pos(pos)	
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::setParticleDefaults(Particle & p)
{
	p.m_vec3Pos = glm::vec3();
	p.m_vec3Vel = glm::vec3();
	p.m_vec4Col = glm::vec4(1.f, 1.f, 1.f, 0.f);	
	p.m_fSize = 0.f;
	p.m_ullEnergy = 0ull;
}
