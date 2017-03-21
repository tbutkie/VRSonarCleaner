#pragma once
#include "ProbeBehavior.h"

#include "FlowVolume.h"

class FlowProbe :
	public ProbeBehavior
{
public:
	FlowProbe(ViveController* controller, FlowVolume* flowVolume);
	~FlowProbe();

	void update();

private:
	bool m_bProbeActive;

	FlowVolume* m_pFlowVolume;
	IllustrativeParticleEmitter* m_pEmitter;

private:
	void activateProbe();
	void deactivateProbe();
};

