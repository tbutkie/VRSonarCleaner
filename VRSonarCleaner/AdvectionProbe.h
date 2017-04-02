#pragma once
#include "ProbeBehavior.h"
#include "FlowVolume.h"
#include "Icosphere.h"

class AdvectionProbe :
	public ProbeBehavior
{
public:
	AdvectionProbe(ViveController* controller, FlowVolume* flowVolume);
	~AdvectionProbe();

	void update();

private:
	FlowVolume* m_pFlowVolume;

private:
	void activateProbe();
	void deactivateProbe();

	glm::vec3 lineSphereIntersection(glm::vec3 linePoint0, glm::vec3 linePoint1, glm::vec3 circleCenter, float circleRadius);
};
