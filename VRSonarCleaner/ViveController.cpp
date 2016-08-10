#include "ViveController.h"

ViveController::ViveController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex, vr::IVRSystem *pHMD, vr::IVRRenderModels *pRenderModels)
	: TrackedDevice(unTrackedDeviceIndex, pHMD, pRenderModels)
	, m_unStatePacketNum(0)
	, m_bShowScrollWheel(false)
	, m_bTouchpadTouched(false)
	, m_bTouchpadClicked(false)
	, m_vec2TouchpadInitialTouchPoint(Vector2(0.f, 0.f))
	, m_vec2TouchpadCurrentTouchPoint(Vector2(0.f, 0.f))
	, m_bTriggerEngaged(false)
	, m_bTriggerClicked(false)
	, m_fHairTriggerThreshold(0.05f)
	, m_unTriggerAxis(0u)
	, m_unTouchpadAxis(0u)
	, m_TouchPointSphere(Icosphere(2))
	, c_vec4TouchPadCenter(Vector4(0.f, 0.00378f, 0.04920f, 1.f))
	, c_vec4TouchPadLeft(Vector4(-0.02023f, 0.00495f, 0.04934f, 1.f))
	, c_vec4TouchPadRight(Vector4(0.02023f, 0.00495f, 0.04934f, 1.f))
	, c_vec4TouchPadTop(Vector4(0.f, 0.00725f, 0.02924f, 1.f))
	, c_vec4TouchPadBottom(Vector4(0.f, 0.00265f, 0.06943f, 1.f))
	, m_pOverlayHandle(vr::k_ulOverlayHandleInvalid)
{

}

ViveController::~ViveController()
{

}

bool ViveController::BInit()
{
	std::string strRenderModelName = getPropertyString(vr::Prop_RenderModelName_String);

	CGLRenderModel *pRenderModel = loadRenderModel(strRenderModelName.c_str());

	if (!pRenderModel)
	{
		std::string sTrackingSystemName = getPropertyString(vr::Prop_TrackingSystemName_String);
		printf("Unable to load render model for controller [device %d] (%s.%s)", m_unDeviceID, sTrackingSystemName.c_str(), strRenderModelName.c_str());
		return false;
	}
	else
	{
		m_strRenderModelName = pRenderModel->GetName();

		std::cout << "Device " << m_unDeviceID << "'s RenderModel name is " << m_strRenderModelName << std::endl;
		setRenderModel(pRenderModel);
	}

	if (pRenderModel)
	{
		const char* pchRenderName = m_strRenderModelName.c_str();

		uint32_t nModelComponents = m_pRenderModels->GetComponentCount(pchRenderName);

		for (uint32_t i = 0; i < nModelComponents; ++i)
		{
			ControllerComponent c;
			c.m_unComponentIndex = i;

			uint32_t unRequiredBufferLen = m_pRenderModels->GetComponentName(pRenderModel->GetName().c_str(), i, NULL, 0);
			if (unRequiredBufferLen == 0)
			{
				printf("Controller [device %d] component %d index out of range.\n", m_unDeviceID, i);
			}
			else
			{
				char *pchBuffer1 = new char[unRequiredBufferLen];
				unRequiredBufferLen = m_pRenderModels->GetComponentName(pchRenderName, i, pchBuffer1, unRequiredBufferLen);
				c.m_strComponentName = pchBuffer1;
				delete[] pchBuffer1;

				unRequiredBufferLen = m_pRenderModels->GetComponentRenderModelName(pchRenderName, c.m_strComponentName.c_str(), NULL, 0);
				if (unRequiredBufferLen == 0)
					c.m_bHasRenderModel = false;
				else
				{
					char *pchBuffer2 = new char[unRequiredBufferLen];
					unRequiredBufferLen = m_pRenderModels->GetComponentRenderModelName(pchRenderName, c.m_strComponentName.c_str(), pchBuffer2, unRequiredBufferLen);
					std::string sComponentRenderModelName = pchBuffer2;

					CGLRenderModel *pComponentRenderModel = loadRenderModel(pchBuffer2);
					delete[] pchBuffer2;

					c.m_pComponentRenderModel = pComponentRenderModel;
					c.m_bHasRenderModel = true;					
				}

				c.m_bInitialized = true;

				m_vComponents.push_back(c);

				std::cout << "\t" << (c.m_bHasRenderModel ? "M -> " : "     ") << i << ": " << c.m_strComponentName << std::endl;
			}
		}
	}

	
	// Figure out controller axis indices
	for (uint32_t i = 0u; i < vr::k_unControllerStateAxisCount; ++i)
	{
		vr::ETrackedDeviceProperty p = static_cast<vr::TrackedDeviceProperty>(vr::TrackedDeviceProperty::Prop_Axis0Type_Int32 + i);
		vr::EVRControllerAxisType axisType = static_cast<vr::EVRControllerAxisType>(getPropertyInt32(p));
		if (axisType == vr::k_eControllerAxis_Trigger) m_unTriggerAxis = i;
		if (axisType == vr::k_eControllerAxis_TrackPad) m_unTouchpadAxis = i;
	}
	

	createShaders();

	return true;
}

void ViveController::update()
{
	vr::VRControllerState_t controllerState;
	if (!m_pHMD->GetControllerState(m_unDeviceID, &controllerState))
		return;


	// check if anything has changed
	if (m_unStatePacketNum == controllerState.unPacketNum)
		return;

	m_unStatePacketNum = controllerState.unPacketNum;

	// Set scrollwheel mode if necessary
	vr::RenderModel_ControllerMode_State_t controllerModeState;
	controllerModeState.bScrollWheelVisible = m_bShowScrollWheel;
	bool bScrollWheelBefore = m_bShowScrollWheel;

	vr::RenderModel_ComponentState_t controllerComponentState;
	
	// Update the controller components
	for (std::vector<ControllerComponent>::iterator it = m_vComponents.begin(); it != m_vComponents.end(); ++it)
	{
		m_pRenderModels->GetComponentState(m_strRenderModelName.c_str(), it->m_strComponentName.c_str(), &controllerState, &controllerModeState, &controllerComponentState);

		it->m_mat3PoseTransform = controllerComponentState.mTrackingToComponentRenderModel;

		uint64_t buttonMask = m_pRenderModels->GetComponentButtonMask(m_strRenderModelName.c_str(), it->m_strComponentName.c_str());
		bool bPressed = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsPressed;
		bool bTouched = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsTouched;

		// Find buttons associated with component and handle state changes/events
		
		if (vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu) & buttonMask)
		{
			// Button pressed
			if(!it->m_bPressed && bPressed)
			{
				menuButtonPressed();
			}

			// Button unpressed
			if (it->m_bPressed && !bPressed)
			{
				menuButtonUnpressed();
			}
		}

		if (vr::ButtonMaskFromId(vr::k_EButton_System) & buttonMask)
		{
			// Button pressed
			if (!it->m_bPressed && bPressed)
			{
				systemButtonPressed();
			}

			// Button unpressed
			if (it->m_bPressed && !bPressed)
			{
				systemButtonUnpressed();
			}
		}

		if (vr::ButtonMaskFromId(vr::k_EButton_Grip) & buttonMask)
		{
			// Button pressed
			if (!isGripButtonPressed() && bPressed)
			{
				gripButtonPressed();
			}

			// Button unpressed
			if (isGripButtonPressed() && !bPressed)
			{
				gripButtonUnpressed();
			}
		}

		if (vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger) & buttonMask)
		{
			float triggerPull = controllerState.rAxis[m_unTriggerAxis].x; // trigger data on x axis

			// Trigger pressed
			if (!isTriggerClicked() && bPressed)
			{
				//printf("(VR Event) Controller (device %u) trigger pressed.\n", m_unDeviceID);
			}

			// Trigger unpressed
			if (isTriggerClicked() && !bPressed)
			{
				//printf("(VR Event) Controller (device %u) trigger unpressed.\n", m_unDeviceID);
			}

			// Trigger touched
			if (!isTriggerEngaged() && bTouched)
			{
				//printf("(VR Event) Controller (device %u) trigger touched.\n", m_unDeviceID);
			}

			// Trigger untouched
			if (isTriggerEngaged() && !bTouched)
			{
				//printf("(VR Event) Controller (device %u) trigger untouched.\n", m_unDeviceID);
			}

			// TRIGGER INTERACTIONS
			if (triggerPull >= getHairTriggerThreshold())
			{
				// TRIGGER ENGAGED
				if (!isTriggerEngaged())
				{
					triggerEngaged();
				}

				// TRIGGER BEING PULLED
				if (!isTriggerClicked())
				{
					triggerBeingPulled(triggerPull);
				}

				// TRIGGER CLICKED
				if (triggerPull >= 1.f && !isTriggerClicked())
				{
					triggerClicked();
				}
				// TRIGGER UNCLICKED
				if (triggerPull < 1.f && isTriggerClicked())
				{
					triggerUnclicked();
				}
			}
			// TRIGGER DISENGAGED
			else if (isTriggerEngaged())
			{
				triggerDisengaged();
			}
		}

		if (vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad) & buttonMask)
		{
			vr::VRControllerAxis_t touchpadAxis = controllerState.rAxis[m_unTouchpadAxis];

			// Touchpad pressed
			if (!it->m_bPressed && bPressed)
			{
				touchPadClicked(touchpadAxis.x, touchpadAxis.y);
			}

			// Touchpad unpressed
			if (it->m_bPressed && !bPressed)
			{
				touchPadUnclicked(touchpadAxis.x, touchpadAxis.y);
			}

			// Touchpad touched
			if (!it->m_bTouched && bTouched)
			{
				touchpadInitialTouch(touchpadAxis.x, touchpadAxis.y);
			}

			// Touchpad untouched
			if (it->m_bTouched && !bTouched)
			{
				this->touchpadUntouched();
			}

			// Touchpad being touched
			if (this->isTouchpadTouched())
			{
				this->touchpadTouch(touchpadAxis.x, touchpadAxis.y);
			}
		}

		// Update the component's properties
		it->m_bStatic = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsStatic;
		it->m_bVisible = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsVisible;
		it->m_bTouched = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsTouched;
		it->m_bPressed = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsPressed;
		it->m_bScrolled = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsScrolled;
	}

	// see if scrollwheel model visibility changed, and update if necessary
	if (bScrollWheelBefore != m_bShowScrollWheel)
	{
		controllerModeState.bScrollWheelVisible = m_bShowScrollWheel;

		for (std::vector<ControllerComponent>::iterator it = m_vComponents.begin(); it != m_vComponents.end(); ++it)
		{
			m_pRenderModels->GetComponentState(m_strRenderModelName.c_str(), it->m_strComponentName.c_str(), &controllerState, &controllerModeState, &controllerComponentState);

			it->m_mat3PoseTransform = controllerComponentState.mTrackingToComponentRenderModel;

			// Update the component's properties
			it->m_bStatic = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsStatic;
			it->m_bVisible = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsVisible;
			it->m_bTouched = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsTouched;
			it->m_bPressed = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsPressed;
			it->m_bScrolled = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsScrolled;
		}
	}
}

bool ViveController::updatePose(vr::TrackedDevicePose_t pose)
{
	m_Pose = pose;
	m_mat4Pose = ConvertSteamVRMatrixToMatrix4(m_Pose.mDeviceToAbsoluteTracking);

	return m_Pose.bPoseIsValid;
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
		insertTouchpadCursor(vertdataarray, m_uiTriVertcount, 0.35f, 0.35f, 0.35f);
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
	
	vr::EVROverlayError oError = vr::VROverlayError_None;

	if (m_pOverlayHandle == vr::k_ulOverlayHandleInvalid)
	{
		printf("Setting up overlay... ");
		std::string filename = "C:\\Users\\field\\Documents\\GitHub\\VRSonarCleaner\\VRSonarCleaner\\test.png";

		oError = vr::VROverlay()->CreateOverlay("test", "An Overlay Test", &m_pOverlayHandle);

		if (oError != vr::EVROverlayError::VROverlayError_None)
			printf("Overlay could not be created: %d\n", oError);

		oError = vr::VROverlay()->SetOverlayFromFile(m_pOverlayHandle, filename.c_str());

		if (oError != vr::EVROverlayError::VROverlayError_None)
			printf("Overlay file could not be loaded: %d\n", oError);

		vr::HmdMatrix34_t overlayDistanceMtx;
		memset(&overlayDistanceMtx, 0, sizeof(overlayDistanceMtx));
		overlayDistanceMtx.m[0][0] = overlayDistanceMtx.m[1][1] = overlayDistanceMtx.m[2][2] = 1.f;
		overlayDistanceMtx.m[0][3] = -0.25f;
		overlayDistanceMtx.m[1][3] = 0.0f;
		overlayDistanceMtx.m[2][3] = 0.0f;
		oError = vr::VROverlay()->SetOverlayTransformTrackedDeviceRelative(m_pOverlayHandle, m_unDeviceID, &overlayDistanceMtx);
		if (oError != vr::EVROverlayError::VROverlayError_None)
			printf("Overlay transform could not be set.\n");

		vr::VROverlay()->SetOverlayWidthInMeters(m_pOverlayHandle, 0.5f);
		
		printf("done!\n");
	}
	
	if (m_pOverlayHandle != vr::k_ulOverlayHandleInvalid)
	{
		oError = vr::VROverlay()->ShowOverlay(m_pOverlayHandle);
		if (oError != vr::EVROverlayError::VROverlayError_None)
			printf("Overlay could not be shown: %d\n", oError);
	}
	else
		printf("Overlay handle invalid.\n");
}

void ViveController::gripButtonUnpressed()
{
	//printf("Controller (device %u) grip unpressed.\n", m_DeviceID);
	m_bGripButtonClicked = false;
	
	if (m_pOverlayHandle != vr::k_ulOverlayHandleInvalid)
	{
		vr::EVROverlayError oError = vr::VROverlay()->HideOverlay(m_pOverlayHandle);
		if (oError != vr::EVROverlayError::VROverlayError_None)
			printf("Overlay could not be hidden: %d\n", oError);
	}
	else
		printf("Overlay handle invalid.\n");
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

float ViveController::getHairTriggerThreshold()
{
	return m_fHairTriggerThreshold;
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

void ViveController::renderModel(Matrix4 & matVP)
{
	if (!poseValid())
		return;

	// ----- Render Model rendering -----
	glUseProgram(m_unRenderModelProgramID);
	
	for(size_t i = 0; i < m_vComponents.size(); ++i)
		if (m_vComponents[i].m_pComponentRenderModel && m_vComponents[i].m_bVisible)
		{
			Matrix4 matMVP;

			if (!m_vComponents[i].m_bStatic)
			{
				vr::TrackedDevicePose_t p;
				m_pHMD->ApplyTransform(&p, &m_Pose, &(m_vComponents[i].m_mat3PoseTransform));
				matMVP = matVP * ConvertSteamVRMatrixToMatrix4(p.mDeviceToAbsoluteTracking);
			}
			else
			{
				matMVP = matVP * m_mat4Pose;
			}

			glUniformMatrix4fv(m_nRenderModelMatrixLocation, 1, GL_FALSE, matMVP.get());
			m_vComponents[i].m_pComponentRenderModel->Draw();
		}
	glUseProgram(0);
}

Vector4 ViveController::transformTouchPointToModelCoords(Vector2 * pt)
{
	Vector4 xVec = (pt->x > 0 ? c_vec4TouchPadRight : c_vec4TouchPadLeft) - c_vec4TouchPadCenter;
	Vector4 yVec = (pt->y > 0 ? c_vec4TouchPadTop : c_vec4TouchPadBottom) - c_vec4TouchPadCenter;

	return c_vec4TouchPadCenter + (xVec * abs(pt->x) + yVec * abs(pt->y));
}

void ViveController::insertTouchpadCursor(std::vector<float> &vertices, unsigned int &nTriangleVertices, float r, float g, float b)
{
	std::vector<float> sphereVertdataarray;
	std::vector<Vector3> sphereVerts = m_TouchPointSphere.getUnindexedVertices();
	Vector4 ctr = transformTouchPointToModelCoords(&m_vec2TouchpadCurrentTouchPoint);

	//Vector3 color(.2f, .2f, .71f);
	Vector3 color(r, g, b);

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
