#pragma once

#include <vector>
#include <windows.h>

#include <openvr.h>

#include <GL/glew.h>

#include "BroadcastSystem.h"
#include "Icosphere.h"

#include <shared/glm/glm.hpp>

class TrackedDevice : public BroadcastSystem::Broadcaster
{
	friend class TrackedDeviceManager;

public:
	TrackedDevice(vr::TrackedDeviceIndex_t id, vr::IVRSystem *pHMD, vr::IVRRenderModels * pRenderModels);
	~TrackedDevice();

	virtual bool BInit();

	vr::TrackedDeviceIndex_t getIndex();
	void setRenderModelName(std::string renderModelName);
	bool hasRenderModel();

	bool toggleAxes();
	bool axesActive();

	bool poseValid();

	char getClassChar();
	void setClassChar(char classChar);

	glm::mat4 getDeviceToWorldTransform();

	virtual void prepareForRendering();

	void render(glm::mat4 & matVP);

protected:
	struct TrackedDeviceComponent {
		uint32_t m_unComponentIndex;
		std::string m_strComponentName;
		std::string m_strComponentRenderModelName;
		vr::HmdMatrix34_t m_mat3PoseTransform;
		bool m_bInitialized;
		bool m_bHasRenderModel;
		bool m_bStatic;
		bool m_bVisible;
		bool m_bTouched;
		bool m_bPressed;
		bool m_bScrolled;

		TrackedDeviceComponent()
			: m_unComponentIndex(0)
			, m_strComponentName("No name")
			, m_strComponentRenderModelName("No render model name")
			, m_mat3PoseTransform(vr::HmdMatrix34_t())
			, m_bInitialized(false)
			, m_bHasRenderModel(false)
			, m_bStatic(false)
			, m_bVisible(false)
			, m_bTouched(false)
			, m_bPressed(false)
			, m_bScrolled(false)
		{}
	};

	std::vector<TrackedDeviceComponent> m_vComponents;

protected:
	bool createShaders();
	virtual bool updatePose(vr::TrackedDevicePose_t pose);
	glm::mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);
	vr::HmdMatrix34_t ConvertMatrix4ToSteamVRMatrix(const glm::mat4 &matPose);
	uint32_t getPropertyInt32(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL);

	vr::IVRSystem *m_pHMD;
	vr::IVRRenderModels *m_pRenderModels;

	vr::TrackedDeviceIndex_t m_unDeviceID;
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
	
	bool m_bShow;
	bool m_bShowAxes;
	bool m_bHasRenderModel;
};

