#include "EditingController.h"



EditingController::EditingController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex)
	: ViveController(unTrackedDeviceIndex)
	, m_bShowCursor(true)
	, m_bCleaningMode(false)
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


EditingController::~EditingController()
{
}

void EditingController::updateState(vr::VRControllerState_t *state)
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

void EditingController::processControllerEvent(const vr::VREvent_t & event, vr::VRControllerState_t & state)
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

bool EditingController::updatePose(vr::TrackedDevicePose_t pose)
{
	m_Pose = pose;
	m_mat4Pose = ConvertSteamVRMatrixToMatrix4(m_Pose.mDeviceToAbsoluteTracking);
	m_mat4CursorLastPose = m_mat4CursorCurrentPose;
	m_mat4CursorCurrentPose = m_mat4Pose * (Matrix4().identity()).translate(
		Vector3(cursorOffsetDirection.x, cursorOffsetDirection.y, cursorOffsetDirection.z) * cursorOffsetAmount);

	return m_Pose.bPoseIsValid;
}

//-----------------------------------------------------------------------------
// Purpose: Draw all of the controllers as X/Y/Z lines
//-----------------------------------------------------------------------------
void EditingController::prepareForRendering()
{
	std::vector<float> vertdataarray;

	m_uiControllerVertcount = 0;

	if (!poseValid())
		return;

	// Draw Axes
	if (axesActive())
	{
		for (int i = 0; i < 3; ++i)
		{
			Vector3 color(0, 0, 0);
			Vector4 center = m_mat4Pose * Vector4(0, 0, 0, 1);
			Vector4 point(0, 0, 0, 1);
			point[i] += 0.1f;  // offset in X, Y, Z
			color[i] = 1.0;  // R, G, B
			point = m_mat4Pose * point;
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
	//{
	//	Vector4 start = mat * Vector4(0, 0, -0.02f, 1);
	//	Vector4 end = mat * Vector4(0, 0, -39.f, 1);
	//	Vector3 color(.92f, .92f, .71f);

	//	vertdataarray.push_back(start.x); vertdataarray.push_back(start.y); vertdataarray.push_back(start.z);
	//	vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

	//	vertdataarray.push_back(end.x); vertdataarray.push_back(end.y); vertdataarray.push_back(end.z);
	//	vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);
	//	m_uiControllerVertcount += 2;
	//}

	// Draw cursor hoop
	if (cursorActive())
	{
		GLuint num_segments = 64;

		Vector3 color;
		if (cleaningActive())
			color = Vector3(1.f, 0.f, 0.f);
		else
			color = Vector3(0.8f, 0.8f, 0.2f);

		Vector4 prevVert = m_mat4CursorCurrentPose * Vector4(cursorRadius, 0.f, 0.f, 1.f);
		for (GLuint i = 1; i < num_segments; i++)
		{
			GLfloat theta = 2.f * 3.14159f * static_cast<GLfloat>(i) / static_cast<GLfloat>(num_segments - 1);

			Vector4 circlePt;
			circlePt.x = cursorRadius * cosf(theta);
			circlePt.y = 0.f;
			circlePt.z = cursorRadius * sinf(theta);
			circlePt.w = 1.f;

			Vector4 thisVert = m_mat4CursorCurrentPose * circlePt;

			vertdataarray.push_back(prevVert.x); vertdataarray.push_back(prevVert.y); vertdataarray.push_back(prevVert.z);
			vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

			vertdataarray.push_back(thisVert.x); vertdataarray.push_back(thisVert.y); vertdataarray.push_back(thisVert.z);
			vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

			m_uiControllerVertcount += 2;

			prevVert = thisVert;
		}

		// DISPLAY CURSOR DOT
		//if (!m_rbTrackedDeviceCleaningMode[unTrackedDevice])
		{
			color = Vector3(1.f, 0.f, 0.f);
			if (cursorActive())
			{
				Vector4 thisCtrPos = m_mat4CursorCurrentPose * Vector4(0.f, 0.f, 0.f, 1.f);
				Vector4 lastCtrPos = m_mat4CursorLastPose * Vector4(0.f, 0.f, 0.f, 1.f);

				vertdataarray.push_back(lastCtrPos.x);
				vertdataarray.push_back(lastCtrPos.y);
				vertdataarray.push_back(lastCtrPos.z);
				vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

				vertdataarray.push_back(thisCtrPos.x);
				vertdataarray.push_back(thisCtrPos.y);
				vertdataarray.push_back(thisCtrPos.z);
				vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

				m_uiControllerVertcount += 2;
			}
		}

		// DISPLAY CONNECTING LINE TO CURSOR
		//if (!m_rbTrackedDeviceCleaningMode[unTrackedDevice])
		{
			color = Vector3(1.f, 1.f, 1.f);
			if (cursorActive())
			{
				Vector4 controllerCtr = m_mat4Pose * Vector4(0.f, 0.f, 0.f, 1.f);
				Vector4 cursorEdge = m_mat4CursorCurrentPose * Vector4(0.f, 0.f, cursorRadius, 1.f);

				vertdataarray.push_back(cursorEdge.x);
				vertdataarray.push_back(cursorEdge.y);
				vertdataarray.push_back(cursorEdge.z);
				vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

				vertdataarray.push_back(controllerCtr.x);
				vertdataarray.push_back(controllerCtr.y);
				vertdataarray.push_back(controllerCtr.z);
				vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

				m_uiControllerVertcount += 2;
			}
		}

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

bool EditingController::triggerDown()
{
	return m_bTriggerClicked;
}

void EditingController::touchpadInitialTouch(float x, float y)
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

void EditingController::touchpadTouch(float x, float y)
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

void EditingController::touchpadUntouched()
{
	m_bTouchpadTouched = false;
	m_vTouchpadInitialTouchPoint = Vector2(0.f, 0.f);
	m_bCursorOffsetMode = false;
	m_bCursorRadiusResizeMode = false;
}

bool EditingController::touchpadActive()
{
	return m_bTouchpadTouched;
}

void EditingController::getCursorPoses(Matrix4 * thisCursorPose, Matrix4 * lastCursorPose)
{
	*thisCursorPose = m_mat4CursorCurrentPose;

	if (lastCursorPose)
		*lastCursorPose = m_mat4CursorLastPose;
}

float EditingController::getCursorRadius()
{
	return cursorRadius;
}

bool EditingController::cursorActive()
{
	return m_bShowCursor;
}

bool EditingController::cleaningActive()
{
	return m_bCleaningMode;
}
