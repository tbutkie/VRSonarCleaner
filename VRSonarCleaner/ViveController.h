#pragma once

#include <GL/glew.h>

#include "TrackedDevice.h"

const Vector4 c_vec4TouchPadCenter = Vector4(0.f, 0.00378f, 0.04920f, 1.f);
const Vector4 c_vec4TouchPadLeft = Vector4(-0.02023f, 0.00495f, 0.04934f, 1.f);
const Vector4 c_vec4TouchPadRight = Vector4(0.02023f, 0.00495f, 0.04934f, 1.f);
const Vector4 c_vec4TouchPadTop = Vector4(0.f, 0.00725f, 0.02924f, 1.f);
const Vector4 c_vec4TouchPadBottom = Vector4(0.f, 0.00265f, 0.06943f, 1.f);

class ViveController :
	public TrackedDevice
{
public:
	ViveController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	~ViveController();

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

	virtual void triggerEngaged();
	virtual void triggerBeingPulled(float amount);
	virtual void triggerDisengaged();
	virtual void triggerClicked();
	virtual void triggerUnclicked();
	bool isTriggerEngaged();
	bool isTriggerClicked();
	float getTriggerThreshold();
	
	virtual void touchpadInitialTouch(float x, float y);
	virtual void touchpadTouch(float x, float y);
	virtual void touchpadUntouched();
	virtual void touchPadClicked(float x, float y);
	virtual void touchPadUnclicked(float x, float y);
	bool isTouchpadTouched();
	bool isTouchpadClicked();

protected:
	Vector4 transformTouchPointToModelCoords(Vector2 *pt);

	bool m_bSystemButtonClicked;
	bool m_bMenuButtonClicked;
	bool m_bGripButtonClicked;
	bool m_bTouchpadTouched;
	bool m_bTouchpadClicked;
	Vector2 m_vec2TouchpadInitialTouchPoint;
	Vector2 m_vec2TouchpadCurrentTouchPoint;
	bool m_bTriggerEngaged;
	bool m_bTriggerClicked;
	float m_fTriggerLowerThreshold; // trigger pulled 5% before being considered engaged

	Icosphere m_TouchPointSphere;
};