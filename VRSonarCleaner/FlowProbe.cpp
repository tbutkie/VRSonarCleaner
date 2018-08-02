#include "FlowProbe.h"

using namespace std::chrono_literals;

FlowProbe::FlowProbe(TrackedDeviceManager* pTDM, FlowVolume* flowVolume)
	: ProbeBehavior(pTDM, flowVolume)
	, m_bProbeActive(false)
	, m_pFlowVolume(flowVolume)
	, m_pEmitter(NULL)
{
}


FlowProbe::~FlowProbe()
{
	if (m_pEmitter)
		m_pFlowVolume->removeDyeEmitterClosestToWorldCoords(m_pTDM->getPrimaryController() ? getPosition() : m_pFlowVolume->getPosition());
}

void FlowProbe::update()
{
	if (!m_pEmitter)
	{
		m_pEmitter = m_pFlowVolume->placeDyeEmitterWorldCoords(m_pTDM->getPrimaryController() ? getPosition() : m_pFlowVolume->getPosition());
		m_pEmitter->incrementColor();
		m_pEmitter->setRadius(5.f);
	}

	ProbeBehavior::update();

	if (m_pTDM->getPrimaryController())
	{
		m_pEmitter->setRate(1000.f + (m_pTDM->getPrimaryController()->getTriggerPullAmount() / 0.85f) * 24000.f);
		//m_pEmitter->setTrailTime(std::chrono::duration_cast<std::chrono::milliseconds>(2500ms - (2000ms) * (m_pTDM->getPrimaryController()->getTriggerPullAmount() / 0.85f)));

		glm::vec3 innerPos = m_pDataVolume->convertToRawDomainCoords(getPosition());
		m_pEmitter->x = innerPos.x;
		m_pEmitter->y = innerPos.y;
		m_pEmitter->z = innerPos.z;
	}
	else
	{
		m_pEmitter->setRate(0.f);
	}
}

void FlowProbe::draw()
{
	if (m_pTDM->getPrimaryController())
		drawProbe(m_fProbeOffset);
}

void FlowProbe::activateProbe()
{
	m_bProbeActive = true;
	//m_pEmitter = m_pFlowVolume->placeDyeEmitterWorldCoords(getPosition());
	m_pEmitter->incrementColor();

	if (m_pTDM->getPrimaryController())
	{
		auto pos = getPosition();
		if (m_pDataVolume->isWorldCoordPointInDomainBounds(pos))
		{
			glm::dvec3 ptXform = m_pDataVolume->convertToRawDomainCoords(pos);
			printf("World Pos (%0.4f, %0.4f, %0.4f) is INSIDE the data volume at (%0.4f, %0.4f, %0.4f).\n", pos.x, pos.y, pos.z, ptXform.x, ptXform.y, ptXform.z);
		}
		else
			printf("World Pos (%0.4f, %0.4f, %0.4f) is outside the data volume.\n", pos.x, pos.y, pos.z);
	}
}

void FlowProbe::deactivateProbe()
{
	m_bProbeActive = false;
}
