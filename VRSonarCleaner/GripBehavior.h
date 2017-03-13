#pragma once
#include "BehaviorBase.h"

#include "DataVolume.h"
#include "ViveController.h"

class GripBehavior :
	public BehaviorBase
{
public:
	GripBehavior(ViveController* controller, DataVolume* dataVolume);
	~GripBehavior();

	void update();

private:
	DataVolume* m_pDataVolume;
	ViveController* m_pController;

	bool m_bGripping;

private:
	void receiveEvent(const int event, void* payloadData);
};

