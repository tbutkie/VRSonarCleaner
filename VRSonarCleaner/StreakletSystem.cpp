#include "StreakletSystem.h"

#include <Windows.h> // for GetTickCount64() and ULONGLONG

StreakletSystem::StreakletSystem(int numParticles, glm::vec3 pos, ConstructionInfo *ci)
	: ParticleSystem(numParticles, pos, STREAKLET)
	, m_fParticleVelocityScale(0.1f)
{
}


StreakletSystem::~StreakletSystem()
{
}

void StreakletSystem::setParticleDefaults(Particle & p)
{
	p.m_vec3Pos = glm::vec3();
	p.m_vec3LastPos = glm::vec3();
	p.m_vec3Vel = glm::vec3();
	p.m_vec4Col = glm::vec4(1.f, 1.f, 1.f, 0.f);
	p.m_fSize = 0.f;
	p.m_ullEnergy = 10000ull; // ms
	p.m_bDead = true;
	p.m_bDying = false;

	m_vpParticles.push_back(&p);
}

bool StreakletSystem::update(float time)
{
	bool allParticlesDead = false;

	unsigned long long tick = GetTickCount64();
	unsigned long long timeSinceLastUpdate = tick - m_ullLastUpdateTime;
	//printf("time since last: %f\n", (float)timeSinceLastUpdate);

	if (timeSinceLastUpdate > 1000)
		timeSinceLastUpdate = 1000; //dont skip or jump start

	//dont update too often
	if (timeSinceLastUpdate <= PARTICLE_SYSTEM_MIN_UPDATE_INTERVAL)
		return;
	else
		m_ullLastUpdateTime = tick;


	for (auto &p : m_vpParticles)
	{
		if (!p->m_bDead)
		{
			if (p->m_bDying)
			{
				updateParticle(p, tick, NULL);
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
				updateParticle(p, tick, &newPos);

			}//end if attached to flow grid
		}//end if not dead
	}
	return allParticlesDead;
}

void StreakletSystem::updateParticle(Particle *p, unsigned long long currentTime, glm::vec3 *newPos)
{
	if (p->m_bDead)
		return;

	if (currentTime >= p->m_ullTimeToStartDying && !p->m_bDying)
	{
		m_ullTimeOfDeath = currentTime;
		p->m_bDying = true;
	}

	if (!p->m_bDying)//translate particle, filling next spot in array with new position/timestamp
	{
		if (m_iBufferHead < MAX_NUM_POSITIONS)//no wrap needed, just fill next spot
		{
			m_vvec3Positions[m_iBufferHead] = m_vec3NewPos;
			m_vullTimes[m_iBufferHead] = currentTime;
			m_iBufferHead++;
		}
		else if (m_iBufferHead == 0 || m_iBufferHead == MAX_NUM_POSITIONS)//wrap around in progress or wrap around needed
		{
			m_vvec3Positions[0] = m_vec3NewPos;
			m_vullTimes[0] = currentTime;
			m_iBufferHead = 1;
		}
	}//end if not dying

	 //move liveStartIndex up past too-old positions
	if (m_iBufferTail < m_iBufferHead) //no wrap around
	{
		for (int i = m_iBufferTail; i < m_iBufferHead; i++)
		{
			m_ullTimeSince = currentTime - m_vullTimes[i];
			if (m_ullTimeSince > m_fTrailTime)
			{
				m_iBufferTail = i + 1;
				if (m_iBufferTail == MAX_NUM_POSITIONS)
				{
					m_iBufferTail = 0;
					m_iBufferHead = 0; //because liveEndIndex must have equaled MAX_NUM_POSITIONS
					break;
				}
			}
			else break;
		}
	}

	if (m_iBufferTail > m_iBufferHead) //wrap around
	{
		//check start to end of array
		foundValid = false;
		for (int i = m_iBufferTail; i < MAX_NUM_POSITIONS; i++)
		{
			m_ullTimeSince = currentTime - m_vullTimes[i];
			if (m_ullTimeSince > m_fTrailTime)
			{
				m_iBufferTail = i + 1;
				if (m_iBufferTail == MAX_NUM_POSITIONS)
				{
					m_iBufferTail = 0;
					break;
				}
			}
			else
			{
				foundValid = true;
				break;
			}
		}

		if (!foundValid)//check start to end of array
		{
			for (int i = 0; i < m_iBufferHead; i++)
			{
				m_ullTimeSince = currentTime - m_vullTimes[i];
				if (m_ullTimeSince > m_fTrailTime)
				{
					m_iBufferTail = i + 1;
				}
				else break;
			}
		}
	}

	if (m_bDying && m_iBufferTail == m_iBufferHead)
	{
		//printf("F");
		m_bDead = true;
	}

	m_ullLiveTimeElapsed = currentTime - m_vullTimes[m_iBufferTail];

	return;
}