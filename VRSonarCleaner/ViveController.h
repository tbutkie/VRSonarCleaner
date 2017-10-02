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

	bool justClickedTrigger();
	bool justUnclickedTrigger();

	bool justPressedGrip();
	bool justUnpressedGrip();

	bool justTouchedTouchpad();
	bool justUntouchedTouchpad();
	bool justPressedTouchpad();
	bool justUpressedTouchpad();

	glm::mat4 getLastDeviceToWorldTransform();

	float getTriggerPullAmount();
	float getHairTriggerThreshold();
	glm::vec2 getCurrentTouchpadTouchPoint();
	glm::vec2 getInitialTouchpadTouchPoint();
	glm::vec3 getCurrentTouchpadTouchPointModelCoords();
	glm::vec3 getInitialTouchpadTouchPointModelCoords();

	void setScrollWheelVisibility(bool visible);

	bool readyToRender();

protected:
	struct CustomState {
		glm::vec2 m_vec2TouchpadInitialTouchPoint;
		glm::vec2 m_vec2TouchpadCurrentTouchPoint;
		bool m_bTriggerEngaged;
		bool m_bTriggerClicked;
		float m_fTriggerPull;

		CustomState()
			: m_vec2TouchpadInitialTouchPoint(glm::vec2(0.f, 0.f))
			, m_vec2TouchpadCurrentTouchPoint(glm::vec2(0.f, 0.f))
			, m_bTriggerEngaged(false)
			, m_bTriggerClicked(false)
			, m_fTriggerPull(0.f)
		{}
	};

	CustomState m_CustomState;
	CustomState m_LastCustomState;

protected:
	glm::vec4 transformTouchPointToModelCoords(glm::vec2 *pt);

	bool m_bStateInitialized;

	vr::TrackedDevicePose_t m_LastPose;
	glm::mat4 m_mat4LastDeviceToWorldTransform;

	vr::VRControllerState_t m_ControllerState;
	vr::VRControllerState_t m_LastControllerState;
	vr::RenderModel_ControllerMode_State_t m_ControllerScrollModeState;

	int32_t m_nTriggerAxis;
	int32_t m_nTouchpadAxis;

	float m_fHairTriggerThreshold; // how much trigger is pulled before being considered engaged

	static const glm::vec4 c_vec4TouchPadCenter;
	static const glm::vec4 c_vec4TouchPadLeft;
	static const glm::vec4 c_vec4TouchPadRight;
	static const glm::vec4 c_vec4TouchPadTop;
	static const glm::vec4 c_vec4TouchPadBottom;
};