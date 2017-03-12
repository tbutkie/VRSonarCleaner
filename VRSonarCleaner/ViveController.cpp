#include "ViveController.h"

#include <iostream>

#include <shared/glm/gtc/type_ptr.hpp>
#include <shared/glm/gtc/matrix_transform.hpp>


const glm::vec4 ViveController::c_vec4TouchPadCenter(glm::vec4(0.f, 0.00378f, 0.04920f, 1.f));
const glm::vec4 ViveController::c_vec4TouchPadLeft(glm::vec4(-0.02023f, 0.00495f, 0.04934f, 1.f));
const glm::vec4 ViveController::c_vec4TouchPadRight(glm::vec4(0.02023f, 0.00495f, 0.04934f, 1.f));
const glm::vec4 ViveController::c_vec4TouchPadTop(glm::vec4(0.f, 0.00725f, 0.02924f, 1.f));
const glm::vec4 ViveController::c_vec4TouchPadBottom(glm::vec4(0.f, 0.00265f, 0.06943f, 1.f));

ViveController::ViveController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex, vr::IVRSystem *pHMD, vr::IVRRenderModels *pRenderModels)
	: TrackedDevice(unTrackedDeviceIndex, pHMD, pRenderModels)
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
	, m_nTriggerAxis(-1)
	, m_nTouchpadAxis(-1)
	, m_TouchPointSphere(Icosphere(2))
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
		vr::ETrackedDeviceProperty prop = static_cast<vr::TrackedDeviceProperty>(vr::TrackedDeviceProperty::Prop_Axis0Type_Int32 + i);
		vr::EVRControllerAxisType axisType = static_cast<vr::EVRControllerAxisType>(getPropertyInt32(prop));
		if (axisType == vr::k_eControllerAxis_Trigger) m_nTriggerAxis = i;
		if (axisType == vr::k_eControllerAxis_TrackPad) m_nTouchpadAxis = i;
	}

	// Check if we were able to figure 'em out
	if (m_nTriggerAxis < 0)
		printf("Unable to find proper axis for controller trigger.\n");
	
	if (m_nTouchpadAxis < 0)
		printf("Unable to find proper axes for controller touchpad.\n");


	// Create shaders used for our own custom geometry
	createShaders();
	
	// Initialize controller state
	m_pHMD->GetControllerState(m_unDeviceID, &m_ControllerState, sizeof(m_ControllerState));
	m_LastControllerState = m_ControllerState;

	return true;
}

glm::mat4 ViveController::getLastPose()
{
	return ConvertSteamVRMatrixToMatrix4(m_LastPose.mDeviceToAbsoluteTracking);
}

bool ViveController::update(vr::TrackedDevicePose_t pose)
{
	m_LastPose = m_Pose;
	m_Pose = pose;
	m_mat4DeviceToWorldTransform = ConvertSteamVRMatrixToMatrix4(m_Pose.mDeviceToAbsoluteTracking);
	
	updateControllerState();

	prepareForRendering();

	return m_Pose.bPoseIsValid;
}

bool ViveController::updateControllerState()
{
	vr::VRControllerState_t tempCtrllrState;
	if (!m_pHMD->GetControllerState(m_unDeviceID, &tempCtrllrState, sizeof(tempCtrllrState)))
		return false; // bad controller index

	// check if any state has changed
	if (tempCtrllrState.unPacketNum > 0 && tempCtrllrState.unPacketNum == m_ControllerState.unPacketNum)
		return false; // no new state to process

	m_LastControllerState = m_ControllerState;
	m_ControllerState = tempCtrllrState;

	// Set scrollwheel mode if necessary
	vr::RenderModel_ControllerMode_State_t controllerModeState;
	controllerModeState.bScrollWheelVisible = m_bShowScrollWheel;
	bool bScrollWheelBefore = m_bShowScrollWheel;

	// Update the controller components
	for (auto &component : m_vComponents)
	{
		component.m_LastState = component.m_State;
		m_pRenderModels->GetComponentState(m_strRenderModelName.c_str(), component.m_strComponentName.c_str(), &m_ControllerState, &controllerModeState, &component.m_State);

		uint64_t buttonMask = m_pRenderModels->GetComponentButtonMask(m_strRenderModelName.c_str(), component.m_strComponentName.c_str());

		// Find buttons associated with component and handle state changes/events

		if (vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu) & buttonMask)
		{
			// Button pressed
			if (component.justPressed())
			{
				m_bMenuButtonClicked = true;
				notify(this, BroadcastSystem::EVENT::VIVE_MENU_BUTTON_DOWN, NULL);
			}

			// Button unpressed
			if (component.justUnpressed())
			{
				m_bMenuButtonClicked = false;
				notify(this, BroadcastSystem::EVENT::VIVE_MENU_BUTTON_UP, NULL);
			}
		}

		if (vr::ButtonMaskFromId(vr::k_EButton_System) & buttonMask)
		{
			// Button pressed
			if (component.justPressed())
			{
				m_bSystemButtonClicked = true;
				notify(this, BroadcastSystem::EVENT::VIVE_SYSTEM_BUTTON_DOWN, NULL);
			}

			// Button unpressed
			if (component.justUnpressed())
			{
				m_bSystemButtonClicked = false;
				notify(this, BroadcastSystem::EVENT::VIVE_SYSTEM_BUTTON_UP, NULL);
			}
		}

		if (vr::ButtonMaskFromId(vr::k_EButton_Grip) & buttonMask)
		{
			// Button pressed
			if (component.justPressed())
			{
				m_bGripButtonClicked = true;
				notify(this, BroadcastSystem::EVENT::VIVE_GRIP_DOWN, NULL);
			}

			// Button unpressed
			if (component.justUnpressed())
			{
				m_bGripButtonClicked = false;
				notify(this, BroadcastSystem::EVENT::VIVE_GRIP_UP, NULL);
			}
		}

		if (vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger) & buttonMask)
		{
			float triggerPull = m_ControllerState.rAxis[m_nTriggerAxis].x; // trigger data on x axis

			// Trigger pressed
			if (component.justPressed())
			{
				//printf("(VR Event) Controller (device %u) trigger pressed.\n", m_unDeviceID);
			}

			// Trigger unpressed
			if (component.justUnpressed())
			{
				//printf("(VR Event) Controller (device %u) trigger unpressed.\n", m_unDeviceID);
			}

			// Trigger touched
			if (component.justTouched())
			{
				//printf("(VR Event) Controller (device %u) trigger touched.\n", m_unDeviceID);
			}

			// Trigger untouched
			if (component.justUntouched())
			{
				//printf("(VR Event) Controller (device %u) trigger untouched.\n", m_unDeviceID);
			}

			// TRIGGER INTERACTIONS
			if (triggerPull >= getHairTriggerThreshold())
			{
				// TRIGGER ENGAGED
				if (!isTriggerEngaged())
				{
					m_bTriggerEngaged = true;
					m_fTriggerPull = triggerPull;

					notify(this, BroadcastSystem::EVENT::VIVE_TRIGGER_ENGAGE , &m_fTriggerPull);
				}

				// TRIGGER BEING PULLED
				if (!isTriggerClicked())
				{
					m_fTriggerPull = triggerPull;
					notify(this, BroadcastSystem::EVENT::VIVE_TRIGGER_PULL, &m_fTriggerPull);
				}

				// TRIGGER CLICKED
				if (triggerPull >= 1.f && !isTriggerClicked())
				{
					m_bTriggerClicked = true;
					m_fTriggerPull = 1.f;
					notify(this, BroadcastSystem::EVENT::VIVE_TRIGGER_DOWN, NULL);
				}
				// TRIGGER UNCLICKED
				if (triggerPull < 1.f && isTriggerClicked())
				{
					m_bTriggerClicked = false;
					m_fTriggerPull = triggerPull;
					notify(this, BroadcastSystem::EVENT::VIVE_TRIGGER_UP, &m_fTriggerPull);
				}
			}
			// TRIGGER DISENGAGED
			else if (isTriggerEngaged())
			{
				m_bTriggerEngaged = false;
				m_fTriggerPull = 0.f;
				notify(this, BroadcastSystem::EVENT::VIVE_TRIGGER_DISENGAGE, NULL);
			}
		}

		if (vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad) & buttonMask)
		{
			glm::vec2 touchPoint = glm::vec2(m_ControllerState.rAxis[m_nTouchpadAxis].x, m_ControllerState.rAxis[m_nTouchpadAxis].y);

			// Touchpad pressed
			if (component.justPressed())
			{
				m_bTouchpadClicked = true;
				glm::vec2 payload[2] = { m_vec2TouchpadInitialTouchPoint, touchPoint };
				notify(this, BroadcastSystem::EVENT::VIVE_TOUCHPAD_DOWN, &payload);
			}

			// Touchpad unpressed
			if (component.justUnpressed())
			{
				m_bTouchpadClicked = false;

				glm::vec2 payload[2] = { m_vec2TouchpadInitialTouchPoint, touchPoint };
				notify(this, BroadcastSystem::EVENT::VIVE_TOUCHPAD_UP, &payload);
			}

			// Touchpad touched
			if (component.justTouched())
			{
				m_bTouchpadTouched = true;
				m_vec2TouchpadInitialTouchPoint = touchPoint;
				m_vec2TouchpadCurrentTouchPoint = m_vec2TouchpadInitialTouchPoint;
				notify(this, BroadcastSystem::EVENT::VIVE_TOUCHPAD_ENGAGE, &touchPoint);
			}

			// Touchpad untouched
			if (component.justUntouched())
			{
				m_bTouchpadTouched = false;

				glm::vec2 payload[2] = { m_vec2TouchpadInitialTouchPoint, touchPoint };
				notify(this, BroadcastSystem::EVENT::VIVE_TOUCHPAD_DISENGAGE, &payload);

				m_vec2TouchpadInitialTouchPoint = glm::vec2(0.f, 0.f);
				m_vec2TouchpadCurrentTouchPoint = glm::vec2(0.f, 0.f);
			}

			// Touchpad being touched
			if (component.continueTouch())
			{
				m_vec2TouchpadCurrentTouchPoint = touchPoint;

				if (m_vec2TouchpadInitialTouchPoint == glm::vec2(0.f, 0.f))
					m_vec2TouchpadInitialTouchPoint = m_vec2TouchpadCurrentTouchPoint;

				glm::vec2 payload[2] = { m_vec2TouchpadInitialTouchPoint, m_vec2TouchpadCurrentTouchPoint };
				notify(this, BroadcastSystem::EVENT::VIVE_TOUCHPAD_TOUCH, &payload);
			}
		}
	}

	// see if scrollwheel model visibility changed, and update if necessary
	if (bScrollWheelBefore != m_bShowScrollWheel)
	{
		controllerModeState.bScrollWheelVisible = m_bShowScrollWheel;

		for (std::vector<TrackedDeviceComponent>::iterator it = m_vComponents.begin(); it != m_vComponents.end(); ++it)
		{
			m_pRenderModels->GetComponentState(m_strRenderModelName.c_str(), it->m_strComponentName.c_str(), &m_ControllerState, &controllerModeState, &it->m_State);
		}
	}

	return true;
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

bool ViveController::isSystemButtonPressed()
{
	return m_bSystemButtonClicked;
}

bool ViveController::isMenuButtonPressed()
{
	return m_bMenuButtonClicked;
}

bool ViveController::isGripButtonPressed()
{
	return m_bGripButtonClicked;
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
