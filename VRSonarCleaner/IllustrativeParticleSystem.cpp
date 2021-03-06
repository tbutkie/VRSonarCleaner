#include "IllustrativeParticleSystem.h"

//NOTE: modified from VTT4D to use x,z lat/long and y as depth dimension!

#include <glm.hpp>
#include <map>

using namespace std::chrono_literals;

IllustrativeParticleSystem::IllustrativeParticleSystem(std::vector<FlowGrid*> FlowGridCollection)
	: m_bReadyToTransferData(false)
	, m_bUseEuler(false)
{
	m_vpFlowGridCollection = FlowGridCollection;

	m_tpLastParticleUpdate = std::chrono::high_resolution_clock::now();
	
	m_nMaxParticles = MAX_PARTICLES;

	printf("Initializing particle system...");
	m_vpParticles.resize(MAX_PARTICLES);
	for (auto &particle : m_vpParticles)
	{
		particle = new IllustrativeParticle();
	}

	initGL();

	printf("Done!\n");
}

IllustrativeParticleSystem::~IllustrativeParticleSystem()
{

}

void IllustrativeParticleSystem::resetParticles()
{
	for (auto p : m_vpParticles)
		p->reset();
}

void IllustrativeParticleSystem::addDyeParticle(double x, double y, double z, float r, float g, float b, std::chrono::milliseconds lifetime)
{
	//find particle to replace:
	int deadParticleToReplace = -1;
	int nonUserParticle = -1;
	for (int i=0;i<m_nMaxParticles;i++)
	{
		if (m_vpParticles[i]->m_bDead)
		{
			deadParticleToReplace = i;
			break;
		}
		else if (!m_vpParticles[i]->m_bUserCreated && nonUserParticle < 0)
		{
			nonUserParticle = i;
		}
	}

	int particleIndexToReplace = deadParticleToReplace >= 0 ? deadParticleToReplace : nonUserParticle;

	if (particleIndexToReplace == -1)
	{
		printf("YOU SHOULD NEVER SEE THIS! 18515 addDyeParticle()");
		particleIndexToReplace = 0;
	}

	//randomize the lifetimes by +\- 25% so they dont all die simultaneously	
	lifetime = std::chrono::duration_cast<std::chrono::milliseconds>(lifetime * ((float)(rand() % 25) / 100.f) + lifetime * 0.75f);				
	
	std::chrono::time_point<std::chrono::high_resolution_clock> tick = std::chrono::high_resolution_clock::now();

	IllustrativeParticle* p = m_vpParticles[particleIndexToReplace];

	p->m_bDead = false;
	p->m_bDying = false;
	p->m_bUserCreated = true;
	p->m_fGravity = 0;
	p->m_msTimeToLive;
	p->m_msTrailTime = 1000ms;
	p->m_iBufferHead = 1;
	p->m_iBufferTail = 0;
	p->m_pFlowGrid = NULL;
	p->m_tpBirthTime = tick;
	p->m_tpLastUpdateTimestamp;
	p->m_msLiveTimeElapsed = 0ms;
	p->m_tpTimeDeathBegan = std::chrono::time_point<std::chrono::high_resolution_clock>();
	p->m_tpTimeToStartDying = tick + lifetime;
	p->m_vec3Color = glm::vec3(0.25f, 0.95f, 1.f);
	p->m_vec3StartingPosition = glm::vec3(x, y, z);
	p->m_vtpTimes.clear();
	p->m_vtpTimes[0] = tick;
	p->m_vvec3Positions.clear();
	p->m_vvec3Positions[0] = p->m_vec3StartingPosition;
	p->m_vvec3Positions[1] = p->m_vec3StartingPosition;
}

void IllustrativeParticleSystem::update(float time)
{
	std::chrono::time_point<std::chrono::high_resolution_clock> tick = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float, std::milli> timeSinceLastUpdate = tick - m_tpLastParticleUpdate;
	//printf("time since last: %f\n", (float)timeSinceLastUpdate);
	if (timeSinceLastUpdate > 1000ms)
		timeSinceLastUpdate = 1000ms; //dont skip or jump start

	//dont update too often
	if (timeSinceLastUpdate <= PARTICLE_SYSTEM_MIN_UPDATE_INTERVAL)
		return;
	else
		m_tpLastParticleUpdate = tick;
	
	//printf("Updating existing particles..\n");
	
	glm::vec3 currentPos, newPos;
	glm::vec3 vel;
	
	// Take inventory of dead particles and particles which can be killed
	std::vector<IllustrativeParticle*> deadParticles;
	std::vector<IllustrativeParticle*> recyclableParticles;
	std::vector<IllustrativeParticle*> activeParticles;
	for (auto const &particle : m_vpParticles)
	{
		if (particle->m_bDead)
			deadParticles.push_back(particle);
		else
		{
			if (!particle->m_bUserCreated)
				recyclableParticles.push_back(particle);

			activeParticles.push_back(particle);
		}
	}

	// initialize a map from flowgrids to active particle counts
	std::map<FlowGrid*, int> activeWithinGridMap;
	for (auto const &grid : m_vpFlowGridCollection)
		activeWithinGridMap[grid] = 0;
	

	std::vector<IllustrativeParticle*> removedParticles;
	//Update all current particles
	for (auto const &particle : activeParticles)
	{
		//get current position
		currentPos = particle->getCurrentXYZ();

		if (!particle->m_pFlowGrid) //if not attached to a flow grid
		{
			for (auto const &fg : m_vpFlowGridCollection)
			{
				if (fg->contains(currentPos.x, currentPos.y, currentPos.z))
				{
					//found one
					particle->m_pFlowGrid = fg;
					break;
				}
			}
		}//end if not attached to flow grid

		if (particle->m_pFlowGrid) // did we find a flow grid?
		{
			newPos = m_bUseEuler ? eulerForward(particle, time, timeSinceLastUpdate.count()) : rk4(particle, time, timeSinceLastUpdate.count());
			activeWithinGridMap[particle->m_pFlowGrid]++;
		}
		else
		{
			//if there are flow grids, lets kill it, so it doesn't clutter the scene
			if (m_vpFlowGridCollection.size() > 0)
			{
				particle->reset();
				removedParticles.push_back(particle);
				deadParticles.push_back(particle);
				continue;
			}
			else
			{
				newPos = currentPos; 
				activeWithinGridMap[particle->m_pFlowGrid]++;
			}
		}

		std::chrono::duration<float> prodTimeSeconds = timeSinceLastUpdate;

		newPos.z += particle->m_fGravity / prodTimeSeconds.count();

		particle->updatePosition(tick, newPos.x, newPos.y, newPos.z);		
	}//end for all active particles

	// Now remove non-active particles
	for (auto const &p : removedParticles)
		activeParticles.erase(std::remove(activeParticles.begin(), activeParticles.end(), p), activeParticles.end());

	//handle dyepoles/emitters

	// DYE POTS
	for (auto const &pot : m_vpDyePots)
	{
		int numToSpawn = pot->getNumParticlesToEmit(tick);

		if (numToSpawn > 0)
		{
			std::vector<glm::vec3> emittedPositions = pot->getParticlesToEmit(numToSpawn);
			for (int i = 0; i < (numToSpawn > MAX_PARTICLES ? MAX_PARTICLES : numToSpawn); ++i )
			{
				IllustrativeParticle* particleToUse = NULL;
				if (deadParticles.size() > 0)
				{
					particleToUse = deadParticles.back();
					deadParticles.pop_back();
					activeParticles.push_back(particleToUse);
				}
				else if (recyclableParticles.size() > 0)
				{
					particleToUse = recyclableParticles.back();
					recyclableParticles.pop_back();
				}
				else
				{
					//no particles left, what do we do here?
					//printf("ERROR: PARTICLE SYSTEM MAXXED OUT!\n  You can try to raise the max number of particles.\n");
				}

				//randomize the lifetimes by +\- 50f% so they dont all die simultaneously
				std::chrono::milliseconds lifetime = std::chrono::duration_cast<std::chrono::milliseconds>(pot->getLifetime() * ((float)(rand() % 25) / 100.f) + pot->getLifetime() * 0.75f);
				
				particleToUse->reset();

				particleToUse->init(emittedPositions[i], pot->getColor(), pot->getGravity(), lifetime, pot->m_msTrailTime, tick, true);
			}//end for numToSpawn
		}
	}

	// DYE POLES
	for (auto const &pole : m_vpDyePoles)
	{
		for (auto const &emitter : pole->emitters)
		{
			int numToSpawn = emitter->getNumParticlesToEmit(tick);
			if (numToSpawn > 0)
			{
				for (auto const &particlePos : emitter->getParticlesToEmit(numToSpawn))
				{
					IllustrativeParticle* particleToUse = NULL;
					if (deadParticles.size() > 0)
					{
						particleToUse = deadParticles.back();
						deadParticles.pop_back();
					}
					else if (recyclableParticles.size() > 0)
					{
						particleToUse = recyclableParticles.back();
						recyclableParticles.pop_back();
					}
					else
					{
						//no particles left, what do we do here?
						//printf("ERROR: PARTICLE SYSTEM MAXXED OUT!\n  You can try to raise the max number of particles.\n");
					}

					std::chrono::milliseconds  lifetime = std::chrono::duration_cast<std::chrono::milliseconds>(emitter->getLifetime() * ((float)(rand() % 25) / 100.f) + emitter->getLifetime() * 0.75f);  //randomize the lifetimes by +\- 50f% so they dont all die simultaneously					
					
					particleToUse->reset();

					particleToUse->init(particlePos, emitter->getColor(), emitter->getGravity(), lifetime, emitter->m_msTrailTime, tick, true);

					activeParticles.push_back(particleToUse);
				}//end for numToSpawn
			}
		}
	}

	//finally add however many seeds we need to keep our desired amount and with however many particle slots we have left

	glm::vec3 randPos;

	//for each grid
	bool inWater, skipGrid;
	int chancesNotInWater;
	float minScaledZ, maxScaledZ, scaledZRange;
	for (auto const &grid : m_vpFlowGridCollection)
	{
		if (grid->m_bIllustrativeParticlesEnabled)
		{
			//if more particles needed
			int numNeeded = grid->m_nIllustrativeParticles - activeWithinGridMap[grid];

			if (numNeeded > 0)
			{
				//spawn some here
				skipGrid = false;
				minScaledZ = abs(grid->getMinDepth());
				maxScaledZ = abs(grid->getMaxDepth());
				scaledZRange = maxScaledZ - minScaledZ;

				while (numNeeded > 0 && !skipGrid && deadParticles.size() > 0u)
				{
					inWater = false;
					chancesNotInWater = 10000;
					while (!inWater && chancesNotInWater > 0)
					{
						randPos.x = grid->m_fXMin + grid->m_fXRange*(((float)(rand()%10000))/10000);
						randPos.y = grid->m_fYMin + grid->m_fYRange*(((float)(rand()%10000))/10000);

						if ( (rand()%100) < 20) //20% surface particles
							randPos.z = 0;
						else
							randPos.z = (1.f * (((((float)(rand()%10000))/10000)*scaledZRange)+minScaledZ)  );
							
						if (grid->getIsWaterAt(randPos.x, randPos.y, randPos.z, time))
							inWater = true;
						else
							chancesNotInWater--;
					}

					if (chancesNotInWater < 1)
					{
						skipGrid = true;
					}
					else
					{
						//randomize the lifetimes by +\- 25f% so they dont all die simultaneously
						std::chrono::milliseconds lifetime = std::chrono::duration_cast<std::chrono::milliseconds>(grid->m_fIllustrativeParticleLifetime * ((float)(rand()%25) / 100.f) + grid->m_fIllustrativeParticleLifetime * 0.75f);

						IllustrativeParticle* particleToUse = deadParticles.back();
						deadParticles.pop_back();

						particleToUse->reset();

						particleToUse->init(randPos, grid->m_vec3IllustrativeParticlesColor, 0, lifetime, grid->m_fIllustrativeParticleTrailTime, tick, false);

						particleToUse->m_pFlowGrid = grid;

						activeParticles.push_back(particleToUse);
							
						numNeeded--;
					}
				}//while more seeds to add and not out of dead particles to revive
				//printf("Added Seeds in %f ms.\n", GetTickCount()-tick);
			}//end if numneeed > 0
		}//end if illust. parts enabled
	}//end for each flowgrid

	m_nLastCountLiveParticles = m_nMaxParticles - static_cast<int>(deadParticles.size());
	m_nLastCountLiveSeeds = static_cast<int>(activeParticles.size());

	size_t count = 0;
	for (size_t i = 0; i < m_vpParticles.size(); ++i)
	{
		int numPositions = m_vpParticles[i]->getNumLivePositions();
		if (numPositions > 1)
		{
			std::chrono::milliseconds timeElapsed = m_vpParticles[i]->m_msLiveTimeElapsed;

			for (int j = 0; j < numPositions - 1; j++)
			{
				int posIndex1 = m_vpParticles[i]->getLivePosition(j);
				int posIndex2 = m_vpParticles[i]->getLivePosition(j + 1);

				glm::vec3 pos1, pos2;

				pos1.x = m_vpParticles[i]->m_vvec3Positions[posIndex1].x;
				pos1.y = m_vpParticles[i]->m_vvec3Positions[posIndex1].y;
				pos1.z = m_vpParticles[i]->m_vvec3Positions[posIndex1].z;

				pos2.x = m_vpParticles[i]->m_vvec3Positions[posIndex2].x;
				pos2.y = m_vpParticles[i]->m_vvec3Positions[posIndex2].y;
				pos2.z = m_vpParticles[i]->m_vvec3Positions[posIndex2].z;

				float opacity1 = 1.f - (m_vpParticles[i]->m_tpLastUpdateTimestamp - m_vpParticles[i]->m_vtpTimes[posIndex1]) / std::chrono::duration<float>(timeElapsed);
				float opacity2 = 1.f - (m_vpParticles[i]->m_tpLastUpdateTimestamp - m_vpParticles[i]->m_vtpTimes[posIndex2]) / std::chrono::duration<float>(timeElapsed);

				glm::vec4 col1(m_vpParticles[i]->m_vec3Color, opacity1);
				glm::vec4 col2(m_vpParticles[i]->m_vec3Color, opacity2);

				m_arrvec3PositionsBuffer[count] = pos1;
				m_arrvec4ColorBuffer[count] = col1;
				m_arruiIndices[count] = static_cast<GLuint>(count);

				count++;

				m_arrvec3PositionsBuffer[count] = pos2;
				m_arrvec4ColorBuffer[count] = col2;
				m_arruiIndices[count] = static_cast<GLuint>(count);

				count++;
			}//end for each position
		}//end if two live positions (enough to draw 1 line segment)
	}//end for each particle

	m_bReadyToTransferData = true;
	m_nIndexCount = static_cast<GLsizei>(count);
}

bool IllustrativeParticleSystem::prepareForRender()
{
	if (!m_bReadyToTransferData)
		return false;

	GLsizei numPositions, numColors;
	numPositions = numColors = m_nIndexCount;

	if (numPositions < 2)
		return false;

	glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
	// Buffer orphaning
	// Sub buffer data for points, then colors
	glBufferSubData(GL_ARRAY_BUFFER, 0, numPositions * sizeof(glm::vec3), m_arrvec3PositionsBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, numPositions * sizeof(glm::vec3), numColors * sizeof(glm::vec4), m_arrvec4ColorBuffer);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_nIndexCount * sizeof(GLuint), 0, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_nIndexCount * sizeof(GLuint), m_arruiIndices, GL_STREAM_DRAW);
	
	// Set color attribute pointer now that point array size is known
	glBindVertexArray(this->m_glVAO);
	glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)(numPositions * sizeof(glm::vec3)));
	glBindVertexArray(0);

	m_bReadyToTransferData = false;

	return true;
}
void IllustrativeParticleSystem::initGL()
{
	glGenVertexArrays(1, &m_glVAO);
	glGenBuffers(1, &m_glVBO);
	glGenBuffers(1, &m_glEBO);

	glBindVertexArray(this->m_glVAO);
	// Bind the array and allocate its storage
	glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
	glNamedBufferStorage(m_glVBO, MAX_PARTICLES * MAX_NUM_TRAIL_POSITIONS * (sizeof(glm::vec3) + sizeof(glm::vec4)), NULL, GL_DYNAMIC_STORAGE_BIT);

	// Now do the same with the element array buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glEBO);
	//glNamedBufferStorage(m_glEBO, MAX_PARTICLES * MAX_NUM_TRAIL_POSITIONS * sizeof(GLuint), NULL, GL_DYNAMIC_STORAGE_BIT);

	// Enable attribute arrays (with layouts to be defined later)
	glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
	glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
	// color attribute pointer will be set once position array size is known for attrib pointer offset

	glBindVertexArray(0);
}


glm::vec3 IllustrativeParticleSystem::eulerForward(IllustrativeParticle* particle, float time, float delta)
{
	glm::vec3 currentPos = particle->getCurrentXYZ();
	glm::vec3 vel;

	//get UVW at current position (checks in bounds or not)
	if (!particle->m_pFlowGrid->getUVWat(currentPos.x, currentPos.y, currentPos.z, time, &vel.x, &vel.y, &vel.z))
	{
		particle->m_bDying = true;
		return currentPos;
	}
	

	//calc new position
	float prodTimeVelocity = delta * particle->m_pFlowGrid->m_fIllustrativeParticleVelocityScale;

	glm::vec3 ret(currentPos + vel * prodTimeVelocity * 10.f);

	if (!particle->m_pFlowGrid->contains(ret.x, ret.y, ret.z))
		particle->m_bDying = true;

	return ret;
}

glm::vec3 IllustrativeParticleSystem::rk4(IllustrativeParticle * particle, float time, float delta)
{
	glm::vec3 startPos = particle->getCurrentXYZ();
	glm::vec3 k1, k2, k3, k4, y1, y2, y3, newPos;

	float prodTimeVelocity = delta * particle->m_pFlowGrid->m_fIllustrativeParticleVelocityScale;

	//get UVW at current position
	if (!particle->m_pFlowGrid->getUVWat(startPos.x, startPos.y, startPos.z, time, &k1.x, &k1.y, &k1.z))
		return eulerForward(particle, time, delta);

	y1 = startPos + k1 * prodTimeVelocity * 0.5f;

	if (!particle->m_pFlowGrid->getUVWat(y1.x, y1.y, y1.z, time, &k2.x, &k2.y, &k2.z))
		return eulerForward(particle, time, delta);

	y2 = startPos + k2 * prodTimeVelocity * 0.5f;

	if (!particle->m_pFlowGrid->getUVWat(y2.x, y2.y, y2.z, time, &k3.x, &k3.y, &k3.z))
		return eulerForward(particle, time, delta);

	y3 = startPos + k3 * prodTimeVelocity;

	if (!particle->m_pFlowGrid->getUVWat(y3.x, y3.y, y3.z, time, &k4.x, &k4.y, &k4.z))
		return eulerForward(particle, time, delta);

	newPos = startPos + prodTimeVelocity * (k1 + 2.f * k2 + 2.f * k3 + k4) / 6.f;

	//check in bounds or not
	if (!particle->m_pFlowGrid->contains(newPos.x, newPos.y, newPos.z))
		particle->m_bDying = true;

	return newPos;
}


int IllustrativeParticleSystem::getNumLiveParticles()
{
	return m_nLastCountLiveParticles;// maxNumParticles;//particles.size();
}

int IllustrativeParticleSystem::getNumLiveSeeds()
{
	return m_nLastCountLiveSeeds;// maxNumParticles;//particles.size();
}

int IllustrativeParticleSystem::getNumDyePoles()
{
	return static_cast<int>(m_vpDyePoles.size());
}

int IllustrativeParticleSystem::getNumDyeEmitters()
{
	size_t total = 0;
	for (size_t i = 0; i < m_vpDyePoles.size(); i++)
		total += m_vpDyePoles.at(i)->emitters.size();
	return static_cast<int>(total);
}

int IllustrativeParticleSystem::getNumDyeParticles()
{
	return m_nLastCountLiveParticles - m_nLastCountLiveSeeds;
}

GLuint IllustrativeParticleSystem::getVAO()
{
	return m_glVAO;
}

GLsizei IllustrativeParticleSystem::getIndexCount()
{
	return m_nIndexCount;
}

void IllustrativeParticleSystem::setEulerIntegration()
{
	m_bUseEuler = true;
}

void IllustrativeParticleSystem::setRK4Integration()
{
	m_bUseEuler = false;
}

int IllustrativeParticleSystem::addDyePole(double x, double y, float minZ, float maxZ)
{
	IllustrativeDyePole* tempDP = new IllustrativeDyePole(static_cast<float>(x), static_cast<float>(y), minZ, maxZ);
	//tempDP->adddEmittersAlongEntireLength(100);
	tempDP->addDefaultEmitter();
	for (int i=0;i<tempDP->getNumEmitters();i++)
	{
		tempDP->changeEmitterColor(i,(m_vpDyePoles.size()%8)+1);
		//float rate = 300*((float)(rand()%100)/100) + 500;  //randomize the spawn rates so they dont all come out in sync
		//tempDP->changeEmitterRate(i,5000);//rate);
		//tempDP->changeEmitterLifetime(i, 40000);
		//tempDP->changeEmitterTrailtime(i, 1000);

	}
	m_vpDyePoles.push_back(tempDP);
	return static_cast<int>(m_vpDyePoles.size())-1;
}

void IllustrativeParticleSystem::drawDyePoles()
{
	//for (int i=0;i<m_vpDyePoles.size();i++)
	//	m_vpDyePoles.at(i)->drawSmall3D();  //TEMP FOR CHRIS
}

void IllustrativeParticleSystem::drawDyePots()
{
	//for (auto const &dp : m_vpDyePots)
	//	dp->drawSmall3D();
}

void IllustrativeParticleSystem::deleteAllDyePoles()
{
	for (int i=0;i<m_vpDyePoles.size();i++)
		m_vpDyePoles.at(i)->kill();
	m_vpDyePoles.clear();
}

void IllustrativeParticleSystem::deleteDyePole(int index)
{
	m_vpDyePoles.at(index)->kill();
	m_vpDyePoles.erase(m_vpDyePoles.begin()+index);
}

IllustrativeDyePole* IllustrativeParticleSystem::getDyePoleClosestTo(double x, double y)
{
	glm::vec3 target(x,y,0);
	float minDist = 10000000;
	int closestIndex;
	float dist;
	for (int i=0;i<m_vpDyePoles.size();i++)
	{
		glm::vec3 candidate(m_vpDyePoles.at(i)->x, m_vpDyePoles.at(i)->y, 0);
		dist = glm::length(target - candidate);
		if (dist < minDist)
		{
			minDist = dist;
			closestIndex = i;
		}
	}
	return m_vpDyePoles.at(closestIndex);
}

IllustrativeParticleEmitter * IllustrativeParticleSystem::getDyePotClosestTo(float x, float y, float z)
{
	glm::vec3 target(x, y, z);
	float minDist = 10000000.f;
	int closestIndex = -1;
	float dist;
	for (int i = 0; i < m_vpDyePots.size(); i++)
	{
		glm::vec3 candidate(m_vpDyePots.at(i)->m_vec3Pos);
		dist = glm::length(target - candidate);
		if (dist < minDist)
		{
			minDist = dist;
			closestIndex = i;
		}
	}

	if (closestIndex < 0)
		return NULL;

	return m_vpDyePots.at(closestIndex);
}

void IllustrativeParticleSystem::pause()
{
	m_tpPauseTime = std::chrono::high_resolution_clock::now();
}

void IllustrativeParticleSystem::unPause()
{
	std::chrono::milliseconds elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_tpPauseTime);

	for (int i=0;i<m_nMaxParticles;i++)
	{
		if (!m_vpParticles[i]->m_bDead)
		{
			m_vpParticles[i]->m_tpBirthTime += elapsedTime;
			m_vpParticles[i]->m_tpLastUpdateTimestamp += elapsedTime;
			for (int j=0;j<100;j++)
				m_vpParticles[i]->m_vtpTimes[j] += elapsedTime;
		}
	}//end for each particle

}//end unPause()
