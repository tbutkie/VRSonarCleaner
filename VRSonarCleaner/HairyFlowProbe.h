#pragma once
#include "ProbeBehavior.h"

#include "FlowVolume.h"

class HairyFlowProbe :
	public ProbeBehavior
{
public:
	HairyFlowProbe(TrackedDeviceManager* pTDM, FlowVolume* flowVolume);
	~HairyFlowProbe();

	void update();

	void draw();

private:
	bool m_bProbeActive;

	FlowVolume* m_pFlowVolume;

private:
	virtual void activateProbe();
	virtual void deactivateProbe();
};

