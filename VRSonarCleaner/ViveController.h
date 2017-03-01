#pragma once

#include <GL/glew.h>

#include "TrackedDevice.h"

class ViveController :
	public TrackedDevice
{
public:
	ViveController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex, vr::IVRSystem *pHMD, vr::IVRRenderModels *pRenderModels);
	~ViveController();

	bool BInit();

	void update();
	virtual bool updatePose(vr::TrackedDevicePose_t pose);

	virtual void prepareForRendering();

	virtual void systemButtonPressed();
	virtual void systemButtonUnpressed();
	bool isSystemButtonPressed();

	virtual void menuButtonPressed();
	virtual void menuButtonUnpressed();
	bool isMenuButtonPressed();

	virtual void gripButtonPressed();
	virtual void gripButtonUnpressed();
	bool isGripButtonPressed();

	virtual void triggerEngaged(float amount);
	virtual void triggerBeingPulled(float amount);
	virtual void triggerDisengaged();
	virtual void triggerClicked();
	virtual void triggerUnclicked(float amount);
	bool isTriggerEngaged();
	bool isTriggerClicked();
	float getTriggerPullAmount();
	float getHairTriggerThreshold();
	
	virtual void touchpadInitialTouch(float x, float y);
	virtual void touchpadTouch(float x, float y);
	virtual void touchpadUntouched();
	virtual void touchPadClicked(float x, float y);
	virtual void touchPadUnclicked(float x, float y);
	bool isTouchpadTouched();
	bool isTouchpadClicked();

protected:
	glm::vec4 transformTouchPointToModelCoords(glm::vec2 *pt);
	void insertTouchpadCursor(std::vector<float> &vertices, unsigned int &nTriangleVertices, float r, float g, float b, float a);
	
	uint32_t m_unStatePacketNum;

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
	uint32_t m_unTriggerAxis;
	uint32_t m_unTouchpadAxis;

	Icosphere m_TouchPointSphere;

	const glm::vec4 c_vec4TouchPadCenter;
	const glm::vec4 c_vec4TouchPadLeft;
	const glm::vec4 c_vec4TouchPadRight;
	const glm::vec4 c_vec4TouchPadTop;
	const glm::vec4 c_vec4TouchPadBottom;

	vr::VROverlayHandle_t m_pOverlayHandle;
};