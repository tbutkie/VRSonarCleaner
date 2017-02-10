#include "IllustrativeParticleSystem.h"

//NOTE: modified from VTT4D to use x,z lat/long and y as depth dimension!

IllustrativeParticleSystem::IllustrativeParticleSystem(CoordinateScaler *Scaler, std::vector <FlowGrid *> *FlowGridCollection)
{
	scaler = Scaler;

	flowGridCollection = FlowGridCollection;

	lastSeeding = GetTickCount64();//timeGetTime();
	lastParticleUpdate = lastSeeding;

	streakletBuffersGenerated = false;
	particleBuffersGenerated = false;

	maxNumParticles = MAX_PARTICLES;
	ULONGLONG tick = GetTickCount64();
	printf("Initializing particle system...");
	for (int i=0;i<maxNumParticles;i++)
	{
		particles[i] = new IllustrativeParticle(0, 0, 0, 1, 100, tick);
		particles[i]->flowGridIndex = -1;
		particles[i]->kill();
	}
	printf("Done!");

}

IllustrativeParticleSystem::~IllustrativeParticleSystem()
{

}

void IllustrativeParticleSystem::addDyeParticleWorldCoords(double x, double y, double z, float r, float g, float b, float lifetime)
{
	double thisX = scaler->getUnscaledLonX(x);
	double thisY = scaler->getUnscaledLatY(y);
	double thisZ = scaler->getUnscaledDepth(z);

	//printf("Dye In:  %0.4f, %0.4f, %0.4f\n", x, y, z);
	//printf("Dye Out: %0.4f, %0.4f, %0.4f\n", thisX, thisY, thisZ);

	addDyeParticle(thisX, thisY, thisZ, r, g, b, lifetime);
}

void IllustrativeParticleSystem::addDyeParticle(double x, double y, double z, float r, float g, float b, float lifetime)
{
	//find particle to replace:
	int particleToReplace = -1;
	for (int i=0;i<maxNumParticles;i++)
	{
		if (particles[i]->dead)
		{
			particleToReplace = i;
			break;
		}
	}
	if (particleToReplace == -1)
	{
		for (int i=0;i<maxNumParticles;i++)
		{
			if (!particles[i]->userCreated && !particles[i]->dead)
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
	tmpPart->gravity = 0;
	tmpPart->userCreated = true;
	tmpPart->flowGridIndex = -1;
	tmpPart->color[0] = r;
	tmpPart->color[1] = g;
	tmpPart->color[2] = b;

	delete particles[particleToReplace];
	particles[particleToReplace] = tmpPart;

	flowGridCollection->at(0)->numIllustrativeParticles++;
}

void IllustrativeParticleSystem::update(float time)
{
	ULONGLONG tick = GetTickCount64();
	ULONGLONG timeSinceLastUpdate = tick - lastParticleUpdate;
	//printf("time since last: %f\n", (float)timeSinceLastUpdate);
	if (timeSinceLastUpdate > 1000)
		timeSinceLastUpdate = 1000; //dont skip or jump start

	//dont update too often
	if (timeSinceLastUpdate <= PARTICLE_SYSTEM_MIN_UPDATE_INTERVAL)
		return;
	else
		lastParticleUpdate = tick;
	
	//printf("Updating existing particles..\n");
	
	float x, y, z;
	float u, v, w;
	float newPos[3];
	float prodTimeVelocity;
	float prodTimeSeconds = 1000*timeSinceLastUpdate;
	bool result;
	//Update all current particles
	for (int i=0;i<maxNumParticles;i++)
	{
		if (!particles[i]->dead)
		{
			if (particles[i]->dying)
			{
				//printf("Dying...\n");
				particles[i]->updatePosition(tick, 0, 0, 0);
			}
			else if (particles[i]->flowGridIndex > -1) //if attached to a flow grid
			{
				//get current position
				particles[i]->getCurrentXYZ(&x, &y, &z);
				//get UVW at current position
				result = flowGridCollection->at(particles[i]->flowGridIndex)->getUVWat(x, y, z, time, &u, &v, &w);
				//printf("Result: %d, U: %f, V: %f\n", result, u ,v);
				//calc new position
				prodTimeVelocity = timeSinceLastUpdate*flowGridCollection->at(particles[i]->flowGridIndex)->illustrativeParticleVelocityScale;
				//printf("Current Pos: %f, %f, %f\n", x, y, z);
				newPos[0] = x + u*prodTimeVelocity;
				newPos[1] = y + v*prodTimeVelocity;
				newPos[2] = z + w*prodTimeVelocity + (particles[i]->gravity/prodTimeSeconds);

				//check in bounds or not
				if (!flowGridCollection->at(particles[i]->flowGridIndex)->contains(newPos[0], newPos[1], newPos[2]))
				{
					//no begin killing off particle
					//printf("OOB: ", newPos[0], newPos[1], newPos[2]);
					particles[i]->dying = true;					
				}
				//printf("P %d, Pos: %f, %f, %f\n", i, newPos[0], newPos[1], newPos[2]);
				particles[i]->updatePosition(tick, newPos[0], newPos[1], newPos[2]);

			}//end if attached to flow grid
			else //not attached to flow grid
			{
				//if there are flow grids, check if in one of them
				bool foundGrid = false;
				if (flowGridCollection->size() > 0)
				{
					//get current position
					particles[i]->getCurrentXYZ(&x, &y, &z);
					//check in bounds or not
					for (int grid = 0;grid<flowGridCollection->size();grid++)
					{
						if (flowGridCollection->at(grid)->contains(x, y, z))
						{
							//found one
							particles[i]->flowGridIndex = grid;
							//printf("particle %d moved to grid %d\n", i, grid);
							//update
							//get UVW at current position
							result = flowGridCollection->at(particles[i]->flowGridIndex)->getUVWat(x, y, z, time, &u, &v, &w);
							//calc new position
							prodTimeVelocity = timeSinceLastUpdate*flowGridCollection->at(particles[i]->flowGridIndex)->illustrativeParticleVelocityScale;
							newPos[0] = x + u*prodTimeVelocity;
							newPos[1] = y + v*prodTimeVelocity;
							newPos[2] = z + w*prodTimeVelocity + (particles[i]->gravity/prodTimeSeconds);
							particles[i]->updatePosition(tick, newPos[0], newPos[1], newPos[2]);
							foundGrid = true;
							break;
						}
					}//end for each grid					
				}//end if flow grids to check

				if (!foundGrid)
				{
					//if there are flow grids, lets kill it, so it doesn't clutter the scene
					if (flowGridCollection->size() > 0)
					{
						particles[i]->kill();
					}
					else
					{
						//if no flow grids, just let it sit there and die out naturally
						particles[i]->getCurrentXYZ(&x, &y, &z);
						newPos[0] = x;
						newPos[1] = y;
						newPos[2] = z + (particles[i]->gravity/prodTimeSeconds); //only gravity for this one
						particles[i]->updatePosition(tick, newPos[0], newPos[1], newPos[2]);
					}
				}

			}//end else not attached to flow grid
		}//end if not dead
	}//end for all particles
	//printf("Updated Particles in %f ms.\n", GetTickCount()-tick);
	
	//handle dyepoles/emitters
	
	//printf("Counting dead...\n");

	bool resortedToKilling = false;
	int* deadParticles = new int[maxNumParticles];
	int numDeadParticles = 0;
	int* killableSeeds = new int[maxNumParticles];
	int numKillableSeeds = 0;
	for (int i=0;i<maxNumParticles;i++)
	{
		if (particles[i]->dead)
		{
			deadParticles[numDeadParticles] = i;
			numDeadParticles++;
		}
		else if (!particles[i]->userCreated && !particles[i]->dead)
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
	for (int i = 0; i<dyePots.size(); i++)
	{
		numToSpawn = dyePots.at(i)->getNumParticlesToEmit(tick);
		if (numToSpawn > 0)
		{
			float* vertsToSpawn = dyePots.at(i)->getParticlesToEmit(numToSpawn);//new float[numToSpawn*3];
			for (int k = 0; k<numToSpawn; k++)
			{
				lifetime = dyePots.at(i)->getLifetime()*((float)(rand() % 25) / 100) + dyePots.at(i)->getLifetime()*0.75;  //randomize the lifetimes by +\- 50f% so they dont all die simultaneously					
				IllustrativeParticle* tmpPart = new IllustrativeParticle(vertsToSpawn[k * 3], vertsToSpawn[k * 3 + 1], vertsToSpawn[k * 3 + 2], lifetime, dyePots.at(i)->trailTime, tick);  //TO DO: edit existing particle instead of creating new particle, will be slightly faster I think
				tmpPart->gravity = dyePots.at(i)->getGravity();
				tmpPart->userCreated = true;

				switch(dyePots.at(i)->getColor())
				{
				case 0:
					tmpPart->color[0] = COLOR_0_R;
					tmpPart->color[1] = COLOR_0_G;
					tmpPart->color[2] = COLOR_0_B;
					break;
				case 1:
					tmpPart->color[0] = COLOR_1_R;
					tmpPart->color[1] = COLOR_1_G;
					tmpPart->color[2] = COLOR_1_B;
					break;
				case 2:
					tmpPart->color[0] = COLOR_2_R;
					tmpPart->color[1] = COLOR_2_G;
					tmpPart->color[2] = COLOR_2_B;
					break;
				case 3:
					tmpPart->color[0] = COLOR_3_R;
					tmpPart->color[1] = COLOR_3_G;
					tmpPart->color[2] = COLOR_3_B;
					break;
				case 4:
					tmpPart->color[0] = COLOR_4_R;
					tmpPart->color[1] = COLOR_4_G;
					tmpPart->color[2] = COLOR_4_B;
					break;
				case 5:
					tmpPart->color[0] = COLOR_5_R;
					tmpPart->color[1] = COLOR_5_G;
					tmpPart->color[2] = COLOR_5_B;
					break;
				case 6:
					tmpPart->color[0] = COLOR_6_R;
					tmpPart->color[1] = COLOR_6_G;
					tmpPart->color[2] = COLOR_6_B;
					break;
				case 7:
					tmpPart->color[0] = COLOR_7_R;
					tmpPart->color[1] = COLOR_7_G;
					tmpPart->color[2] = COLOR_7_B;
					break;
				case 8:
					tmpPart->color[0] = COLOR_8_R;
					tmpPart->color[1] = COLOR_8_G;
					tmpPart->color[2] = COLOR_8_B;
					break;
				}

				//= dyePoles.at(i)->emitters.at(j)->getColor();
				if (!resortedToKilling) //if we havent already resorted to killing particles because no dead ones left
				{
					if (numDeadParticles > 0)
					{
						delete particles[deadParticles[numDeadParticles - 1]];
						particles[deadParticles[numDeadParticles - 1]] = tmpPart;
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
						delete particles[killableSeeds[numKillableSeeds - 1]];
						particles[killableSeeds[numKillableSeeds - 1]] = tmpPart;
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
	for (int i=0;i<dyePoles.size();i++)
	{
		for (int j=0;j<dyePoles.at(i)->emitters.size();j++)
		{
			numToSpawn = dyePoles.at(i)->emitters.at(j)->getNumParticlesToEmit(tick);
			if (numToSpawn > 0)
			{
				float* vertsToSpawn = dyePoles.at(i)->emitters.at(j)->getParticlesToEmit(numToSpawn);//new float[numToSpawn*3];
				for (int k=0;k<numToSpawn;k++)
				{
					lifetime = dyePoles.at(i)->emitters.at(j)->getLifetime()*((float)(rand()%25)/100) + dyePoles.at(i)->emitters.at(j)->getLifetime()*0.75;  //randomize the lifetimes by +\- 50f% so they dont all die simultaneously					
					IllustrativeParticle* tmpPart = new IllustrativeParticle(vertsToSpawn[k*3], vertsToSpawn[k*3+1], vertsToSpawn[k*3+2], lifetime, dyePoles.at(i)->emitters.at(j)->trailTime, tick);  //TO DO: edit existing particle instead of creating new particle, will be slightly faster I think
					tmpPart->gravity = dyePoles.at(i)->emitters.at(j)->getGravity();
					tmpPart->userCreated = true;
					tmpPart->color[0] = 1;
					tmpPart->color[1] = 1;
					tmpPart->color[2] = 0;

					//= dyePoles.at(i)->emitters.at(j)->getColor();
					if (!resortedToKilling) //if we havent already resorted to killing particles because no dead ones left
					{
						if (numDeadParticles > 0)
						{
							delete particles[deadParticles[numDeadParticles-1]];
							particles[deadParticles[numDeadParticles-1]] = tmpPart;
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
							delete particles[killableSeeds[numKillableSeeds-1]];
							particles[killableSeeds[numKillableSeeds-1]] = tmpPart;
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
	for (int i=0;i<maxNumParticles;i++)
	{
		if (!particles[i]->userCreated && !particles[i]->dead)
			numSeedsActive++;
		else if (particles[i]->dead)
			numParticlesDead++;
	}

	//printf("LatLonMinMAx: %f, %f, %f, %f\n", dataset->minLonVal, dataset->maxLonVal, dataset->minLatVal, dataset->maxLatVal);
	//printf("CORners: %f, %f, %f, %f\n", seedBBox[0], seedBBox[1], seedBBox[2], seedBBox[3]);

	float maxAddable = numParticlesDead;
	//count particles in each grid
	int *activeWithinGrid;
	activeWithinGrid = new int[flowGridCollection->size()];
	//count active in each grid
	for (int i=0;i<flowGridCollection->size();i++)
	{
		activeWithinGrid[i] = 0;
	}
	for (int i=0;i<maxNumParticles;i++)
	{
		if (!particles[i]->dead)
		{
			if (particles[i]->flowGridIndex > -1 && particles[i]->flowGridIndex < flowGridCollection->size())
				activeWithinGrid[particles[i]->flowGridIndex]++;
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
	for (int i=0;i<flowGridCollection->size();i++)
	{
		//printf("FG1, ");
		//if particles enabled
		if (flowGridCollection->at(i)->enableIllustrativeParticles)
		{
			//if more particles needed
			int numNeeded =  flowGridCollection->at(i)->numIllustrativeParticles - activeWithinGrid[i];
			//printf("Req: %d, Active: %d, Need: %d\n", flowGridCollection->at(i)->numIllustrativeParticles, activeWithinGrid[i], numNeeded);
			if (numNeeded > 0)
			{
				//spawn some here
				skipGrid = false;
				minScaledZ = abs(scaler->getScaledDepth(flowGridCollection->at(i)->getMinDepth()));
				maxScaledZ = abs(scaler->getScaledDepth(flowGridCollection->at(i)->getMaxDepth()));
				scaledZRange = maxScaledZ - minScaledZ;
				//printf("minZ: %f, maxZ: %f, rangeZ: %f\n", flowGridCollection->at(i)->getMinDepth(), flowGridCollection->at(i)->getMaxDepth(), flowGridCollection->at(i)->getMaxDepth()-flowGridCollection->at(i)->getMinDepth());
				//printf("minSZ: %f, maxSZ: %f, rangeSZ: %f\n", minScaledZ, maxScaledZ, scaledZRange);
				//printf("NN: %d, SI: %d, MNP: %d, SG: %d\n", numNeeded, searchIndex, maxNumParticles, skipGrid);
				while (numNeeded > 0 && searchIndex < maxNumParticles && !skipGrid)
				{
					//printf("SI: %d");
					if (particles[searchIndex]->dead)
					{
						//printf(" dead\n");
						inWater = false;
						chancesNotInWater = 10000;
						while (!inWater && chancesNotInWater > 0)
						{
							
							//randX = seedBBox[0] + onscreenXRange*(((float)(rand()%10000))/10000);
							///randY = seedBBox[2] + onscreenYRange*(((float)(rand()%10000))/10000);

							randX = flowGridCollection->at(i)->xMin + flowGridCollection->at(i)->xRange*(((float)(rand()%10000))/10000);
							randY = flowGridCollection->at(i)->yMin + flowGridCollection->at(i)->yRange*(((float)(rand()%10000))/10000);
		
							//randZ = dataset->minDepthVal + rand()%(int)dataset->rangeDepthVal; 
							//randZlevel = rand()%(int)dataset->cellsD;
							//randZ = dataset->getActualDepthAtLayer(randZlevel);

							if ( (rand()%100) < 20) //20% surface particles
								randZ = 0;
							else
								randZ = scaler->getUnscaledDepth( -1.0*  (((((float)(rand()%10000))/10000)*scaledZRange)+minScaledZ)  );

							//printf("Chance: %d, rand: %f, %f, %f\n", chancesNotInWater, randX, randY, randZ);

							if (flowGridCollection->at(i)->getIsWaterAt(randX, randY, randZ, time))
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
							lifetime = flowGridCollection->at(i)->illustrativeParticleLifetime*((float)(rand()%25)/100) + flowGridCollection->at(i)->illustrativeParticleLifetime*0.75;  //randomize the lifetimes by +\- 25f% so they dont all die simultaneously
							IllustrativeParticle* tmpPart = new IllustrativeParticle(randX, randY, randZ, lifetime, flowGridCollection->at(i)->illustrativeParticleTrailTime, tick);
							tmpPart->flowGridIndex = i;
							tmpPart->color[0] = flowGridCollection->at(i)->colorIllustrativeParticles[0];
							tmpPart->color[1] = flowGridCollection->at(i)->colorIllustrativeParticles[1];
							tmpPart->color[2] = flowGridCollection->at(i)->colorIllustrativeParticles[2];

							if (searchIndex == 1234)
								printf("Spawed at: %0.4f, %0.4f, %0.4f\n", randX, randY, randZ);

							delete particles[searchIndex];
							particles[searchIndex] = tmpPart;
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
	for (int i=0;i<maxNumParticles;i++)
	{
		if (!particles[i]->userCreated && !particles[i]->dead)
			numSeedsActive++;
		else if (particles[i]->dead)
			numParticlesDead++;
	}
	lastCountLiveParticles = maxNumParticles - numParticlesDead;
	lastCountLiveSeeds = numSeedsActive;
	//printf("Live: %d, Dead: %d\n", numSeedsActive, numParticlesDead);
}

void IllustrativeParticleSystem::loadStreakVBOs()
{
	numStreakSegments = 0;
	for (int i=0;i<maxNumParticles;i++)
	{
		numStreakSegments += particles[i]->getNumLivePositions();
	}
	//printf("num Segs: %d\n", numSegments);
	if (numStreakSegments < 1)
	 return;
	
	
	//printf("Loading Particle System VBOs...\n");
	//if (GLEW_OK != glewInit())
	//{
	//	printf("ERROR with glew init!\n");
	//	return;
	//}
	//glDeleteBuffers(1, &streakletPositionsVBO);
	//glDeleteBuffers(1, &streakletColorsVBO);

	if (!streakletBuffersGenerated)
	{
		glGenBuffers(1, &streakletPositionsVBO);
		glGenBuffers(1, &streakletColorsVBO);
		streakletBuffersGenerated = true;
	}
	else
	{
		glDeleteBuffers(1, &streakletPositionsVBO);
		glDeleteBuffers(1, &streakletColorsVBO);
		glGenBuffers(1, &streakletPositionsVBO);
		glGenBuffers(1, &streakletColorsVBO);
		streakletBuffersGenerated = true;
	}
	
	
	GLsizeiptr positionsSize = numStreakSegments * 6 * sizeof(GLfloat); //XYZ
	GLsizeiptr colorsSize = numStreakSegments * 8 * sizeof(GLfloat);  //RGBA
	
	glBindBuffer(GL_ARRAY_BUFFER, streakletPositionsVBO);
	glBufferData(GL_ARRAY_BUFFER, positionsSize, NULL, GL_STATIC_DRAW);
	GLfloat* positions = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	glBindBuffer(GL_ARRAY_BUFFER, streakletColorsVBO);
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
	for (int i=0;i<maxNumParticles;i++)
	{
		numPositions = particles[i]->getNumLivePositions();
		if (numPositions > 1)//particles[i]->liveIndex+1 < particles[i]->positions.size()-1)
		{
			timeElapsed = particles[i]->liveTimeElapsed;//particles[i]->times.at(particles[i]->times.size()-1)-particles[i]->times.at(particles[i]->liveIndex)+0.001;
			for (int j=1;j<numPositions;j++)//(int j=particles[i]->liveIndex+1;j<particles[i]->positions.size();j++)
			{
				posIndex1 = particles[i]->getLivePosition(j-1);
				posIndex2 = particles[i]->getLivePosition(j);
				posIndex1x3 = posIndex1 * 3;
				posIndex2x3 = posIndex2 * 3;

				positions[(index*6)] = scaler->getScaledLonX(particles[i]->positions[posIndex1x3]);
				positions[(index*6)+1] = scaler->getScaledDepth(particles[i]->positions[posIndex1x3 + 2]);  //SWAPPED
				positions[(index*6)+2] = scaler->getScaledLatY(particles[i]->positions[posIndex1x3 + 1]); //SWAPPED

				positions[(index*6)+3] = scaler->getScaledLonX(particles[i]->positions[posIndex2x3]);
				positions[(index*6)+4] = scaler->getScaledDepth(particles[i]->positions[posIndex2x3 + 2]); //SWAPPED //flowGridCollection->at(particles[i]->getFlowGridIndex())->getScaledMaxDepth() - 
				positions[(index*6)+5] = scaler->getScaledLatY(particles[i]->positions[posIndex2x3 + 1]);  //SWAPPED

				//printf("line at: %f, %f, %f - %f, %f, %f\n", positions[(index*6)], positions[(index*6)+1], positions[(index*6)+2], positions[(index*6)+3], positions[(index*6)+4], positions[(index*6)+5]);

				opacity1 = 1 - (particles[i]->lastUpdateTimestamp - particles[i]->times[posIndex1]+0.001)/timeElapsed;
				opacity2 = 1 - (particles[i]->lastUpdateTimestamp - particles[i]->times[posIndex2]+0.001)/timeElapsed;

				//depthColorFactor = particles[i]->positions[particles[i]->getLivePosition(j) * 3 + 2]*.00015;//particles[i]->positions.at(j).z*.001;
					
				colors[(index*8)+0] = particles[i]->color[0];
				colors[(index*8)+1] = particles[i]->color[1];
				colors[(index*8)+2] = particles[i]->color[2];
				colors[(index*8)+3] = opacity1;

				colors[(index*8)+4] = particles[i]->color[0];
				colors[(index*8)+5] = particles[i]->color[1];
				colors[(index*8)+6] = particles[i]->color[2];
				colors[(index*8)+7] = opacity2;
				
				index++;
			}//end for each position
		}//end if two live positions (enough to draw 1 line segment)
	}//end for each particle

	glBindBuffer(GL_ARRAY_BUFFER, streakletPositionsVBO);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, streakletColorsVBO);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glColorPointer(4, GL_FLOAT, 0, NULL);
	
}//end loadVBOs()

void IllustrativeParticleSystem::drawStreakVBOs()
{
	if (!streakletBuffersGenerated  || numStreakSegments < 1)
		return;
	//printf("Drawing Particle System VBOs...\n");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glColor4f(1,1,1,1);
	glLineWidth(1.5);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glBindBuffer(GL_ARRAY_BUFFER, streakletPositionsVBO);
	glVertexPointer(3, GL_FLOAT, 0, (char*)NULL);

	glBindBuffer(GL_ARRAY_BUFFER, streakletColorsVBO);
	glColorPointer(4, GL_FLOAT, 0, (char*)NULL);

	glDrawArrays(GL_LINES, 0, numStreakSegments*2);
	
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );	
	
}

void IllustrativeParticleSystem::loadParticleVBOs()
{
	numParticlePoints = 0;
	for (int i=0;i<maxNumParticles;i++)
	{
		if (particles[i]->getNumLivePositions() > 0)
			numParticlePoints++;
	}
	if (numParticlePoints < 1)
	 return;

	////printf("Loading Particle System VBOs...\n");
	//if (GLEW_OK != glewInit())
	//{
	//	printf("ERROR with glew init!\n");
	//	return;
	//}
	//glDeleteBuffers(1, &streakletPositionsVBO);
	//glDeleteBuffers(1, &streakletColorsVBO);

	if (!particleBuffersGenerated)
	{
		glGenBuffers(1, &particlePositionsVBO);
		glGenBuffers(1, &particleColorsVBO);
		particleBuffersGenerated = true;
	}
	else
	{
		glDeleteBuffers(1, &particlePositionsVBO);
		glDeleteBuffers(1, &particleColorsVBO);
		glGenBuffers(1, &particlePositionsVBO);
		glGenBuffers(1, &particleColorsVBO);
		particleBuffersGenerated = true;
	}
	
	
	//printf("num Segs: %d\n", numSegments);

	GLsizeiptr positionsSize = numParticlePoints * 3 * sizeof(GLfloat); //XYZ
	GLsizeiptr colorsSize = numParticlePoints * 4 * sizeof(GLfloat);  //RGBA
	
	glBindBuffer(GL_ARRAY_BUFFER, particlePositionsVBO);
	glBufferData(GL_ARRAY_BUFFER, positionsSize, NULL, GL_STATIC_DRAW);
	GLfloat* positions = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	glBindBuffer(GL_ARRAY_BUFFER, particleColorsVBO);
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
	for (int i=0;i<maxNumParticles;i++)
	{
		if (particles[i]->getNumLivePositions() > 0)
		{
			particles[i]->getCurrentXYZ(&currentX, &currentY, &currentZ);

			positions[(index*3)] = scaler->getScaledLonX(currentX);
			positions[(index*3)+1] = -scaler->getMaxScaledDepth()-scaler->getScaledDepth(currentZ); //SWAPPED and negated
			positions[(index*3)+2] = scaler->getScaledLatY(currentY); //SWAPPED

			opacity = particles[i]->getFadeInFadeOutOpacity();

			//depthColorFactor = particles[i]->positions[particles[i]->getLivePosition(j) * 3 + 2]*.00015;//particles[i]->positions.at(j).z*.001;
					
			colors[(index*4)+0] = particles[i]->color[0];
			colors[(index*4)+1] = particles[i]->color[1];//COLOR_0_G - depthColorFactor;
			colors[(index*4)+2] = particles[i]->color[2];
			colors[(index*4)+3] = opacity;
			//printf("P %d's opacity is %f\n", i, opacity);
							
			index++;

		}//end if has a live position	
	}//end for each particle

	glBindBuffer(GL_ARRAY_BUFFER, particlePositionsVBO);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, particleColorsVBO);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glColorPointer(4, GL_FLOAT, 0, NULL);
	
}//end loadVBOs()

void IllustrativeParticleSystem::drawParticleVBOs()
{
	if (!particleBuffersGenerated || numParticlePoints < 1)
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
	
	glBindBuffer(GL_ARRAY_BUFFER, particlePositionsVBO);
	glVertexPointer(3, GL_FLOAT, 0, (char*)NULL);

	glBindBuffer(GL_ARRAY_BUFFER, particleColorsVBO);
	glColorPointer(4, GL_FLOAT, 0, (char*)NULL);

	glDrawArrays(GL_POINTS, 0, numParticlePoints);
	
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
	return lastCountLiveParticles;// maxNumParticles;//particles.size();
}

int IllustrativeParticleSystem::getNumLiveSeeds()
{
	return lastCountLiveSeeds;// maxNumParticles;//particles.size();
}

int IllustrativeParticleSystem::getNumDyePoles()
{
	return dyePoles.size();
}

int IllustrativeParticleSystem::getNumDyeEmitters()
{
	int total = 0;
	for (int i=0;i<dyePoles.size();i++)
		total += dyePoles.at(i)->emitters.size();
	return total;
}

int IllustrativeParticleSystem::getNumDyeParticles()
{
	return lastCountLiveParticles-lastCountLiveSeeds;
}

int IllustrativeParticleSystem::addDyePole(double x, double y, float minZ, float maxZ)
{
	IllustrativeDyePole* tempDP = new IllustrativeDyePole(x, y, minZ, maxZ, scaler);
	//tempDP->adddEmittersAlongEntireLength(100);
	tempDP->addDefaultEmitter();
	for (int i=0;i<tempDP->getNumEmitters();i++)
	{
		tempDP->changeEmitterColor(i,(dyePoles.size()%8)+1);	
		//float rate = 300*((float)(rand()%100)/100) + 500;  //randomize the spawn rates so they dont all come out in sync
		//tempDP->changeEmitterRate(i,5000);//rate);
		//tempDP->changeEmitterLifetime(i, 40000);
		//tempDP->changeEmitterTrailtime(i, 1000);

	}
	dyePoles.push_back(tempDP);
	return dyePoles.size()-1;
}

void IllustrativeParticleSystem::drawDyePoles()
{
	for (int i=0;i<dyePoles.size();i++)
		dyePoles.at(i)->drawSmall3D();  //TEMP FOR CHRIS
}

void IllustrativeParticleSystem::deleteAllDyePoles()
{
	for (int i=0;i<dyePoles.size();i++)
		dyePoles.at(i)->kill();
	dyePoles.clear();
}

void IllustrativeParticleSystem::deleteDyePole(int index)
{
	dyePoles.at(index)->kill();
	dyePoles.erase(dyePoles.begin()+index);
}

IllustrativeDyePole* IllustrativeParticleSystem::getDyePoleClosestTo(double x, double y)
{
	Vec3 target(x,y,0);
	float minDist = 10000000;
	int closestIndex;
	float dist;
	for (int i=0;i<dyePoles.size();i++)
	{
		Vec3 candidate(dyePoles.at(i)->x, dyePoles.at(i)->y, 0);
		dist = target.dist(candidate);
		if (dist < minDist)
		{
			minDist = dist;
			closestIndex = i;
		}
	}
	return dyePoles.at(closestIndex);
}

IllustrativeParticleEmitter * IllustrativeParticleSystem::getDyePotClosestTo(float x, float y, float z)
{
	Vec3 target(x, y, z);
	float minDist = 10000000.f;
	int closestIndex = -1;
	float dist;
	for (int i = 0; i < dyePots.size(); i++)
	{
		Vec3 candidate(dyePots.at(i)->x, dyePots.at(i)->y, (dyePots.at(i)->depthTop - dyePots.at(i)->depthBottom) * 0.5f);
		dist = target.dist(candidate);
		if (dist < minDist)
		{
			minDist = dist;
			closestIndex = i;
		}
	}

	if (closestIndex < 0)
		return NULL;

	return dyePots.at(closestIndex);
}

void IllustrativeParticleSystem::pause()
{
	pauseTime = GetTickCount64();	
}

void IllustrativeParticleSystem::unPause()
{
	ULONGLONG elapsedTime = GetTickCount64()-pauseTime;

	for (int i=0;i<maxNumParticles;i++)
	{
		if (!particles[i]->dead)
		{
			particles[i]->birthTime += elapsedTime;
			particles[i]->lastUpdateTimestamp += elapsedTime;
			for (int j=0;j<100;j++)
				particles[i]->times[j] += elapsedTime;
		}
	}//end for each particle

}//end unPause()
