#pragma once
#include "DualControllerBehavior.h"

#include "DataVolume.h"

class SelectAreaBehavior :
	public DualControllerBehavior
{
public:
	SelectAreaBehavior(ViveController* primaryController, ViveController* secondaryController, DataVolume* dataVolume);
	~SelectAreaBehavior();

	void update();

	void draw();

private:
	DataVolume* m_pDataVolume;

private:
	void receiveEvent(const int event, void* payloadData);
};

