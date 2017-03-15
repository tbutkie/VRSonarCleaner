#include "FlowProbe.h"



FlowProbe::FlowProbe(ViveController* controller, FlowRoom* flowRoom)
	: ProbeBehavior(controller, flowRoom->getDataVolume())
	, m_bProbeActive(false)
	, m_pFlowRoom(flowRoom)
{
}


FlowProbe::~FlowProbe()
{
}

void FlowProbe::update()
{
}

void FlowProbe::activateProbe()
{
	m_bProbeActive = true;
	m_pFlowRoom->placeDyeEmitterWorldCoords(glm::vec3(getPose()[3]));
}

void FlowProbe::deactivateProbe()
{
	m_bProbeActive = false;
}
