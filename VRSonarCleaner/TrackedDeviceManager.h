#pragma once

#include <openvr.h>

#include "TrackedDevice.h"
#include "ViveController.h"
#include "EditingController.h"
#include "CGLRenderModel.h"
#include "BroadcastSystem.h"

#include <shared/glm/glm.hpp>

#include <map>

class TrackedDeviceManager : public BroadcastSystem::Broadcaster
{
public:
	TrackedDeviceManager(vr::IVRSystem* pHMD);
	~TrackedDeviceManager();

	bool BInit();

	void handleEvents();

	virtual void attach(BroadcastSystem::Listener *obs);
	virtual void detach(BroadcastSystem::Listener *obs);

	bool cleaningModeActive();
	bool getCleaningCursorData(glm::mat4 *thisCursorPose, glm::mat4 *lastCursorPose, float *radius);
	bool getManipulationData(glm::mat4 &controllerPose);
	void cleaningHit();

	glm::mat4& getHMDPose();
	glm::mat4& getEditControllerPose();
	glm::mat4& getManipControllerPose();

	void updateTrackedDeviceRenderModels();
	void updateTrackedDevices();

	void renderControllerCustomizations(glm::mat4 *matVP);

private:
	void TrackedDeviceManager::initDevices();
	bool setupTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	void removeTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);

	std::string getPropertyString(vr::TrackedDeviceIndex_t deviceID, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL);
	
	vr::IVRSystem *m_pHMD;
	vr::IVRRenderModels *m_pRenderModels;

	TrackedDevice* m_rpTrackedDevices[vr::k_unMaxTrackedDeviceCount];
	EditingController* m_pPrimaryController;
	ViveController* m_pSecondaryController;

	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;

	std::string m_strPoseClasses;                         // what classes we saw poses for this frame

	glm::mat4 m_mat4HMDPose;
};

