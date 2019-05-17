#include "CosmoVolume.h"

#include "Renderer.h"

using namespace std::chrono_literals;

CosmoVolume::CosmoVolume(std::string cosmoDataDir)
	: DataVolume(
		glm::vec3(0.f, 0.f, -0.f),
		glm::quat(),//glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		glm::vec3(0.2f))
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

std::vector<glm::vec3> CosmoVolume::getStreamline(glm::vec3 pos, float propagation_unit, int propagation_max_units, float terminal_speed, bool reverse, bool clipToDomain)
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

		glm::vec3 newPos = m_pCosmoGrid->rk4(streamline.back(), reverse ? -propagation_unit : propagation_unit);

		if (newPos == streamline.back() || (clipToDomain && !m_pCosmoGrid->contains(newPos)))
			break;
		else
			streamline.push_back(newPos);
	}

	return streamline;
}

float CosmoVolume::getRelativeVelocity(glm::vec3 pos)
{
	return (m_pCosmoGrid->getVelocityAt(pos) - m_pCosmoGrid->getMinVelocity()) / (m_pCosmoGrid->getMaxVelocity() - m_pCosmoGrid->getMinVelocity());
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