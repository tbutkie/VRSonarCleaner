#pragma once

#include "BehaviorBase.h"
#include "TrackedDeviceManager.h"
#include "Object3D.h"

class GrabObjectBehavior :
	public BehaviorBase
{
public:
	GrabObjectBehavior(TrackedDeviceManager* m_pTDM, Object3D* object);
	virtual ~GrabObjectBehavior();

	void update();

	void draw();

	bool isBeingRotated();

private:
	TrackedDeviceManager* m_pTDM;
	Object3D* m_pObject;

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
};

