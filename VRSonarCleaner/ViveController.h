#pragma once

#include <GL/glew.h>
#include <map>
#include <functional>

#include "TrackedDevice.h"

class ViveController :
	public TrackedDevice
{
public:
	enum VIVE_ACTION {
		SYSTEM_BUTTON_DOWN,	 // System button pressed
		SYSTEM_BUTTON_UP,	 // System button unpressed
		MENU_BUTTON_DOWN,	 // Menu button pressed
		MENU_BUTTON_UP,		 // Menu button unpressed
		GRIP_DOWN,			 // Grip button pressed
		GRIP_UP,			 // Grip button unpressed
		TRIGGER_ENGAGE,		 // Trigger analog initial engagement
		TRIGGER_PULL,		 // Trigger analog continued engagement
		TRIGGER_DISENGAGE,	 // Trigger analog disengagement
		TRIGGER_DOWN,		 // Trigger button pressed (clicked)
		TRIGGER_UP,			 // Trigger button unpressed (unclicked)
		TOUCHPAD_ENGAGE,	 // Touchpad initial touch
		TOUCHPAD_TOUCH,		 // Touchpad continued touch
		TOUCHPAD_DISENGAGE, // Touchpad untouch
		TOUCHPAD_DOWN,		 // Touchpad pressed (clicked)
		TOUCHPAD_UP			 // Touchpad unpressed (unclicked)
	};

public:
	ViveController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex, vr::IVRSystem *pHMD, vr::IVRRenderModels *pRenderModels);
	~ViveController();

	// Overridden from TrackedDevice
	bool BInit();
	virtual void prepareForRendering();

	virtual bool updatePose(vr::TrackedDevicePose_t pose);
	bool updateControllerState();

	bool isSystemButtonPressed();
	bool isMenuButtonPressed();
	bool isGripButtonPressed();
	bool isTriggerEngaged();
	bool isTriggerClicked();
	bool isTouchpadTouched();
	bool isTouchpadClicked();

	float getTriggerPullAmount();
	float getHairTriggerThreshold();

protected:
	glm::vec4 transformTouchPointToModelCoords(glm::vec2 *pt);
	void insertTouchpadCursor(std::vector<float> &vertices, unsigned int &nTriangleVertices, float r, float g, float b, float a);

	void initProfiles();
	void initDefaultProfile();
	void initEditingProfile();
	static bool s_bProfilesInitialized;
	std::map<VIVE_ACTION, std::function<void()>> *m_pmapCurrentProfile;
	static std::map<VIVE_ACTION, std::function<void()>> m_mapDefaultProfile;
	static std::map<VIVE_ACTION, std::function<void()>> m_mapEditingProfile;

	vr::TrackedDevicePose_t m_LastPose;

	vr::VRControllerState_t m_ControllerState;
	vr::VRControllerState_t m_LastControllerState;

	bool m_bShowScrollWheel;
	bool m_bSystemButtonClicked;
	bool m_bMenuButtonClicked;
	bool m_bGripButtonClicked;
	bool m_bTouchpadTouched;
	bool m_bTouchpadClicked;
	glm::vec2 m_vec2TouchpadInitialTouchPoint;
	glm::vec2 m_vec2TouchpadCurrentTouchPoint;
	bool m_bTriggerEngaged;
	bool m_bTriggerClicked;
	float m_fHairTriggerThreshold; // how much trigger is pulled before being considered engaged
	float m_fTriggerPull;
	int32_t m_nTriggerAxis;
	int32_t m_nTouchpadAxis;

	Icosphere m_TouchPointSphere;

	const glm::vec4 c_vec4TouchPadCenter;
	const glm::vec4 c_vec4TouchPadLeft;
	const glm::vec4 c_vec4TouchPadRight;
	const glm::vec4 c_vec4TouchPadTop;
	const glm::vec4 c_vec4TouchPadBottom;
};