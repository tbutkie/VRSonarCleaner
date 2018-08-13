#pragma once
#include "SceneBase.h"
#include "TrackedDeviceManager.h"
#include "FlowVolume.h"

class FlowScene :
	public Scene
{
public:
	FlowScene(TrackedDeviceManager* pTDM);
	~FlowScene();

	void init();

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;
	FlowVolume* m_pFlowVolume;

private:
	bool loadRandomFlowGrid();
};

