#include "ParticleManager.h"

#include <algorithm>

// CTOR
ParticleManager::ParticleManager()
{
	_init();
}

void ParticleManager::_init()
{
	// populate free particle index pool
	for (int i = MAX_NUM_PARTICLES - 1; i >= 0; --i)
		m_viFreeParticles.push_back(i);
	
	// initialize dynamic buffer
	glm::mat4 initBufferVals;
	initBufferVals[3].w = 0.f;
	std::fill_n(m_rmat4DynamicBuffer, MAX_NUM_PARTICLES, initBufferVals);
}

void ParticleManager::add(ParticleSystem * ps)
{
	m_vpParticleSystems.push_back(ps);
}

void ParticleManager::remove(ParticleSystem * ps)
{
	m_vpParticleSystems.erase(std::remove(m_vpParticleSystems.begin(), m_vpParticleSystems.end(), ps), m_vpParticleSystems.end());
}

bool ParticleManager::exists(ParticleSystem * ps) const
{
	return std::find(m_vpParticleSystems.begin(), m_vpParticleSystems.end(), ps) != m_vpParticleSystems.end();
}

void ParticleManager::getParticles(int num, std::vector<Particle*>& particles)
{
	for (int i = 0; i < num; ++i)
	{
		particles.push_back(&m_rParticles[m_viFreeParticles.back()]);
		m_viFreeParticles.pop_back();
	}
}

void ParticleManager::update(float time)
{
	for (auto &ps : m_vpParticleSystems)
		if (!ps->update(time))
			remove(ps);
}
