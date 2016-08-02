#pragma once

//#include <SDL.h>
#include <GL/glew.h>
//#include <math.h>
//#include <SDL_opengl.h>
//#include <gl/glu.h>
//#include <stdio.h>
//#include <string>
//#include <cstdlib>

//#include <openvr.h>

//#include "../shared/lodepng.h"
//#include "../shared/Matrices.h"
//#include "../shared/pathtools.h"

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
	virtual ~ViveController();

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
	bool m_bSystemButtonClicked;
	bool m_bMenuButtonClicked;
	bool m_bGripButtonClicked;
	bool m_bTouchpadTouched;
	bool m_bTouchpadClicked;
	Vector2 m_vTouchpadInitialTouchPoint;
	bool m_bTriggerEngaged;
	bool m_bTriggerClicked;
	float m_fTriggerLowerThreshold; // trigger pulled 5% before being considered engaged
};