#pragma once
#include "ProbeBehavior.h"

#include "FlowVolume.h"

class FlowProbe :
	public ProbeBehavior
{
public:
	FlowProbe(TrackedDeviceManager* pTDM, FlowVolume* flowVolume);
	~FlowProbe();

	void update();

	void draw();

private:
	bool m_bProbeActive;

	FlowVolume* m_pFlowVolume;
	IllustrativeParticleEmitter* m_pEmitter;

	float m_fTipEmitterRadius;

private:
	virtual void activateProbe();
	virtual void deactivateProbe();
	void placeDyePot();
};

