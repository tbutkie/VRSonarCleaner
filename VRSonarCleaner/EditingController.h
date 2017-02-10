#pragma once
#include "ViveController.h"
#include <chrono>

class EditingController :
	public ViveController
{
public:
	EditingController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex, vr::IVRSystem *pHMD, vr::IVRRenderModels *pRenderModels);
	~EditingController();

	bool updatePose(vr::TrackedDevicePose_t pose);

	void prepareForRendering();

	void triggerEngaged(float amount);
	void triggerDisengaged();
	void triggerClicked();
	void triggerUnclicked(float amount);
	
	void gripButtonPressed();

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

	float m_fCursorRadius;
	float m_fCursorRadiusMin;
	float m_fCursorRadiusMax;
	bool m_bCursorRadiusResizeMode;
	float m_fCursorRadiusResizeModeInitialRadius;

	float m_fCursorOffsetAmount;
	float m_fCursorOffsetAmountMin;
	float m_fCursorOffsetAmountMax;
	Vector4 m_vec4CursorOffsetDirection;
	bool m_bCursorOffsetMode;
	float m_fCursorOffsetModeInitialOffset;

	std::chrono::time_point<std::chrono::steady_clock> m_LastTime;
	float m_fCursorHoopAngle;
};

