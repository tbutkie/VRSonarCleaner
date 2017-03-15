#include "FlowProbe.h"



FlowProbe::FlowProbe(ViveController* controller, FlowRoom* flowRoom)
	: ProbeBehavior(controller, flowRoom->getDataVolume())
	, m_bProbeActive(false)
	, m_pFlowRoom(flowRoom)
{
	m_pEmitter = m_pFlowRoom->placeDyeEmitterWorldCoords(glm::vec3(getPose()[3]));
}


FlowProbe::~FlowProbe()
{
}

void FlowProbe::update()
{
	glm::vec3 innerPos = m_pDataVolume->convertToInnerCoords(glm::vec3(getPose()[3]));
	m_pEmitter->x = innerPos.x;
	m_pEmitter->y = innerPos.z;
	m_pEmitter->z = innerPos.y;
}

void FlowProbe::activateProbe()
{
	m_bProbeActive = true;
	m_pEmitter = m_pFlowRoom->placeDyeEmitterWorldCoords(glm::vec3(getPose()[3]));
}

void FlowProbe::deactivateProbe()
{
	m_bProbeActive = false;
}
