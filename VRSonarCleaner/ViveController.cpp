#include "ViveController.h"

#include <iostream>

#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>

const glm::vec4 ViveController::c_vec4TouchPadCenter(glm::vec4(0.f, 0.00378f, 0.04920f, 1.f));
const glm::vec4 ViveController::c_vec4TouchPadLeft(glm::vec4(-0.02023f, 0.00495f, 0.04934f, 1.f));
const glm::vec4 ViveController::c_vec4TouchPadRight(glm::vec4(0.02023f, 0.00495f, 0.04934f, 1.f));
const glm::vec4 ViveController::c_vec4TouchPadTop(glm::vec4(0.f, 0.00725f, 0.02924f, 1.f));
const glm::vec4 ViveController::c_vec4TouchPadBottom(glm::vec4(0.f, 0.00265f, 0.06943f, 1.f));

//const glm::vec4 ViveController::c_vec4HoleCenter(glm::vec4(-0.00032f, 0.02732f, -0.02553f, 1.f));
//const glm::vec4 ViveController::c_vec4HoleNormal(glm::vec4(0.6948f, 0.3879f, 0.6056f, 0.f));

const glm::vec4 ViveController::c_vec4HoleCenter(glm::vec4(0.f, -0.02591f, -0.02755f, 1.f));
const glm::vec4 ViveController::c_vec4HoleNormal(glm::vec4(glm::normalize(glm::cross(glm::vec3(0.f, 0.00227, -0.00985f) - glm::vec3(c_vec4HoleCenter), glm::vec3(0.03320f, -0.02774f, -0.02872f) - glm::vec3(c_vec4HoleCenter))), 0.f));

ViveController::ViveController(vr::TrackedDeviceIndex_t unTrackedDeviceIndex, vr::IVRSystem *pHMD, vr::IVRRenderModels *pRenderModels)
	: TrackedDevice(unTrackedDeviceIndex, pHMD, pRenderModels)
	, m_bStateInitialized(false)
	, m_nTriggerAxis(-1)
	, m_nTouchpadAxis(-1)
	, m_fHairTriggerThreshold(0.05f)
{
	m_ControllerScrollModeState.bScrollWheelVisible = false;
}

ViveController::~ViveController()
{
	for (auto &c : m_vpComponents)
		delete c;
	m_vpComponents.clear();
}

bool ViveController::BInit()
{
	TrackedDevice::BInit();

	m_pHMD->GetControllerState(
		m_unDeviceID,
		&m_ControllerState,
		sizeof(m_ControllerState)
	);

	m_LastControllerState = m_ControllerState;

	// Check if there are model components
	uint32_t nModelComponents = m_pRenderModels->GetComponentCount(m_strRenderModelName.c_str());

	// Loop over model components and add them to the tracked device
	for (uint32_t i = 0; i < nModelComponents; ++i)
	{
		TrackedDevice::TrackedDeviceComponent *component = new TrackedDevice::TrackedDeviceComponent();
		component->m_unComponentIndex = i;

		uint32_t unRequiredBufferLen = m_pRenderModels->GetComponentName(m_strRenderModelName.c_str(), i, NULL, 0);
		if (unRequiredBufferLen == 0)
		{
			printf("Controller [device %d] component %d index out of range.\n", m_unDeviceID, i);
		}
		else
		{
			char *pchBuffer1 = new char[unRequiredBufferLen];
			unRequiredBufferLen = m_pRenderModels->GetComponentName(m_strRenderModelName.c_str(), i, pchBuffer1, unRequiredBufferLen);
			component->m_strComponentName = pchBuffer1;
			delete[] pchBuffer1;

			unRequiredBufferLen = m_pRenderModels->GetComponentRenderModelName(m_strRenderModelName.c_str(), component->m_strComponentName.c_str(), NULL, 0);
			if (unRequiredBufferLen == 0)
				component->m_bHasRenderModel = false;
			else
			{
				char *pchBuffer2 = new char[unRequiredBufferLen];
				unRequiredBufferLen = m_pRenderModels->GetComponentRenderModelName(m_strRenderModelName.c_str(), component->m_strComponentName.c_str(), pchBuffer2, unRequiredBufferLen);
				component->m_strComponentRenderModelName = pchBuffer2;

				delete[] pchBuffer2;

				component->m_bHasRenderModel = true;

				vr::RenderModel_ControllerMode_State_t controllerModeState;
				m_pRenderModels->GetComponentState(
					m_strRenderModelName.c_str(),
					component->m_strComponentRenderModelName.c_str(),
					&m_ControllerState,
					&controllerModeState,
					&(component->m_State)
				);

				component->m_LastState = component->m_State;
			}

			uint64_t buttonMask = m_pRenderModels->GetComponentButtonMask(m_strRenderModelName.c_str(), component->m_strComponentName.c_str());

			for (int i = 0; i != vr::EVRButtonId::k_EButton_Max; ++i)
			{
				if (vr::ButtonMaskFromId((vr::EVRButtonId)i) & buttonMask)
				{
					m_mapButtonToComponentMap[(vr::EVRButtonId)i].push_back(component);
					component->m_vButtonsAssociated.push_back((vr::EVRButtonId)i);
				}
			}

			component->m_bInitialized = true;

			m_vpComponents.push_back(component);

			{
				std::cout << "\t" << (component->m_bHasRenderModel ? "Model -> " : "         ") << i << ": " << component->m_strComponentName.c_str();
				for (auto &b : component->m_vButtonsAssociated) std::cout << " |->(" << m_pHMD->GetButtonIdNameFromEnum(b) << ")";
				std::cout << std::endl;
			}
		}
	}

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
	m_mat4LastDeviceToWorldTransform = ConvertSteamVRMatrixToMatrix4(m_LastPose.mDeviceToAbsoluteTracking);
	
	updateControllerState();

	return m_Pose.bPoseIsValid;
}

bool ViveController::updateControllerState()
{
	vr::VRControllerState_t tempCtrllrState;
	if (!m_pHMD->GetControllerState(m_unDeviceID, &tempCtrllrState, sizeof(tempCtrllrState)))
		return false; // bad controller index
	
	// check if any state has changed
	//if (m_bStateInitialized && tempCtrllrState.unPacketNum == m_ControllerState.unPacketNum)
	//	return false; // no new state to process

	m_LastControllerState = m_ControllerState;
	m_ControllerState = tempCtrllrState;

	m_LastCustomState = m_CustomState;

	// Update the controller components
	for (auto &component : m_vpComponents)
	{
		component->m_LastState = component->m_State;
		m_pRenderModels->GetComponentState(m_strRenderModelName.c_str(), component->m_strComponentName.c_str(), &m_ControllerState, &m_ControllerScrollModeState, &component->m_State);

		// Find buttons associated with component and handle state changes/events

		// HACK: Probe for Index Controller
		if (m_strRenderModelName.compare(0, 17, "{indexcontroller}") == 0)
		{
			if (std::find(component->m_vButtonsAssociated.begin(), component->m_vButtonsAssociated.end(), vr::k_EButton_SteamVR_Trigger) != component->m_vButtonsAssociated.end())
			{
				if (component->justPressed())
				{
					m_CustomState.m_bTriggerEngaged = true;
					m_CustomState.m_bTriggerClicked = true;
					m_CustomState.m_fTriggerPull = 1.f;
				}
				else if (component->justUnpressed()) // TRIGGER DISENGAGED
				{
					m_CustomState.m_bTriggerEngaged = false;
					m_CustomState.m_bTriggerClicked = false;
					m_CustomState.m_fTriggerPull = 0.f;
				}
			}
		}
		else
		{
			float triggerPull = m_ControllerState.rAxis[m_nTriggerAxis].x; // trigger data on x axis

			if (triggerPull >= getHairTriggerThreshold())
			{
				m_CustomState.m_fTriggerPull = triggerPull;

				m_CustomState.m_bTriggerEngaged = true;

				m_CustomState.m_bTriggerClicked = triggerPull >= 1.f;
			}
			// TRIGGER DISENGAGED
			else
			{
				m_CustomState.m_bTriggerEngaged = false;
				m_CustomState.m_bTriggerClicked = false;
				m_CustomState.m_fTriggerPull = 0.f;
			}
		}

		if (std::find(component->m_vButtonsAssociated.begin(), component->m_vButtonsAssociated.end(), vr::k_EButton_SteamVR_Touchpad) != component->m_vButtonsAssociated.end())
		{
			glm::vec2 touchPoint = glm::vec2(m_ControllerState.rAxis[m_nTouchpadAxis].x, m_ControllerState.rAxis[m_nTouchpadAxis].y);

			// Touchpad touched
			if (component->justTouched())
			{
				m_CustomState.m_vec2TouchpadInitialTouchPoint = touchPoint;
				m_CustomState.m_vec2TouchpadCurrentTouchPoint = m_CustomState.m_vec2TouchpadInitialTouchPoint;
			}

			// Touchpad untouched
			if (component->justUntouched())
			{
				m_CustomState.m_vec2TouchpadInitialTouchPoint = glm::vec2(0.f, 0.f);
				m_CustomState.m_vec2TouchpadCurrentTouchPoint = glm::vec2(0.f, 0.f);
			}

			// Touchpad being touched
			if (component->continueTouch())
			{
				m_CustomState.m_vec2TouchpadCurrentTouchPoint = touchPoint;

				if (m_CustomState.m_vec2TouchpadInitialTouchPoint == glm::vec2(0.f, 0.f))
					m_CustomState.m_vec2TouchpadInitialTouchPoint = m_CustomState.m_vec2TouchpadCurrentTouchPoint;
			}
		}
	}

	if (!m_bStateInitialized)
		m_bStateInitialized = true;

	return true;
}

bool ViveController::isSystemButtonPressed()
{
	for (auto const &c : m_mapButtonToComponentMap[vr::EVRButtonId::k_EButton_System])
		if (!c->isPressed())
			return false;

	return true;
}

bool ViveController::isMenuButtonPressed()
{
	for (auto const &c : m_mapButtonToComponentMap[vr::EVRButtonId::k_EButton_ApplicationMenu])
		if (!c->isPressed())
			return false;

	return true;
}

bool ViveController::isGripButtonPressed()
{
	for (auto const &c : m_mapButtonToComponentMap[vr::EVRButtonId::k_EButton_Grip])
		if (!c->isPressed())
			return false;

	return true;
}

bool ViveController::isTriggerEngaged()
{
	return m_CustomState.m_bTriggerEngaged;
}

bool ViveController::isTriggerClicked()
{
	return m_CustomState.m_bTriggerClicked;
}

float ViveController::getTriggerPullAmount()
{
	return m_CustomState.m_fTriggerPull;
}

float ViveController::getHairTriggerThreshold()
{
	return m_fHairTriggerThreshold;
}

glm::vec2 ViveController::getCurrentTouchpadTouchPoint()
{
	return m_CustomState.m_vec2TouchpadCurrentTouchPoint;
}

glm::vec2 ViveController::getInitialTouchpadTouchPoint()
{
	return m_CustomState.m_vec2TouchpadInitialTouchPoint;
}

// In model coordinates
glm::vec3 ViveController::getCurrentTouchpadTouchPointModelCoords()
{
	return glm::vec3(transformTouchPointToModelCoords(&m_CustomState.m_vec2TouchpadCurrentTouchPoint));
}

// In model coordinates
glm::vec3 ViveController::getInitialTouchpadTouchPointModelCoords()
{
	return glm::vec3(transformTouchPointToModelCoords(&m_CustomState.m_vec2TouchpadInitialTouchPoint));
}

void ViveController::setScrollWheelVisibility(bool visible)
{
	m_ControllerScrollModeState.bScrollWheelVisible = visible;

	for (auto &component : m_vpComponents)
	{
		if (component->m_strComponentName.compare("scroll_wheel") == 0 ||
			component->m_strComponentName.compare("touchpad") == 0 ||
			component->m_strComponentName.compare("touchpad_scroll_cut") == 0)
		{
			component->m_LastState = component->m_State;
			m_pRenderModels->GetComponentState(m_strRenderModelName.c_str(), component->m_strComponentName.c_str(), &m_ControllerState, &m_ControllerScrollModeState, &component->m_State);
		}
	}
}

glm::vec3 ViveController::getTriggerPoint()
{
	return (m_mat4DeviceToWorldTransform * glm::translate(glm::mat4(), glm::vec3(0.f, -0.031f, 0.05f)))[3];
}

glm::vec3 ViveController::getLeftGripPoint()
{
	return (m_mat4DeviceToWorldTransform * glm::translate(glm::mat4(), glm::vec3(-0.0225f, -0.015f, 0.085f)))[3];
}

glm::vec3 ViveController::getRightGripPoint()
{
	return (m_mat4DeviceToWorldTransform * glm::translate(glm::mat4(), glm::vec3(0.0225f, -0.015f, 0.085f)))[3];
}

bool ViveController::isTouchpadTouched()
{
	for (auto const &c : m_mapButtonToComponentMap[vr::EVRButtonId::k_EButton_SteamVR_Touchpad])
		if (!c->isTouched())
			return false;

	return true;
}

bool ViveController::isTouchpadClicked()
{
	for (auto const &c : m_mapButtonToComponentMap[vr::EVRButtonId::k_EButton_SteamVR_Touchpad])
		if (!c->isPressed())
			return false;

	return true;
}

bool ViveController::justClickedTrigger()
{
	return m_CustomState.m_bTriggerClicked && !m_LastCustomState.m_bTriggerClicked;
}

bool ViveController::justUnclickedTrigger()
{
	return !m_CustomState.m_bTriggerClicked && m_LastCustomState.m_bTriggerClicked;
}

bool ViveController::justPressedGrip()
{
	for (auto const &c : m_mapButtonToComponentMap[vr::EVRButtonId::k_EButton_Grip])
		if (!c->justPressed())
			return false;

	return true;
}

bool ViveController::justUnpressedGrip()
{
	for (auto const &c : m_mapButtonToComponentMap[vr::EVRButtonId::k_EButton_Grip])
		if (!c->justUnpressed())
			return false;

	return true;
}

bool ViveController::justTouchedTouchpad()
{
	for (auto const &c : m_mapButtonToComponentMap[vr::EVRButtonId::k_EButton_SteamVR_Touchpad])
		if (!c->justTouched())
			return false;

	return true;
}

bool ViveController::justUntouchedTouchpad()
{
	for (auto const &c : m_mapButtonToComponentMap[vr::EVRButtonId::k_EButton_SteamVR_Touchpad])
		if (!c->justUntouched())
			return false;

	return true;
}

bool ViveController::justPressedTouchpad()
{
	for (auto const &c : m_mapButtonToComponentMap[vr::EVRButtonId::k_EButton_SteamVR_Touchpad])
		if (!c->justPressed())
			return false;

	return true;
}

bool ViveController::justUnpressedTouchpad()
{
	for (auto const &c : m_mapButtonToComponentMap[vr::EVRButtonId::k_EButton_SteamVR_Touchpad])
		if (!c->justUnpressed())
			return false;

	return true;
}

bool ViveController::justPressedMenu()
{
	for (auto const &c : m_mapButtonToComponentMap[vr::EVRButtonId::k_EButton_ApplicationMenu])
		if (!c->justPressed())
			return false;

	return true;
}

bool ViveController::justUnpressedMenu()
{
	for (auto const &c : m_mapButtonToComponentMap[vr::EVRButtonId::k_EButton_ApplicationMenu])
		if (!c->justUnpressed())
			return false;

	return true;
}



glm::mat4 ViveController::getLastDeviceToWorldTransform()
{
	return m_mat4LastDeviceToWorldTransform;
}

glm::vec4 ViveController::transformTouchPointToModelCoords(glm::vec2 * pt)
{
	glm::vec4 xVec = (pt->x > 0 ? c_vec4TouchPadRight : c_vec4TouchPadLeft) - c_vec4TouchPadCenter;
	glm::vec4 yVec = (pt->y > 0 ? c_vec4TouchPadTop : c_vec4TouchPadBottom) - c_vec4TouchPadCenter;

	return c_vec4TouchPadCenter + (xVec * abs(pt->x) + yVec * abs(pt->y));
}
