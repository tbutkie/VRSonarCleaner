#include "StreakletSystem.h"

#include <Windows.h> // for GetTickCount64() and ULONGLONG

StreakletSystem::StreakletSystem(int numParticles, int streakLength, glm::vec3 pos, ConstructionInfo *ci)
	: ParticleSystem(numParticles * (streakLength + 1), pos, STREAKLET)
	, m_fParticleVelocityScale(0.1f)
{
}


StreakletSystem::~StreakletSystem()
{
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
				updatePosition(p, tick);
			}
			else if (m_vpParticles[i]->m_iFlowGridIndex > -1) //if attached to a flow grid
			{
				//get current position
				p->m_vec3Pos;
				glm::vec3 flow;
				//get UVW at current position
				m_vpFlowGridCollection.at(0)->getUVWat(p->m_vec3Pos.x, p->m_vec3Pos.y, p->m_vec3Pos.z, time, &flow.x, &flow.y, &flow.z);
				//printf("Result: %d, U: %f, V: %f\n", result, u ,v);
				//calc new position
				float prodTimeVelocity = static_cast<float>(timeSinceLastUpdate) * m_fParticleVelocityScale;
				//printf("Current Pos: %f, %f, %f\n", x, y, z);
				glm::vec3 newPos = p->m_vec3Pos + flow * prodTimeVelocity;
				newPos.z += m_vpParticles[i]->m_fGravity / (1000ull * timeSinceLastUpdate);

				//check in bounds or not
				if (!m_vpFlowGridCollection.at(m_vpParticles[i]->m_iFlowGridIndex)->contains(newPos[0], newPos[1], newPos[2]))
				{
					//no begin killing off particle
					//printf("OOB: ", newPos[0], newPos[1], newPos[2]);
					m_vpParticles[i]->m_bDying = true;
				}
				//printf("P %d, Pos: %f, %f, %f\n", i, newPos[0], newPos[1], newPos[2]);
				m_vpParticles[i]->updatePosition(tick, newPos[0], newPos[1], newPos[2]);

			}//end if attached to flow grid
			else //not attached to flow grid
			{
				//if there are flow grids, check if in one of them
				bool foundGrid = false;
				if (m_vpFlowGridCollection.size() > 0)
				{
					//get current position
					m_vpParticles[i]->getCurrentXYZ(&x, &y, &z);
					//check in bounds or not
					for (int gridID = 0; gridID < m_vpFlowGridCollection.size(); ++gridID)
					{
						if (m_vpFlowGridCollection.at(gridID)->contains(x, y, z))
						{
							//found one
							m_vpParticles[i]->m_iFlowGridIndex = gridID;
							//printf("particle %d moved to grid %d\n", i, grid);
							//update
							//get UVW at current position
							result = m_vpFlowGridCollection.at(gridID)->getUVWat(x, y, z, time, &u, &v, &w);
							//calc new position
							prodTimeVelocity = timeSinceLastUpdate*m_vpFlowGridCollection.at(gridID)->illustrativeParticleVelocityScale;
							newPos[0] = x + u*prodTimeVelocity;
							newPos[1] = y + v*prodTimeVelocity;
							newPos[2] = z + w*prodTimeVelocity + (m_vpParticles[i]->m_fGravity / prodTimeSeconds);
							m_vpParticles[i]->updatePosition(tick, newPos[0], newPos[1], newPos[2]);
							foundGrid = true;
							break;
						}
					}//end for each grid					
				}//end if flow grids to check

				if (!foundGrid)
				{
					//if there are flow grids, lets kill it, so it doesn't clutter the scene
					if (m_vpFlowGridCollection.size() > 0)
					{
						m_vpParticles[i]->kill();
					}
					else
					{
						//if no flow grids, just let it sit there and die out naturally
						m_vpParticles[i]->getCurrentXYZ(&x, &y, &z);
						newPos[0] = x;
						newPos[1] = y;
						newPos[2] = z + (m_vpParticles[i]->m_fGravity / prodTimeSeconds); //only gravity for this one
						m_vpParticles[i]->updatePosition(tick, newPos[0], newPos[1], newPos[2]);
					}
				}

			}//end else not attached to flow grid
		}//end if not dead
	}
	return allParticlesDead;
}

void StreakletSystem::updatePosition(Particle *p, unsigned long long currentTime)
{

}