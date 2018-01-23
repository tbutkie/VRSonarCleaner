#include "FlowVolume.h"

#include "Renderer.h"

using namespace std::chrono_literals;

FlowVolume::FlowVolume(FlowGrid* flowGrid)
	: DataVolume(
		glm::vec3(0.f, 1.f, 0.f), 
		glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		glm::vec3(1.f))
	, m_pFlowGrid(flowGrid)
	, m_msLoopTime(35s)
	, m_bParticleSystemUpdating(false)
{
	//X-Right-Left
	//Y-UP-vertical
	//Z-toward monitor

	add(flowGrid);

	m_pScaler = m_pFlowGrid->scaler;
	
	m_fFlowRoomMinTime = m_pFlowGrid->m_fMinTime;
	m_fFlowRoomMaxTime = m_pFlowGrid->m_fMaxTime;
	m_fFlowRoomTime = m_fFlowRoomMinTime;
	m_tpLastTimeUpdate = std::chrono::high_resolution_clock::now();
	
	std::vector<FlowGrid*> flowGrids;
	flowGrids.push_back(m_pFlowGrid);
	m_pParticleSystem = new IllustrativeParticleSystem(m_pScaler, flowGrids);
}

FlowVolume::~FlowVolume()
{

}

void FlowVolume::recalcVolumeBounds()
{
	glm::vec3 minCoords(
		m_pFlowGrid->getScaledXMin(),
		m_pFlowGrid->getScaledYMin(),
		-m_pFlowGrid->getScaledMaxDepth()
	);
	glm::vec3 maxCoords(
		m_pFlowGrid->getScaledXMax(),
		m_pFlowGrid->getScaledYMax(),
		-m_pFlowGrid->getScaledMinDepth()
	);

	//setInnerCoords(minCoords, maxCoords);
}

IllustrativeParticleEmitter* FlowVolume::placeDyeEmitterWorldCoords(glm::vec3 pos)
{
	glm::vec3 innerPos = convertToDataCoords(m_vpDatasets[0], pos);

	printf("Dye In:  %0.4f, %0.4f, %0.4f\n", innerPos.x, innerPos.y, innerPos.z);

	IllustrativeParticleEmitter *tmp = new IllustrativeParticleEmitter(innerPos.x, innerPos.y, innerPos.z, m_pScaler);
	tmp->setRate(10.f);
	tmp->changeColor(m_pParticleSystem->m_vpDyePots.size() % 9);
	m_pParticleSystem->m_vpDyePots.push_back(tmp);

	return tmp;
}

bool FlowVolume::removeDyeEmitterClosestToWorldCoords(glm::vec3 pos)
{
	glm::vec3 innerPos = convertToDataCoords(m_vpDatasets[0], pos);

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
	rs.modelToWorldTransform = getCurrentDataTransform(m_vpDatasets[0]);

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

		if (timeRange > 0)
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