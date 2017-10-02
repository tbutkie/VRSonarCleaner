#pragma once

#include <openvr.h>

#include "TrackedDevice.h"
#include "ViveController.h"
#include "RenderModel.h"

#include <shared/glm/glm.hpp>

#include <map>

#include <future>

class TrackedDeviceManager
{
public:
	TrackedDeviceManager(vr::IVRSystem* pHMD);
	~TrackedDeviceManager();

	bool init();

	void handleEvents();

	void update();
	void draw();

	void hideBaseStations(bool hidden);

	glm::mat4& getWorldToHMDTransform();
	glm::mat4& getHMDToWorldTransform();
	uint32_t getDeviceComponentID(uint32_t deviceID, std::string componentName);
	glm::mat4 getDeviceComponentPose(uint32_t deviceID, uint32_t componentID);
	ViveController* getPrimaryController();
	ViveController* getSecondaryController();
	glm::mat4& getPrimaryControllerPose();
	glm::mat4& getSecondaryControllerPose();
	
	glm::mat4 getHMDEyeProjection(vr::Hmd_Eye nEye, float nearClipPlane, float farClipPlane);
	glm::mat4 getHMDEyeToHeadTransform(vr::Hmd_Eye nEye);

private:
	void TrackedDeviceManager::initDevices();
	bool setupTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	void removeTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	RenderModel* findOrLoadRenderModel(const char *pchRenderModelName);
	
	vr::IVRSystem *m_pHMD;
	vr::IVRRenderModels *m_pRenderModels;
	std::map<std::string, RenderModel*> m_mapModelCache;
	std::future<RenderModel*> m_futRenderModel;

	TrackedDevice* m_rpTrackedDevices[vr::k_unMaxTrackedDeviceCount];
	ViveController* m_pPrimaryController;
	ViveController* m_pSecondaryController;

	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;

	std::string m_strPoseClasses;

	glm::mat4 m_mat4WorldToHMDTransform;
	glm::mat4 m_mat4HMDToWorldTransform;
	
	float m_fCursorRadius;
	float m_fCursorRadiusMin;
	float m_fCursorRadiusMax;
	bool m_bCursorRadiusResizeMode;
	float m_fCursorRadiusResizeModeInitialRadius;

	float m_fCursorOffsetAmount;
	float m_fCursorOffsetAmountMin;
	float m_fCursorOffsetAmountMax;
	glm::vec3 m_vec3CursorOffsetDirection;
	bool m_bCursorOffsetMode;
	float m_fCursorOffsetModeInitialOffset;
	float m_fCursorHoopAngle;
};

