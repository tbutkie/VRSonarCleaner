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
	, m_vec4CursorOffsetDirection(glm::vec4(0.f, 0.f, -1.f, 0.f))
	, m_fCursorOffsetAmount(0.1f)
	, m_fCursorOffsetAmountMin(0.1f)
	, m_fCursorOffsetAmountMax(1.5f)
	, m_LastTime(std::chrono::high_resolution_clock::now())
	, m_fCursorHoopAngle(0.f)
{
	m_fCursorRadiusResizeModeInitialRadius = m_fCursorRadius;
	m_fCursorOffsetModeInitialOffset = m_fCursorOffsetAmount;
}


EditingController::~EditingController()
{
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

	glm::mat4 cursorPose = m_mat4DeviceToWorldTransform * glm::translate(glm::mat4(), glm::vec3(
		m_vec4CursorOffsetDirection.x,
		m_vec4CursorOffsetDirection.y,
		m_vec4CursorOffsetDirection.z) * m_fCursorOffsetAmount
	);

	// Draw Axes
	if (m_bShowAxes)
	{
		DebugDrawer::getInstance().setTransform(glm::value_ptr(m_mat4DeviceToWorldTransform));
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

		glm::vec4 color;
		if (cleaningActive())
			color = glm::vec4(1.f, 0.f, 0.f, 1.f);
		else
			color = glm::vec4(1.f, 1.f, 1.f - m_fTriggerPull, 0.75f);

		std::vector<glm::vec4> circlePoints;
		for (GLuint i = 0; i < num_segments; i++)
		{
			GLfloat theta = glm::two_pi<float>() * static_cast<GLfloat>(i) / static_cast<GLfloat>(num_segments - 1);

			glm::vec4 circlePt;
			circlePt.x = cosf(theta);
			circlePt.y = sinf(theta);
			circlePt.z = 0.f;
			circlePt.w = 1.f;

			circlePoints.push_back(circlePt);
		}

		auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_LastTime);
		m_LastTime = std::chrono::high_resolution_clock::now();

		long long rate_ms_per_rev = 2000ll / (1.f + 10.f * m_fTriggerPull);

		glm::mat4 scl = glm::scale(glm::mat4(), glm::vec3(m_fCursorRadius));
		glm::mat4 rot;

		float angleNeeded = 360.f * (elapsed_ms.count() % rate_ms_per_rev) / rate_ms_per_rev;
		m_fCursorHoopAngle += angleNeeded;

		for (int n = 0; n < 3; ++n)
		{
			if (n == 0)
				rot = glm::rotate(glm::mat4(), glm::radians(m_fCursorHoopAngle), glm::vec3(1.f, 0.f, 0.f));
			if (n == 1)
				rot = glm::rotate(glm::mat4(), glm::radians(m_fCursorHoopAngle), glm::vec3(0.f, 1.f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
			if (n == 2)
				rot = glm::rotate(glm::mat4(), glm::radians(m_fCursorHoopAngle), glm::vec3(0.f, 0.f, 1.f)) * glm::rotate(glm::mat4(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));

			glm::vec4 prevVert = cursorPose * rot * scl * circlePoints.back();
			for (size_t i = 0; i < circlePoints.size(); ++i)
			{
				glm::vec4 thisVert = cursorPose * rot * scl * circlePoints[i];

				vertdataarray.push_back(prevVert.x); vertdataarray.push_back(prevVert.y); vertdataarray.push_back(prevVert.z);
				vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z); vertdataarray.push_back(color.w);

				vertdataarray.push_back(thisVert.x); vertdataarray.push_back(thisVert.y); vertdataarray.push_back(thisVert.z);
				vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z); vertdataarray.push_back(color.w);

				m_uiLineVertcount += 2;

				prevVert = thisVert;
			}
		}

		// DISPLAY CURSOR DOT
		//if (!m_rbTrackedDeviceCleaningMode[unTrackedDevice])
		//{
		//	color = Vector3(1.f, 0.f, 0.f);
		//	if (cursorActive())
		//	{
		//		Vector4 thisCtrPos = m_mat4CursorCurrentPose * Vector4(0.f, 0.f, 0.f, 1.f);
		//		Vector4 lastCtrPos = m_mat4CursorLastPose * Vector4(0.f, 0.f, 0.f, 1.f);

		//		vertdataarray.push_back(lastCtrPos.x);
		//		vertdataarray.push_back(lastCtrPos.y);
		//		vertdataarray.push_back(lastCtrPos.z);
		//		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

		//		vertdataarray.push_back(thisCtrPos.x);
		//		vertdataarray.push_back(thisCtrPos.y);
		//		vertdataarray.push_back(thisCtrPos.z);
		//		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

		//		m_uiLineVertcount += 2;
		//	}
		//}

		// DISPLAY CONNECTING LINE TO CURSOR
		//if (!m_rbTrackedDeviceCleaningMode[unTrackedDevice])
		{
			color = glm::vec4(1.f, 1.f, 1.f, 0.8f);
			if (m_bShowCursor)
			{
				glm::vec4 controllerCtr = m_mat4DeviceToWorldTransform * glm::vec4(0.f, 0.f, 0.f, 1.f);
				glm::vec4 cursorEdge = cursorPose * glm::vec4(0.f, 0.f, m_fCursorRadius, 1.f);

				vertdataarray.push_back(cursorEdge.x);
				vertdataarray.push_back(cursorEdge.y);
				vertdataarray.push_back(cursorEdge.z);
				vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z); vertdataarray.push_back(color.w);

				vertdataarray.push_back(controllerCtr.x);
				vertdataarray.push_back(controllerCtr.y);
				vertdataarray.push_back(controllerCtr.z);
				vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z); vertdataarray.push_back(color.w);

				m_uiLineVertcount += 2;
			}
		}

		// Draw touchpad touch point sphere
		if (m_bTouchpadTouched && !m_bShowScrollWheel)
		{
			float range = m_fCursorRadiusMax - m_fCursorRadiusMin;
			float ratio = (m_fCursorRadius - m_fCursorRadiusMin) / range;
			insertTouchpadCursor(vertdataarray, m_uiTriVertcount, ratio, 0.f, 1.f - ratio, 0.75f);
		}
	}

	// Setup the VAO the first time through.
	if (m_unVAO == 0)
	{
		glGenVertexArrays(1, &m_unVAO);
		glBindVertexArray(m_unVAO);

		glGenBuffers(1, &m_glVertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);

		GLuint stride = 3 * sizeof(float) + 4 * sizeof(float);
		GLuint offset = 0;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof(glm::vec3);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

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


	glm::mat4 cursorPose = m_mat4DeviceToWorldTransform * glm::translate(glm::mat4(), glm::vec3(
		m_vec4CursorOffsetDirection.x,
		m_vec4CursorOffsetDirection.y,
		m_vec4CursorOffsetDirection.z) * m_fCursorOffsetAmount
	);
	notify(this, BroadcastSystem::EVENT::EDIT_TRIGGER_CLICKED, &cursorPose);
}

void EditingController::triggerUnclicked(float amount)
{
	//printf("Controller (device %u) trigger unclicked.\n", m_DeviceID);
	m_bTriggerClicked = false;
	m_fTriggerPull = amount;
	m_bCleaningMode = false;
}

void EditingController::gripButtonPressed()
{
	m_bGripButtonClicked = true;

	glm::mat4 cursorPose = m_mat4DeviceToWorldTransform * glm::translate(glm::mat4(), glm::vec3(
		m_vec4CursorOffsetDirection.x,
		m_vec4CursorOffsetDirection.y,
		m_vec4CursorOffsetDirection.z) * m_fCursorOffsetAmount
	);
	notify(this, BroadcastSystem::EVENT::EDIT_GRIP_PRESSED, &cursorPose);
}

void EditingController::touchpadInitialTouch(float x, float y)
{
	m_bTouchpadTouched = true;
	m_vec2TouchpadInitialTouchPoint = glm::vec2(x, y);
	m_vec2TouchpadCurrentTouchPoint = glm::vec2(x, y);

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
	m_vec2TouchpadCurrentTouchPoint = glm::vec2(x, y);

	if (m_vec2TouchpadInitialTouchPoint == glm::vec2(0.f, 0.f))
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
	m_vec2TouchpadInitialTouchPoint = glm::vec2(0.f, 0.f);
	m_vec2TouchpadCurrentTouchPoint = glm::vec2(0.f, 0.f);
	m_bCursorOffsetMode = false;
	m_bCursorRadiusResizeMode = false;
	m_bShowScrollWheel = false;
}

void EditingController::getCursorPoses(glm::mat4 &thisCursorPose, glm::mat4 &lastCursorPose)
{
	glm::mat4 cursorPose = m_mat4DeviceToWorldTransform * glm::translate(glm::mat4(), glm::vec3(
		m_vec4CursorOffsetDirection.x,
		m_vec4CursorOffsetDirection.y,
		m_vec4CursorOffsetDirection.z) * m_fCursorOffsetAmount
	);


	glm::mat4 cursorPoseLast = ConvertSteamVRMatrixToMatrix4(m_LastPose.mDeviceToAbsoluteTracking) * glm::translate(glm::mat4(), glm::vec3(
		m_vec4CursorOffsetDirection.x,
		m_vec4CursorOffsetDirection.y,
		m_vec4CursorOffsetDirection.z) * m_fCursorOffsetAmount
	);

	thisCursorPose = cursorPose;
	lastCursorPose = cursorPoseLast;
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
