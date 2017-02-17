#pragma once
#include "ParticleSystem.h"
#include "CoordinateScaler.h"
#include "FlowGrid.h"

class StreakletSystem :
	public ParticleSystem
{
public:
	struct ConstructionInfo {
		CoordinateScaler *scaler;
		std::vector<FlowGrid*> flowGridCollection;
	};

public:
	StreakletSystem(int numParticles, int streakLength, glm::vec3 pos, ConstructionInfo *ci = NULL);
	~StreakletSystem();

	virtual bool update(float time);

private:
	CoordinateScaler *m_pScaler;
	std::vector<FlowGrid*> m_vpFlowGridCollection;
};

