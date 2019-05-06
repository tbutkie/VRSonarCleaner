#pragma once
#include "ProbeBehavior.h"

#include "CosmoVolume.h"

class HairyCosmoProbe :
	public ProbeBehavior
{
public:
	enum SEEDTYPE {
		PARTICLE,
		GLYPHS,
		STREAMTUBES
	};

public:
	HairyCosmoProbe(ViveController* pController, CosmoVolume* cosmoVolume);
	~HairyCosmoProbe();

	void update();

	void draw();

private:
	bool m_bProbeActive;

	CosmoVolume* m_pCosmoVolume;

	SEEDTYPE m_eSeedType;

private:
	virtual void activateProbe();
	virtual void deactivateProbe();
};

