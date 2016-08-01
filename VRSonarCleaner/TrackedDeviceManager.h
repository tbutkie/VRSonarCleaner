#pragma once

#include <openvr.h>

#include "TrackedDevice.h"
#include "ViveController.h"
#include "CGLRenderModel.h"


class TrackedDeviceManager
{
public:
	TrackedDeviceManager(vr::IVRSystem* pHMD);
	~TrackedDeviceManager();

	void init();

	void handleEvents();

	void processVREvent(const vr::VREvent_t & event);
	void updateControllerStates();

	bool getCleaningCursorData(Matrix4 *thisCursorPose, Matrix4 *lastCursorPose, float *radius);
	void cleaningHit();

	void renderTrackedDevices(Matrix4 & matVP);
	void postRenderUpdate();

	Matrix4 & getHMDPose();

	void UpdateHMDMatrixPose();
private:
	void TrackedDeviceManager::setupRenderModels();
	CGLRenderModel* findOrLoadRenderModel(const char *pchRenderModelName);
	void setupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	bool createShaders();
	void renderDeviceModels(Matrix4 & matVP);
	
	std::string getTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL);
	
	vr::IVRSystem *m_pHMD;

	TrackedDevice* m_rpTrackedDevices[vr::k_unMaxTrackedDeviceCount];
	ViveController* editingController;

	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;

	std::string m_strPoseClasses;                         // what classes we saw poses for this frame

	GLuint m_unRenderModelProgramID;
	GLint m_nRenderModelMatrixLocation;

	Matrix4 m_mat4HMDPose;

	
	std::vector< CGLRenderModel * > m_vecRenderModels;
	CGLRenderModel *m_rTrackedDeviceToRenderModel[vr::k_unMaxTrackedDeviceCount];
};

