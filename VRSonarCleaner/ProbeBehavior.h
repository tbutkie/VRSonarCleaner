#pragma once
#include "BehaviorBase.h"
#include "ViveController.h"
#include "DataVolume.h"

#include <chrono>

class ProbeBehavior :
	public BehaviorBase
{
public:
	ProbeBehavior(ViveController* controller, DataVolume* dataVolume);
	virtual ~ProbeBehavior();

	void activateDemoMode();
	void deactivateDemoMode();

	glm::vec3 getPosition();
	glm::vec3 getLastPosition();
	glm::mat4 getProbeToWorldTransform();
	glm::mat4 getLastProbeToWorldTransform();

	virtual void update();

	virtual void draw() = 0;

protected:
	ViveController *m_pController;
	DataVolume* m_pDataVolume;

	bool m_bShowProbe;

	const float c_fTouchDeltaThreshold;
	glm::vec2 m_vec2InitialMeasurementPoint; // The touch point from which we will take displacement measurements

	bool m_bVerticalSwipeMode;
	bool m_bHorizontalSwipeMode;

	float m_fProbeOffset;
	float m_fProbeOffsetMin;
	float m_fProbeOffsetMax;
	float m_fProbeInitialOffset;
	glm::vec3 m_vec3ProbeOffsetDirection;

	float m_fProbeRadius; 
	float m_fProbeRadiusMin;
	float m_fProbeRadiusMax;
	float m_fProbeRadiusInitial;

protected:
	virtual void activateProbe() = 0;
	virtual void deactivateProbe() = 0;

	virtual void drawProbe(float length);
};

