#pragma once

#include <openvr.h>

#include "TrackedDevice.h"
#include "CGLRenderModel.h"


class TrackedDeviceManager
{
public:
	TrackedDeviceManager(vr::IVRSystem* pHMD);
	~TrackedDeviceManager();

	void Init();

	void handleEvents();

	void ProcessVREvent(const vr::VREvent_t & event);
	void processControllerEvent(const vr::VREvent_t & event);
	void updateControllerStates();

private:
	void TrackedDeviceManager::SetupRenderModels();
	CGLRenderModel* FindOrLoadRenderModel(const char *pchRenderModelName);
	void SetupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);

	std::string GetTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL);

	vr::IVRSystem *m_pHMD;

	TrackedDevice* m_rpTrackedDevices[vr::k_unMaxTrackedDeviceCount];

	std::vector< CGLRenderModel* > m_vecRenderModels;
};

