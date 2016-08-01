#include "ViveController.h"

ViveController::ViveController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex)
	: TrackedDevice(unTrackedDeviceIndex)
	, m_bTouchpadTouched(false)
	, m_vTouchpadInitialTouchPoint(Vector2(0.f, 0.f))
	, m_bTriggerEngaged(false)
	, m_bTriggerClicked(false)
{
}

ViveController::~ViveController()
{

}

void ViveController::updateState(vr::VRControllerState_t *state)
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
		}
		// TRIGGER UNCLICKED
		if (state->rAxis[1].x != 1.f && m_bTriggerClicked)
		{
			printf("Controller (device %u) trigger unclicked.\n", id);
			m_bTriggerClicked = false;
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

void ViveController::processControllerEvent(const vr::VREvent_t & event, vr::VRControllerState_t & state)
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

bool ViveController::updatePose(vr::TrackedDevicePose_t pose)
{
	m_Pose = pose;
	m_mat4Pose = ConvertSteamVRMatrixToMatrix4(m_Pose.mDeviceToAbsoluteTracking);

	return m_Pose.bPoseIsValid;
}

//-----------------------------------------------------------------------------
// Purpose: Draw all of the controllers as X/Y/Z lines
//-----------------------------------------------------------------------------
void ViveController::prepareForRendering()
{
	std::vector<float> vertdataarray;

	m_uiControllerVertcount = 0;

	if (!poseValid())
		return;

	const Matrix4 & mat = getPose();

	// Draw Axes
	if (axesActive())
	{
		for (int i = 0; i < 3; ++i)
		{
			Vector3 color(0, 0, 0);
			Vector4 center = mat * Vector4(0, 0, 0, 1);
			Vector4 point(0, 0, 0, 1);
			point[i] += 0.1f;  // offset in X, Y, Z
			color[i] = 1.0;  // R, G, B
			point = mat * point;
			vertdataarray.push_back(center.x);
			vertdataarray.push_back(center.y);
			vertdataarray.push_back(center.z);

			//printf("Controller #%d at %f, %f, %f\n", unTrackedDevice, center.x, center.y, center.z);

			vertdataarray.push_back(color.x);
			vertdataarray.push_back(color.y);
			vertdataarray.push_back(color.z);

			vertdataarray.push_back(point.x);
			vertdataarray.push_back(point.y);
			vertdataarray.push_back(point.z);

			vertdataarray.push_back(color.x);
			vertdataarray.push_back(color.y);
			vertdataarray.push_back(color.z);

			m_uiControllerVertcount += 2;
		}
	}

	// Draw pointing line
	{
		Vector4 start = mat * Vector4(0, 0, -0.02f, 1);
		Vector4 end = mat * Vector4(0, 0, -39.f, 1);
		Vector3 color(.92f, .92f, .71f);

		vertdataarray.push_back(start.x); vertdataarray.push_back(start.y); vertdataarray.push_back(start.z);
		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

		vertdataarray.push_back(end.x); vertdataarray.push_back(end.y); vertdataarray.push_back(end.z);
		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);
		m_uiControllerVertcount += 2;
	}

	// Setup the VAO the first time through.
	if (m_unControllerVAO == 0)
	{
		glGenVertexArrays(1, &m_unControllerVAO);
		glBindVertexArray(m_unControllerVAO);

		glGenBuffers(1, &m_glControllerVertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_glControllerVertBuffer);

		GLuint stride = 2 * 3 * sizeof(float);
		GLuint offset = 0;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof(Vector3);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray(0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_glControllerVertBuffer);

	// set vertex data if we have some
	if (vertdataarray.size() > 0)
	{
		//$ TODO: Use glBufferSubData for this...
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW);
	}
}

bool ViveController::triggerDown()
{
	return m_bTriggerClicked;
}

void ViveController::touchpadInitialTouch(float x, float y)
{
	m_bTouchpadTouched = true;
	m_vTouchpadInitialTouchPoint.x = x;
	m_vTouchpadInitialTouchPoint.y = y;

}

void ViveController::touchpadTouch(float x, float y)
{
	if (m_vTouchpadInitialTouchPoint.equal(Vector2(0.f, 0.f), 0.000001))
		m_vTouchpadInitialTouchPoint = Vector2(x, y);
}

void ViveController::touchpadUntouched()
{
	m_bTouchpadTouched = false;
	m_vTouchpadInitialTouchPoint = Vector2(0.f, 0.f);
}

bool ViveController::touchpadActive()
{
	return m_bTouchpadTouched;
}