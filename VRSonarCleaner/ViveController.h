#pragma once

#include <GL/glew.h>

#include "TrackedDevice.h"

class ViveController :
	public TrackedDevice
{
	friend class TrackedDeviceManager;
public:
	ViveController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex, vr::IVRSystem *pHMD, vr::IVRRenderModels *pRenderModels);
	~ViveController();

	// Overridden from TrackedDevice
	bool BInit();

	glm::mat4 getLastPose();
	bool update(vr::TrackedDevicePose_t pose);
	bool updateControllerState();

	bool isSystemButtonPressed();
	bool isMenuButtonPressed();
	bool isGripButtonPressed();
	bool isTriggerEngaged();
	bool isTriggerClicked();
	bool isTouchpadTouched();
	bool isTouchpadClicked();

	glm::mat4 getLastDeviceToWorldTransform();

	float getTriggerPullAmount();
	float getHairTriggerThreshold();
	glm::vec3 getCurrentTouchpadTouchPoint();
	glm::vec3 getInitialTouchpadTouchPoint();

	void setScrollWheelVisibility(bool visible);

protected:
	glm::vec4 transformTouchPointToModelCoords(glm::vec2 *pt);

	bool m_bStateInitialized;

	vr::TrackedDevicePose_t m_LastPose;
	glm::mat4 m_mat4LastDeviceToWorldTransform;

	vr::VRControllerState_t m_ControllerState;
	vr::VRControllerState_t m_LastControllerState;
	vr::RenderModel_ControllerMode_State_t m_ControllerScrollModeState;

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

	static const glm::vec4 c_vec4TouchPadCenter;
	static const glm::vec4 c_vec4TouchPadLeft;
	static const glm::vec4 c_vec4TouchPadRight;
	static const glm::vec4 c_vec4TouchPadTop;
	static const glm::vec4 c_vec4TouchPadBottom;
};