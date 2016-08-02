#pragma once

#include <vector>
#include <windows.h>

#include <openvr.h>

#include "CGLRenderModel.h"

#include "../shared/Matrices.h"

class TrackedDevice
{
public:
	TrackedDevice(vr::TrackedDeviceIndex_t id);
	~TrackedDevice();

	vr::TrackedDeviceIndex_t getIndex();
	void setRenderModel(CGLRenderModel *renderModel);

	bool toggleAxes();
	bool axesActive();

	bool poseValid();

	char getClassChar();
	void setClassChar(char classChar);

	Matrix4 getPose();
	virtual bool updatePose(vr::TrackedDevicePose_t pose);
	
	void renderModel();
	virtual void render(Matrix4 & matVP);

protected:
	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);
	bool createShader();

	vr::TrackedDeviceIndex_t m_DeviceID;
	CGLRenderModel *m_pTrackedDeviceToRenderModel;
	
	char m_ClassChar;   // for each device, a character representing its class
	
	vr::TrackedDevicePose_t m_Pose;

	Matrix4 m_mat4Pose;
	
	GLuint m_glControllerVertBuffer;
	GLuint m_unControllerVAO;
	unsigned int m_uiControllerVertcount;
	GLuint m_unControllerTransformProgramID;
	GLint m_nControllerMatrixLocation;
	
	bool m_bShow;
	bool m_bShowAxes;
};

