#include "IllustrativeParticle.h"

IllustrativeParticle::IllustrativeParticle()
	: m_pFlowGrid(NULL)
	, m_bDead(true)
	, m_bDying(false)
{
	reset();
}

IllustrativeParticle::~IllustrativeParticle()
{

}

void IllustrativeParticle::init(glm::vec3 pos, glm::vec3 color, float gravity, float timeToLive, float trailTime, ULONGLONG currentTime, bool userCreated)
{
	m_bDead = false;
	m_bDying = false;
	m_ullTimeToStartDying = currentTime + timeToLive;
	m_vec3StartingPosition = pos;
	m_fTrailTime = trailTime;
	m_bUserCreated = userCreated;
	m_vec3Color = glm::vec3(0.25f, 0.95f, 1.f);
	m_fGravity = gravity;
	
	m_ullBirthTime = currentTime;
	m_vullTimes[0] = currentTime;

	m_vvec3Positions[0] = m_vec3StartingPosition;
	m_vvec3Positions[1] = m_vec3StartingPosition;

	m_iBufferTail = 0;
	m_iBufferHead = 1;
	m_ullLiveTimeElapsed = 0ull;

	m_vec3Color = color;
}

void IllustrativeParticle::reset()
{
	m_bDead = true;
	m_bDying = false;
	m_bUserCreated = false;
	m_fGravity = 0.f;
	m_fTimeToLive = 0.f;
	m_fTrailTime = 0.f;
	m_iBufferTail = 0;
	m_iBufferHead = 0;
	m_ullBirthTime = 0ull;
	m_ullLastUpdateTimestamp = 0ull;
	m_ullLiveTimeElapsed = 0ull;
	m_ullTimeDeathBegan = 0ull;
	m_ullTimeSince = 0ull;
	m_ullTimeToStartDying = 0ull;
	m_vec3Color = glm::vec3(0.f);
	m_vec3StartingPosition = glm::vec3(0.f);

	m_vullTimes.clear();
	m_vullTimes.resize(MAX_NUM_TRAIL_POSITIONS);
	m_vvec3Positions.clear();
	m_vvec3Positions.resize(MAX_NUM_TRAIL_POSITIONS);
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
		m_ullTimeDeathBegan = currentTime;
		m_bDying = true;
	}

	if (!m_bDying)//translate particle, filling next spot in array with new position/timestamp
	{
		if (m_iBufferHead < MAX_NUM_TRAIL_POSITIONS)//no wrap needed, just fill next spot
		{
			m_vvec3Positions[m_iBufferHead] = m_vec3NewPos;
			m_vullTimes[m_iBufferHead] = currentTime;
			m_iBufferHead++;
		}
		else if (m_iBufferHead == 0 || m_iBufferHead == MAX_NUM_TRAIL_POSITIONS)//wrap around in progress or wrap around needed
		{
			m_vvec3Positions[0] = m_vec3NewPos;
			m_vullTimes[0] = currentTime;
			m_iBufferHead = 1;
		}
	}//end if not dying
	
	m_ullLiveTimeElapsed = currentTime - m_vullTimes[m_iBufferTail];

	updateBufferIndices(currentTime);
}

void IllustrativeParticle::updateBufferIndices(ULONGLONG currentTime)
{
	//move liveStartIndex up past too-old positions
	if (m_iBufferTail < m_iBufferHead) //no wrap around
	{
		for (int i = m_iBufferTail; i < m_iBufferHead; i++)
		{
			m_ullTimeSince = currentTime - m_vullTimes[i];
			if (m_ullTimeSince > m_fTrailTime)
			{
				m_iBufferTail = i + 1;
				if (m_iBufferTail == MAX_NUM_TRAIL_POSITIONS)
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
		for (int i = m_iBufferTail; i < MAX_NUM_TRAIL_POSITIONS; i++)
		{
			m_ullTimeSince = currentTime - m_vullTimes[i];
			if (m_ullTimeSince > m_fTrailTime)
			{
				m_iBufferTail = getWrappedIndex(i + 1);
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
}


int IllustrativeParticle::getNumLivePositions()
{
	if (m_bDead)//	if (!updated || dead)
		return 0;
	if (m_iBufferTail < m_iBufferHead) //no wrap around
	{
		return m_iBufferHead - m_iBufferTail;
	}
	else if (m_iBufferTail > m_iBufferHead) //wrap around
	{
		return (MAX_NUM_TRAIL_POSITIONS - m_iBufferTail) + m_iBufferHead;
	}
	else if (m_iBufferTail = m_iBufferHead) //this should not happen
	{
		printf("ERROR in get num: liveStartIndex equals liveEndIndex!!!\n");
		return 0;
	}
}

int IllustrativeParticle::getLivePosition(int index)
{
	if (m_iBufferTail + index < MAX_NUM_TRAIL_POSITIONS) //no wrap around needed
	{
		return m_iBufferTail + index;
	}
	else //wrap around needed
	{
		return index - (MAX_NUM_TRAIL_POSITIONS - m_iBufferTail);
	}
}

float IllustrativeParticle::getCurrentX()
{
	if (m_iBufferHead == 0)
		return m_vvec3Positions[MAX_NUM_TRAIL_POSITIONS -1].x;
	else
		return m_vvec3Positions[m_iBufferHead -1].x;
}

float IllustrativeParticle::getCurrentY()
{
	if (m_iBufferHead == 0)
		return m_vvec3Positions[MAX_NUM_TRAIL_POSITIONS -1].y;
	else
		return m_vvec3Positions[m_iBufferHead -1].y;
}

float IllustrativeParticle::getCurrentZ()
{
	if (m_iBufferHead == 0)
		return m_vvec3Positions[MAX_NUM_TRAIL_POSITIONS - 1].z;
	else
		return m_vvec3Positions[m_iBufferHead - 1].z;
}

glm::vec3 IllustrativeParticle::getCurrentXYZ()
{
	return m_vvec3Positions[getWrappedIndex(m_iBufferHead - 1)];
}

float IllustrativeParticle::getFadeInFadeOutOpacity()
{
	if (m_bDying)
	{
		float opacity = (m_ullLastUpdateTimestamp - m_ullTimeDeathBegan) / m_fTrailTime;
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

int IllustrativeParticle::getWrappedIndex(int index)
{
	return ((index % MAX_NUM_TRAIL_POSITIONS) + MAX_NUM_TRAIL_POSITIONS) % MAX_NUM_TRAIL_POSITIONS;
}
