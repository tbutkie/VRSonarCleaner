#include "FlowVolume.h"

#include "Renderer.h"

using namespace std::chrono_literals;

FlowVolume::FlowVolume(std::vector<std::string> flowGrids, bool useZInsteadOfDepth)
	: DataVolume(
		glm::vec3(0.f, 1.f, 0.f), 
		glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		glm::vec3(1.f))
	, m_msLoopTime(35s)
	, m_bParticleSystemUpdating(false)
	, m_fFlowRoomMinTime(std::numeric_limits<float>::max())
	, m_fFlowRoomMaxTime(std::numeric_limits<float>::min())
{
	//X-Right-Left
	//Y-UP-vertical
	//Z-toward monitor

	for (auto fg : flowGrids)
	{
		FlowGrid* tempFG = new FlowGrid(fg.c_str(), useZInsteadOfDepth);
		m_vpFlowGrids.push_back(tempFG);
		add(tempFG);

		if (tempFG->m_fMinTime < m_fFlowRoomMinTime)
			m_fFlowRoomMinTime = tempFG->m_fMinTime;
		if (tempFG->m_fMaxTime > m_fFlowRoomMaxTime)
			m_fFlowRoomMaxTime = tempFG->m_fMaxTime;
	}
		
	m_fFlowRoomTime = m_fFlowRoomMinTime;
	m_tpLastTimeUpdate = std::chrono::high_resolution_clock::now();
	
	m_pParticleSystem = new IllustrativeParticleSystem(m_vpFlowGrids);
}

FlowVolume::~FlowVolume()
{

}

void FlowVolume::recalcVolumeBounds()
{
	glm::vec3 minCoords(std::numeric_limits<float>::max());
	glm::vec3 maxCoords(std::numeric_limits<float>::min());

	for (auto fg : m_vpFlowGrids)
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

			float zMin = fg->m_bUsesZInsteadOfDepth ? fg->getMinDepth() : -fg->getMinDepth();
			float zMax = fg->m_bUsesZInsteadOfDepth ? fg->getMaxDepth() : -fg->getMaxDepth();

			if (zMin < minCoords.z)
				minCoords.z = zMin;
			if (zMax < maxCoords.z)
				maxCoords.z = zMax;
		}
	}

	//setCustomBounds(minCoords, maxCoords);
}

IllustrativeParticleEmitter* FlowVolume::placeDyeEmitterWorldCoords(glm::vec3 pos)
{
	glm::vec3 innerPos = convertToRawDomainCoords(pos);

	printf("Dye In:  %0.4f, %0.4f, %0.4f\n", innerPos.x, innerPos.y, innerPos.z);

	IllustrativeParticleEmitter *tmp = new IllustrativeParticleEmitter(innerPos.x, innerPos.y, innerPos.z);
	tmp->setRate(0.f);
	tmp->changeColor(m_pParticleSystem->m_vpDyePots.size() % 9);
	m_pParticleSystem->m_vpDyePots.push_back(tmp);

	return tmp;
}

bool FlowVolume::removeDyeEmitterClosestToWorldCoords(glm::vec3 pos)
{
	glm::vec3 innerPos = convertToRawDomainCoords(pos);

	printf("Deleting Dye Pot Closest to:  %0.4f, %0.4f, %0.4f\n", innerPos.x, innerPos.y, innerPos.z);

	IllustrativeParticleEmitter *tmp = m_pParticleSystem->getDyePotClosestTo(innerPos.x, innerPos.y, innerPos.z);
	if (tmp)
	{
		m_pParticleSystem->m_vpDyePots.erase(std::remove(m_pParticleSystem->m_vpDyePots.begin(), m_pParticleSystem->m_vpDyePots.end(), tmp), m_pParticleSystem->m_vpDyePots.end());
		delete tmp;
	}
	return false;
}

void FlowVolume::draw()
{
	if (m_Future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
	{
		std::chrono::high_resolution_clock::time_point before = std::chrono::high_resolution_clock::now();
		m_pParticleSystem->prepareForRender();
		std::chrono::duration<float, std::milli> dur = std::chrono::high_resolution_clock::now() - before;

		//std::cout << "\t\t" << dur.count() << "ms\tParticle System Data Transfer" << std::endl;

		m_bParticleSystemUpdating = false;
	}

	if (m_pParticleSystem->getIndexCount() < 2)
		return;

	Renderer::RendererSubmission rs;

	rs.glPrimitiveType = GL_LINES;
	rs.shaderName = "flat";	
	rs.VAO = m_pParticleSystem->getVAO();
	rs.vertCount = m_pParticleSystem->getIndexCount();
	rs.indexType = GL_UNSIGNED_INT;
	rs.hasTransparency = true;
	rs.transparencySortPosition = getCurrentVolumeTransform()[3];
	rs.modelToWorldTransform = getRawDomainToVolumeTransform();

	Renderer::getInstance().addToDynamicRenderQueue(rs);
}

void FlowVolume::preRenderUpdates()
{
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

	//m_pParticleSystem->update(m_fFlowRoomTime);
	if (!m_bParticleSystemUpdating)
	{
		m_Future = std::async(std::launch::async, [&] { m_pParticleSystem->update(m_fFlowRoomTime); });
		m_bParticleSystemUpdating = true;
	}
}

void FlowVolume::setParticleVelocityScale(float velocityScale)
{
	for (auto fg : m_vpFlowGrids)
		if (fg)
			fg->m_fIllustrativeParticleVelocityScale = velocityScale;
}

float FlowVolume::getParticleVelocityScale()
{
	for (auto fg : m_vpFlowGrids)
		if (fg)
			return fg->m_fIllustrativeParticleVelocityScale;
	
	return -1.f;
}
