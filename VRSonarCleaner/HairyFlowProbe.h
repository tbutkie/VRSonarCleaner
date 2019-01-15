#pragma once
#include "ProbeBehavior.h"

#include "FlowVolume.h"

class HairyFlowProbe :
	public ProbeBehavior
{
public:
	enum SEEDTYPE {
		PARTICLE,
		GLYPHS,
		STREAMTUBES
	};

public:
	HairyFlowProbe(ViveController* pController, FlowVolume* flowVolume);
	~HairyFlowProbe();

	void update();

	void draw();

private:
	bool m_bProbeActive;

	FlowVolume* m_pFlowVolume;

	SEEDTYPE m_eSeedType;

private:
	virtual void activateProbe();
	virtual void deactivateProbe();
};

