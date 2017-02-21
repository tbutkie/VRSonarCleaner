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
	StreakletSystem(int numParticles, glm::vec3 pos, ConstructionInfo *ci = NULL);
	~StreakletSystem();

	virtual void setParticleDefaults(Particle &p);

	virtual bool update(float time);

private:
	CoordinateScaler *m_pScaler;
	FlowGrid* m_pFlowGrid;

	float m_fParticleVelocityScale;

private:
	void updateParticle(Particle *p, unsigned long long currentTime, glm::vec3 *newPos);
};

