#include "StreakletSystem.h"

#include <Windows.h> // for GetTickCount64()

StreakletSystem::StreakletSystem(int numParticles, glm::vec3 pos, ConstructionInfo *ci)
	: ParticleSystem(numParticles, pos, STREAKLET)
	, m_fParticleVelocityScale(0.1f)
{
	m_pDataVolume = ci->dataVolume;
	m_pFlowGrid = ci->flowGrid;
	m_pScaler = ci->scaler;

	std::random_device rand_dev;
	m_RandGenerator = std::mt19937(rand_dev());
	m_xDistrib = std::uniform_int_distribution<int>(m_pFlowGrid->getScaledXMin(), m_pFlowGrid->getScaledXMax());
	m_yDistrib = std::uniform_int_distribution<int>(m_pFlowGrid->getScaledMaxDepth(), m_pFlowGrid->getScaledMinDepth());
	m_zDistrib = std::uniform_int_distribution<int>(m_pFlowGrid->getScaledYMin(), m_pFlowGrid->getScaledYMax());
}


StreakletSystem::~StreakletSystem()
{
}

void StreakletSystem::setParticleDefaults(Particle & p)
{
	glm::vec3 randPos = glm::vec3(m_xDistrib(m_RandGenerator), m_yDistrib(m_RandGenerator), m_zDistrib(m_RandGenerator));

	p.m_vec3Pos = randPos;
	p.m_vec3LastPos = randPos;
	p.m_vec3Vel = glm::vec3();
	p.m_vec4Col = glm::vec4(1.f, 1.f, 1.f, 0.f);
	p.m_fSize = 0.f;
	p.m_ullEnergy = 10000ull; // ms
	p.m_bDead = false;
	p.m_bDying = false;

	m_vpParticles.push_back(&p);
}

bool StreakletSystem::update(float time)
{
	bool allParticlesDead = true;

	unsigned long long tick = GetTickCount64();
	unsigned long long timeSinceLastUpdate = tick - m_ullLastUpdateTime;
	//printf("time since last: %f\n", (float)timeSinceLastUpdate);

	if (timeSinceLastUpdate > 1000)
		timeSinceLastUpdate = 1000; //dont skip or jump start

	//dont update too often
	if (timeSinceLastUpdate <= PARTICLE_SYSTEM_MIN_UPDATE_INTERVAL)
		return false;
	else
		m_ullLastUpdateTime = tick;
	
	for (auto &p : m_vpParticles)
	{
		if (!p->m_bDead)
		{
			allParticlesDead = false;

			if (p->m_bDying)
			{
				updateParticle(p, timeSinceLastUpdate, NULL);
			}
			else
			{
				glm::vec3 flow;
				//get UVW at current position
				m_pFlowGrid->getUVWat(p->m_vec3Pos.x, p->m_vec3Pos.y, p->m_vec3Pos.z, time, &flow.x, &flow.y, &flow.z);
				//printf("Result: %d, U: %f, V: %f\n", result, u ,v);
				//calc new position
				float prodTimeVelocity = static_cast<float>(timeSinceLastUpdate) * m_fParticleVelocityScale;
				//printf("Current Pos: %f, %f, %f\n", x, y, z);
				glm::vec3 newPos = p->m_vec3Pos + flow * prodTimeVelocity;
				//newPos.z += p->m_fGravity / (1000ull * timeSinceLastUpdate); // Apply gravity

				//check in bounds or not
				if (!m_pFlowGrid->contains(newPos[0], newPos[1], newPos[2]))
					p->m_bDying = true;

				//printf("P %d, Pos: %f, %f, %f\n", i, newPos[0], newPos[1], newPos[2]);
				updateParticle(p, timeSinceLastUpdate, &newPos);

			}//end if attached to flow grid
		}//end if not dead
	}
	return allParticlesDead;
}

void StreakletSystem::updateParticle(Particle *p, unsigned long long elapsedTime, glm::vec3 *newPos)
{
	if (p->m_bDead)
		return;

	if (elapsedTime >= p->m_ullEnergy)
	{
		if (!p->m_bDying && !p->m_bDead)
		{
			p->m_ullEnergy = 1000ull;
			p->m_bDying = true;
		}

		p->m_ullEnergy = 0ull;
		p->m_bDead = true;
	}

	if (!p->m_bDying)
	{
		p->m_ullEnergy -= elapsedTime;

		p->m_vec3LastPos = p->m_vec3Pos;
		p->m_vec3Pos = *newPos;
	}
	else
	{
		p->m_ullEnergy -= elapsedTime;
	}

	return;
}