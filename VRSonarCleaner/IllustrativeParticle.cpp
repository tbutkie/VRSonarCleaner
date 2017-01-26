#include "IllustrativeParticle.h"

IllustrativeParticle::IllustrativeParticle(float x, float y, float z, float TimeToLive, float TrailTime, ULONGLONG currentTime)
{
	reset();
	timeToLive = TimeToLive;
	timeToStartDying = currentTime + TimeToLive;
	startingPosition[0] = x;
	startingPosition[1] = y;
	startingPosition[2] = z;
	trailTime = TrailTime;
	userCreated = false;
	color[0] = 0.25;
	color[1] = 0.95;
	color[2] = 1.0;
	oldLastSpeed = -1;
	lastSpeed = -1;
	gravity = 0;

	flowGridIndex = 0;

	birthTime = currentTime;
	times[0] = currentTime;
	positions[0] = startingPosition[0];
	positions[1] = startingPosition[1];
	positions[2] = startingPosition[2];

	positions[3] = startingPosition[0];
	positions[4] = startingPosition[1];
	positions[5] = startingPosition[2];

	liveStartIndex = 0;
	liveEndIndex = 1;
	liveTimeElapsed = .001;
}

IllustrativeParticle::~IllustrativeParticle()
{

}

void IllustrativeParticle::reset()
{
	dead = false;
	dying = false;
	//updated = false;
	liveTimeElapsed = .001;
	liveStartIndex = 0;
	liveEndIndex = 0;
}

void IllustrativeParticle::reset(float x, float y, float z)
{
	reset();
	startingPosition[0] = x;
	startingPosition[1] = y;
	startingPosition[2] = z;
}

void IllustrativeParticle::kill()
{
	//printf("K");
	dead = true;
	dying = true;
	//updated = false;
	liveStartIndex = 0;
	liveEndIndex = 0;
	liveTimeElapsed = .001;
}

//returns true if needs to be deleted
void IllustrativeParticle::updatePosition(ULONGLONG currentTime, float newX, float newY, float newZ)
{
	if (dead)
		return;
	else
		lastUpdateTimestamp = currentTime;
	//update time and check if should die off
	//if (!updated)
	//{
	//	updated = true;
	//	birthTime = currentTime;
	//	times[0] = currentTime;
	//	positions[0] = startingPosition[0];
	//	positions[1] = startingPosition[1];
	//	positions[2] = startingPosition[2];
	//	liveStartIndex = 0;
	//	liveEndIndex = 1;
	//	liveTimeElapsed = .001;
	//	return; //first update always just starts the particle at its starting position with the current time
	//}
	//else
	if (currentTime >= timeToStartDying && !dying)
	{
		timeOfDeath = currentTime;
		dying = true;
	}

	if (!dying)//translate particle, filling next spot in array with new position/timestamp
	{
		//timeSinceLast = currentTime - times[liveEndIndex-1];
		//if (liveEndIndex == 0)
		//{
		//	currentPos[0] = positions[(MAX_NUM_POSITIONS-1)*3];
		//	currentPos[1] = positions[(MAX_NUM_POSITIONS-1)*3+1];
		//	currentPos[2] = positions[(MAX_NUM_POSITIONS-1)*3+2];
		//}
		//else
		//{
		//	currentPos[0] = positions[(liveEndIndex-1)*3];
		//	currentPos[1] = positions[(liveEndIndex-1)*3+1];
		//	currentPos[2] = positions[(liveEndIndex-1)*3+2];
		//}
		//	
		if (liveEndIndex < MAX_NUM_POSITIONS)//no wrap needed, just fill next spot
		{
			positions[liveEndIndex*3] = newX;
			positions[liveEndIndex*3+1] = newY;
			positions[liveEndIndex*3+2] = newZ;
			times[liveEndIndex] = currentTime;
			liveEndIndex++;
		}
		else if (liveEndIndex == 0 || liveEndIndex == MAX_NUM_POSITIONS)//wrap around in progress or wrap around needed
		{
			positions[0] = newX;
			positions[1] = newY;
			positions[2] = newZ;
			times[0] = currentTime;
			liveEndIndex = 1;
		}
		//store speed of this movement
		//oldLastSpeed = lastSpeed;
		//lastSpeed = sqrt( (newPos[0]-currentPos[0])*(newPos[0]-currentPos[0]) + (newPos[1]-currentPos[1])* (newPos[1]-currentPos[1]) +  (newPos[2]-currentPos[2])* (newPos[2]-currentPos[2]) ) / timeSinceLast;
		//check if moved away from starting position yet
		//if (!movedAway)
		//{
		//	if ( sqrt( (newPos[0]-startingPosition[0])*(newPos[0]-startingPosition[0]) + (newPos[1]-startingPosition[1])* (newPos[1]-startingPosition[1]) +  (newPos[2]-startingPosition[2])* (newPos[2]-startingPosition[2]) ) > MOVED_AWAY_THRESHOLD)
		//	{
		//		movedAway = true;
		//	}
		//}
		//else //check if back at start
		//{
		//	if (!movedBack)
		//	{
		//		if ( sqrt( (newPos[0]-startingPosition[0])*(newPos[0]-startingPosition[0]) + (newPos[1]-startingPosition[1])* (newPos[1]-startingPosition[1]) +  (newPos[2]-startingPosition[2])* (newPos[2]-startingPosition[2]) ) < MOVED_BACK_THRESHOLD)
		//		{
		//			movedBack = true;
		//		}
		//	}
		//}
	}//end if not dying

	//move liveStartIndex up past too-old positions
	if (liveStartIndex < liveEndIndex) //no wrap around
	{
		for (int i=liveStartIndex;i<liveEndIndex;i++)
		{
			timeSince = currentTime-times[i];
			if (timeSince > trailTime)
			{
				liveStartIndex = i+1;
				if (liveStartIndex == MAX_NUM_POSITIONS)
				{
					liveStartIndex = 0;
					liveEndIndex = 0; //because liveEndIndex must have equaled MAX_NUM_POSITIONS
					break;
				}
			}
			else break;
		}
	}
	
	if (liveStartIndex > liveEndIndex) //wrap around
	{
		//check start to end of array
		foundValid = false;
		for (int i=liveStartIndex;i<MAX_NUM_POSITIONS;i++)
		{
			timeSince = currentTime-times[i];
			if (timeSince > trailTime)
			{
				liveStartIndex = i+1;
				if (liveStartIndex == MAX_NUM_POSITIONS)
				{
					liveStartIndex = 0;
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
			for (int i=0;i<liveEndIndex;i++)
			{
				timeSince = currentTime-times[i];
				if (timeSince > trailTime)
				{
					liveStartIndex = i+1;
					/*if (liveStartIndex == liveEndIndex)
					{
						if (dying)
						{
							dead = true;
							return true;
						}
					}*/
				}
				else break;
			}
		}
	}
	
	if (dying && liveStartIndex == liveEndIndex)
	{
		//printf("F");
		dead = true;
	}
	if (dying)
		liveTimeElapsed = currentTime - times[liveStartIndex] + 0.001;
	else
		liveTimeElapsed = currentTime - times[liveStartIndex] + 0.001;
	return;
}


int IllustrativeParticle::getNumLivePositions()
{
	if (dead)//	if (!updated || dead)
		return 0;
	if (liveStartIndex < liveEndIndex) //no wrap around
	{
		return liveEndIndex - liveStartIndex;
	}
	else if (liveStartIndex > liveEndIndex) //wrap around
	{
		return (MAX_NUM_POSITIONS - liveStartIndex ) + liveEndIndex;
	}
	else if (liveStartIndex = liveEndIndex) //this should not happen
	{
		printf("ERROR in get num: liveStartIndex equals liveEndIndex!!!\n");
		return 0;
	}
}

int IllustrativeParticle::getLivePosition(int index)
{
	if (liveStartIndex + index < MAX_NUM_POSITIONS) //no wrap around needed
	{
		return liveStartIndex + index;
	}
	else //wrap around needed
	{
		return index - (MAX_NUM_POSITIONS - liveStartIndex);
	}
}

float IllustrativeParticle::getLastSpeed()
{
	return lastSpeed;	
}

float IllustrativeParticle::getOldLastSpeed()
{
	return oldLastSpeed;	
}

float IllustrativeParticle::getCurrentX()
{
	if (liveEndIndex == 0)
		return positions[(MAX_NUM_POSITIONS-1)*3];
	else
		return positions[(liveEndIndex-1)*3];	
}

float IllustrativeParticle::getCurrentY()
{
	if (liveEndIndex == 0)
		return positions[(MAX_NUM_POSITIONS-1)*3+1];
	else
		return positions[(liveEndIndex-1)*3+1];
}

float IllustrativeParticle::getCurrentZ()
{
	if (liveEndIndex == 0)
		return positions[(MAX_NUM_POSITIONS-1)*3+2];
	else
		return positions[(liveEndIndex-1)*3+2];
}

void IllustrativeParticle::getCurrentXYZ(float *x, float *y, float *z)
{
	int index;
	if (liveEndIndex == 0)
	{
		index = (MAX_NUM_POSITIONS-1)*3;
	}
	else
	{
		index = (liveEndIndex-1)*3;
	}

	*x = positions[index];
	*y = positions[index+1];
	*z = positions[index+2];
}

float IllustrativeParticle::getFadeInFadeOutOpacity()
{
	if (dying)
	{
		float opacity = (lastUpdateTimestamp-timeOfDeath)/trailTime;
		if (opacity > 1)
			return 0;
		else if (opacity < 0)
			return 1;
		else
			return (1-opacity);
	}
	else
	{
		float timeSinceStart = lastUpdateTimestamp - birthTime;
		if (timeSinceStart < 200) //first 200 ms
		{
			return (timeSinceStart)/200;
		}
		else
			return 1;
	}
}

void IllustrativeParticle::getColor(float *r, float *g, float *b)
{
	*r = color[0];
	*g = color[1];
	*b = color[2];
}

void IllustrativeParticle::setFlowGridIndex(int index)
{
	flowGridIndex = index;
}

int IllustrativeParticle::getFlowGridIndex()
{
	return flowGridIndex;
}