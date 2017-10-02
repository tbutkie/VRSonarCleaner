#pragma once
#include "DualControllerBehavior.h"

#include "DataVolume.h"

#include <queue>
#include <functional>

class TutorialBehavior :
	public DualControllerBehavior
{
	typedef std::function<void()> InitFunc;
	typedef std::function<bool()> UpdateFunc;
	typedef std::function<void()> CleanupFunc;
	typedef std::tuple<InitFunc, UpdateFunc, CleanupFunc> TutorialEntry;

public:
	TutorialBehavior(ViveController* gripController, ViveController* scaleController, DataVolume* dataVolume);
	~TutorialBehavior();

	void update();

	void draw();

private:
	std::queue<TutorialEntry> m_qTutorialQueue;

	DataVolume* m_pDataVolume;

	bool m_bPreGripping;
	bool m_bGripping;
	bool m_bScaling;
	float m_fInitialDistance;
	glm::vec3 m_vec3InitialDimensions;

	//rotate action
	bool m_bRotationInProgress; // Data Volume Rotating
	glm::mat4 m_mat4DataVolumePoseAtRotationStart; // Data Volume Initial Position and Orientation when Rotating
	glm::mat4 m_mat4ControllerPoseAtRotationStart; // Controller's Initial Position and Orientation when Rotating
	glm::mat4 m_mat4ControllerToVolumeTransform; // Transformation from Controller Space to Data Volume Space

private:
	void receiveEvent(const int event, void* payloadData);

	void createTutorialQueue();

	float controllerDistance();

	void startRotation();
	void continueRotation();
	void endRotation();
	bool isBeingRotated();

	void preRotation(float ratio);
};

