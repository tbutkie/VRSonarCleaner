#include "ViveController.h"

#include <iostream>

#include <shared/glm/gtc/type_ptr.hpp>
#include <shared/glm/gtc/matrix_transform.hpp>

ViveController::ViveController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex, vr::IVRSystem *pHMD, vr::IVRRenderModels *pRenderModels)
	: TrackedDevice(unTrackedDeviceIndex, pHMD, pRenderModels)
	, m_unStatePacketNum(0)
	, m_bShowScrollWheel(false)
	, m_bSystemButtonClicked(false)
	, m_bMenuButtonClicked(false)
	, m_bGripButtonClicked(false)
	, m_bTouchpadTouched(false)
	, m_bTouchpadClicked(false)
	, m_vec2TouchpadInitialTouchPoint(glm::vec2(0.f, 0.f))
	, m_vec2TouchpadCurrentTouchPoint(glm::vec2(0.f, 0.f))
	, m_bTriggerEngaged(false)
	, m_bTriggerClicked(false)
	, m_fTriggerPull(0.f)
	, m_fHairTriggerThreshold(0.05f)
	, m_unTriggerAxis(vr::k_unControllerStateAxisCount)
	, m_unTouchpadAxis(vr::k_unControllerStateAxisCount)
	, m_TouchPointSphere(Icosphere(2))
	, c_vec4TouchPadCenter(glm::vec4(0.f, 0.00378f, 0.04920f, 1.f))
	, c_vec4TouchPadLeft(glm::vec4(-0.02023f, 0.00495f, 0.04934f, 1.f))
	, c_vec4TouchPadRight(glm::vec4(0.02023f, 0.00495f, 0.04934f, 1.f))
	, c_vec4TouchPadTop(glm::vec4(0.f, 0.00725f, 0.02924f, 1.f))
	, c_vec4TouchPadBottom(glm::vec4(0.f, 0.00265f, 0.06943f, 1.f))
	, m_pOverlayHandle(vr::k_ulOverlayHandleInvalid)
{

}

ViveController::~ViveController()
{
	m_vComponents.clear();
}

bool ViveController::BInit()
{	
	// Figure out controller axis indices
	for (uint32_t i = 0u; i < vr::k_unControllerStateAxisCount; ++i)
	{
		vr::ETrackedDeviceProperty p = static_cast<vr::TrackedDeviceProperty>(vr::TrackedDeviceProperty::Prop_Axis0Type_Int32 + i);
		vr::EVRControllerAxisType axisType = static_cast<vr::EVRControllerAxisType>(getPropertyInt32(p));
		if (axisType == vr::k_eControllerAxis_Trigger) m_unTriggerAxis = i;
		if (axisType == vr::k_eControllerAxis_TrackPad) m_unTouchpadAxis = i;
	}

	// Check if we were able to figure 'em out
	if (m_unTriggerAxis == vr::k_unControllerStateAxisCount)
		printf("Unable to find proper axis for controller trigger.\n");
	
	if (m_unTouchpadAxis == vr::k_unControllerStateAxisCount)
		printf("Unable to find proper axes for controller touchpad.\n");


	// Create shaders used for rendering controller model as well as our own custom geometry
	createShaders();


	// Create Overlay
	vr::EVROverlayError oError = vr::VROverlayError_None;

	if (m_pOverlayHandle == vr::k_ulOverlayHandleInvalid)
	{
		printf("Setting up overlay... ");
		std::string filename = "overlay_thumb_placeholder.png"; // relative to SteamVR resources folder or absolute filepath

		oError = vr::VROverlay()->CreateOverlay("test", "An Overlay Test", &m_pOverlayHandle);

		if (oError != vr::EVROverlayError::VROverlayError_None)
			printf("Overlay could not be created: %d\n", oError);

		oError = vr::VROverlay()->SetOverlayFromFile(m_pOverlayHandle, filename.c_str());

		if (oError != vr::EVROverlayError::VROverlayError_None)
			printf("Overlay file could not be loaded: %d\n", oError);

		vr::VROverlay()->SetOverlayWidthInMeters(m_pOverlayHandle, 0.1f);

		printf("done!\n");
	}

	return true;
}

void ViveController::update()
{
	if (!m_Pose.bDeviceIsConnected || !m_Pose.bPoseIsValid)
		return;

	vr::VRControllerState_t controllerState;
	if (!m_pHMD->GetControllerState(m_unDeviceID, &controllerState, sizeof(controllerState)))
		return;


	// check if anything has changed
	if (controllerState.unPacketNum > 0 && m_unStatePacketNum == controllerState.unPacketNum)
		return;

	m_unStatePacketNum = controllerState.unPacketNum;

	// Set scrollwheel mode if necessary
	vr::RenderModel_ControllerMode_State_t controllerModeState;
	controllerModeState.bScrollWheelVisible = m_bShowScrollWheel;
	bool bScrollWheelBefore = m_bShowScrollWheel;

	vr::RenderModel_ComponentState_t controllerComponentState;
	
	// Update the controller components
	for (auto &component : m_vComponents)
	{
		m_pRenderModels->GetComponentState(m_strRenderModelName.c_str(), component.m_strComponentName.c_str(), &controllerState, &controllerModeState, &controllerComponentState);

		component.m_mat3PoseTransform = controllerComponentState.mTrackingToComponentRenderModel;

		uint64_t buttonMask = m_pRenderModels->GetComponentButtonMask(m_strRenderModelName.c_str(), component.m_strComponentName.c_str());
		bool bPressed = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsPressed;
		bool bTouched = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsTouched;

		// Find buttons associated with component and handle state changes/events
		
		if (vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu) & buttonMask)
		{
			// Button pressed
			if(!component.m_bPressed && bPressed)
			{
				menuButtonPressed();
			}

			// Button unpressed
			if (component.m_bPressed && !bPressed)
			{
				menuButtonUnpressed();
			}
		}

		if (vr::ButtonMaskFromId(vr::k_EButton_System) & buttonMask)
		{
			// Button pressed
			if (!component.m_bPressed && bPressed)
			{
				systemButtonPressed();
			}

			// Button unpressed
			if (component.m_bPressed && !bPressed)
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
					triggerEngaged(triggerPull);
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
					triggerUnclicked(triggerPull);
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
			if (!component.m_bPressed && bPressed)
			{
				touchPadClicked(touchpadAxis.x, touchpadAxis.y);
			}

			// Touchpad unpressed
			if (component.m_bPressed && !bPressed)
			{
				touchPadUnclicked(touchpadAxis.x, touchpadAxis.y);
			}

			// Touchpad touched
			if (!component.m_bTouched && bTouched)
			{
				touchpadInitialTouch(touchpadAxis.x, touchpadAxis.y);
			}

			// Touchpad untouched
			if (component.m_bTouched && !bTouched)
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
		component.m_bStatic = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsStatic;
		component.m_bVisible = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsVisible;
		component.m_bTouched = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsTouched;
		component.m_bPressed = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsPressed;
		component.m_bScrolled = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsScrolled;
	}

	// see if scrollwheel model visibility changed, and update if necessary
	//if (bScrollWheelBefore != m_bShowScrollWheel)
	//{
	//	controllerModeState.bScrollWheelVisible = m_bShowScrollWheel;

	//	for (std::vector<TrackedDeviceComponent>::iterator it = m_vComponents.begin(); it != m_vComponents.end(); ++it)
	//	{
	//		m_pRenderModels->GetComponentState(m_strRenderModelName.c_str(), it->m_strComponentName.c_str(), &controllerState, &controllerModeState, &controllerComponentState);

	//		it->m_mat3PoseTransform = controllerComponentState.mTrackingToComponentRenderModel;

	//		// Update the component's properties
	//		it->m_bStatic = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsStatic;
	//		it->m_bVisible = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsVisible;
	//		it->m_bTouched = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsTouched;
	//		it->m_bPressed = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsPressed;
	//		it->m_bScrolled = controllerComponentState.uProperties & vr::EVRComponentProperty::VRComponentProperty_IsScrolled;
	//	}
	//}
}

bool ViveController::updatePose(vr::TrackedDevicePose_t pose)
{
	m_Pose = pose;
	m_mat4DeviceToWorldTransform = ConvertSteamVRMatrixToMatrix4(m_Pose.mDeviceToAbsoluteTracking);

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

	const glm::mat4 & mat = getDeviceToWorldTransform();

	// Draw Axes
	if (axesActive())
	{
		for (int i = 0; i < 3; ++i)
		{
			glm::vec4 color(0, 0, 0, 1);
			glm::vec4 center = mat * glm::vec4(0, 0, 0, 1);
			glm::vec4 point(0, 0, 0, 1);
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
			vertdataarray.push_back(color.w);

			vertdataarray.push_back(point.x);
			vertdataarray.push_back(point.y);
			vertdataarray.push_back(point.z);

			vertdataarray.push_back(color.x);
			vertdataarray.push_back(color.y);
			vertdataarray.push_back(color.z);
			vertdataarray.push_back(color.w);

			m_uiLineVertcount += 2;
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
	//	m_uiLineVertcount += 2;
	//}

	// Draw Touchpad line
	if(m_bTouchpadTouched)
	{

		glm::vec4 start = mat * transformTouchPointToModelCoords(&m_vec2TouchpadInitialTouchPoint);
		glm::vec4 end = mat * transformTouchPointToModelCoords(&m_vec2TouchpadCurrentTouchPoint);
		glm::vec4 color(.9f, .2f, .1f, 0.75f);

		vertdataarray.push_back(start.x); vertdataarray.push_back(start.y); vertdataarray.push_back(start.z);
		vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z); vertdataarray.push_back(color.w);

		vertdataarray.push_back(end.x); vertdataarray.push_back(end.y); vertdataarray.push_back(end.z);
		vertdataarray.push_back(color.z); vertdataarray.push_back(color.y); vertdataarray.push_back(color.x); vertdataarray.push_back(color.w);
		m_uiLineVertcount += 2;
	}

	// Draw touchpad touch point sphere
	if (m_bTouchpadTouched)
	{
		insertTouchpadCursor(vertdataarray, m_uiTriVertcount, 0.35f, 0.35f, 0.35f, 0.75f);
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

void ViveController::systemButtonPressed()
{
	//printf("Controller (device %u) system button pressed.\n", m_unDeviceID);
	m_bSystemButtonClicked = true;
}

void ViveController::systemButtonUnpressed()
{
	//printf("Controller (device %u) system button unpressed.\n", m_unDeviceID);
	m_bSystemButtonClicked = false;
}

bool ViveController::isSystemButtonPressed()
{
	return m_bSystemButtonClicked;
}

void ViveController::menuButtonPressed()
{
	//printf("Controller (device %u) menu button pressed.\n", m_unDeviceID);
	m_bMenuButtonClicked = true;
}

void ViveController::menuButtonUnpressed()
{
	//printf("Controller (device %u) menu button unpressed.\n", m_unDeviceID);
	m_bMenuButtonClicked = false;
}

bool ViveController::isMenuButtonPressed()
{
	return m_bMenuButtonClicked;
}

void ViveController::gripButtonPressed()
{
	//printf("Controller (device %u) grip pressed.\n", m_unDeviceID);
	m_bGripButtonClicked = true;
	//toggleAxes();
}

void ViveController::gripButtonUnpressed()
{
	//printf("Controller (device %u) grip unpressed.\n", m_unDeviceID);
	m_bGripButtonClicked = false;
	//toggleAxes();
}

bool ViveController::isGripButtonPressed()
{
	return m_bGripButtonClicked;
}

void ViveController::triggerEngaged(float amount)
{
	//printf("Controller (device %u) trigger engaged).\n", m_unDeviceID);
	m_bTriggerEngaged = true;
	m_fTriggerPull = amount;
}

void ViveController::triggerBeingPulled(float amount)
{
	//printf("Controller (device %u) trigger at %f%%).\n", m_unDeviceID, amount * 100.f);
	m_fTriggerPull = amount;
}

void ViveController::triggerDisengaged()
{
	//printf("Controller (device %u) trigger disengaged).\n", m_unDeviceID);
	m_bTriggerEngaged = false;
	m_fTriggerPull = 0.f;
}

void ViveController::triggerClicked()
{
	//printf("Controller (device %u) trigger clicked.\n", m_unDeviceID);
	m_bTriggerClicked = true;
	m_fTriggerPull = 1.f;
}

void ViveController::triggerUnclicked(float amount)
{
	//printf("Controller (device %u) trigger unclicked.\n", m_unDeviceID);
	m_bTriggerClicked = false;
	m_fTriggerPull = amount;
}

bool ViveController::isTriggerEngaged()
{
	return m_bTriggerEngaged;
}

bool ViveController::isTriggerClicked()
{
	return m_bTriggerClicked;
}

float ViveController::getTriggerPullAmount()
{
	return m_fTriggerPull;
}

float ViveController::getHairTriggerThreshold()
{
	return m_fHairTriggerThreshold;
}

void ViveController::touchpadInitialTouch(float x, float y)
{
	//printf("Controller (device %u) touchpad touched at initial position (%f, %f).\n", m_unDeviceID, x, y);
	m_bTouchpadTouched = true;
	m_vec2TouchpadInitialTouchPoint = glm::vec2(x, y);
	m_vec2TouchpadCurrentTouchPoint = m_vec2TouchpadInitialTouchPoint;

}

void ViveController::touchpadTouch(float x, float y)
{
	//printf("Controller (device %u) touchpad touch tracked at (%f, %f).\n" , m_unDeviceID, x, y);
	m_vec2TouchpadCurrentTouchPoint = glm::vec2(x, y);

	if (m_vec2TouchpadInitialTouchPoint == glm::vec2(0.f, 0.f))
		m_vec2TouchpadInitialTouchPoint = m_vec2TouchpadCurrentTouchPoint;
}

void ViveController::touchpadUntouched()
{
	//printf("Controller (device %u) touchpad untouched.\n", m_unDeviceID);
	m_bTouchpadTouched = false;
	m_vec2TouchpadInitialTouchPoint = glm::vec2(0.f, 0.f);
	m_vec2TouchpadCurrentTouchPoint = glm::vec2(0.f, 0.f);
}

void ViveController::touchPadClicked(float x, float y)
{
	//printf("Controller (device %u) touchpad pressed at (%f, %f).\n", m_unDeviceID, x, y);
	m_bTouchpadClicked = true;
}

void ViveController::touchPadUnclicked(float x, float y)
{
	//printf("Controller (device %u) touchpad pressed at (%f, %f).\n", m_unDeviceID, x, y);
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

glm::vec4 ViveController::transformTouchPointToModelCoords(glm::vec2 * pt)
{
	glm::vec4 xVec = (pt->x > 0 ? c_vec4TouchPadRight : c_vec4TouchPadLeft) - c_vec4TouchPadCenter;
	glm::vec4 yVec = (pt->y > 0 ? c_vec4TouchPadTop : c_vec4TouchPadBottom) - c_vec4TouchPadCenter;

	return c_vec4TouchPadCenter + (xVec * abs(pt->x) + yVec * abs(pt->y));
}

void ViveController::insertTouchpadCursor(std::vector<float> &vertices, unsigned int &nTriangleVertices, float r, float g, float b, float a)
{
	std::vector<float> sphereVertdataarray;
	std::vector<glm::vec3> sphereVerts = m_TouchPointSphere.getUnindexedVertices();
	glm::vec4 ctr = transformTouchPointToModelCoords(&m_vec2TouchpadCurrentTouchPoint);

	//Vector3 color(.2f, .2f, .71f);
	glm::vec4 color(r, g, b, a);

	glm::mat4 & sphereMat = m_mat4DeviceToWorldTransform * glm::translate(glm::mat4(), glm::vec3(ctr.x, ctr.y, ctr.z)) * glm::scale(glm::mat4(), glm::vec3(0.0025f));

	for (size_t i = 0; i < sphereVerts.size(); ++i)
	{
		glm::vec4 thisPt = sphereMat * glm::vec4(sphereVerts[i].x, sphereVerts[i].y, sphereVerts[i].z, 1.f);

		sphereVertdataarray.push_back(thisPt.x);
		sphereVertdataarray.push_back(thisPt.y);
		sphereVertdataarray.push_back(thisPt.z);

		sphereVertdataarray.push_back(color.x);
		sphereVertdataarray.push_back(color.y);
		sphereVertdataarray.push_back(color.z);
		sphereVertdataarray.push_back(color.w);

		nTriangleVertices++;
	}

	vertices.insert(vertices.end(), sphereVertdataarray.begin(), sphereVertdataarray.end());
}
