#pragma once
#include "ProbeBehavior.h"

#include "DataVolume.h"

class DebugProbe :
	public ProbeBehavior
{
public:
	DebugProbe(TrackedDeviceManager* pTDM, DataVolume* dataVolume);
	~DebugProbe();

	void update();

	void draw();

private:
	bool m_bProbeActive;

	DataVolume* m_pDataVolume;

private:
	virtual void activateProbe();
	virtual void deactivateProbe();
};

