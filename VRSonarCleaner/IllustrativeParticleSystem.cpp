#include "IllustrativeParticleSystem.h"

//NOTE: modified from VTT4D to use x,z lat/long and y as depth dimension!

#include <shared/glm/glm.hpp>

IllustrativeParticleSystem::IllustrativeParticleSystem(CoordinateScaler *Scaler, std::vector<FlowGrid*> FlowGridCollection)
{
	m_pScaler = Scaler;

	m_vpFlowGridCollection = FlowGridCollection;

	m_ullLastParticleUpdate = GetTickCount64();

	m_bStreakletBuffersGenerated = false;
	m_bParticleBuffersGenerated = false;

	m_nMaxParticles = MAX_PARTICLES;
	ULONGLONG tick = GetTickCount64();
	printf("Initializing particle system...");
	m_vpParticles.resize(MAX_PARTICLES);
	for (int i=0;i<m_nMaxParticles;i++)
	{
		m_vpParticles[i] = new IllustrativeParticle(0, 0, 0, 1, 100, tick);
		m_vpParticles[i]->kill();
	}
	printf("Done!");

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
	int particleToReplace = -1;
	for (int i=0;i<m_nMaxParticles;i++)
	{
		if (m_vpParticles[i]->m_bDead)
		{
			particleToReplace = i;
			break;
		}
	}
	if (particleToReplace == -1)
	{
		for (int i=0;i<m_nMaxParticles;i++)
		{
			if (!m_vpParticles[i]->m_bUserCreated && !m_vpParticles[i]->m_bDead)
			{
				particleToReplace = i;
				break;	
			}
		}
	}
	if (particleToReplace == -1)
	{
		printf("YOU SHOULD NEVER SEE THIS! 18515 addDyeParticle()");
		particleToReplace = 0;
	}
	
	float tick = GetTickCount64();
	lifetime = lifetime*((float)(rand()%25)/100) + lifetime*0.75;  //randomize the lifetimes by +\- 25% so they dont all die simultaneously					
	IllustrativeParticle* tmpPart = new IllustrativeParticle(x, y, z, lifetime, 1000, tick);  //TO DO: edit existing particle instead of creating new particle, will be slightly faster I think
	tmpPart->m_fGravity = 0;
	tmpPart->m_bUserCreated = true;
	tmpPart->m_iFlowGridIndex = -1;
	tmpPart->m_vec3Color.r = r;
	tmpPart->m_vec3Color.g = g;
	tmpPart->m_vec3Color.b = b;

	delete m_vpParticles[particleToReplace];
	m_vpParticles[particleToReplace] = tmpPart;
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
	
	float x, y, z;
	float u, v, w;
	float newPos[3];
	float prodTimeVelocity;
	float prodTimeSeconds = 1000*timeSinceLastUpdate;
	bool result;
	//Update all current particles
	for (int i=0;i<m_nMaxParticles;i++)
	{
		if (!m_vpParticles[i]->m_bDead)
		{
			if (m_vpParticles[i]->m_bDying)
			{
				//printf("Dying...\n");
				m_vpParticles[i]->updatePosition(tick, 0, 0, 0);
			}
			else if (m_vpParticles[i]->m_iFlowGridIndex > -1) //if attached to a flow grid
			{
				//get current position
				m_vpParticles[i]->getCurrentXYZ(&x, &y, &z);
				//get UVW at current position
				result = m_vpFlowGridCollection.at(m_vpParticles[i]->m_iFlowGridIndex)->getUVWat(x, y, z, time, &u, &v, &w);
				//printf("Result: %d, U: %f, V: %f\n", result, u ,v);
				//calc new position
				prodTimeVelocity = timeSinceLastUpdate*m_vpFlowGridCollection.at(m_vpParticles[i]->m_iFlowGridIndex)->illustrativeParticleVelocityScale;
				//printf("Current Pos: %f, %f, %f\n", x, y, z);
				newPos[0] = x + u*prodTimeVelocity;
				newPos[1] = y + v*prodTimeVelocity;
				newPos[2] = z + w*prodTimeVelocity + (m_vpParticles[i]->m_fGravity / prodTimeSeconds);

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
	}//end for all particles
	//printf("Updated Particles in %f ms.\n", GetTickCount()-tick);
	
	//handle dyepoles/emitters
	
	//printf("Counting dead...\n");

	bool resortedToKilling = false;
	int* deadParticles = new int[m_nMaxParticles];
	int numDeadParticles = 0;
	int* killableSeeds = new int[m_nMaxParticles];
	int numKillableSeeds = 0;
	for (int i=0;i<m_nMaxParticles;i++)
	{
		if (m_vpParticles[i]->m_bDead)
		{
			deadParticles[numDeadParticles] = i;
			numDeadParticles++;
		}
		else if (!m_vpParticles[i]->m_bUserCreated && !m_vpParticles[i]->m_bDead)
		{
			killableSeeds[numKillableSeeds] = i;
			numKillableSeeds++;
		}
	}

	//printf("Dead: %d\n", numDeadParticles);

	//printf("Emitting from Emitters...\n");
	
	float lifetime;
	int numToSpawn;

	// DYE POTS
	for (int i = 0; i<m_vpDyePots.size(); i++)
	{
		numToSpawn = m_vpDyePots.at(i)->getNumParticlesToEmit(tick);
		if (numToSpawn > 0)
		{
			float* vertsToSpawn = m_vpDyePots.at(i)->getParticlesToEmit(numToSpawn);//new float[numToSpawn*3];
			for (int k = 0; k<numToSpawn; k++)
			{
				lifetime = m_vpDyePots.at(i)->getLifetime()*((float)(rand() % 25) / 100) + m_vpDyePots.at(i)->getLifetime()*0.75;  //randomize the lifetimes by +\- 50f% so they dont all die simultaneously					
				IllustrativeParticle* tmpPart = new IllustrativeParticle(vertsToSpawn[k * 3], vertsToSpawn[k * 3 + 1], vertsToSpawn[k * 3 + 2], lifetime, m_vpDyePots.at(i)->trailTime, tick);  //TO DO: edit existing particle instead of creating new particle, will be slightly faster I think
				tmpPart->m_fGravity = m_vpDyePots.at(i)->getGravity();
				tmpPart->m_bUserCreated = true;

				switch(m_vpDyePots.at(i)->getColor())
				{
				case 0:
					tmpPart->m_vec3Color.r = COLOR_0_R;
					tmpPart->m_vec3Color.g = COLOR_0_G;
					tmpPart->m_vec3Color.b = COLOR_0_B;
					break;
				case 1:
					tmpPart->m_vec3Color.r = COLOR_1_R;
					tmpPart->m_vec3Color.g = COLOR_1_G;
					tmpPart->m_vec3Color.b = COLOR_1_B;
					break;
				case 2:
					tmpPart->m_vec3Color.r = COLOR_2_R;
					tmpPart->m_vec3Color.g = COLOR_2_G;
					tmpPart->m_vec3Color.b = COLOR_2_B;
					break;
				case 3:
					tmpPart->m_vec3Color.r = COLOR_3_R;
					tmpPart->m_vec3Color.g = COLOR_3_G;
					tmpPart->m_vec3Color.b = COLOR_3_B;
					break;
				case 4:
					tmpPart->m_vec3Color.r = COLOR_4_R;
					tmpPart->m_vec3Color.g = COLOR_4_G;
					tmpPart->m_vec3Color.b = COLOR_4_B;
					break;
				case 5:
					tmpPart->m_vec3Color.r = COLOR_5_R;
					tmpPart->m_vec3Color.g = COLOR_5_G;
					tmpPart->m_vec3Color.b = COLOR_5_B;
					break;
				case 6:
					tmpPart->m_vec3Color.r = COLOR_6_R;
					tmpPart->m_vec3Color.g = COLOR_6_G;
					tmpPart->m_vec3Color.b = COLOR_6_B;
					break;
				case 7:
					tmpPart->m_vec3Color.r = COLOR_7_R;
					tmpPart->m_vec3Color.g = COLOR_7_G;
					tmpPart->m_vec3Color.b = COLOR_7_B;
					break;
				case 8:
					tmpPart->m_vec3Color.r = COLOR_8_R;
					tmpPart->m_vec3Color.g = COLOR_8_G;
					tmpPart->m_vec3Color.b = COLOR_8_B;
					break;
				}

				//= dyePoles.at(i)->emitters.at(j)->getColor();
				if (!resortedToKilling) //if we havent already resorted to killing particles because no dead ones left
				{
					if (numDeadParticles > 0)
					{
						delete m_vpParticles[deadParticles[numDeadParticles - 1]];
						m_vpParticles[deadParticles[numDeadParticles - 1]] = tmpPart;
						numDeadParticles--;
					}
					else
					{
						resortedToKilling = true;
					}
				}
				else //replace a live seed particle
				{
					if (numKillableSeeds > 0)
					{
						delete m_vpParticles[killableSeeds[numKillableSeeds - 1]];
						m_vpParticles[killableSeeds[numKillableSeeds - 1]] = tmpPart;
						numKillableSeeds--;
					}
					else
					{
						//no particles left, what do we do here?
						//printf("ERROR: PARTICLE SYSTEM MAXXED OUT!\n  You can try to raise the max number of particles.\n");
					}
				}

			}//end for numToSpawn
			delete vertsToSpawn;
		}
	}

	// DYE POLES
	for (int i=0;i<m_vpDyePoles.size();i++)
	{
		for (int j=0;j<m_vpDyePoles.at(i)->emitters.size();j++)
		{
			numToSpawn = m_vpDyePoles.at(i)->emitters.at(j)->getNumParticlesToEmit(tick);
			if (numToSpawn > 0)
			{
				float* vertsToSpawn = m_vpDyePoles.at(i)->emitters.at(j)->getParticlesToEmit(numToSpawn);//new float[numToSpawn*3];
				for (int k=0;k<numToSpawn;k++)
				{
					lifetime = m_vpDyePoles.at(i)->emitters.at(j)->getLifetime()*((float)(rand()%25)/100) + m_vpDyePoles.at(i)->emitters.at(j)->getLifetime()*0.75;  //randomize the lifetimes by +\- 50f% so they dont all die simultaneously					
					IllustrativeParticle* tmpPart = new IllustrativeParticle(vertsToSpawn[k*3], vertsToSpawn[k*3+1], vertsToSpawn[k*3+2], lifetime, m_vpDyePoles.at(i)->emitters.at(j)->trailTime, tick);  //TO DO: edit existing particle instead of creating new particle, will be slightly faster I think
					tmpPart->m_fGravity = m_vpDyePoles.at(i)->emitters.at(j)->getGravity();
					tmpPart->m_bUserCreated = true;
					tmpPart->m_vec3Color.r = 1;
					tmpPart->m_vec3Color.g = 1;
					tmpPart->m_vec3Color.b = 0;

					//= dyePoles.at(i)->emitters.at(j)->getColor();
					if (!resortedToKilling) //if we havent already resorted to killing particles because no dead ones left
					{
						if (numDeadParticles > 0)
						{
							delete m_vpParticles[deadParticles[numDeadParticles-1]];
							m_vpParticles[deadParticles[numDeadParticles-1]] = tmpPart;
							numDeadParticles--;
						}
						else
						{
							resortedToKilling = true;	
						}
					}
					else //replace a live seed particle
					{
						if (numKillableSeeds > 0)
						{
							delete m_vpParticles[killableSeeds[numKillableSeeds-1]];
							m_vpParticles[killableSeeds[numKillableSeeds-1]] = tmpPart;
							numKillableSeeds--;
						}
						else
						{
							//no particles left, what do we do here?
							//printf("ERROR: PARTICLE SYSTEM MAXXED OUT!\n  You can try to raise the max number of particles.\n");
						}
					}

				}//end for numToSpawn
				delete vertsToSpawn;
			}
		}
	}

	delete[] deadParticles;
	delete[] killableSeeds;

	//printf("Counting Again...\n");

 //finally add however many seeds we need to keep our desired amount and with however many particle slots we have left
	//recount available slots:
	int numSeedsActive = 0;
	int numParticlesDead = 0;
	for (int i=0;i<m_nMaxParticles;i++)
	{
		if (!m_vpParticles[i]->m_bUserCreated && !m_vpParticles[i]->m_bDead)
			numSeedsActive++;
		else if (m_vpParticles[i]->m_bDead)
			numParticlesDead++;
	}

	//printf("LatLonMinMAx: %f, %f, %f, %f\n", dataset->minLonVal, dataset->maxLonVal, dataset->minLatVal, dataset->maxLatVal);
	//printf("CORners: %f, %f, %f, %f\n", seedBBox[0], seedBBox[1], seedBBox[2], seedBBox[3]);

	float maxAddable = numParticlesDead;
	//count particles in each grid
	int *activeWithinGrid;
	activeWithinGrid = new int[m_vpFlowGridCollection.size()];
	//count active in each grid
	for (int i=0;i<m_vpFlowGridCollection.size();i++)
	{
		activeWithinGrid[i] = 0;
	}
	for (int i=0;i<m_nMaxParticles;i++)
	{
		if (!m_vpParticles[i]->m_bDead)
		{
			if (m_vpParticles[i]->m_iFlowGridIndex > -1 && m_vpParticles[i]->m_iFlowGridIndex < m_vpFlowGridCollection.size())
				activeWithinGrid[m_vpParticles[i]->m_iFlowGridIndex]++;
		}
	}

	float randX;
	float randY;
	float randZ;

	//for each grid
	int searchIndex = 0;
	bool inWater, skipGrid;
	int chancesNotInWater;
	float minScaledZ, maxScaledZ, scaledZRange;
	for (int i=0;i<m_vpFlowGridCollection.size();i++)
	{
		//printf("FG1, ");
		//if particles enabled
		if (m_vpFlowGridCollection.at(i)->enableIllustrativeParticles)
		{
			//if more particles needed
			int numNeeded = m_vpFlowGridCollection.at(i)->numIllustrativeParticles - activeWithinGrid[i];
			//printf("Req: %d, Active: %d, Need: %d\n", flowGridCollection->at(i)->numIllustrativeParticles, activeWithinGrid[i], numNeeded);
			if (numNeeded > 0)
			{
				//spawn some here
				skipGrid = false;
				minScaledZ = abs(m_pScaler->getScaledDepth(m_vpFlowGridCollection.at(i)->getMinDepth()));
				maxScaledZ = abs(m_pScaler->getScaledDepth(m_vpFlowGridCollection.at(i)->getMaxDepth()));
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

							randX = m_vpFlowGridCollection.at(i)->xMin + m_vpFlowGridCollection.at(i)->xRange*(((float)(rand()%10000))/10000);
							randY = m_vpFlowGridCollection.at(i)->yMin + m_vpFlowGridCollection.at(i)->yRange*(((float)(rand()%10000))/10000);
		
							//randZ = dataset->minDepthVal + rand()%(int)dataset->rangeDepthVal; 
							//randZlevel = rand()%(int)dataset->cellsD;
							//randZ = dataset->getActualDepthAtLayer(randZlevel);

							if ( (rand()%100) < 20) //20% surface particles
								randZ = 0;
							else
								randZ = m_pScaler->getUnscaledDepth( -1.0*  (((((float)(rand()%10000))/10000)*scaledZRange)+minScaledZ)  );

							//printf("Chance: %d, rand: %f, %f, %f\n", chancesNotInWater, randX, randY, randZ);

							if (m_vpFlowGridCollection.at(i)->getIsWaterAt(randX, randY, randZ, time))
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
							lifetime = m_vpFlowGridCollection.at(i)->illustrativeParticleLifetime*((float)(rand()%25)/100) + m_vpFlowGridCollection.at(i)->illustrativeParticleLifetime*0.75;  //randomize the lifetimes by +\- 25f% so they dont all die simultaneously
							IllustrativeParticle* tmpPart = new IllustrativeParticle(randX, randY, randZ, lifetime, m_vpFlowGridCollection.at(i)->illustrativeParticleTrailTime, tick);
							tmpPart->m_iFlowGridIndex = i;
							tmpPart->m_vec3Color.r = m_vpFlowGridCollection.at(i)->colorIllustrativeParticles[0];
							tmpPart->m_vec3Color.g = m_vpFlowGridCollection.at(i)->colorIllustrativeParticles[1];
							tmpPart->m_vec3Color.b = m_vpFlowGridCollection.at(i)->colorIllustrativeParticles[2];

							if (searchIndex == 1234)
								printf("Spawed at: %0.4f, %0.4f, %0.4f\n", randX, randY, randZ);

							delete m_vpParticles[searchIndex];
							m_vpParticles[searchIndex] = tmpPart;
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
	//printf("Live: %d, Dead: %d\n", numSeedsActive, numParticlesDead);
}

void IllustrativeParticleSystem::loadStreakVBOs()
{
	m_nStreakSegments = 0;
	for (int i=0;i<m_nMaxParticles;i++)
	{
		m_nStreakSegments += m_vpParticles[i]->getNumLivePositions();
	}
	//printf("num Segs: %d\n", numSegments);
	if (m_nStreakSegments < 1)
	 return;
	
	
	//printf("Loading Particle System VBOs...\n");
	//if (GLEW_OK != glewInit())
	//{
	//	printf("ERROR with glew init!\n");
	//	return;
	//}
	//glDeleteBuffers(1, &streakletPositionsVBO);
	//glDeleteBuffers(1, &streakletColorsVBO);

	if (!m_bStreakletBuffersGenerated)
	{
		glGenBuffers(1, &m_glStreakletPositionsVBO);
		glGenBuffers(1, &m_glStreakletColorsVBO);
		m_bStreakletBuffersGenerated = true;
	}	
	
	GLsizeiptr positionsSize = m_nStreakSegments * 6 * sizeof(GLfloat); //XYZ
	GLsizeiptr colorsSize = m_nStreakSegments * 8 * sizeof(GLfloat);  //RGBA
	
	glBindBuffer(GL_ARRAY_BUFFER, m_glStreakletPositionsVBO);
	glBufferData(GL_ARRAY_BUFFER, positionsSize, NULL, GL_STATIC_DRAW);
	GLfloat* positions = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	glBindBuffer(GL_ARRAY_BUFFER, m_glStreakletColorsVBO);
	glBufferData(GL_ARRAY_BUFFER, colorsSize, NULL, GL_STATIC_DRAW);
	GLfloat* colors = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	
	ULONGLONG currentTime = GetTickCount64();

	int index = 0;
	float timeElapsed;
	float timeSinceNewest;
	float opacity1, opacity2;
	float depthColorFactor;
	int numPositions;
	
	int posIndex1, posIndex2, posIndex1x3, posIndex2x3;
	for (int i=0;i<m_nMaxParticles;i++)
	{
		numPositions = m_vpParticles[i]->getNumLivePositions();
		if (numPositions > 1)
		{
			timeElapsed = m_vpParticles[i]->m_ullLiveTimeElapsed;
			for (int j=1;j<numPositions;j++)
			{
				posIndex1 = m_vpParticles[i]->getLivePosition(j-1);
				posIndex2 = m_vpParticles[i]->getLivePosition(j);

				positions[(index*6)] = m_pScaler->getScaledLonX(m_vpParticles[i]->m_vvec3Positions[posIndex1].x);
				positions[(index*6)+1] = m_pScaler->getScaledDepth(m_vpParticles[i]->m_vvec3Positions[posIndex1].z);  //SWAPPED
				positions[(index*6)+2] = m_pScaler->getScaledLatY(m_vpParticles[i]->m_vvec3Positions[posIndex1].y); //SWAPPED

				positions[(index*6)+3] = m_pScaler->getScaledLonX(m_vpParticles[i]->m_vvec3Positions[posIndex2].x);
				positions[(index*6)+4] = m_pScaler->getScaledDepth(m_vpParticles[i]->m_vvec3Positions[posIndex2].z); //SWAPPED
				positions[(index*6)+5] = m_pScaler->getScaledLatY(m_vpParticles[i]->m_vvec3Positions[posIndex2].y);  //SWAPPED

				//printf("line at: %f, %f, %f - %f, %f, %f\n", positions[(index*6)], positions[(index*6)+1], positions[(index*6)+2], positions[(index*6)+3], positions[(index*6)+4], positions[(index*6)+5]);

				opacity1 = 1 - (m_vpParticles[i]->m_ullLastUpdateTimestamp - m_vpParticles[i]->m_vullTimes[posIndex1])/timeElapsed;
				opacity2 = 1 - (m_vpParticles[i]->m_ullLastUpdateTimestamp - m_vpParticles[i]->m_vullTimes[posIndex2])/timeElapsed;

				//depthColorFactor = particles[i]->positions[particles[i]->getLivePosition(j) * 3 + 2]*.00015;//particles[i]->positions.at(j).z*.001;
					
				colors[(index * 8) + 0] = m_vpParticles[i]->m_vec3Color.r;
				colors[(index * 8) + 1] = m_vpParticles[i]->m_vec3Color.g;
				colors[(index * 8) + 2] = m_vpParticles[i]->m_vec3Color.b;
				colors[(index * 8) + 3] = opacity1;

				colors[(index * 8) + 4] = m_vpParticles[i]->m_vec3Color.r;
				colors[(index * 8) + 5] = m_vpParticles[i]->m_vec3Color.g;
				colors[(index * 8) + 6] = m_vpParticles[i]->m_vec3Color.b;
				colors[(index * 8) + 7] = opacity2;
				
				index++;
			}//end for each position
		}//end if two live positions (enough to draw 1 line segment)
	}//end for each particle

	glBindBuffer(GL_ARRAY_BUFFER, m_glStreakletPositionsVBO);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, m_glStreakletColorsVBO);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glColorPointer(4, GL_FLOAT, 0, NULL);
	
}//end loadVBOs()

void IllustrativeParticleSystem::drawStreakVBOs()
{
	if (!m_bStreakletBuffersGenerated || m_nStreakSegments < 1)
		return;
	//printf("Drawing Particle System VBOs...\n");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glColor4f(1,1,1,1);
	glLineWidth(1.5);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_glStreakletPositionsVBO);
	glVertexPointer(3, GL_FLOAT, 0, (char*)NULL);

	glBindBuffer(GL_ARRAY_BUFFER, m_glStreakletColorsVBO);
	glColorPointer(4, GL_FLOAT, 0, (char*)NULL);

	glDrawArrays(GL_LINES, 0, m_nStreakSegments * 2);
	
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );	
	
}

void IllustrativeParticleSystem::loadParticleVBOs()
{
	m_nParticlePoints = 0;
	for (int i=0;i<m_nMaxParticles;i++)
	{
		if (m_vpParticles[i]->getNumLivePositions() > 0)
			m_nParticlePoints++;
	}
	if (m_nParticlePoints < 1)
	 return;

	////printf("Loading Particle System VBOs...\n");
	//if (GLEW_OK != glewInit())
	//{
	//	printf("ERROR with glew init!\n");
	//	return;
	//}
	//glDeleteBuffers(1, &streakletPositionsVBO);
	//glDeleteBuffers(1, &streakletColorsVBO);

	if (!m_bParticleBuffersGenerated)
	{
		glGenBuffers(1, &m_glParticlePositionsVBO);
		glGenBuffers(1, &m_glParticleColorsVBO);
		m_bParticleBuffersGenerated = true;
	}	
	
	//printf("num Segs: %d\n", numSegments);

	GLsizeiptr positionsSize = m_nParticlePoints * 3 * sizeof(GLfloat); //XYZ
	GLsizeiptr colorsSize = m_nParticlePoints * 4 * sizeof(GLfloat);  //RGBA
	
	glBindBuffer(GL_ARRAY_BUFFER, m_glParticlePositionsVBO);
	glBufferData(GL_ARRAY_BUFFER, positionsSize, NULL, GL_STATIC_DRAW);
	GLfloat* positions = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	glBindBuffer(GL_ARRAY_BUFFER, m_glParticleColorsVBO);
	glBufferData(GL_ARRAY_BUFFER, colorsSize, NULL, GL_STATIC_DRAW);
	GLfloat* colors = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	
	ULONGLONG currentTime = GetTickCount64();

	int index = 0;
	float timeElapsed;
	float timeSinceNewest;
	float opacity;
	float depthColorFactor;
	int numPositions;
	float currentX, currentY, currentZ;
	for (int i=0;i<m_nMaxParticles;i++)
	{
		if (m_vpParticles[i]->getNumLivePositions() > 0)
		{
			m_vpParticles[i]->getCurrentXYZ(&currentX, &currentY, &currentZ);

			positions[(index*3)] = m_pScaler->getScaledLonX(currentX);
			positions[(index*3)+1] = -m_pScaler->getMaxScaledDepth() - m_pScaler->getScaledDepth(currentZ); //SWAPPED and negated
			positions[(index*3)+2] = m_pScaler->getScaledLatY(currentY); //SWAPPED

			opacity = m_vpParticles[i]->getFadeInFadeOutOpacity();

			//depthColorFactor = particles[i]->positions[particles[i]->getLivePosition(j) * 3 + 2]*.00015;//particles[i]->positions.at(j).z*.001;
					
			colors[(index * 4) + 0] = m_vpParticles[i]->m_vec3Color.r;
			colors[(index * 4) + 1] = m_vpParticles[i]->m_vec3Color.g;
			colors[(index * 4) + 2] = m_vpParticles[i]->m_vec3Color.b;
			colors[(index * 4) + 3] = opacity;
			//printf("P %d's opacity is %f\n", i, opacity);
							
			index++;

		}//end if has a live position	
	}//end for each particle

	glBindBuffer(GL_ARRAY_BUFFER, m_glParticlePositionsVBO);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, m_glParticleColorsVBO);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glColorPointer(4, GL_FLOAT, 0, NULL);
	
}//end loadVBOs()

void IllustrativeParticleSystem::drawParticleVBOs()
{
	if (!m_bParticleBuffersGenerated || m_nParticlePoints < 1)
		return;
	//printf("Drawing Particle System VBOs...\n");

	float ptSizes[2];
	float ptQuadratic[3];
	ptQuadratic[0] = 0.0000001;//a = -0.01 this is the "falloff" speed that decreases the point size as distance grows
	ptQuadratic[1] = 0.001;//b = 0 center parabola on x (dist) = 0
	ptQuadratic[2] = 0.0001;//c = 4 (this is the minimum size, when dist = 0)



	glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, ptSizes);
    //glEnable( GL_POINT_SPRITE_ARB );
    glPointParameterfARB(GL_POINT_SIZE_MIN_ARB, ptSizes[0]);
	glPointParameterfARB(GL_POINT_SIZE_MAX_ARB, ptSizes[1]);
    glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, ptQuadratic);
    //glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);


	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	//glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glColor4f(1,1,1,1);
	glPointSize(3);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glBindBuffer(GL_ARRAY_BUFFER, m_glParticlePositionsVBO);
	glVertexPointer(3, GL_FLOAT, 0, (char*)NULL);

	glBindBuffer(GL_ARRAY_BUFFER, m_glParticleColorsVBO);
	glColorPointer(4, GL_FLOAT, 0, (char*)NULL);

	glDrawArrays(GL_POINTS, 0, m_nParticlePoints);
	
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );	

	glDisable(GL_POINT_SPRITE_ARB);
	
	/*
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH);
	glColor4f(1,1,1,1);
	glPointSize(3);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glBindBuffer(GL_ARRAY_BUFFER, particlePositionsVBO);
	glVertexPointer(3, GL_FLOAT, 0, (char*)NULL);

	glBindBuffer(GL_ARRAY_BUFFER, particleColorsVBO);
	glColorPointer(4, GL_FLOAT, 0, (char*)NULL);

	glDrawArrays(GL_POINTS, 0, numParticlePoints);
	
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );	
	*/
	

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
		glm::vec3 candidate(m_vpDyePots.at(i)->x, m_vpDyePots.at(i)->y, (m_vpDyePots.at(i)->depthTop - m_vpDyePots.at(i)->depthBottom) * 0.5f);
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