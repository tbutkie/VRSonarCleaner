#pragma once

#include <vector>
#include <windows.h>

#include <openvr.h>

#include "CGLRenderModel.h"

#include "../shared/Matrices.h"

class TrackedDevice
{
public:
	TrackedDevice(vr::TrackedDeviceIndex_t id);
	~TrackedDevice();

	bool BInit();


	vr::TrackedDeviceIndex_t getIndex();
	void setRenderModel(CGLRenderModel *renderModel);

	bool toggleAxes();
	bool axesActive();

	bool triggerDown();

	void touchpadInitialTouch(float x, float y);
	void touchpadTouch(float x, float y);
	void touchpadUntouched();
	bool touchpadActive();

	bool cursorActive();
	bool cleaningActive();

	void updateState(vr::VRControllerState_t *state);

	void renderModel();

private:	
	vr::TrackedDeviceIndex_t id;

	CGLRenderModel * pRenderModel;
	
	char m_rDevClassChar;   // for each device, a character representing its class

	Matrix4 m_mat4Pose;
	Matrix4 m_mat4CursorCurrentPose;
	Matrix4 m_mat4CursorLastPose;

	CGLRenderModel *m_rTrackedDeviceToRenderModel;

	bool m_bShow;
	bool m_bShowAxes;
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

