#include "EditingController.h"

#include "DebugDrawer.h"

EditingController::EditingController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex, vr::IVRSystem *pHMD, vr::IVRRenderModels *pRenderModels)
	: ViveController(unTrackedDeviceIndex, pHMD, pRenderModels)
	, m_bShowCursor(true)
	, m_bCleaningMode(false)
	, m_bCursorRadiusResizeMode(false)
	, m_bCursorOffsetMode(false)
	, m_fCursorRadius(0.05f)
	, m_fCursorRadiusMin(0.005f)
	, m_fCursorRadiusMax(0.1f)
	, m_vec4CursorOffsetDirection(Vector4(0.f, 0.f, -1.f, 0.f))
	, m_fCursorOffsetAmount(0.1f)
	, m_fCursorOffsetAmountMin(0.1f)
	, m_fCursorOffsetAmountMax(1.5f)
{
	m_fCursorRadiusResizeModeInitialRadius = m_fCursorRadius;
	m_fCursorOffsetModeInitialOffset = m_fCursorOffsetAmount;
}


EditingController::~EditingController()
{
}

bool EditingController::updatePose(vr::TrackedDevicePose_t pose)
{
	m_Pose = pose;
	m_mat4Pose = ConvertSteamVRMatrixToMatrix4(m_Pose.mDeviceToAbsoluteTracking);
	m_mat4CursorLastPose = m_mat4CursorCurrentPose;
	m_mat4CursorCurrentPose = m_mat4Pose * (Matrix4().identity()).translate(
		Vector3(m_vec4CursorOffsetDirection.x, m_vec4CursorOffsetDirection.y, m_vec4CursorOffsetDirection.z) * m_fCursorOffsetAmount);

	
	// Show overlay
	if (m_pOverlayHandle != vr::k_ulOverlayHandleInvalid)
	{
		vr::EVROverlayError oError = vr::VROverlayError_None;

		if (m_bTriggerEngaged)
		{
			vr::HmdMatrix34_t overlayDistanceMtx;

			if (m_bTriggerClicked)
			{
				float ratio = (m_fCursorRadius - m_fCursorRadiusMin) / (m_fCursorRadiusMax - m_fCursorRadiusMin);
				float overlayWidth;
				vr::VROverlay()->GetOverlayWidthInMeters(m_pOverlayHandle, &overlayWidth);

				Matrix4 mat = Matrix4().translate(-m_fCursorRadius - overlayWidth / 2.f, 0.f, 0.f) * Matrix4().rotateX(-90.f) * Matrix4().rotateZ(90.f);// *Matrix4().scale(0.9f * ratio + 0.1f);

				overlayDistanceMtx = ConvertMatrix4ToSteamVRMatrix(m_mat4CursorCurrentPose * mat);

				oError = vr::VROverlay()->SetOverlayTransformAbsolute(m_pOverlayHandle, vr::TrackingUniverseStanding, &overlayDistanceMtx);
				if (oError != vr::EVROverlayError::VROverlayError_None)
					printf("Overlay transform could not be set.\n");

				vr::VROverlay()->SetOverlayAlpha(m_pOverlayHandle, 0.75f);
				vr::VROverlay()->SetOverlayColor(m_pOverlayHandle, ratio, 0.f, 1.f - ratio);
			}
			else
			{
				Matrix4 mat = Matrix4().translate(0.0f, -0.1f, 0.05f) * Matrix4().rotateZ(90.f) * Matrix4().rotateY(90.f) * Matrix4().rotateX(-90.f);
				Matrix4 triggerPose = ConvertSteamVRMatrixToMatrix4(m_vComponents[15].m_mat3PoseTransform);
				Matrix4 offset = triggerPose * mat;

				overlayDistanceMtx = ConvertMatrix4ToSteamVRMatrix(offset);

				oError = vr::VROverlay()->SetOverlayTransformTrackedDeviceRelative(m_pOverlayHandle, m_unDeviceID, &overlayDistanceMtx);
				if (oError != vr::EVROverlayError::VROverlayError_None)
					printf("Overlay transform could not be set.\n");

				vr::VROverlay()->SetOverlayAlpha(m_pOverlayHandle, 0.5f  + 0.5f * m_fTriggerPull);
				vr::VROverlay()->SetOverlayColor(m_pOverlayHandle, 0.f, m_fTriggerPull, 0.f);
			}


			oError = vr::VROverlay()->ShowOverlay(m_pOverlayHandle);
			if (oError != vr::EVROverlayError::VROverlayError_None)
				printf("Overlay could not be shown: %d\n", oError);
		}
		else
		{
			oError = vr::VROverlay()->HideOverlay(m_pOverlayHandle);
			if (oError != vr::EVROverlayError::VROverlayError_None)
				printf("Overlay could not be hidden: %d\n", oError);
		}
	}
	else
	{
		printf("Overlay handle invalid.\n");
	}


	return m_Pose.bPoseIsValid;
}

//-----------------------------------------------------------------------------
// Purpose: Draw all of the controllers as X/Y/Z lines
//-----------------------------------------------------------------------------
void EditingController::prepareForRendering()
{
	std::vector<float> vertdataarray;

	m_uiLineVertcount = 0;

	if (!poseValid())
		return;

	// Draw Axes
	if (m_bShowAxes)
	{
		DebugDrawer::getInstance().setTransform(m_mat4Pose.get());
		DebugDrawer::getInstance().drawTransform(0.1f);
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
	if (m_bShowCursor)
	{
		GLuint num_segments = 64;

		Vector3 color;
		if (cleaningActive())
			color = Vector3(1.f, 0.f, 0.f);
		else
			color = Vector3(0.8f, 0.8f, 0.2f);

		Vector4 prevVert = m_mat4CursorCurrentPose * Vector4(m_fCursorRadius, 0.f, 0.f, 1.f);
		for (GLuint i = 1; i < num_segments; i++)
		{
			GLfloat theta = 2.f * 3.14159f * static_cast<GLfloat>(i) / static_cast<GLfloat>(num_segments - 1);

			Vector4 circlePt;
			circlePt.x = m_fCursorRadius * cosf(theta);
			circlePt.y = 0.f;
			circlePt.z = m_fCursorRadius * sinf(theta);
			circlePt.w = 1.f;

			Vector4 thisVert = m_mat4CursorCurrentPose * circlePt;

			vertdataarray.push_back(prevVert.x); vertdataarray.push_back(prevVert.y); vertdataarray.push_back(prevVert.z);
			vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

			vertdataarray.push_back(thisVert.x); vertdataarray.push_back(thisVert.y); vertdataarray.push_back(thisVert.z);
			vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

			m_uiLineVertcount += 2;

			prevVert = thisVert;
		}

		prevVert = m_mat4CursorCurrentPose * Vector4(0.f, m_fCursorRadius, 0.f, 1.f);
		for (GLuint i = 1; i < num_segments; i++)
		{
			GLfloat theta = 2.f * 3.14159f * static_cast<GLfloat>(i) / static_cast<GLfloat>(num_segments - 1);

			Vector4 circlePt;
			circlePt.x = 0.f;
			circlePt.y = m_fCursorRadius * cosf(theta);
			circlePt.z = m_fCursorRadius * sinf(theta);
			circlePt.w = 1.f;

			Vector4 thisVert = m_mat4CursorCurrentPose * circlePt;

			vertdataarray.push_back(prevVert.x); vertdataarray.push_back(prevVert.y); vertdataarray.push_back(prevVert.z);
			vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

			vertdataarray.push_back(thisVert.x); vertdataarray.push_back(thisVert.y); vertdataarray.push_back(thisVert.z);
			vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

			m_uiLineVertcount += 2;

			prevVert = thisVert;
		}

		prevVert = m_mat4CursorCurrentPose * Vector4(0.f, m_fCursorRadius, 0.f, 1.f);
		for (GLuint i = 1; i < num_segments; i++)
		{
			GLfloat theta = 2.f * 3.14159f * static_cast<GLfloat>(i) / static_cast<GLfloat>(num_segments - 1);

			Vector4 circlePt;
			circlePt.x = m_fCursorRadius * sinf(theta);
			circlePt.y = m_fCursorRadius * cosf(theta);
			circlePt.z = 0.f;
			circlePt.w = 1.f;

			Vector4 thisVert = m_mat4CursorCurrentPose * circlePt;

			vertdataarray.push_back(prevVert.x); vertdataarray.push_back(prevVert.y); vertdataarray.push_back(prevVert.z);
			vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

			vertdataarray.push_back(thisVert.x); vertdataarray.push_back(thisVert.y); vertdataarray.push_back(thisVert.z);
			vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

			m_uiLineVertcount += 2;

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

				m_uiLineVertcount += 2;
			}
		}

		// DISPLAY CONNECTING LINE TO CURSOR
		//if (!m_rbTrackedDeviceCleaningMode[unTrackedDevice])
		{
			color = Vector3(1.f, 1.f, 1.f);
			if (m_bShowCursor)
			{
				Vector4 controllerCtr = m_mat4Pose * Vector4(0.f, 0.f, 0.f, 1.f);
				Vector4 cursorEdge = m_mat4CursorCurrentPose * Vector4(0.f, 0.f, m_fCursorRadius, 1.f);

				vertdataarray.push_back(cursorEdge.x);
				vertdataarray.push_back(cursorEdge.y);
				vertdataarray.push_back(cursorEdge.z);
				vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

				vertdataarray.push_back(controllerCtr.x);
				vertdataarray.push_back(controllerCtr.y);
				vertdataarray.push_back(controllerCtr.z);
				vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

				m_uiLineVertcount += 2;
			}
		}

		// Draw touchpad touch point sphere
		if (m_bTouchpadTouched && !m_bShowScrollWheel)
		{
			float range = m_fCursorRadiusMax - m_fCursorRadiusMin;
			float ratio = (m_fCursorRadius - m_fCursorRadiusMin) / range;
			insertTouchpadCursor(vertdataarray, m_uiTriVertcount, ratio, 0.f, 1.f - ratio);
		}
	}

	// Setup the VAO the first time through.
	if (m_unVAO == 0)
	{
		glGenVertexArrays(1, &m_unVAO);
		glBindVertexArray(m_unVAO);

		glGenBuffers(1, &m_glVertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);

		GLuint stride = 2 * 3 * sizeof(float);
		GLuint offset = 0;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof(Vector3);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray(0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);

	// set vertex data if we have some
	if (vertdataarray.size() > 0)
	{
		//$ TODO: Use glBufferSubData for this...
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW);
	}
}

void EditingController::triggerEngaged(float amount)
{
	//printf("Controller (device %u) trigger engaged).\n", m_DeviceID);
	m_bTriggerEngaged = true;
	m_fTriggerPull = amount;
	//m_bShowCursor = true;
}

void EditingController::triggerDisengaged()
{
	//printf("Controller (device %u) trigger disengaged).\n", m_DeviceID);
	m_bTriggerEngaged = false;
	m_fTriggerPull = 0.f;
	//m_bShowCursor = false;
}

void EditingController::triggerClicked()
{
	//printf("Controller (device %u) trigger clicked.\n", m_DeviceID);
	m_bTriggerClicked = true;
	m_fTriggerPull = 1.f;
	m_bCleaningMode = true;
}

void EditingController::triggerUnclicked(float amount)
{
	//printf("Controller (device %u) trigger unclicked.\n", m_DeviceID);
	m_bTriggerClicked = false;
	m_fTriggerPull = amount;
	m_bCleaningMode = false;
}

void EditingController::touchpadInitialTouch(float x, float y)
{
	m_bTouchpadTouched = true;
	m_vec2TouchpadInitialTouchPoint = Vector2(x, y);
	m_vec2TouchpadCurrentTouchPoint = Vector2(x, y);

	// if cursor mode on, start modfication interactions
	if (m_bShowCursor)
	{
		if (y > 0.5f || y < -0.5f)
		{
			m_bCursorOffsetMode = true;
			m_fCursorOffsetModeInitialOffset = m_fCursorOffsetAmount;
			m_bShowScrollWheel = true;
		}
		else
		{
			m_bCursorRadiusResizeMode = true;
			m_fCursorRadiusResizeModeInitialRadius = m_fCursorRadius;
		}
	}
}

void EditingController::touchpadTouch(float x, float y)
{
	m_vec2TouchpadCurrentTouchPoint = Vector2(x, y);

	if (m_vec2TouchpadInitialTouchPoint.equal(Vector2(0.f, 0.f), 0.000001))
		m_vec2TouchpadInitialTouchPoint = m_vec2TouchpadCurrentTouchPoint;

	if (m_bShowCursor)
	{
		// Cursor repositioning
		if (m_bCursorOffsetMode)
		{
			float dy = y - m_vec2TouchpadInitialTouchPoint.y;

			float range = m_fCursorOffsetAmountMax - m_fCursorOffsetAmountMin;

			if (dy > 0.f)
				m_fCursorOffsetAmount = m_fCursorOffsetModeInitialOffset + dy * range * 0.5f;
			else if (dy < 0.f)
				m_fCursorOffsetAmount = m_fCursorOffsetModeInitialOffset + dy * range * 0.5f;
			else if (dy == 0.f)
				m_fCursorOffsetAmount = m_fCursorOffsetModeInitialOffset;

			if (m_fCursorOffsetAmount > m_fCursorOffsetAmountMax)
			{
				m_fCursorOffsetAmount = m_fCursorOffsetAmountMax;
				m_fCursorOffsetModeInitialOffset = m_fCursorOffsetAmountMax;
				m_vec2TouchpadInitialTouchPoint.y = y;
			}
			else if (m_fCursorOffsetAmount < m_fCursorOffsetAmountMin)
			{
				m_fCursorOffsetAmount = m_fCursorOffsetAmountMin;
				m_fCursorOffsetModeInitialOffset = m_fCursorOffsetAmountMin;
				m_vec2TouchpadInitialTouchPoint.y = y;
			}
		}

		// Cursor resizing
		if (m_bCursorRadiusResizeMode)
		{
			float dx = x - m_vec2TouchpadInitialTouchPoint.x;

			float range = m_fCursorRadiusMax - m_fCursorRadiusMin;

			if (dx > 0.f)
				m_fCursorRadius = m_fCursorRadiusResizeModeInitialRadius + dx * range;
			else if (dx < 0.f)
				m_fCursorRadius = m_fCursorRadiusResizeModeInitialRadius + dx * range;
			else if (dx == 0.f)
				m_fCursorRadius = m_fCursorRadiusResizeModeInitialRadius;

			if (m_fCursorRadius > m_fCursorRadiusMax)
			{
				m_fCursorRadius = m_fCursorRadiusMax;
				m_fCursorRadiusResizeModeInitialRadius = m_fCursorRadiusMax;
				m_vec2TouchpadInitialTouchPoint.x = x;
			}
			else if (m_fCursorRadius < m_fCursorRadiusMin)
			{
				m_fCursorRadius = m_fCursorRadiusMin;
				m_fCursorRadiusResizeModeInitialRadius = m_fCursorRadiusMin;
				m_vec2TouchpadInitialTouchPoint.x = x;
			}
		}
	}
}

void EditingController::touchpadUntouched()
{
	m_bTouchpadTouched = false;
	m_vec2TouchpadInitialTouchPoint = Vector2(0.f, 0.f);
	m_vec2TouchpadCurrentTouchPoint = Vector2(0.f, 0.f);
	m_bCursorOffsetMode = false;
	m_bCursorRadiusResizeMode = false;
	m_bShowScrollWheel = false;
}

bool EditingController::touchpadActive()
{
	return m_bTouchpadTouched;
}

void EditingController::getCursorPoses(Matrix4 * thisCursorPose, Matrix4 * lastCursorPose)
{
	*thisCursorPose = m_mat4CursorCurrentPose;
	*lastCursorPose = m_mat4CursorLastPose;
}

float EditingController::getCursorRadius()
{
	return m_fCursorRadius;
}

bool EditingController::cursorActive()
{
	return m_bShowCursor;
}

bool EditingController::cleaningActive()
{
	return m_bCleaningMode;
}
