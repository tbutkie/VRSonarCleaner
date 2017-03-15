#pragma once
#include "ProbeBehavior.h"

#include "FlowRoom.h"

class FlowProbe :
	public ProbeBehavior
{
public:
	FlowProbe(ViveController* controller, FlowRoom* flowRoom);
	~FlowProbe();

	void update();

private:
	bool m_bProbeActive;

	FlowRoom* m_pFlowRoom;
	IllustrativeParticleEmitter* m_pEmitter;

private:
	void activateProbe();
	void deactivateProbe();
};

