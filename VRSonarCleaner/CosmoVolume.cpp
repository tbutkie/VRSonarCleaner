#include "CosmoVolume.h"

#include "Renderer.h"

using namespace std::chrono_literals;

CosmoVolume::CosmoVolume(std::string cosmoDataDir)
	: DataVolume(
		glm::vec3(0.f, 1.f, 0.f), 
		glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		glm::vec3(1.f))
	, m_msLoopTime(35s)
{
	m_pCosmoGrid = new CosmoGrid(cosmoDataDir.c_str());
	add(m_pCosmoGrid);
		
	m_tpLastTimeUpdate = std::chrono::high_resolution_clock::now();
}

CosmoVolume::~CosmoVolume()
{
	delete m_pCosmoGrid;
	m_pCosmoGrid = NULL;
}

glm::vec3 CosmoVolume::getFlowWorldCoords(glm::vec3 pt_WorldCoords)
{
	glm::vec3 domainPt = convertToRawDomainCoords(pt_WorldCoords);

	glm::vec3 flow = m_pCosmoGrid->getUVWat(domainPt);

	if (flow != glm::vec3())
		return convertToWorldCoords(glm::dvec3(domainPt) + glm::dvec3(flow)) - pt_WorldCoords;	

	return glm::vec3(0.f);
}

void CosmoVolume::recalcVolumeBounds()
{
	glm::vec3 minCoords(std::numeric_limits<float>::max());
	glm::vec3 maxCoords(std::numeric_limits<float>::min());

	if (m_pCosmoGrid->getXMin() < minCoords.x)
		minCoords.x = m_pCosmoGrid->getXMin();
	if (m_pCosmoGrid->getXMax() > maxCoords.x)
		maxCoords.x = m_pCosmoGrid->getXMax();

	if (m_pCosmoGrid->getYMin() < minCoords.y)
		minCoords.y = m_pCosmoGrid->getYMin();
	if (m_pCosmoGrid->getYMax() < maxCoords.y)
		maxCoords.y = m_pCosmoGrid->getYMax();

	if (m_pCosmoGrid->getZMin() < minCoords.z)
		minCoords.z = m_pCosmoGrid->getZMin();
	if (m_pCosmoGrid->getZMax() < maxCoords.z)
		maxCoords.z = m_pCosmoGrid->getZMax();

	//setCustomBounds(minCoords, maxCoords);
}

std::vector<glm::vec3> CosmoVolume::getStreamline(glm::vec3 pos, float propagation_unit, int propagation_max_units, float terminal_speed, bool clipToDomain)
{
	std::vector<glm::vec3> streamline;
	streamline.push_back(pos);

	for (int i = 0; i < propagation_max_units; ++i)
	{
		glm::vec3 flowhere = m_pCosmoGrid->getUVWat(streamline.back());
		float velhere = m_pCosmoGrid->getVelocityAt(streamline.back());

		// Short-circuit loop if at or below terminal speed
		if (velhere <= terminal_speed)
			break;

		glm::vec3 newPos = m_pCosmoGrid->rk4(streamline.back(), propagation_unit);

		if (newPos == streamline.back() || (clipToDomain && !m_pCosmoGrid->contains(newPos)))
			break;
		else
			streamline.push_back(newPos);
	}

	return streamline;
}

void CosmoVolume::update()
{
	// update the data volume first
	DataVolume::update();

	//update time
	auto tick = std::chrono::high_resolution_clock::now();
}

void CosmoVolume::draw()
{
	
}