#include "TrackedDevice.h"



TrackedDevice::TrackedDevice(vr::TrackedDeviceIndex_t id)
	: id(id)
	, m_pTrackedDeviceToRenderModel(NULL)
	, m_ClassChar(0)
	, m_bShow(true)
	, m_bShowAxes(false)
	, m_bShowCursor(true)
	, m_bCleaningMode(false)
	, m_bTouchpadTouched(false)
	, m_vTouchpadInitialTouchPoint(Vector2(0.f, 0.f))
	, m_bTriggerEngaged(false)
	, m_bTriggerClicked(false)
	, m_bCursorRadiusResizeMode(false)
	, m_bCursorOffsetMode(false)
	, cursorRadius(0.05f)
	, cursorRadiusMin(0.005f)
	, cursorRadiusMax(0.1f)
	, cursorOffsetDirection(Vector4(0.f, 0.f, -1.f, 0.f))
	, cursorOffsetAmount(0.1f)
	, cursorOffsetAmountMin(0.1f)
	, cursorOffsetAmountMax(1.5f)
{	
	m_fCursorRadiusResizeModeInitialRadius = cursorRadius;
	m_fCursorOffsetModeInitialOffset = cursorOffsetAmount;
}


TrackedDevice::~TrackedDevice()
{
}

bool TrackedDevice::BInit()
{
	return false;
}

vr::TrackedDeviceIndex_t TrackedDevice::getIndex()
{
	return id;
}

void TrackedDevice::setRenderModel(CGLRenderModel * renderModel)
{
	m_pTrackedDeviceToRenderModel = renderModel;
}

bool TrackedDevice::toggleAxes()
{
	return m_bShowAxes = !m_bShowAxes;
}

bool TrackedDevice::axesActive()
{
	return m_bShowAxes;
}

bool TrackedDevice::triggerDown()
{
	return m_bTriggerClicked;
}

void TrackedDevice::touchpadInitialTouch(float x, float y)
{
	m_bTouchpadTouched = true;
	m_vTouchpadInitialTouchPoint.x = x;
	m_vTouchpadInitialTouchPoint.y = y;

	// if cursor mode on, start modfication interactions
	if (m_bShowCursor)
	{
		if (y > 0.5f || y < -0.5f)
		{
			m_bCursorOffsetMode = true;
			m_fCursorOffsetModeInitialOffset = cursorOffsetAmount;
		}
		else
		{
			m_bCursorRadiusResizeMode = true;
			m_fCursorRadiusResizeModeInitialRadius = cursorRadius;
		}
	}
}

void TrackedDevice::touchpadTouch(float x, float y)
{
	if (m_vTouchpadInitialTouchPoint.equal(Vector2(0.f, 0.f), 0.000001))
		m_vTouchpadInitialTouchPoint = Vector2(x, y);
	
	if (m_bShowCursor)
	{
		// Cursor repositioning
		if (m_bCursorOffsetMode)
		{
			float dy = y - m_vTouchpadInitialTouchPoint.y;

			float range = cursorOffsetAmountMax - cursorOffsetAmountMin;

			if (dy > 0.f)
				cursorOffsetAmount = m_fCursorOffsetModeInitialOffset + dy * range * 0.5f;
			else if (dy < 0.f)
				cursorOffsetAmount = m_fCursorOffsetModeInitialOffset + dy * range * 0.5f;
			else if (dy == 0.f)
				cursorOffsetAmount = m_fCursorOffsetModeInitialOffset;

			if (cursorOffsetAmount > cursorOffsetAmountMax)
			{
				cursorOffsetAmount = cursorOffsetAmountMax;
				m_fCursorOffsetModeInitialOffset = cursorOffsetAmountMax;
				m_vTouchpadInitialTouchPoint.y = y;
			}
			else if (cursorOffsetAmount < cursorOffsetAmountMin)
			{
				cursorOffsetAmount = cursorOffsetAmountMin;
				m_fCursorOffsetModeInitialOffset = cursorOffsetAmountMin;
				m_vTouchpadInitialTouchPoint.y = y;
			}
		}

		// Cursor resizing
		if (m_bCursorRadiusResizeMode)
		{
			float dx = x - m_vTouchpadInitialTouchPoint.x;

			float range = cursorRadiusMax - cursorRadiusMin;

			if (dx > 0.f)
				cursorRadius = m_fCursorRadiusResizeModeInitialRadius + dx * range;
			else if (dx < 0.f)
				cursorRadius = m_fCursorRadiusResizeModeInitialRadius + dx * range;
			else if (dx == 0.f)
				cursorRadius = m_fCursorRadiusResizeModeInitialRadius;

			if (cursorRadius > cursorRadiusMax)
			{
				cursorRadius = cursorRadiusMax;
				m_fCursorRadiusResizeModeInitialRadius = cursorRadiusMax;
				m_vTouchpadInitialTouchPoint.x = x;
			}
			else if (cursorRadius < cursorRadiusMin)
			{
				cursorRadius = cursorRadiusMin;
				m_fCursorRadiusResizeModeInitialRadius = cursorRadiusMin;
				m_vTouchpadInitialTouchPoint.x = x;
			}
		}
	}
}

void TrackedDevice::touchpadUntouched()
{
	m_bTouchpadTouched = false;
	m_vTouchpadInitialTouchPoint = Vector2(0.f, 0.f);
	m_bCursorOffsetMode = false;
	m_bCursorRadiusResizeMode = false;
}

bool TrackedDevice::touchpadActive()
{
	return m_bTouchpadTouched;
}

bool TrackedDevice::cursorActive()
{
	return m_bShowCursor;
}

bool TrackedDevice::cleaningActive()
{
	return m_bCleaningMode;
}

bool TrackedDevice::poseValid()
{
	return m_Pose.bPoseIsValid;
}

void TrackedDevice::updateState(vr::VRControllerState_t *state)
{
	// TOUCHPAD BEING TOUCHED
	if (m_bTouchpadTouched)
	{
		printf("Controller (device %u) touchpad touch tracked at (%f, %f).\n"
			, id
			, state->rAxis[0].x
			, state->rAxis[0].y);

		touchpadTouch(state->rAxis[0].x, state->rAxis[0].y);
	}

	// TRIGGER INTERACTIONS
	if (state->rAxis[1].x >= 0.05f) // lower limit is 5%
	{
		// TRIGGER ENGAGED
		if (!m_bTriggerEngaged)
		{
			printf("Controller (device %u) trigger engaged).\n", id);

			m_bTriggerEngaged = true;
			//m_bShowCursor = true;
		}

		// TRIGGER BEING PULLED
		if (!m_bTriggerClicked)
		{
			//printf("Controller (device %u) trigger at %f%%).\n"
			//	, id
			//	, state->rAxis[1].x * 100.f);
		}

		// TRIGGER CLICKED
		if (state->rAxis[1].x == 1.f && !m_bTriggerClicked)
		{
			printf("Controller (device %u) trigger clicked.\n", id);
			m_bTriggerClicked = true;
			m_bCleaningMode = true;
		}
		// TRIGGER UNCLICKED
		if (state->rAxis[1].x != 1.f && m_bTriggerClicked)
		{
			printf("Controller (device %u) trigger unclicked.\n", id);
			m_bTriggerClicked = false;
			m_bCleaningMode = false;
		}
	}
	// TRIGGER DISENGAGED
	else if (m_bTriggerEngaged)
	{
		printf("Controller (device %u) trigger disengaged).\n", id);
		m_bTriggerEngaged = false;
		//m_bShowCursor = false;
	}
}

void TrackedDevice::processControllerEvent(const vr::VREvent_t & event, vr::VRControllerState_t & state)
{


	switch (event.eventType)
	{
	case vr::VREvent_ButtonPress:
	{
		// TRIGGER DOWN
		if (event.data.controller.button == vr::k_EButton_SteamVR_Trigger)
		{
			printf("Controller (device %u) trigger pressed.\n", event.trackedDeviceIndex);
		}

		// MENU BUTTON DOWN
		if (event.data.controller.button == vr::k_EButton_ApplicationMenu)
		{
			printf("Controller (device %u) menu button pressed.\n", event.trackedDeviceIndex);
		}

		// TOUCHPAD DOWN
		if (event.data.controller.button == vr::k_EButton_SteamVR_Touchpad)
		{
			printf("Controller (device %u) touchpad pressed at (%f, %f).\n"
				, event.trackedDeviceIndex
				, state.rAxis[vr::k_eControllerAxis_None].x
				, state.rAxis[vr::k_eControllerAxis_None].y);
		}

		// GRIP DOWN
		if (event.data.controller.button == vr::k_EButton_Grip)
		{
			printf("Controller (device %u) grip pressed.\n", event.trackedDeviceIndex);
			toggleAxes();			
		}
	}
	break;
	case vr::VREvent_ButtonUnpress:
	{
		// TRIGGER UP
		if (event.data.controller.button == vr::k_EButton_SteamVR_Trigger)
		{
			printf("Controller (device %u) trigger unpressed.\n", event.trackedDeviceIndex);
		}

		// MENU BUTTON
		if (event.data.controller.button == vr::k_EButton_ApplicationMenu)
		{
			printf("Controller (device %u) menu button unpressed.\n", event.trackedDeviceIndex);
		}

		// TOUCHPAD UP
		if (event.data.controller.button == vr::k_EButton_SteamVR_Touchpad)
		{
			printf("Controller (device %u) touchpad pressed at (%f, %f).\n"
				, event.trackedDeviceIndex
				, state.rAxis[vr::k_eControllerAxis_None].x
				, state.rAxis[vr::k_eControllerAxis_None].y);
		}

		// GRIP UP
		if (event.data.controller.button == vr::k_EButton_Grip)
		{
			printf("Controller (device %u) grip unpressed.\n", event.trackedDeviceIndex);
		}
	}
	break;
	case vr::VREvent_ButtonTouch:
	{
		// TRIGGER TOUCH
		if (event.data.controller.button == vr::k_EButton_SteamVR_Trigger)
		{
			printf("(VR Event) Controller (device %u) trigger touched.\n", event.trackedDeviceIndex);
		}

		// TOUCHPAD TOUCH
		if (event.data.controller.button == vr::k_EButton_SteamVR_Touchpad)
		{
			printf("Controller (device %u) touchpad touched at initial position (%f, %f).\n"
				, event.trackedDeviceIndex
				, state.rAxis[0].x
				, state.rAxis[0].y);
			touchpadInitialTouch(state.rAxis[0].x, state.rAxis[0].y);
		}
	}
	break;
	case vr::VREvent_ButtonUntouch:
	{
		// TRIGGER UNTOUCH
		if (event.data.controller.button == vr::k_EButton_SteamVR_Trigger)
		{
			printf("(VR Event) Controller (device %u) trigger untouched.\n", event.trackedDeviceIndex);
		}

		// TOUCHPAD UNTOUCH
		if (event.data.controller.button == vr::k_EButton_SteamVR_Touchpad)
		{
			printf("Controller (device %u) touchpad untouched.\n", event.trackedDeviceIndex);
			touchpadUntouched();
		}
	}
	break;
	default:
		printf("Controller (device %u) uncaught event %u.\n", event.trackedDeviceIndex, event.eventType);
	}
}

char TrackedDevice::getClassChar()
{
	return m_ClassChar;
}

void TrackedDevice::setClassChar(char classChar)
{
	m_ClassChar = classChar;
}

Matrix4 TrackedDevice::getPose()
{
	return m_mat4Pose;
}

bool TrackedDevice::updatePose(vr::TrackedDevicePose_t pose)
{
	m_Pose = pose;
	m_mat4Pose = ConvertSteamVRMatrixToMatrix4(m_Pose.mDeviceToAbsoluteTracking);
	m_mat4CursorLastPose = m_mat4CursorCurrentPose;
	m_mat4CursorCurrentPose = m_mat4Pose * (Matrix4().identity()).translate(
		Vector3(cursorOffsetDirection.x, cursorOffsetDirection.y, cursorOffsetDirection.z) * cursorOffsetAmount);

	return m_Pose.bPoseIsValid;
}

void TrackedDevice::getCursorPoses(Matrix4 * thisCursorPose, Matrix4 * lastCursorPose)
{
	*thisCursorPose = m_mat4CursorCurrentPose;

	if (lastCursorPose)
		*lastCursorPose = m_mat4CursorLastPose;
}

float TrackedDevice::getCursorRadius()
{
	return cursorRadius;
}

void TrackedDevice::renderModel()
{
	if(m_pTrackedDeviceToRenderModel)
		m_pTrackedDeviceToRenderModel->Draw();
}

//-----------------------------------------------------------------------------
// Purpose: Converts a SteamVR matrix to our local matrix class
//-----------------------------------------------------------------------------
Matrix4 TrackedDevice::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
{
	Matrix4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	return matrixObj;
}