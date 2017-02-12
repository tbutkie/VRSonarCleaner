#include "IllustrativeParticle.h"

IllustrativeParticle::IllustrativeParticle(float x, float y, float z, float TimeToLive, float TrailTime, ULONGLONG currentTime)
{
	reset();
	m_ullTimeToStartDying = currentTime + TimeToLive;
	m_vec3StartingPosition = glm::vec3(x, y, z);
	m_fTrailTime = TrailTime;
	m_bUserCreated = false;
	m_vec3Color.r = 0.25;
	m_vec3Color.g = 0.95;
	m_vec3Color.b = 1.0;
	m_fGravity = 0;

	m_iFlowGridIndex = -1;

	m_ullBirthTime = currentTime;

	m_vullTimes.resize(MAX_NUM_POSITIONS);
	m_vullTimes[0] = currentTime;

	m_vvec3Positions.resize(MAX_NUM_POSITIONS);

	m_vvec3Positions[0] = m_vec3StartingPosition;
	m_vvec3Positions[1] = m_vec3StartingPosition;

	m_iLiveStartIndex = 0;
	m_iLiveEndIndex = 1;
	m_ullLiveTimeElapsed = 0ull;
}

IllustrativeParticle::~IllustrativeParticle()
{

}

void IllustrativeParticle::reset()
{
	m_bDead = false;
	m_bDying = false;
	//updated = false;
	m_ullLiveTimeElapsed = 0ull;
	m_iLiveStartIndex = 0;
	m_iLiveEndIndex = 0;
}

void IllustrativeParticle::reset(float x, float y, float z)
{
	reset();
	m_vec3StartingPosition = glm::vec3(x, y, z);
}

void IllustrativeParticle::kill()
{
	//printf("K");
	m_bDead = true;
	m_bDying = true;
	//updated = false;
	m_iLiveStartIndex = 0;
	m_iLiveEndIndex = 0;
	m_ullLiveTimeElapsed = 0ull;
}

//returns true if needs to be deleted
void IllustrativeParticle::updatePosition(ULONGLONG currentTime, float newX, float newY, float newZ)
{
	if (m_bDead)
		return;
	else
		m_ullLastUpdateTimestamp = currentTime;

	glm::vec3 m_vec3NewPos(newX, newY, newZ);

	if (currentTime >= m_ullTimeToStartDying && !m_bDying)
	{
		m_ullTimeOfDeath = currentTime;
		m_bDying = true;
	}

	if (!m_bDying)//translate particle, filling next spot in array with new position/timestamp
	{
		if (m_iLiveEndIndex < MAX_NUM_POSITIONS)//no wrap needed, just fill next spot
		{
			m_vvec3Positions[m_iLiveEndIndex] = m_vec3NewPos;
			m_vullTimes[m_iLiveEndIndex] = currentTime;
			m_iLiveEndIndex++;
		}
		else if (m_iLiveEndIndex == 0 || m_iLiveEndIndex == MAX_NUM_POSITIONS)//wrap around in progress or wrap around needed
		{
			m_vvec3Positions[0] = m_vec3NewPos;
			m_vullTimes[0] = currentTime;
			m_iLiveEndIndex = 1;
		}
	}//end if not dying

	//move liveStartIndex up past too-old positions
	if (m_iLiveStartIndex < m_iLiveEndIndex) //no wrap around
	{
		for (int i = m_iLiveStartIndex; i < m_iLiveEndIndex; i++)
		{
			m_ullTimeSince = currentTime - m_vullTimes[i];
			if (m_ullTimeSince > m_fTrailTime)
			{
				m_iLiveStartIndex = i + 1;
				if (m_iLiveStartIndex == MAX_NUM_POSITIONS)
				{
					m_iLiveStartIndex = 0;
					m_iLiveEndIndex = 0; //because liveEndIndex must have equaled MAX_NUM_POSITIONS
					break;
				}
			}
			else break;
		}
	}
	
	if (m_iLiveStartIndex > m_iLiveEndIndex) //wrap around
	{
		//check start to end of array
		foundValid = false;
		for (int i = m_iLiveStartIndex; i < MAX_NUM_POSITIONS; i++)
		{
			m_ullTimeSince = currentTime - m_vullTimes[i];
			if (m_ullTimeSince > m_fTrailTime)
			{
				m_iLiveStartIndex = i + 1;
				if (m_iLiveStartIndex == MAX_NUM_POSITIONS)
				{
					m_iLiveStartIndex = 0;
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
			for (int i = 0; i < m_iLiveEndIndex; i++)
			{
				m_ullTimeSince = currentTime - m_vullTimes[i];
				if (m_ullTimeSince > m_fTrailTime)
				{
					m_iLiveStartIndex = i+1;
				}
				else break;
			}
		}
	}
	
	if (m_bDying && m_iLiveStartIndex == m_iLiveEndIndex)
	{
		//printf("F");
		m_bDead = true;
	}
	
	m_ullLiveTimeElapsed = currentTime - m_vullTimes[m_iLiveStartIndex];

	return;
}


int IllustrativeParticle::getNumLivePositions()
{
	if (m_bDead)//	if (!updated || dead)
		return 0;
	if (m_iLiveStartIndex < m_iLiveEndIndex) //no wrap around
	{
		return m_iLiveEndIndex - m_iLiveStartIndex;
	}
	else if (m_iLiveStartIndex > m_iLiveEndIndex) //wrap around
	{
		return (MAX_NUM_POSITIONS - m_iLiveStartIndex) + m_iLiveEndIndex;
	}
	else if (m_iLiveStartIndex = m_iLiveEndIndex) //this should not happen
	{
		printf("ERROR in get num: liveStartIndex equals liveEndIndex!!!\n");
		return 0;
	}
}

int IllustrativeParticle::getLivePosition(int index)
{
	if (m_iLiveStartIndex + index < MAX_NUM_POSITIONS) //no wrap around needed
	{
		return m_iLiveStartIndex + index;
	}
	else //wrap around needed
	{
		return index - (MAX_NUM_POSITIONS - m_iLiveStartIndex);
	}
}

float IllustrativeParticle::getCurrentX()
{
	if (m_iLiveEndIndex == 0)
		return m_vvec3Positions[MAX_NUM_POSITIONS-1].x;
	else
		return m_vvec3Positions[m_iLiveEndIndex -1].x;
}

float IllustrativeParticle::getCurrentY()
{
	if (m_iLiveEndIndex == 0)
		return m_vvec3Positions[MAX_NUM_POSITIONS-1].y;
	else
		return m_vvec3Positions[m_iLiveEndIndex -1].y;
}

float IllustrativeParticle::getCurrentZ()
{
	if (m_iLiveEndIndex == 0)
		return m_vvec3Positions[MAX_NUM_POSITIONS - 1].z;
	else
		return m_vvec3Positions[m_iLiveEndIndex - 1].z;
}

void IllustrativeParticle::getCurrentXYZ(float *x, float *y, float *z)
{
	int index;
	if (m_iLiveEndIndex == 0)
	{
		index = MAX_NUM_POSITIONS-1;
	}
	else
	{
		index = m_iLiveEndIndex -1;
	}

	*x = m_vvec3Positions[index].x;
	*y = m_vvec3Positions[index].y;
	*z = m_vvec3Positions[index].z;
}

float IllustrativeParticle::getFadeInFadeOutOpacity()
{
	if (m_bDying)
	{
		float opacity = (m_ullLastUpdateTimestamp - m_ullTimeOfDeath) / m_fTrailTime;
		if (opacity > 1)
			return 0;
		else if (opacity < 0)
			return 1;
		else
			return (1-opacity);
	}
	else
	{
		float timeSinceStart = m_ullLastUpdateTimestamp - m_ullBirthTime;
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
	*r = m_vec3Color.r;
	*g = m_vec3Color.g;
	*b = m_vec3Color.b;
}

void IllustrativeParticle::setFlowGridIndex(int index)
{
	m_iFlowGridIndex = index;
}

int IllustrativeParticle::getFlowGridIndex()
{
	return m_iFlowGridIndex;
}