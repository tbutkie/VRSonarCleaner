#include "FlowVolume.h"
#include "DebugDrawer.h"

#include "ParticleManager.h"

#include "Renderer.h"

FlowVolume::FlowVolume(FlowGrid* flowGrid)
	: DataVolume(
		glm::vec3(0.f, 1.f, 0.f), 
		0, 
		glm::vec3(1.f), 
		glm::vec3(
			flowGrid->getScaledXMin(),
			flowGrid->getScaledYMin(),
			flowGrid->getScaledMinDepth()
		),
		glm::vec3(
			flowGrid->getScaledXMax(),
			flowGrid->getScaledYMax(),
			flowGrid->getScaledMaxDepth()
		))
	, m_pFlowGrid(flowGrid)
{
	//X-Right-Left
	//Y-UP-vertical
	//Z-toward monitor

	m_pScaler = m_pFlowGrid->scaler;
	
	m_fFlowRoomMinTime = m_pFlowGrid->m_fMinTime;
	m_fFlowRoomMaxTime = m_pFlowGrid->m_fMaxTime;
	m_fFlowRoomTime = m_fFlowRoomMinTime;
	m_ullLastTimeUpdate = GetTickCount64();
	
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
		m_pFlowGrid->getScaledMinDepth()
	);
	glm::vec3 maxCoords(
		m_pFlowGrid->getScaledXMax(),
		m_pFlowGrid->getScaledYMax(),
		m_pFlowGrid->getScaledMaxDepth()
	);

	setInnerCoords(minCoords, maxCoords);
}

IllustrativeParticleEmitter* FlowVolume::placeDyeEmitterWorldCoords(glm::vec3 pos)
{
	glm::vec3 innerPos = convertToInnerCoords(pos);

	printf("Dye In:  %0.4f, %0.4f, %0.4f\n", innerPos.x, innerPos.y, innerPos.z);

	IllustrativeParticleEmitter *tmp = new IllustrativeParticleEmitter(innerPos.x, innerPos.y, innerPos.z, m_pScaler);
	tmp->setRate(10.f);
	tmp->changeColor(m_pParticleSystem->m_vpDyePots.size() % 9);
	m_pParticleSystem->m_vpDyePots.push_back(tmp);

	return tmp;
}

bool FlowVolume::removeDyeEmitterClosestToWorldCoords(glm::vec3 pos)
{
	glm::vec3 innerPos = convertToInnerCoords(pos);

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
	//draw debug
	drawBBox();
	//drawBacking();
	//drawAxes();
	
	//draw model
	//DebugDrawer::getInstance().setTransform(getCurrentDataTransform());
	//m_pParticleSystem->drawDyePots();                
	//m_pParticleSystem->drawParticleVBOs();
	Renderer::RendererSubmission rs;

	rs.shaderName = "flat";
	rs.modelToWorldTransform = getCurrentDataTransform();

	if (m_pParticleSystem->prepareForRender(rs))
		Renderer::getInstance().addToDynamicRenderQueue(rs);
		
}

void FlowVolume::preRenderUpdates()
{
	//update time
	ULONGLONG tick = GetTickCount64();
	ULONGLONG timeSinceLast = tick - m_ullLastTimeUpdate;
	if (timeSinceLast > 20)
	{
		m_ullLastTimeUpdate = tick;
		float currentTime = m_fFlowRoomTime;
		float minTime = m_fFlowRoomMinTime;
		float maxTime = m_fFlowRoomMaxTime;
		float timeRange = maxTime - minTime;

		if (timeRange > 0)
		{
			if (true) ///playing v paused
			{
				float factorToAdvance = (float)timeSinceLast / 35000; //35 second loop time		
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

	//std::cout << "Updating particle system with time " << flowRoomTime << std::endl;
	m_pParticleSystem->update(m_fFlowRoomTime);
	//ParticleManager::getInstance().update(timeSinceLast);
	
	//m_pLighting->update();
}