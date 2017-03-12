#pragma once

#include <GL/glew.h>
#include <map>
#include <functional>

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
	virtual void prepareForRendering();

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

	float getTriggerPullAmount();
	float getHairTriggerThreshold();

protected:
	glm::vec4 transformTouchPointToModelCoords(glm::vec2 *pt);
	void insertTouchpadCursor(std::vector<float> &vertices, unsigned int &nTriangleVertices, float r, float g, float b, float a);

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

	static const glm::vec4 c_vec4TouchPadCenter;
	static const glm::vec4 c_vec4TouchPadLeft;
	static const glm::vec4 c_vec4TouchPadRight;
	static const glm::vec4 c_vec4TouchPadTop;
	static const glm::vec4 c_vec4TouchPadBottom;
};