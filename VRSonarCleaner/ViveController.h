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

class ViveController :
	public TrackedDevice
{
public:
	ViveController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	virtual ~ViveController();

	virtual void updateState(vr::VRControllerState_t *state);
	virtual void processControllerEvent(const vr::VREvent_t & event, vr::VRControllerState_t & state);
	virtual bool updatePose(vr::TrackedDevicePose_t pose);

	virtual void prepareForRendering();

	virtual bool triggerDown();

	virtual void touchpadInitialTouch(float x, float y);
	virtual void touchpadTouch(float x, float y);
	virtual void touchpadUntouched();
	virtual bool touchpadActive();

protected:
	bool m_bTouchpadTouched;
	Vector2 m_vTouchpadInitialTouchPoint;
	bool m_bTriggerEngaged;
	bool m_bTriggerClicked;
};