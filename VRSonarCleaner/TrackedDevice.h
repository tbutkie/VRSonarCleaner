#pragma once

#include <vector>
#include <windows.h>

#include <openvr.h>

#include "CGLRenderModel.h"
#include "Icosphere.h"

#include "../shared/Matrices.h"

class TrackedDevice
{
public:
	TrackedDevice(vr::TrackedDeviceIndex_t id, vr::IVRSystem *pHMD, vr::IVRRenderModels *pRenderModels);
	~TrackedDevice();

	virtual bool BInit();

	vr::TrackedDeviceIndex_t getIndex();
	void setRenderModel(CGLRenderModel *renderModel);
	inline bool hasRenderModel() { return !(m_pTrackedDeviceToRenderModel == NULL); }

	bool toggleAxes();
	bool axesActive();

	bool poseValid();

	char getClassChar();
	void setClassChar(char classChar);

	Matrix4 getPose();
	virtual bool updatePose(vr::TrackedDevicePose_t pose);

	virtual void prepareForRendering();

	virtual void render(Matrix4 & matVP); 
	virtual void renderModel(Matrix4 & matVP);

protected:

	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);
	bool createShaders();
	CGLRenderModel* loadRenderModel(const char *pchRenderModelName);
	std::string getPropertyString(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL);
	uint32_t getPropertyInt32(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL);

	vr::IVRSystem *m_pHMD;
	vr::IVRRenderModels *m_pRenderModels;

	vr::TrackedDeviceIndex_t m_unDeviceID;
	CGLRenderModel *m_pTrackedDeviceToRenderModel;
	std::string m_strRenderModelName;
	
	char m_ClassChar;   // for each device, a character representing its class
	
	vr::TrackedDevicePose_t m_Pose;

	Matrix4 m_mat4Pose;
	
	GLuint m_glVertBuffer;
	GLuint m_unVAO;
	unsigned int m_uiLineVertcount;
	unsigned int m_uiTriVertcount;
	GLuint m_unTransformProgramID;
	GLint m_nMatrixLocation;

	GLuint m_unRenderModelProgramID;
	GLint m_nRenderModelMatrixLocation;
	
	bool m_bShow;
	bool m_bShowAxes;
};

