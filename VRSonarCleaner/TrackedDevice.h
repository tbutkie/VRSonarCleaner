#pragma once

#include <vector>
#include <windows.h>

#include <openvr.h>

#include "BroadcastSystem.h"
#include "CGLRenderModel.h"
#include "Icosphere.h"

#include <shared/glm/glm.hpp>

class TrackedDevice : public BroadcastSystem::Broadcaster
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

	glm::mat4 getDeviceToWorldTransform();
	virtual bool updateDeviceToWorldTransform(vr::TrackedDevicePose_t pose);

	virtual void prepareForRendering();

	virtual void render(glm::mat4 & matVP);
	virtual void renderModel(glm::mat4 & matVP);

protected:

	glm::mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);
	vr::HmdMatrix34_t ConvertMatrix4ToSteamVRMatrix(const glm::mat4 &matPose);
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

	glm::mat4 m_mat4DeviceToWorldTransform;
	
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

