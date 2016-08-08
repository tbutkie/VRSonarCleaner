#include "ViveController.h"

ViveController::ViveController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex)
	: TrackedDevice(unTrackedDeviceIndex)
	, m_bTouchpadTouched(false)
	, m_bTouchpadClicked(false)
	, m_vec2TouchpadInitialTouchPoint(Vector2(0.f, 0.f))
	, m_vec2TouchpadCurrentTouchPoint(Vector2(0.f, 0.f))
	, m_bTriggerEngaged(false)
	, m_bTriggerClicked(false)
	, m_fTriggerLowerThreshold(0.05f)
	, m_TouchPointSphere(Icosphere(2))
	, c_vec4TouchPadCenter(Vector4(0.f, 0.00378f, 0.04920f, 1.f))
	, c_vec4TouchPadLeft(Vector4(-0.02023f, 0.00495f, 0.04934f, 1.f))
	, c_vec4TouchPadRight(Vector4(0.02023f, 0.00495f, 0.04934f, 1.f))
	, c_vec4TouchPadTop(Vector4(0.f, 0.00725f, 0.02924f, 1.f))
	, c_vec4TouchPadBottom(Vector4(0.f, 0.00265f, 0.06943f, 1.f))
{

}

ViveController::~ViveController()
{

}

void ViveController::update(const vr::VREvent_t *event)
{
	vr::EVRInitError eError = vr::VRInitError_None;
	vr::IVRSystem *pHMD = (vr::IVRSystem *)vr::VR_GetGenericInterface(vr::IVRSystem_Version, &eError);
	
	vr::VRControllerState_t state;
	if (!pHMD->GetControllerState(m_unDeviceID, &state))
		return;

	if (event)
	{
		switch (event->data.controller.button)
		{
		case vr::k_EButton_ApplicationMenu:
		{
			if (event->eventType == vr::VREvent_ButtonPress)
			{
				this->menuButtonPressed();
			}

			if (event->eventType == vr::VREvent_ButtonUnpress)
			{
				this->menuButtonUnpressed();
			}
		}
		case vr::k_EButton_System:
		{
			if (event->eventType == vr::VREvent_ButtonPress)
			{
				this->systemButtonPressed();
			}

			if (event->eventType == vr::VREvent_ButtonUnpress)
			{
				this->systemButtonUnpressed();
			}
		}
		break;
		case vr::k_EButton_Grip:
		{
			if (event->eventType == vr::VREvent_ButtonPress)
			{
				this->gripButtonPressed();
			}

			if (event->eventType == vr::VREvent_ButtonUnpress)
			{
				this->gripButtonUnpressed();
			}
		}
		break;
		case vr::k_EButton_SteamVR_Trigger:
		{
			if (event->eventType == vr::VREvent_ButtonPress)
			{
				//printf("(VR Event) Controller (device %u) trigger pressed.\n", event.trackedDeviceIndex);
			}

			if (event->eventType == vr::VREvent_ButtonUnpress)
			{
				//printf("(VR Event) Controller (device %u) trigger unpressed.\n", event.trackedDeviceIndex);
			}

			if (event->eventType == vr::VREvent_ButtonTouch)
			{
				//printf("(VR Event) Controller (device %u) trigger touched.\n", event.trackedDeviceIndex);
			}

			if (event->eventType == vr::VREvent_ButtonUntouch)
			{
				//printf("(VR Event) Controller (device %u) trigger untouched.\n", event.trackedDeviceIndex);
			}
		}
		break;
		case vr::k_EButton_SteamVR_Touchpad:
		{
			if (event->eventType == vr::VREvent_ButtonPress)
			{
				this->touchPadClicked(state.rAxis[0].x, state.rAxis[0].y);
			}

			if (event->eventType == vr::VREvent_ButtonUnpress)
			{
				this->touchPadUnclicked(state.rAxis[0].x, state.rAxis[0].y);
			}

			if (event->eventType == vr::VREvent_ButtonTouch)
			{
				this->touchpadInitialTouch(state.rAxis[0].x, state.rAxis[0].y);
			}

			if (event->eventType == vr::VREvent_ButtonUntouch)
			{
				this->touchpadUntouched();
			}
		}
		break;
		default:
			;//printf("Controller (device %u) event not processed: %u.\n", event.trackedDeviceIndex, event.eventType);
		}
	}

	// TOUCHPAD BEING TOUCHED
	if (this->isTouchpadTouched())
	{
		this->touchpadTouch(state.rAxis[0].x, state.rAxis[0].y);
	}

	// TRIGGER INTERACTIONS
	if (state.rAxis[1].x >= this->getTriggerThreshold())
	{
		// TRIGGER ENGAGED
		if (!this->isTriggerEngaged())
		{
			this->triggerEngaged();
		}

		// TRIGGER BEING PULLED
		if (!this->isTriggerClicked())
		{
			this->triggerBeingPulled(state.rAxis[1].x);
		}

		// TRIGGER CLICKED
		if (state.rAxis[1].x == 1.f && !this->isTriggerClicked())
		{
			this->triggerClicked();
		}
		// TRIGGER UNCLICKED
		if (state.rAxis[1].x != 1.f && this->isTriggerClicked())
		{
			this->triggerUnclicked();
		}
	}
	// TRIGGER DISENGAGED
	else if (this->isTriggerEngaged())
	{
		this->triggerDisengaged();
	}
}

bool ViveController::updatePose(vr::TrackedDevicePose_t pose)
{
	m_Pose = pose;
	m_mat4Pose = ConvertSteamVRMatrixToMatrix4(m_Pose.mDeviceToAbsoluteTracking);

	return m_Pose.bPoseIsValid;
}

void ViveController::addComponentRenderModel(uint32_t unComponentIndex, std::string pchComponentName, CGLRenderModel * pComponentRenderModel)
{
	ControllerComponent newComponent;

	newComponent.m_unComponentIndex = unComponentIndex;
	newComponent.m_strComponentName = pchComponentName;
	newComponent.m_pComponentRenderModel = pComponentRenderModel;

	m_vComponents.push_back(newComponent);
}

Matrix4 ViveController::getComponentPose(uint32_t unComponentIndex)
{
	uint32_t nComponents = getComponentCount();
	if (nComponents == 0 || unComponentIndex >= nComponents)
		return Matrix4().identity();

	ControllerComponent *c = &m_vComponents[unComponentIndex];

	vr::EVRInitError eError = vr::VRInitError_None;
	vr::IVRSystem *pHMD = (vr::IVRSystem *)vr::VR_GetGenericInterface(vr::IVRSystem_Version, &eError);
	vr::IVRRenderModels *pRenderModelsInterface = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);

	vr::VRControllerState_t pControllerState;
	vr::RenderModel_ControllerMode_State_t pState;
	vr::RenderModel_ComponentState_t pComponentState;

	pHMD->GetControllerState(m_unDeviceID, &pControllerState);
	pRenderModelsInterface->GetComponentState(m_strRenderModelName.c_str(), c->m_strComponentName.c_str(), &pControllerState, &pState, &pComponentState);

	//return ConvertSteamVRMatrixToMatrix4(pComponentState->mTrackingToComponentRenderModel);
	return ConvertSteamVRMatrixToMatrix4(pComponentState.mTrackingToComponentRenderModel);
}

//-----------------------------------------------------------------------------
// Purpose: Draw all of the line-based controller augmentations
//-----------------------------------------------------------------------------
void ViveController::prepareForRendering()
{
	std::vector<float> vertdataarray;

	m_uiLineVertcount = 0;
	m_uiTriVertcount = 0;

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

			m_uiLineVertcount += 2;
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
		m_uiLineVertcount += 2;
	}

	// Draw Touchpad line
	if(m_bTouchpadTouched)
	{

		Vector4 start = mat * transformTouchPointToModelCoords(&m_vec2TouchpadInitialTouchPoint);
		Vector4 end = mat * transformTouchPointToModelCoords(&m_vec2TouchpadCurrentTouchPoint);
		Vector3 color(.9f, .2f, .1f);

		vertdataarray.push_back(start.x); vertdataarray.push_back(start.y); vertdataarray.push_back(start.z);
		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

		vertdataarray.push_back(end.x); vertdataarray.push_back(end.y); vertdataarray.push_back(end.z);
		vertdataarray.push_back(color.z); vertdataarray.push_back(color.y); vertdataarray.push_back(color.x);
		m_uiLineVertcount += 2;
	}

	// Draw touchpad touch point sphere
	if (m_bTouchpadTouched)
	{
		insertTouchpadCursor(vertdataarray, m_uiTriVertcount);
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

void ViveController::systemButtonPressed()
{
	//printf("Controller (device %u) system button pressed.\n", m_DeviceID);
	m_bSystemButtonClicked = true;
}

void ViveController::systemButtonUnpressed()
{
	//printf("Controller (device %u) system button unpressed.\n", m_DeviceID);
	m_bSystemButtonClicked = false;
}

bool ViveController::isSystemButtonPressed()
{
	return m_bSystemButtonClicked;
}

void ViveController::menuButtonPressed()
{
	//printf("Controller (device %u) menu button pressed.\n", m_DeviceID);
	m_bMenuButtonClicked = true;
}

void ViveController::menuButtonUnpressed()
{
	//printf("Controller (device %u) menu button unpressed.\n", m_DeviceID);
	m_bMenuButtonClicked = false;
}

bool ViveController::isMenuButtonPressed()
{
	return m_bMenuButtonClicked;
}

void ViveController::gripButtonPressed()
{
	//printf("Controller (device %u) grip pressed.\n", m_DeviceID);
	m_bGripButtonClicked = true;
	toggleAxes();
}

void ViveController::gripButtonUnpressed()
{
	//printf("Controller (device %u) grip unpressed.\n", m_DeviceID);
	m_bGripButtonClicked = false;
}

bool ViveController::isGripButtonPressed()
{
	return m_bGripButtonClicked;
}

void ViveController::triggerEngaged()
{
	//printf("Controller (device %u) trigger engaged).\n", m_DeviceID);
	m_bTriggerEngaged = true;
}

void ViveController::triggerBeingPulled(float amount)
{
	//printf("Controller (device %u) trigger at %f%%).\n", m_DeviceID, amount * 100.f);
}

void ViveController::triggerDisengaged()
{
	//printf("Controller (device %u) trigger disengaged).\n", m_DeviceID);
	m_bTriggerEngaged = false;
}

void ViveController::triggerClicked()
{
	//printf("Controller (device %u) trigger clicked.\n", m_DeviceID);
	m_bTriggerClicked = true;
}

void ViveController::triggerUnclicked()
{
	//printf("Controller (device %u) trigger unclicked.\n", m_DeviceID);
	m_bTriggerClicked = false;
}

bool ViveController::isTriggerEngaged()
{
	return m_bTriggerEngaged;
}

bool ViveController::isTriggerClicked()
{
	return m_bTriggerClicked;
}

float ViveController::getTriggerThreshold()
{
	return m_fTriggerLowerThreshold;
}

void ViveController::touchpadInitialTouch(float x, float y)
{
	//printf("Controller (device %u) touchpad touched at initial position (%f, %f).\n", m_DeviceID, x, y);
	m_bTouchpadTouched = true;
	m_vec2TouchpadInitialTouchPoint = Vector2(x, y);
	m_vec2TouchpadCurrentTouchPoint = m_vec2TouchpadInitialTouchPoint;

}

void ViveController::touchpadTouch(float x, float y)
{
	//printf("Controller (device %u) touchpad touch tracked at (%f, %f).\n" , m_DeviceID, x, y);
	m_vec2TouchpadCurrentTouchPoint = Vector2(x, y);

	if (m_vec2TouchpadInitialTouchPoint.equal(Vector2(0.f, 0.f), 0.000001))
		m_vec2TouchpadInitialTouchPoint = m_vec2TouchpadCurrentTouchPoint;
}

void ViveController::touchpadUntouched()
{
	//printf("Controller (device %u) touchpad untouched.\n", m_DeviceID);
	m_bTouchpadTouched = false;
	m_vec2TouchpadInitialTouchPoint = Vector2(0.f, 0.f);
	m_vec2TouchpadCurrentTouchPoint = Vector2(0.f, 0.f);
}

void ViveController::touchPadClicked(float x, float y)
{
	//printf("Controller (device %u) touchpad pressed at (%f, %f).\n", m_DeviceID, x, y);
	m_bTouchpadClicked = true;
}

void ViveController::touchPadUnclicked(float x, float y)
{
	//printf("Controller (device %u) touchpad pressed at (%f, %f).\n", m_DeviceID, x, y);
	m_bTouchpadClicked = false;
}

bool ViveController::isTouchpadTouched()
{
	return m_bTouchpadTouched;
}

bool ViveController::isTouchpadClicked()
{
	return m_bTouchpadClicked;
}

void ViveController::renderModel()
{
	for(size_t i = 0; i < m_vComponents.size(); ++i)
		if (m_vComponents[i].m_pComponentRenderModel)
			m_vComponents[i].m_pComponentRenderModel->Draw();
}

Vector4 ViveController::transformTouchPointToModelCoords(Vector2 * pt)
{
	Vector4 xVec = (pt->x > 0 ? c_vec4TouchPadRight : c_vec4TouchPadLeft) - c_vec4TouchPadCenter;
	Vector4 yVec = (pt->y > 0 ? c_vec4TouchPadTop : c_vec4TouchPadBottom) - c_vec4TouchPadCenter;

	return c_vec4TouchPadCenter + (xVec * abs(pt->x) + yVec * abs(pt->y));
}

void ViveController::insertTouchpadCursor(std::vector<float> &vertices, unsigned int &nTriangleVertices)
{
	std::vector<float> sphereVertdataarray;
	std::vector<Vector3> sphereVerts = m_TouchPointSphere.getUnindexedVertices();
	Vector4 ctr = transformTouchPointToModelCoords(&m_vec2TouchpadCurrentTouchPoint);

	//Vector3 color(.2f, .2f, .71f);
	Vector3 color(.65f, .65f, .65f);

	Matrix4 & sphereMat = m_mat4Pose * Matrix4().translate(Vector3(ctr.x, ctr.y, ctr.z)) * Matrix4().scale(0.0025f);

	for (size_t i = 0; i < sphereVerts.size(); ++i)
	{
		Vector4 thisPt = sphereMat * Vector4(sphereVerts[i].x, sphereVerts[i].y, sphereVerts[i].z, 1.f);

		sphereVertdataarray.push_back(thisPt.x);
		sphereVertdataarray.push_back(thisPt.y);
		sphereVertdataarray.push_back(thisPt.z);

		sphereVertdataarray.push_back(color.x);
		sphereVertdataarray.push_back(color.y);
		sphereVertdataarray.push_back(color.z);

		nTriangleVertices++;
	}

	vertices.insert(vertices.end(), sphereVertdataarray.begin(), sphereVertdataarray.end());
}
