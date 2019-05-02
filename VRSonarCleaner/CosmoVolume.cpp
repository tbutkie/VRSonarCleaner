#include "CosmoVolume.h"

#include "Renderer.h"

using namespace std::chrono_literals;

CosmoVolume::CosmoVolume(std::vector<std::string> flowGrids, bool useZInsteadOfDepth, bool fgFile)
	: DataVolume(
		glm::vec3(0.f, 1.f, 0.f), 
		glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		glm::vec3(1.f))
	, m_msLoopTime(35s)
	, m_fFlowRoomMinTime(std::numeric_limits<float>::max())
	, m_fFlowRoomMaxTime(std::numeric_limits<float>::min())
{
	//X-Right-Left
	//Y-UP-vertical
	//Z-toward monitor

	for (auto fg : flowGrids)
		addCosmoGrid(fg);
		
	m_fFlowRoomTime = m_fFlowRoomMinTime;
	m_tpLastTimeUpdate = std::chrono::high_resolution_clock::now();
}

CosmoVolume::~CosmoVolume()
{

}

void CosmoVolume::addCosmoGrid(std::string fileName)
{
	CosmoGrid* tempCG = new CosmoGrid(fileName.c_str());
	m_vpCosmoGrids.push_back(tempCG);
	add(tempCG);

	if (tempCG->m_fMinTime < m_fFlowRoomMinTime)
		m_fFlowRoomMinTime = tempCG->m_fMinTime;
	if (tempCG->m_fMaxTime > m_fFlowRoomMaxTime)
		m_fFlowRoomMaxTime = tempCG->m_fMaxTime;
}

void CosmoVolume::removeCosmoGrid(std::string fileName)
{
	auto fgPresent = std::find_if(m_vpCosmoGrids.begin(), m_vpCosmoGrids.end(), [&fileName](FlowGrid* fg) { return strcmp(fg->getName(), fileName.c_str()) == 0; });

	if (fgPresent != m_vpCosmoGrids.end())
	{
		remove(*fgPresent);
		m_vpCosmoGrids.erase(fgPresent);
	}
}

glm::vec3 CosmoVolume::getFlowWorldCoords(glm::vec3 pt_WorldCoords)
{
	glm::vec3 domainPt = convertToRawDomainCoords(pt_WorldCoords);

	for (auto &fg : m_vpCosmoGrids)
	{
		float u, v, w;
		if (fg->getUVWat(domainPt.x, domainPt.y, domainPt.z, 0, &u, &v, &w))
			return convertToWorldCoords(glm::dvec3(domainPt) + glm::dvec3(u, v, w)) - pt_WorldCoords;		
	}
		return glm::vec3(0.f);
}

void CosmoVolume::recalcVolumeBounds()
{
	glm::vec3 minCoords(std::numeric_limits<float>::max());
	glm::vec3 maxCoords(std::numeric_limits<float>::min());

	for (auto fg : m_vpCosmoGrids)
	{
		if (fg)
		{
			if (fg->getXMin() < minCoords.x)
				minCoords.x = fg->getXMin();
			if (fg->getXMax() > maxCoords.x)
				maxCoords.x = fg->getXMax();

			if (fg->getYMin() < minCoords.y)
				minCoords.y = fg->getYMin();
			if (fg->getYMax() < maxCoords.y)
				maxCoords.y = fg->getYMax();

			if (fg->getZMin() < minCoords.z)
				minCoords.z = fg->getZMin();
			if (fg->getZMax() < maxCoords.z)
				maxCoords.z = fg->getZMax();
		}
	}

	//setCustomBounds(minCoords, maxCoords);
}

std::vector<glm::vec3> CosmoVolume::getStreamline(glm::vec3 pos, float propagation_unit, int propagation_max_units, float terminal_speed, bool clipToDomain)
{
	std::vector<glm::vec3> streamline;
	streamline.push_back(pos);

	for (int i = 0; i < propagation_max_units; ++i)
	{
		glm::vec3 flowhere = interpolate(streamline.back());

		// Short-circuit loop if at or below terminal speed
		if (glm::length(flowhere) <= terminal_speed)
			break;

		glm::vec3 newPos = rk4(streamline.back(), propagation_unit);

		if (newPos == streamline.back() || (clipToDomain && !inBounds(newPos)))
			break;
		else
			streamline.push_back(newPos);
	}

	return streamline;
}

bool CosmoVolume::inBounds(glm::vec3 pos)
{
	if (pos.x >= -1.f && pos.x <= 1.f &&
		pos.y >= -1.f && pos.y <= 1.f &&
		pos.z >= -1.f && pos.z <= 1.f)
		return true;

	return false;
}

glm::vec3 CosmoVolume::rk4(glm::vec3 pos, float delta)
{
	if (!inBounds(pos))
		return pos;

	glm::vec3 k1 = interpolate(pos);

	glm::vec3 y1 = pos + k1 * delta * 0.5f;

	glm::vec3 k2 = interpolate(y1);

	glm::vec3 y2 = pos + k2 * delta * 0.5f;

	glm::vec3 k3 = interpolate(y2);

	glm::vec3 y3 = pos + k3 * delta;

	glm::vec3 k4 = interpolate(y3);

	glm::vec3 newPos = pos + delta * (k1 + 2.f * k2 + 2.f * k3 + k4) / 6.f;

	//check in bounds or not
	if (!inBounds(newPos))
		return pos;

	return newPos;
}

void CosmoVolume::update()
{
	// update the data volume first
	DataVolume::update();

	//update time
	auto tick = std::chrono::high_resolution_clock::now();
	auto timeSinceLast = tick - m_tpLastTimeUpdate;
	if (timeSinceLast > 20ms)
	{
		m_tpLastTimeUpdate = tick;
		float currentTime = m_fFlowRoomTime;
		float minTime = m_fFlowRoomMinTime;
		float maxTime = m_fFlowRoomMaxTime;
		float timeRange = maxTime - minTime;

		if (timeRange > 0.000001f)
		{
			if (true) ///playing v paused
			{
				float factorToAdvance = std::chrono::duration<float, std::milli>(timeSinceLast).count() / std::chrono::duration<float, std::milli>(m_msLoopTime).count();
				float newTime = currentTime + (factorToAdvance * timeRange);
				if (newTime > maxTime)
					newTime = minTime;
				m_fFlowRoomTime = newTime;
			}
		}//end if timerange greater than zero
		else //no time range
		{
			//playing = false;
		}
	}//end if need update
}

void CosmoVolume::draw()
{
	
}