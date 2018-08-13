#pragma once
#include "BehaviorBase.h"
#include "TrackedDeviceManager.h"
#include "FlowVolume.h"

class FlowScene :
	public BehaviorBase
{
public:
	FlowScene(TrackedDeviceManager* pTDM);
	~FlowScene();

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;
	FlowVolume* m_pFlowVolume;

private:
	bool loadRandomFlowGrid();
};

