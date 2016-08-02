#pragma once
#include "ViveController.h"
class EditingController :
	public ViveController
{
public:
	EditingController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	~EditingController();

	bool updatePose(vr::TrackedDevicePose_t pose);

	void prepareForRendering();

	void triggerEngaged();
	void triggerDisengaged();
	void triggerClicked();
	void triggerUnclicked();

	void touchpadInitialTouch(float x, float y);
	void touchpadTouch(float x, float y);
	void touchpadUntouched();
	bool touchpadActive();

	void getCursorPoses(Matrix4 *thisCursorPose, Matrix4 *lastCursorPose);
	float getCursorRadius();

	bool cursorActive();
	bool cleaningActive();

private:
	Matrix4 m_mat4CursorCurrentPose;
	Matrix4 m_mat4CursorLastPose;
	bool m_bShowCursor;
	bool m_bCleaningMode;
	bool m_bCursorRadiusResizeMode;
	float m_fCursorRadiusResizeModeInitialRadius;
	bool m_bCursorOffsetMode;
	float m_fCursorOffsetModeInitialOffset;

	float cursorRadius, cursorRadiusMin, cursorRadiusMax, cursorOffsetAmount, cursorOffsetAmountMin, cursorOffsetAmountMax;
	Vector4 cursorOffsetDirection;
};

