#pragma once

#include "BehaviorBase.h"
#include "TrackedDeviceManager.h"
#include "DataVolume.h"

class ScaleDataVolumeBehavior :
	public BehaviorBase
{
public:
	ScaleDataVolumeBehavior(TrackedDeviceManager* m_pTDM, DataVolume* dataVolume);
	virtual ~ScaleDataVolumeBehavior();

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;
	DataVolume* m_pDataVolume;

	bool m_bScaling;
	float m_fInitialDistance;
	glm::vec3 m_vec3InitialDimensions;

private:
	void updateState();

	float controllerDistance();
};

