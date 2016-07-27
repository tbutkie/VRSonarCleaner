#pragma once

#include <openvr.h>

#include "../shared/Matrices.h"

class TrackedDevice
{
public:
	TrackedDevice(vr::TrackedDeviceIndex_t id);
	~TrackedDevice();

	bool BInit();

private:
	vr::TrackedDeviceIndex_t id;

	vr::IVRRenderModels *m_pRenderModel;

	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;

	std::string m_strPoseClasses;                          // what classes we saw poses for this frame
	char m_rDevClassChar;   // for each device, a character representing its class

	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];

	bool m_rbShowTrackedDevice;
	bool m_rbShowTrackedDeviceAxes;
	bool m_rbTrackedDeviceShowCursor;
	bool m_rbTrackedDeviceCleaningMode;
	bool m_rbTrackedDeviceTouchpadTouched;
	Vector2 m_rvTrackedDeviceTouchpadInitialTouchPoint;
	bool m_rbTrackedDeviceTriggerEngaged;
	bool m_rbTrackedDeviceTriggerClicked;
	bool m_rbTrackedDeviceCursorRadiusResizeMode;
	float m_rfTrackedDeviceCursorRadiusResizeModeInitialRadius;
	bool m_rbTrackedDeviceCursorOffsetMode;
	float m_rfTrackedDeviceCursorOffsetModeInitialOffset;
	Matrix4 m_rmat4DeviceCursorCurrentPose;
	Matrix4 m_rmat4DeviceCursorLastPose;
};

