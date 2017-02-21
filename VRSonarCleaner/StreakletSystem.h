#pragma once
#include "ParticleSystem.h"
#include "CoordinateScaler.h"
#include "FlowGrid.h"
#include "DataVolume.h"

#include <random>

class StreakletSystem :
	public ParticleSystem
{
public:
	struct ConstructionInfo {
		DataVolume *dataVolume;
		CoordinateScaler *scaler;
		FlowGrid *flowGrid;
	};

public:
	StreakletSystem(int numParticles, glm::vec3 pos, ConstructionInfo *ci = NULL);
	~StreakletSystem();

	virtual void setParticleDefaults(Particle &p);

	virtual bool update(float time);

private:
	DataVolume *m_pDataVolume;
	CoordinateScaler *m_pScaler;
	FlowGrid* m_pFlowGrid;

	float m_fParticleVelocityScale;

	std::mt19937 m_RandGenerator;
	std::uniform_int_distribution<int> m_xDistrib;
	std::uniform_int_distribution<int> m_yDistrib;
	std::uniform_int_distribution<int> m_zDistrib;

private:
	void updateParticle(Particle *p, unsigned long long elapsedTime, glm::vec3 *newPos);
};

