#pragma once
#include "BehaviorBase.h"
#include "TrackedDeviceManager.h"
#include "FlowVolume.h"
#include <unordered_map>

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

	struct ControlPoint {
		glm::vec3 pos;
		glm::vec3 dir;
		glm::vec3 lamda;
	};

	std::unordered_map<std::string, ControlPoint> m_vCPs;

private:
	bool loadRandomFlowGrid();
	void loadMetaFile(std::string metaFileName);
};

