#pragma once
#include "DualControllerBehavior.h"

#include "DataVolume.h"

class ManipulateDataVolumeBehavior :
	public DualControllerBehavior
{
public:
	ManipulateDataVolumeBehavior(ViveController* gripController, ViveController* scaleController, DataVolume* dataVolume);
	~ManipulateDataVolumeBehavior();

	void update();

	void draw();

private:
	DataVolume* m_pDataVolume;
	ViveController* m_pGripController;
	ViveController* m_pScaleController;

	bool m_bGripping;
	bool m_bScaling;
	float m_fInitialDistance;
	glm::vec3 m_vec3InitialScale;

	//rotate action
	bool m_bRotationInProgress; // Data Volume Rotating
	glm::mat4 m_mat4DataVolumePoseAtRotationStart; // Data Volume Initial Position and Orientation when Rotating
	glm::mat4 m_mat4ControllerPoseAtRotationStart; // Controller's Initial Position and Orientation when Rotating
	glm::mat4 m_mat4ControllerToVolumePose; // Transformation from Controller Space to Data Volume Space

private:
	void receiveEvent(const int event, void* payloadData);
	float controllerDistance();

	void startRotation();
	void continueRotation();
	void endRotation();
	bool isBeingRotated();
};

