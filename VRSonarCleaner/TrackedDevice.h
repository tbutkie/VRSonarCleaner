#pragma once

#include <vector>

#include <openvr.h>

#include <GL/glew.h>

#include "BroadcastSystem.h"

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
	glm::mat4 getPose();

	char getClassChar();
	void setClassChar(char classChar);

	glm::mat4 getDeviceToWorldTransform();

protected:
	struct TrackedDeviceComponent {
		uint32_t							m_unComponentIndex;
		std::string							m_strComponentName;
		std::string							m_strComponentRenderModelName;
		vr::RenderModel_ComponentState_t	m_State;
		vr::RenderModel_ComponentState_t	m_LastState;
		bool								m_bInitialized;
		bool								m_bHasRenderModel;

		TrackedDeviceComponent()
			: m_unComponentIndex			(0)
			, m_strComponentName			("No name")
			, m_strComponentRenderModelName	("No render model name")
			, m_bInitialized				(false)
			, m_bHasRenderModel				(false)
		{}

		bool isPressed()		{ return m_State.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsPressed; }
		bool wasPressed()		{ return m_LastState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsPressed; }
		bool justPressed()		{ return !wasPressed() && isPressed(); }
		bool justUnpressed()	{ return wasPressed() && !isPressed(); }
		bool continuePress()	{ return wasPressed() && isPressed(); }
		bool isTouched()		{ return m_State.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsTouched; }
		bool wasTouched()		{ return m_LastState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsTouched; }
		bool justTouched()		{ return !wasTouched() && isTouched(); }
		bool justUntouched()	{ return wasTouched() && !isTouched(); }
		bool continueTouch()	{ return wasTouched() && isTouched(); }
		bool isScrolled()		{ return m_State.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsScrolled; }
		bool wasScrolled()		{ return m_LastState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsScrolled; }
		bool isVisible()		{ return m_State.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsVisible; }
		bool wasVisible()		{ return m_LastState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsVisible; }
		bool isStatic()			{ return m_State.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsStatic; }
	};

	std::vector<TrackedDeviceComponent> m_vComponents;

protected:
	virtual bool update(vr::TrackedDevicePose_t pose);

	glm::mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);
	vr::HmdMatrix34_t ConvertMatrix4ToSteamVRMatrix(const glm::mat4 &matPose);
	uint32_t getPropertyInt32(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL);
	std::string getPropertyString(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL);

	vr::IVRSystem *m_pHMD;
	vr::IVRRenderModels *m_pRenderModels;

	vr::TrackedDeviceIndex_t m_unDeviceID;
	std::string m_strRenderModelName;
	
	char m_ClassChar;   // for each device, a character representing its class
	
	vr::TrackedDevicePose_t m_Pose;

	glm::mat4 m_mat4DeviceToWorldTransform;

	bool m_bShowAxes;
	bool m_bHasRenderModel;
	bool m_bHidden;
};

