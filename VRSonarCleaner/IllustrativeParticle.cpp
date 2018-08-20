#include "IllustrativeParticle.h"
#include "utilities.h"

using namespace std::chrono_literals;

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

void IllustrativeParticle::init(glm::vec3 pos, glm::vec3 color, float gravity, std::chrono::milliseconds timeToLive, std::chrono::milliseconds trailTime, std::chrono::time_point<std::chrono::high_resolution_clock> currentTime, bool userCreated)
{
	m_bDead = false;
	m_bDying = false;
	m_tpTimeToStartDying = currentTime + timeToLive;
	m_vec3StartingPosition = pos;
	m_msTrailTime = trailTime;
	m_bUserCreated = userCreated;
	m_vec3Color = glm::vec3(0.25f, 0.95f, 1.f);
	m_fGravity = gravity;
	
	m_tpBirthTime = currentTime;
	m_vtpTimes[0] = currentTime;

	m_vvec3Positions[0] = m_vec3StartingPosition;
	m_vvec3Positions[1] = m_vec3StartingPosition;

	m_iBufferTail = 0;
	m_iBufferHead = 1;
	m_msLiveTimeElapsed = std::chrono::milliseconds::zero();

	m_vec3Color = color;
}

void IllustrativeParticle::reset()
{
	m_bDead = true;
	m_bDying = false;
	m_bUserCreated = false;
	m_fGravity = 0.f;
	m_msTimeToLive = 0ms;
	m_msTrailTime = 0ms;
	m_msLiveTimeElapsed = 0ms;
	m_msTimeSince = 0ms;
	m_iBufferTail = 0;
	m_iBufferHead = 0;
	m_tpBirthTime = std::chrono::time_point<std::chrono::high_resolution_clock>();
	m_tpLastUpdateTimestamp = std::chrono::time_point<std::chrono::high_resolution_clock>();
	m_tpTimeDeathBegan = std::chrono::time_point<std::chrono::high_resolution_clock>();
	m_tpTimeToStartDying = std::chrono::time_point<std::chrono::high_resolution_clock>();
	m_vec3Color = glm::vec3(0.f);
	m_vec3StartingPosition = glm::vec3(0.f);

	m_vtpTimes.clear();
	m_vtpTimes.resize(MAX_NUM_TRAIL_POSITIONS);
	m_vvec3Positions.clear();
	m_vvec3Positions.resize(MAX_NUM_TRAIL_POSITIONS);

	m_pFlowGrid = NULL;
}

//returns true if needs to be deleted
void IllustrativeParticle::updatePosition(std::chrono::time_point<std::chrono::high_resolution_clock> currentTime, float newX, float newY, float newZ)
{
	if (m_bDead)
		return;
	else
		m_tpLastUpdateTimestamp = currentTime;

	glm::vec3 m_vec3NewPos(newX, newY, newZ);

	if (currentTime >= m_tpTimeToStartDying && !m_bDying)
	{
		m_tpTimeDeathBegan = currentTime;
		m_bDying = true;
	}

	if (!m_bDying)//translate particle, filling next spot in array with new position/timestamp
	{
		m_vvec3Positions[m_iBufferHead] = m_vec3NewPos;
		m_vtpTimes[m_iBufferHead] = currentTime;
		m_iBufferHead = utils::getIndexWrapped(m_iBufferHead + 1, MAX_NUM_TRAIL_POSITIONS);
	}//end if not dying
	
	m_msLiveTimeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_vtpTimes[m_iBufferTail]);

	updateBufferIndices(currentTime);
}

void IllustrativeParticle::updateBufferIndices(std::chrono::time_point<std::chrono::high_resolution_clock> currentTime)
{
	for (int i = m_iBufferTail; i != m_iBufferHead; i = utils::getIndexWrapped(i + 1, MAX_NUM_TRAIL_POSITIONS))
	{
		m_msTimeSince = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_vtpTimes[i]);
		if (m_msTimeSince > m_msTrailTime)
			m_iBufferTail = utils::getIndexWrapped(i + 1, MAX_NUM_TRAIL_POSITIONS);
		else 
			break;
	}	

	if (m_bDying && m_iBufferTail == m_iBufferHead)
		m_bDead = true;
}


int IllustrativeParticle::getNumLivePositions()
{
	if (m_bDead)
		return 0;

	int numLivePos = 0;
	
	for(int i = m_iBufferTail; i != m_iBufferHead; i = utils::getIndexWrapped(i + 1, MAX_NUM_TRAIL_POSITIONS))
		numLivePos++;

	return numLivePos;
}

int IllustrativeParticle::getLivePosition(int index)
{
	return utils::getIndexWrapped(m_iBufferTail + index, MAX_NUM_TRAIL_POSITIONS);
}

float IllustrativeParticle::getCurrentX()
{
	return m_vvec3Positions[utils::getIndexWrapped(m_iBufferHead - 1, MAX_NUM_TRAIL_POSITIONS)].x;
}

float IllustrativeParticle::getCurrentY()
{
	return m_vvec3Positions[utils::getIndexWrapped(m_iBufferHead - 1, MAX_NUM_TRAIL_POSITIONS)].y;
}

float IllustrativeParticle::getCurrentZ()
{
	return m_vvec3Positions[utils::getIndexWrapped(m_iBufferHead - 1, MAX_NUM_TRAIL_POSITIONS)].z;
}

glm::vec3 IllustrativeParticle::getCurrentXYZ()
{
	return m_vvec3Positions[utils::getIndexWrapped(m_iBufferHead - 1, MAX_NUM_TRAIL_POSITIONS)];
}

float IllustrativeParticle::getFadeInFadeOutOpacity()
{
	if (m_bDying)
	{
		float opacity = std::chrono::duration<float, std::milli>(m_tpLastUpdateTimestamp - m_tpTimeDeathBegan).count() / m_msTrailTime.count();
		if (opacity > 1.f)
			return 0.f;
		else if (opacity < 0.f)
			return 1.f;
		else
			return (1.f - opacity);
	}
	else
	{
		std::chrono::milliseconds timeSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(m_tpLastUpdateTimestamp - m_tpBirthTime);
		if (timeSinceStart < 200ms) //first 200 ms
		{
			return static_cast<float>(timeSinceStart / 200ms);
		}
		else
			return 1.f;
	}
}

void IllustrativeParticle::getColor(float *r, float *g, float *b)
{
	*r = m_vec3Color.r;
	*g = m_vec3Color.g;
	*b = m_vec3Color.b;
}
