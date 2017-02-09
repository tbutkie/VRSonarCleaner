#pragma once

#include <openvr.h>

#include "TrackedDevice.h"
#include "ViveController.h"
#include "EditingController.h"
#include "CGLRenderModel.h"
#include "BroadcastSystem.h"

class TrackedDeviceManager : public BroadcastSystem::Broadcaster
{
public:
	TrackedDeviceManager(vr::IVRSystem* pHMD);
	~TrackedDeviceManager();

	bool BInit();

	void handleEvents();

	void processVREvent(const vr::VREvent_t & event);

	void updateControllerStates();

	bool cleaningModeActive();
	bool getCleaningCursorData(Matrix4 *thisCursorPose, Matrix4 *lastCursorPose, float *radius);
	Matrix4* getManipulationData();
	void cleaningHit();

	void renderTrackedDevices(Matrix4 & matVP);
	void postRenderUpdate();

	Matrix4 & getHMDPose();
	Matrix4 & getEditControllerPose();
	Matrix4 & getManipControllerPose();

	void UpdateHMDMatrixPose();
private:
	void TrackedDeviceManager::initDevices();
	void setupTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	void removeTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	
	vr::IVRSystem *m_pHMD;
	vr::IVRRenderModels *m_pRenderModels;

	TrackedDevice* m_rpTrackedDevices[vr::k_unMaxTrackedDeviceCount];
	EditingController* m_pEditController;
	ViveController* m_pManipController;

	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;

	std::string m_strPoseClasses;                         // what classes we saw poses for this frame

	Matrix4 m_mat4HMDPose;
};

