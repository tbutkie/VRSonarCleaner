#pragma once
#include "ProbeBehavior.h"

#include "FlowVolume.h"

class FlowFieldCurator :
	public ProbeBehavior
{
public:
	FlowFieldCurator(TrackedDeviceManager* pTDM, FlowVolume* flowVolume);
	~FlowFieldCurator();

	void update();

	void draw();

private:
	bool m_bProbeActive;

	FlowVolume* m_pFlowVolume;
	IllustrativeParticleEmitter* m_pEmitter;

private:
	virtual void activateProbe();
	virtual void deactivateProbe();
};

