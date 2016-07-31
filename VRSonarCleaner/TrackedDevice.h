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

	bool poseValid();

	void updateState(vr::VRControllerState_t *state);
	void processControllerEvent(const vr::VREvent_t & event, vr::VRControllerState_t & state);

	char getClassChar();
	void setClassChar(char classChar);

	Matrix4 getPose();
	bool updatePose(vr::TrackedDevicePose_t pose);
	void getCursorPoses(Matrix4 *thisCursorPose, Matrix4 *lastCursorPose = NULL);
	float getCursorRadius();
	
	void renderModel();

private:
	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);

	vr::TrackedDeviceIndex_t id;
	CGLRenderModel *m_pTrackedDeviceToRenderModel;
	
	char m_ClassChar;   // for each device, a character representing its class
	
	vr::TrackedDevicePose_t m_Pose;

	Matrix4 m_mat4Pose;
	Matrix4 m_mat4CursorCurrentPose;
	Matrix4 m_mat4CursorLastPose;
	
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

