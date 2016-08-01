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

class ViveController : public TrackedDevice
{
public:
	ViveController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	virtual ~ViveController();

	void updateState(vr::VRControllerState_t *state);
	void processControllerEvent(const vr::VREvent_t & event, vr::VRControllerState_t & state);
	bool updatePose(vr::TrackedDevicePose_t pose);

	void prepareForRendering();

	void getCursorPoses(Matrix4 *thisCursorPose, Matrix4 *lastCursorPose = NULL);
	float getCursorRadius();

	bool triggerDown();

	void touchpadInitialTouch(float x, float y);
	void touchpadTouch(float x, float y);
	void touchpadUntouched();
	bool touchpadActive();

	bool cursorActive();
	bool cleaningActive();

private:
	Matrix4 m_mat4CursorCurrentPose;
	Matrix4 m_mat4CursorLastPose;
	bool m_bShowCursor;
	bool m_bCleaningMode;
	bool m_bTouchpadTouched;
	Vector2 m_vTouchpadInitialTouchPoint;
	bool m_bTriggerEngaged;
	bool m_bTriggerClicked;
	bool m_bCursorRadiusResizeMode;
	float m_fCursorRadiusResizeModeInitialRadius;
	bool m_bCursorOffsetMode;
	float m_fCursorOffsetModeInitialOffset;

	float cursorRadius, cursorRadiusMin, cursorRadiusMax, cursorOffsetAmount, cursorOffsetAmountMin, cursorOffsetAmountMax;
	Vector4 cursorOffsetDirection;
};