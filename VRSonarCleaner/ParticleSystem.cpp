#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(int numParticles, glm::vec3 pos, ParticleSystemType type)
	: m_nParticles(numParticles)
	, m_vec3Pos(pos)	
	, m_iType(type)
	, m_ullLastUpdateTime(0ull)
{
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::setParticleDefaults(Particle & p)
{
	p.m_vec3Pos = glm::vec3();
	p.m_vec3LastPos = glm::vec3();
	p.m_vec3Vel = glm::vec3();
	p.m_vec4Col = glm::vec4(1.f, 1.f, 1.f, 0.f);	
	p.m_fSize = 0.f;
	p.m_ullEnergy = 0ull;
	p.m_bDead = true;
	p.m_bDying = false;

	m_vpParticles.push_back(&p);
}

std::vector<int> ParticleSystem::releaseParticles()
{
	std::vector<int> releasedIndices;
	for (auto const &p : m_vpParticles)
		releasedIndices.push_back(p->m_iID);

	m_vpParticles.clear();

	return releasedIndices;
}
