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

	std::vector<std::vector<glm::vec3>> m_vvvec3RawStreamlines;
	std::vector<glm::vec3> m_vvec3StreamlineSeedsDomain;

	GLuint m_glVBO, m_glEBO, m_glVAO;
	GLuint m_glHaloVBO, m_glHaloVAO;

	void sampleCuttingPlane(bool reseed);
	void sampleVolume(unsigned int gridRes = 10u);
	void buildStreamTubes();

private:
	virtual void activateProbe();
	virtual void deactivateProbe();
};

