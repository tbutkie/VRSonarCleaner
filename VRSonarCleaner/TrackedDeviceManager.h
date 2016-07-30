#pragma once

#include <openvr.h>

#include "TrackedDevice.h"
#include "CGLRenderModel.h"


class TrackedDeviceManager
{
public:
	TrackedDeviceManager(vr::IVRSystem* pHMD);
	~TrackedDeviceManager();

	void init();

	void handleEvents();

	void processVREvent(const vr::VREvent_t & event);
	void processControllerEvent(const vr::VREvent_t & event);
	void updateControllerStates();

	float getCleaningCursorData(Matrix4 * thisCursorPose, Matrix4 * lastCursorPose);
	void cleaningHit();

	void prepareControllersForRendering();
	void renderTrackedDevices(Matrix4 & matVP);
	void postRenderUpdate();

	Matrix4 & getHMDPose();

	void UpdateHMDMatrixPose();
private:
	void TrackedDeviceManager::setupRenderModels();
	CGLRenderModel* findOrLoadRenderModel(const char *pchRenderModelName);
	void setupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	bool createShaders();
	GLuint CompileGLShader(const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader);
	void renderControllers(Matrix4 & matVP);
	void renderDeviceModels(Matrix4 & matVP);

	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);

	std::string getTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL);
	
	vr::IVRSystem *m_pHMD;

	TrackedDevice* m_rpTrackedDevices[vr::k_unMaxTrackedDeviceCount];

	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;

	std::string m_strPoseClasses;                         // what classes we saw poses for this frame
	char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];  // for each device, a character representing its class
	
	GLuint m_glControllerVertBuffer;
	GLuint m_unControllerVAO;
	unsigned int m_uiControllerVertcount;

	GLuint m_unControllerTransformProgramID;
	GLuint m_unRenderModelProgramID;

	GLint m_nControllerMatrixLocation;
	GLint m_nRenderModelMatrixLocation;

	Matrix4 m_mat4HMDPose;

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];

	Matrix4 m_rmat4DeviceCursorCurrentPose[vr::k_unMaxTrackedDeviceCount];
	Matrix4 m_rmat4DeviceCursorLastPose[vr::k_unMaxTrackedDeviceCount];

	std::vector< CGLRenderModel * > m_vecRenderModels;
	CGLRenderModel *m_rTrackedDeviceToRenderModel[vr::k_unMaxTrackedDeviceCount];

	float cursorRadius, cursorRadiusMin, cursorRadiusMax, cursorOffsetAmount, cursorOffsetAmountMin, cursorOffsetAmountMax;
	Vector4 cursorOffsetDirection;
};

