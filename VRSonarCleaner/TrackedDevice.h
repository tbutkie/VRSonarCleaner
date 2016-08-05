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
	TrackedDevice(vr::TrackedDeviceIndex_t id);
	~TrackedDevice();

	vr::TrackedDeviceIndex_t getIndex();
	void setRenderModel(CGLRenderModel *renderModel);
	inline bool hasRenderModel() { return !(m_pTrackedDeviceToRenderModel == NULL); }

	void addComponent(uint32_t unComponentIndex, const char *pchComponentName, CGLRenderModel *pComponentRenderModel);
	inline size_t getComponentCount() { return m_Components.size();  }
	Matrix4 getComponentPose(uint32_t unComponentIndex);

	bool toggleAxes();
	bool axesActive();

	bool poseValid();

	char getClassChar();
	void setClassChar(char classChar);

	Matrix4 getPose();
	virtual bool updatePose(vr::TrackedDevicePose_t pose);

	virtual void prepareForRendering();

	virtual void render(Matrix4 & matVP); 
	void renderModel();
	void renderModelComponent(uint32_t unComponent);

protected:

	struct TrackedDeviceComponent {
		uint32_t m_unComponentIndex;
		const char *m_pchComponentName;
		CGLRenderModel *m_pComponentRenderModel;

		TrackedDeviceComponent()
			: m_unComponentIndex(-1)
			, m_pchComponentName("No name")
			, m_pComponentRenderModel(NULL)
		{}
	};


	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);
	bool createShader();

	vr::TrackedDeviceIndex_t m_unDeviceID;
	CGLRenderModel *m_pTrackedDeviceToRenderModel;
	std::string m_strRenderModelName;

	std::vector<TrackedDeviceComponent> m_Components;
	
	char m_ClassChar;   // for each device, a character representing its class
	
	vr::TrackedDevicePose_t m_Pose;

	Matrix4 m_mat4Pose;
	
	GLuint m_glVertBuffer;
	GLuint m_unVAO;
	unsigned int m_uiLineVertcount;
	unsigned int m_uiTriVertcount;
	GLuint m_unTransformProgramID;
	GLint m_nMatrixLocation;
	
	bool m_bShow;
	bool m_bShowAxes;
};

