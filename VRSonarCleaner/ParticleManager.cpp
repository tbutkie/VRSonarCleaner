#include "ParticleManager.h"

#include <algorithm>
#include <iterator>

// CTOR
ParticleManager::ParticleManager()
{
	_init();
}

void ParticleManager::_init()
{
	// populate free particle index pool
	for (int i = MAX_NUM_PARTICLES - 1; i >= 0; --i)
	{
		m_rParticles[i].m_iID = i;
	}
	
	// initialize dynamic buffer
	glm::mat4 initBufferVals;
	initBufferVals[3].w = 0.f;
	std::fill_n(m_rmat4DynamicBuffer, MAX_NUM_PARTICLES, initBufferVals);
}

void ParticleManager::add(ParticleSystem * ps)
{
	// check if we have enough free particles available for this system
	if (ps->m_nParticles > (MAX_NUM_PARTICLES - m_iLiveParticleCount))
	{
		std::cerr << "Not enough free particles to add particle system!" << std::endl;
		std::cerr << "\t" << ps->m_nParticles << " requested, " << (MAX_NUM_PARTICLES - m_iLiveParticleCount) << " available" << std::endl;
		return;
	}

	// add to the group of particle systems we will manage
	m_vpParticleSystems.push_back(ps);

	for (int i = 0; i < ps->m_nParticles; ++i)
	{
		ps->setParticleDefaults(m_rParticles[m_iLiveParticleCount++]);
	}
}

void ParticleManager::remove(ParticleSystem * ps)
{
	// get the indices of the freed particles
	std::vector<int> freedParticleIndices = ps->releaseParticles();

	// swap dead particles for live ones at the end of the array index
	for (auto const &i : freedParticleIndices)
	{
		Particle dead = m_rParticles[i]; // get dead particle
		m_rParticles[i] = m_rParticles[m_iLiveParticleCount - 1]; // replace dead particle with last live particle
		m_rParticles[m_iLiveParticleCount - 1] = dead; // put the dead particle at the end of the array
		m_iLiveParticleCount--; // decrement number of live particles
	}

	// remove particle system
	m_vpParticleSystems.erase(std::remove(m_vpParticleSystems.begin(), m_vpParticleSystems.end(), ps), m_vpParticleSystems.end());
}

bool ParticleManager::exists(ParticleSystem * ps) const
{
	return std::find(m_vpParticleSystems.begin(), m_vpParticleSystems.end(), ps) != m_vpParticleSystems.end();
}

void ParticleManager::update(float time)
{
	for (auto &ps : m_vpParticleSystems)
		if (!ps->update(time))
			remove(ps);
}
