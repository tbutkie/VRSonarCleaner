#pragma once

#include "BehaviorBase.h"
#include "TrackedDeviceManager.h"
#include "DataVolume.h"

class GrabDataVolumeBehavior :
	public BehaviorBase
{
public:
	GrabDataVolumeBehavior(TrackedDeviceManager* m_pTDM, DataVolume* dataVolume);
	virtual ~GrabDataVolumeBehavior();

	void update();

	void draw();

private:
	TrackedDeviceManager* m_pTDM;
	DataVolume* m_pDataVolume;

	bool m_bPreGripping;
	bool m_bGripping;

	//rotate action
	bool m_bRotationInProgress; // Data Volume Rotating
	glm::mat4 m_mat4DataVolumePoseAtRotationStart; // Data Volume Initial Position and Orientation when Rotating
	glm::mat4 m_mat4ControllerPoseAtRotationStart; // Controller's Initial Position and Orientation when Rotating
	glm::mat4 m_mat4ControllerToVolumeTransform; // Transformation from Controller Space to Data Volume Space

private:
	void updateState();

	void startRotation();
	void continueRotation();
	void endRotation();
	bool isBeingRotated();

	void preRotation(float ratio);
};

