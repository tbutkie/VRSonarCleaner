#include "TrackedDevice.h"
#include "ShaderUtils.h"


TrackedDevice::TrackedDevice(vr::TrackedDeviceIndex_t id)
	: id(id)
	, m_pTrackedDeviceToRenderModel(NULL)
	, m_ClassChar(0)
	, m_unControllerTransformProgramID(0)
	, m_glControllerVertBuffer(0)
	, m_uiControllerVertcount(0)
	, m_unControllerVAO(0)
	, m_nControllerMatrixLocation(-1)
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

	createShader();
}


TrackedDevice::~TrackedDevice()
{
	if (m_unControllerVAO != 0)
	{
		glDeleteVertexArrays(1, &m_unControllerVAO);
	}
	if (m_unControllerTransformProgramID)
	{
		glDeleteProgram(m_unControllerTransformProgramID);
	}
}

vr::TrackedDeviceIndex_t TrackedDevice::getIndex()
{
	return id;
}

void TrackedDevice::setRenderModel(CGLRenderModel * renderModel)
{
	m_pTrackedDeviceToRenderModel = renderModel;
}

bool TrackedDevice::createShader()
{
	m_unControllerTransformProgramID = CompileGLShader(
		"Controller",

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3ColorIn;\n"
		"out vec4 v4Color;\n"
		"void main()\n"
		"{\n"
		"	v4Color.xyz = v3ColorIn; v4Color.a = 1.0;\n"
		"	gl_Position = matrix * position;\n"
		"}\n",

		// fragment shader
		"#version 410\n"
		"in vec4 v4Color;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = v4Color;\n"
		"}\n"
	);
	m_nControllerMatrixLocation = glGetUniformLocation(m_unControllerTransformProgramID, "matrix");
	if (m_nControllerMatrixLocation == -1)
	{
		printf("Unable to find matrix uniform in controller shader\n");
		return false;
	}

	return m_unControllerTransformProgramID != 0;
}

//-----------------------------------------------------------------------------
// Purpose: Draw all of the controllers as X/Y/Z lines
//-----------------------------------------------------------------------------
void TrackedDevice::prepareForRendering()
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

	Matrix4 cursorMat, lastCursorMat;
	getCursorPoses(&cursorMat, &lastCursorMat);
	float cursorRadius = getCursorRadius();

	// Draw cursor hoop
	if (cursorActive())
	{
		GLuint num_segments = 64;

		Vector3 color;
		if (cleaningActive())
			color = Vector3(1.f, 0.f, 0.f);
		else
			color = Vector3(0.8f, 0.8f, 0.2f);

		Vector4 prevVert = cursorMat * Vector4(cursorRadius, 0.f, 0.f, 1.f);
		for (GLuint i = 1; i < num_segments; i++)
		{
			GLfloat theta = 2.f * 3.14159f * static_cast<GLfloat>(i) / static_cast<GLfloat>(num_segments - 1);

			Vector4 circlePt;
			circlePt.x = cursorRadius * cosf(theta);
			circlePt.y = 0.f;
			circlePt.z = cursorRadius * sinf(theta);
			circlePt.w = 1.f;

			Vector4 thisVert = cursorMat * circlePt;

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
				Vector4 thisCtrPos = cursorMat * Vector4(0.f, 0.f, 0.f, 1.f);
				Vector4 lastCtrPos = lastCursorMat * Vector4(0.f, 0.f, 0.f, 1.f);

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
				Vector4 controllerCtr = mat * Vector4(0.f, 0.f, 0.f, 1.f);
				Vector4 cursorEdge = cursorMat * Vector4(0.f, 0.f, cursorRadius, 1.f);

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

void TrackedDevice::render(Matrix4 & matVP)
{
	// draw the controller axis lines
	glUseProgram(m_unControllerTransformProgramID);
	glUniformMatrix4fv(m_nControllerMatrixLocation, 1, GL_FALSE, matVP.get());
	glBindVertexArray(m_unControllerVAO);
	glDrawArrays(GL_LINES, 0, m_uiControllerVertcount);
	glBindVertexArray(0);
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