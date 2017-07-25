#include "FlowProbe.h"



FlowProbe::FlowProbe(ViveController* controller, FlowVolume* flowVolume)
	: ProbeBehavior(controller, flowVolume)
	, m_bProbeActive(false)
	, m_pFlowVolume(flowVolume)
{
	m_pEmitter = m_pFlowVolume->placeDyeEmitterWorldCoords(getPosition());
	m_pEmitter->setRate(100.f);
	m_pEmitter->incrementColor();
}


FlowProbe::~FlowProbe()
{
}

void FlowProbe::update()
{
	//m_pEmitter->setRate(1.f + (m_pController->getTriggerPullAmount() / 0.85f) * 9.f);
	m_pEmitter->setTrailTime(2500.f - 2000.f * (m_pController->getTriggerPullAmount() / 0.85f));

	glm::vec3 innerPos = m_pDataVolume->convertToInnerCoords(getPosition());
	m_pEmitter->x = innerPos.x;
	m_pEmitter->y = innerPos.y;
	m_pEmitter->z = innerPos.z;
}

void FlowProbe::draw()
{
	drawProbe(m_fProbeOffset);
}

void FlowProbe::activateProbe()
{
	m_bProbeActive = true;
	//m_pEmitter = m_pFlowVolume->placeDyeEmitterWorldCoords(getPosition());
	m_pEmitter->incrementColor();
}

void FlowProbe::deactivateProbe()
{
	m_bProbeActive = false;
}
