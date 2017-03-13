#pragma once
#include "DualCOntrollerBehavior.h"

#include "DataVolume.h"
#include "ViveController.h"

class ManipulateDataVolumeBehavior :
	public DualControllerBehavior
{
public:
	ManipulateDataVolumeBehavior(ViveController* gripController, ViveController* scaleController, DataVolume* dataVolume);
	~ManipulateDataVolumeBehavior();

	void update();

private:
	DataVolume* m_pDataVolume;
	ViveController* m_pGripController;
	ViveController* m_pScaleController;

	bool m_bGripping;
	bool m_bScaling;
	float m_fInitialDistance;
	glm::vec3 m_vec3InitialScale;

private:
	void receiveEvent(const int event, void* payloadData);
	float controllerDistance();
};

