#pragma once
#include "BehaviorBase.h"
#include "TrackedDeviceManager.h"
#include "FlowVolume.h"

class FlowFieldCurator :
	public BehaviorBase
{
public:
	FlowFieldCurator(TrackedDeviceManager* pTDM, FlowVolume* flowVol);
	~FlowFieldCurator();

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;
	FlowVolume* m_pFlowVolume;

private:
	bool loadRandomFlowGrid();
};

