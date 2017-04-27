#include "IllustrativeParticleSystem.h"

//NOTE: modified from VTT4D to use x,z lat/long and y as depth dimension!

#include <shared/glm/glm.hpp>
#include <map>

#include "DebugDrawer.h"

IllustrativeParticleSystem::IllustrativeParticleSystem(CoordinateScaler *Scaler, std::vector<FlowGrid*> FlowGridCollection)
{
	m_pScaler = Scaler;

	m_vpFlowGridCollection = FlowGridCollection;

	m_ullLastParticleUpdate = GetTickCount64();
	
	m_nMaxParticles = MAX_PARTICLES;
	ULONGLONG tick = GetTickCount64();
	printf("Initializing particle system...");
	m_vpParticles.resize(MAX_PARTICLES);
	for (int i = 0; i < m_nMaxParticles; i++)
	{
		m_vpParticles[i] = new IllustrativeParticle();
	}
	printf("Done!\n");
}

IllustrativeParticleSystem::~IllustrativeParticleSystem()
{

}

void IllustrativeParticleSystem::addDyeParticleWorldCoords(double x, double y, double z, float r, float g, float b, float lifetime)
{
	double thisX = m_pScaler->getUnscaledLonX(x);
	double thisY = m_pScaler->getUnscaledLatY(y);
	double thisZ = m_pScaler->getUnscaledDepth(z);

	//printf("Dye In:  %0.4f, %0.4f, %0.4f\n", x, y, z);
	//printf("Dye Out: %0.4f, %0.4f, %0.4f\n", thisX, thisY, thisZ);

	addDyeParticle(thisX, thisY, thisZ, r, g, b, lifetime);
}

void IllustrativeParticleSystem::addDyeParticle(double x, double y, double z, float r, float g, float b, float lifetime)
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
	
	float tick = GetTickCount64();
	lifetime = lifetime*((float)(rand()%25)/100) + lifetime*0.75;  //randomize the lifetimes by +\- 25% so they dont all die simultaneously					
	
	IllustrativeParticle* p = m_vpParticles[particleIndexToReplace];

	p->m_bDead = false;
	p->m_bDying = false;
	p->m_bUserCreated = true;
	p->m_fGravity = 0;
	p->m_fTimeToLive;
	p->m_fTrailTime = 1000.f;
	p->m_iBufferHead = 1;
	p->m_iBufferTail = 0;
	p->m_pFlowGrid = NULL;
	p->m_ullBirthTime = tick;
	p->m_ullLastUpdateTimestamp;
	p->m_ullLiveTimeElapsed = 0ull;
	p->m_ullTimeDeathBegan = 0ull;
	p->m_ullTimeToStartDying = tick + lifetime;
	p->m_vec3Color = glm::vec3(0.25f, 0.95f, 1.f);
	p->m_vec3StartingPosition = glm::vec3(x, y, z);
	p->m_vullTimes.clear();
	p->m_vullTimes[0] = tick;
	p->m_vvec3Positions.clear();
	p->m_vvec3Positions[0] = p->m_vec3StartingPosition;
	p->m_vvec3Positions[1] = p->m_vec3StartingPosition;
}

void IllustrativeParticleSystem::update(float time)
{
	ULONGLONG tick = GetTickCount64();
	ULONGLONG timeSinceLastUpdate = tick - m_ullLastParticleUpdate;
	//printf("time since last: %f\n", (float)timeSinceLastUpdate);
	if (timeSinceLastUpdate > 1000)
		timeSinceLastUpdate = 1000; //dont skip or jump start

	//dont update too often
	if (timeSinceLastUpdate <= PARTICLE_SYSTEM_MIN_UPDATE_INTERVAL)
		return;
	else
		m_ullLastParticleUpdate = tick;
	
	//printf("Updating existing particles..\n");
	
	glm::vec3 currentPos, newPos;
	glm::vec3 vel;
	float prodTimeVelocity;
	float prodTimeSeconds = 1000*timeSinceLastUpdate;

	//Update all current particles
	for (auto &particle : m_vpParticles)
	{
		if (!particle->m_bDead)
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
			}//end else not attached to flow grid

			if (particle->m_pFlowGrid)
			{
				//get UVW at current position
				bool result = particle->m_pFlowGrid->getUVWat(currentPos.x, currentPos.y, currentPos.z, time, &vel.x, &vel.y, &vel.z);

				//calc new position
				prodTimeVelocity = timeSinceLastUpdate * particle->m_pFlowGrid->illustrativeParticleVelocityScale;

				newPos = currentPos + vel * prodTimeVelocity;

				//check in bounds or not
				if (!particle->m_pFlowGrid->contains(newPos.x, newPos.y, newPos.z) || !result)
				{
					particle->m_bDying = true;
				}
			}
			else
			{
				//if there are flow grids, lets kill it, so it doesn't clutter the scene
				if (m_vpFlowGridCollection.size() > 0)
				{
					particle->kill();
				}
				else
				{
					newPos = currentPos;
				}
			}

			newPos.z += particle->m_fGravity / prodTimeSeconds;
			particle->updatePosition(tick, newPos.x, newPos.y, newPos.z);
		}//end if not dead
		
		particle->updateBufferIndices(tick);
	}//end for all particles
	
	//handle dyepoles/emitters

	// Take inventory of dead particles and particles which can be killed
	bool resortedToKilling = false;
	std::vector<IllustrativeParticle*> deadParticles;
	std::vector<IllustrativeParticle*> killableSeeds;
	for (auto const &particle : m_vpParticles)
	{
		if (particle->m_bDead)
			deadParticles.push_back(particle);
		else if (!particle->m_bUserCreated && !particle->m_bDead)
			killableSeeds.push_back(particle);
	}

	// DYE POTS
	for (auto const &pot : m_vpDyePots)
	{
		int numToSpawn = pot->getNumParticlesToEmit(tick);
		if (numToSpawn > 0)
		{
			std::vector<glm::vec3> vertsToSpawn = pot->getParticlesToEmit(numToSpawn);
			for (auto const &vert : vertsToSpawn)
			{
				IllustrativeParticle* particleToUse = NULL;
				if (deadParticles.size() > 0)
				{
					particleToUse = deadParticles.back();
					deadParticles.pop_back();
				}
				else if (killableSeeds.size() > 0)
				{
					particleToUse = killableSeeds.back();
					killableSeeds.pop_back();
				}
				else
				{
					//no particles left, what do we do here?
					//printf("ERROR: PARTICLE SYSTEM MAXXED OUT!\n  You can try to raise the max number of particles.\n");
				}

				float lifetime = pot->getLifetime()*((float)(rand() % 25) / 100) + pot->getLifetime()*0.75;  //randomize the lifetimes by +\- 50f% so they dont all die simultaneously					
				
				particleToUse->reset();

				particleToUse->init(vert, pot->getColor(), pot->getGravity(), lifetime, pot->trailTime, tick, true);
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
				std::vector<glm::vec3> vertsToSpawn = emitter->getParticlesToEmit(numToSpawn);//new float[numToSpawn*3];
				for (auto const &vert : vertsToSpawn)
				{
					IllustrativeParticle* particleToUse = NULL;
					if (deadParticles.size() > 0)
					{
						particleToUse = deadParticles.back();
						deadParticles.pop_back();
					}
					else if (killableSeeds.size() > 0)
					{
						particleToUse = killableSeeds.back();
						killableSeeds.pop_back();
					}
					else
					{
						//no particles left, what do we do here?
						//printf("ERROR: PARTICLE SYSTEM MAXXED OUT!\n  You can try to raise the max number of particles.\n");
					}

					float lifetime = emitter->getLifetime()*((float)(rand() % 25) / 100) + emitter->getLifetime()*0.75;  //randomize the lifetimes by +\- 50f% so they dont all die simultaneously					
					
					particleToUse->reset();

					particleToUse->init(vert, emitter->getColor(), emitter->getGravity(), lifetime, emitter->trailTime, tick, true);
				}//end for numToSpawn
			}
		}
	}

	//finally add however many seeds we need to keep our desired amount and with however many particle slots we have left
	//recount available slots:
	int numSeedsActive = 0;
	int numParticlesDead = 0;
	for (auto const & particle : m_vpParticles)
	{
		if (!particle->m_bUserCreated && !particle->m_bDead)
			numSeedsActive++;
		else if (particle->m_bDead)
			numParticlesDead++;
	}

	float maxAddable = numParticlesDead;
	//count particles in each grid
	std::map<FlowGrid*, int> activeWithinGridMap;
	//count active in each grid
	for (auto &grid : m_vpFlowGridCollection)
		activeWithinGridMap[grid] = 0;

	for (auto const & particle : m_vpParticles)
		if (!particle->m_bDead && particle->m_pFlowGrid)
				activeWithinGridMap[particle->m_pFlowGrid]++;

	float randX;
	float randY;
	float randZ;

	//for each grid
	int searchIndex = 0;
	bool inWater, skipGrid;
	int chancesNotInWater;
	float minScaledZ, maxScaledZ, scaledZRange;
	for (auto &grid : m_vpFlowGridCollection)
	{
		//printf("FG1, ");
		//if particles enabled
		if (grid->enableIllustrativeParticles)
		{
			//if more particles needed
			int numNeeded = grid->numIllustrativeParticles - activeWithinGridMap[grid];
			//printf("Req: %d, Active: %d, Need: %d\n", flowGridCollection->at(i)->numIllustrativeParticles, activeWithinGrid[i], numNeeded);
			if (numNeeded > 0)
			{
				//spawn some here
				skipGrid = false;
				minScaledZ = abs(m_pScaler->getScaledDepth(grid->getMinDepth()));
				maxScaledZ = abs(m_pScaler->getScaledDepth(grid->getMaxDepth()));
				scaledZRange = maxScaledZ - minScaledZ;
				//printf("minZ: %f, maxZ: %f, rangeZ: %f\n", flowGridCollection->at(i)->getMinDepth(), flowGridCollection->at(i)->getMaxDepth(), flowGridCollection->at(i)->getMaxDepth()-flowGridCollection->at(i)->getMinDepth());
				//printf("minSZ: %f, maxSZ: %f, rangeSZ: %f\n", minScaledZ, maxScaledZ, scaledZRange);
				//printf("NN: %d, SI: %d, MNP: %d, SG: %d\n", numNeeded, searchIndex, maxNumParticles, skipGrid);
				while (numNeeded > 0 && searchIndex < m_nMaxParticles && !skipGrid)
				{
					//printf("SI: %d");
					if (m_vpParticles[searchIndex]->m_bDead)
					{
						//printf(" dead\n");
						inWater = false;
						chancesNotInWater = 10000;
						while (!inWater && chancesNotInWater > 0)
						{
							
							//randX = seedBBox[0] + onscreenXRange*(((float)(rand()%10000))/10000);
							///randY = seedBBox[2] + onscreenYRange*(((float)(rand()%10000))/10000);

							randX = grid->xMin + grid->xRange*(((float)(rand()%10000))/10000);
							randY = grid->yMin + grid->yRange*(((float)(rand()%10000))/10000);
		
							//randZ = dataset->minDepthVal + rand()%(int)dataset->rangeDepthVal; 
							//randZlevel = rand()%(int)dataset->cellsD;
							//randZ = dataset->getActualDepthAtLayer(randZlevel);

							if ( (rand()%100) < 20) //20% surface particles
								randZ = 0;
							else
								randZ = m_pScaler->getUnscaledDepth( -1.0*  (((((float)(rand()%10000))/10000)*scaledZRange)+minScaledZ)  );

							//printf("Chance: %d, rand: %f, %f, %f\n", chancesNotInWater, randX, randY, randZ);

							if (grid->getIsWaterAt(randX, randY, randZ, time))
								inWater = true;
							else
								chancesNotInWater--;
						}
						if (chancesNotInWater < 1)
						{
							//printf("Couldn't find water!!!\n");
							skipGrid = true;
						}
						else
						{
							//printf("found water!\n");
							//printf("C:%d, Spawning at: %f, %f, %f\n", chancesNotInWater, randX, randY, randZ);
							float lifetime = grid->illustrativeParticleLifetime*((float)(rand()%25)/100) + grid->illustrativeParticleLifetime*0.75;  //randomize the lifetimes by +\- 25f% so they dont all die simultaneously

							m_vpParticles[searchIndex]->reset();

							m_vpParticles[searchIndex]->init(glm::vec3(randX, randY, randZ), grid->colorIllustrativeParticles, 0, lifetime, grid->illustrativeParticleTrailTime, tick, false);

							m_vpParticles[searchIndex]->m_pFlowGrid = grid;
							
							numNeeded--;
							searchIndex++;
						}
					}//if found dead particle
					else
					{
						//printf("Live\n");
						searchIndex++;
					}
				}//while more seeds to add and not out of dead particles to revive
				//printf("Added Seeds in %f ms.\n", GetTickCount()-tick);
			}//end if numneeed > 0
		}//end if illust. parts enabled
	}//end for each flowgrid

	//final count
	numSeedsActive = 0;
	numParticlesDead = 0;
	for (int i=0;i<m_nMaxParticles;i++)
	{
		if (!m_vpParticles[i]->m_bUserCreated && !m_vpParticles[i]->m_bDead)
			numSeedsActive++;
		else if (m_vpParticles[i]->m_bDead)
			numParticlesDead++;
	}
	m_nLastCountLiveParticles = m_nMaxParticles - numParticlesDead;
	m_nLastCountLiveSeeds = numSeedsActive;
}

void IllustrativeParticleSystem::drawStreakVBOs()
{	
	for (auto particle : m_vpParticles)
	{
		int numPositions = particle->getNumLivePositions();
		if (numPositions > 1)
		{
			float timeElapsed = particle ->m_ullLiveTimeElapsed;
			for (int j=1;j<numPositions;j++)
			{
				int posIndex1 = particle->getLivePosition(j-1);
				int posIndex2 = particle->getLivePosition(j);

				glm::vec3 pos1, pos2;
				glm::vec4 col1, col2;

				pos1.x = m_pScaler->getScaledLonX(particle->m_vvec3Positions[posIndex1].x);
				pos1.y = m_pScaler->getScaledDepth(particle->m_vvec3Positions[posIndex1].z);  //SWAPPED
				pos1.z = m_pScaler->getScaledLatY(particle->m_vvec3Positions[posIndex1].y); //SWAPPED

				pos2.x = m_pScaler->getScaledLonX(particle->m_vvec3Positions[posIndex2].x);
				pos2.y = m_pScaler->getScaledDepth(particle->m_vvec3Positions[posIndex2].z); //SWAPPED
				pos2.z = m_pScaler->getScaledLatY(particle->m_vvec3Positions[posIndex2].y);  //SWAPPED

				//printf("line at: %f, %f, %f - %f, %f, %f\n", positions[(index*6)], positions[(index*6)+1], positions[(index*6)+2], positions[(index*6)+3], positions[(index*6)+4], positions[(index*6)+5]);

				float opacity1 = 1 - (particle->m_ullLastUpdateTimestamp - particle->m_vullTimes[posIndex1])/timeElapsed;
				float opacity2 = 1 - (particle->m_ullLastUpdateTimestamp - particle->m_vullTimes[posIndex2])/timeElapsed;

				//depthColorFactor = particles[i]->positions[particles[i]->getLivePosition(j) * 3 + 2]*.00015;//particles[i]->positions.at(j).z*.001;
					
				col1.r = particle->m_vec3Color.r;
				col1.g = particle->m_vec3Color.g;
				col1.b = particle->m_vec3Color.b;
				col1.a = opacity1;

				col2 = col1;
				col2.a = opacity2;

				DebugDrawer::getInstance().drawLine(pos1, pos2, col1, col2);
			}//end for each position
		}//end if two live positions (enough to draw 1 line segment)
	}//end for each particle	
}//end drawStreakVBOs()

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
	return m_vpDyePoles.size();
}

int IllustrativeParticleSystem::getNumDyeEmitters()
{
	int total = 0;
	for (int i=0;i<m_vpDyePoles.size();i++)
		total += m_vpDyePoles.at(i)->emitters.size();
	return total;
}

int IllustrativeParticleSystem::getNumDyeParticles()
{
	return m_nLastCountLiveParticles - m_nLastCountLiveSeeds;
}

int IllustrativeParticleSystem::addDyePole(double x, double y, float minZ, float maxZ)
{
	IllustrativeDyePole* tempDP = new IllustrativeDyePole(x, y, minZ, maxZ, m_pScaler);
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
	return m_vpDyePoles.size()-1;
}

void IllustrativeParticleSystem::drawDyePoles()
{
	for (int i=0;i<m_vpDyePoles.size();i++)
		m_vpDyePoles.at(i)->drawSmall3D();  //TEMP FOR CHRIS
}

void IllustrativeParticleSystem::drawDyePots()
{
	for (auto const &dp : m_vpDyePots)
		dp->drawSmall3D();
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
		glm::vec3 candidate(m_vpDyePots.at(i)->x, m_vpDyePots.at(i)->y, m_vpDyePots.at(i)->z);
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
	m_ullPauseTime = GetTickCount64();
}

void IllustrativeParticleSystem::unPause()
{
	ULONGLONG elapsedTime = GetTickCount64() - m_ullPauseTime;

	for (int i=0;i<m_nMaxParticles;i++)
	{
		if (!m_vpParticles[i]->m_bDead)
		{
			m_vpParticles[i]->m_ullBirthTime += elapsedTime;
			m_vpParticles[i]->m_ullLastUpdateTimestamp += elapsedTime;
			for (int j=0;j<100;j++)
				m_vpParticles[i]->m_vullTimes[j] += elapsedTime;
		}
	}//end for each particle

}//end unPause()
