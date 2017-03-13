#pragma once
#include "SingleControllerBehavior.h"

#include "DataVolume.h"

class ProbeBehavior :
	public SingleControllerBehavior
{
public:
	ProbeBehavior(ViveController* controller, DataVolume* dataVolume);
	virtual ~ProbeBehavior();

	glm::mat4 getPose();

	void update();

	void draw();

private:
	DataVolume* m_pDataVolume;

	bool m_bShowProbe;

	const float c_fTouchDeltaThreshold;
	glm::vec2 m_vec2InitialMeasurementPoint; // The touch point from which we will take displacement measurements

	bool m_bVerticalSwipeMode;
	bool m_bHorizontalSwipeMode;

	float m_fProbeOffsetAmount;
	float m_fProbeOffsetAmountMin;
	float m_fProbeOffsetAmountMax;
	float m_fProbeInitialOffsetAmount;
	glm::vec3 m_vec3ProbeOffsetDirection;

	float m_fProbeRadius; 
	float m_fProbeRadiusMin;
	float m_fProbeRadiusMax;

private:
	virtual void receiveEvent(const int event, void* payloadData);
	virtual void activateProbe();
};

