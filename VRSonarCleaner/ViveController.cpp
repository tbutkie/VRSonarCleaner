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
	, m_bStateInitialized(false)
	, m_vec2TouchpadInitialTouchPoint(glm::vec2(0.f, 0.f))
	, m_vec2TouchpadCurrentTouchPoint(glm::vec2(0.f, 0.f))
	, m_bTriggerEngaged(false)
	, m_bTriggerClicked(false)
	, m_fTriggerPull(0.f)
	, m_fHairTriggerThreshold(0.05f)
	, m_nTriggerAxis(-1)
	, m_nTouchpadAxis(-1)
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
	switch (m_pHMD->GetTrackedDeviceClass(m_unDeviceID))
	{
	case vr::TrackedDeviceClass_Controller:		   setClassChar('C'); break;
	case vr::TrackedDeviceClass_HMD:               setClassChar('H'); break;
	case vr::TrackedDeviceClass_Invalid:           setClassChar('I'); break;
	case vr::TrackedDeviceClass_GenericTracker:    setClassChar('G'); break;
	case vr::TrackedDeviceClass_TrackingReference: setClassChar('T'); break;
	default:                                       setClassChar('?'); break;
	}

	setRenderModelName(getPropertyString(vr::Prop_RenderModelName_String));

	std::cout << "Device " << m_unDeviceID << "'s RenderModel name is " << m_strRenderModelName.c_str() << std::endl;
	
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

			std::cout << "\t" << (component->m_bHasRenderModel ? "Model -> " : "         ") << i << ": " << component->m_strComponentName.c_str();
			for (auto &b : component->m_vButtonsAssociated) std::cout << " |->(" << m_pHMD->GetButtonIdNameFromEnum(b) << ")";
			std::cout << std::endl;
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
	if (m_bStateInitialized && tempCtrllrState.unPacketNum == m_ControllerState.unPacketNum)
		return false; // no new state to process

	m_LastControllerState = m_ControllerState;
	m_ControllerState = tempCtrllrState;

	// Update the controller components
	for (auto &component : m_vpComponents)
	{
		component->m_LastState = component->m_State;
		m_pRenderModels->GetComponentState(m_strRenderModelName.c_str(), component->m_strComponentName.c_str(), &m_ControllerState, &m_ControllerScrollModeState, &component->m_State);

		// Find buttons associated with component and handle state changes/events

		if (std::find(component->m_vButtonsAssociated.begin(), component->m_vButtonsAssociated.end(), vr::k_EButton_ApplicationMenu) != component->m_vButtonsAssociated.end())
		{
			// Button pressed
			if (component->justPressed())
			{
				notify(BroadcastSystem::EVENT::VIVE_MENU_BUTTON_DOWN, this);
			}

			// Button unpressed
			if (component->justUnpressed())
			{
				notify(BroadcastSystem::EVENT::VIVE_MENU_BUTTON_UP, this);
			}
		}

		if (std::find(component->m_vButtonsAssociated.begin(), component->m_vButtonsAssociated.end(), vr::k_EButton_System) != component->m_vButtonsAssociated.end())
		{
			// Button pressed
			if (component->justPressed())
			{
				notify(BroadcastSystem::EVENT::VIVE_SYSTEM_BUTTON_DOWN, this);
			}

			// Button unpressed
			if (component->justUnpressed())
			{
				notify(BroadcastSystem::EVENT::VIVE_SYSTEM_BUTTON_UP, this);
			}
		}

		if (std::find(component->m_vButtonsAssociated.begin(), component->m_vButtonsAssociated.end(), vr::k_EButton_Grip) != component->m_vButtonsAssociated.end())
		{
			// Button pressed
			if (component->justPressed())
			{
				notify(BroadcastSystem::EVENT::VIVE_GRIP_DOWN, this);
			}

			// Button unpressed
			if (component->justUnpressed())
			{
				notify(BroadcastSystem::EVENT::VIVE_GRIP_UP, this);
			}
		}

		if (std::find(component->m_vButtonsAssociated.begin(), component->m_vButtonsAssociated.end(), vr::k_EButton_SteamVR_Trigger) != component->m_vButtonsAssociated.end())
		{
			float triggerPull = m_ControllerState.rAxis[m_nTriggerAxis].x; // trigger data on x axis

			// Trigger pressed
			if (component->justPressed())
			{
				//printf("(VR Event) Controller (device %u) trigger pressed.\n", m_unDeviceID);
			}

			// Trigger unpressed
			if (component->justUnpressed())
			{
				//printf("(VR Event) Controller (device %u) trigger unpressed.\n", m_unDeviceID);
			}

			// Trigger touched
			if (component->justTouched())
			{
				//printf("(VR Event) Controller (device %u) trigger touched.\n", m_unDeviceID);
			}

			// Trigger untouched
			if (component->justUntouched())
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

					BroadcastSystem::Payload::Trigger payload = { this, m_fTriggerPull };
					notify(BroadcastSystem::EVENT::VIVE_TRIGGER_ENGAGE , &payload);
				}

				// TRIGGER BEING PULLED
				if (!isTriggerClicked())
				{
					m_fTriggerPull = triggerPull;

					BroadcastSystem::Payload::Trigger payload = { this, m_fTriggerPull };
					notify(BroadcastSystem::EVENT::VIVE_TRIGGER_PULL, &payload);
				}

				// TRIGGER CLICKED
				if (triggerPull >= 1.f && !isTriggerClicked())
				{
					m_bTriggerClicked = true;
					m_fTriggerPull = 1.f;

					BroadcastSystem::Payload::Trigger payload = { this, m_fTriggerPull };
					notify(BroadcastSystem::EVENT::VIVE_TRIGGER_DOWN, &payload);
				}
				// TRIGGER UNCLICKED
				if (triggerPull < 1.f && isTriggerClicked())
				{
					m_bTriggerClicked = false;
					m_fTriggerPull = triggerPull;

					BroadcastSystem::Payload::Trigger payload = { this, m_fTriggerPull };
					notify(BroadcastSystem::EVENT::VIVE_TRIGGER_UP, &payload);
				}
			}
			// TRIGGER DISENGAGED
			else if (isTriggerEngaged())
			{
				m_bTriggerEngaged = false;
				m_fTriggerPull = 0.f;

				BroadcastSystem::Payload::Trigger payload = { this, m_fTriggerPull };
				notify(BroadcastSystem::EVENT::VIVE_TRIGGER_DISENGAGE, &payload);
			}
		}

		if (std::find(component->m_vButtonsAssociated.begin(), component->m_vButtonsAssociated.end(), vr::k_EButton_SteamVR_Touchpad) != component->m_vButtonsAssociated.end())
		{
			glm::vec2 touchPoint = glm::vec2(m_ControllerState.rAxis[m_nTouchpadAxis].x, m_ControllerState.rAxis[m_nTouchpadAxis].y);

			// Touchpad pressed
			if (component->justPressed())
			{
				BroadcastSystem::Payload::Touchpad payload = { this, m_vec2TouchpadInitialTouchPoint, touchPoint };
				notify(BroadcastSystem::EVENT::VIVE_TOUCHPAD_DOWN, &payload);
			}

			// Touchpad unpressed
			if (component->justUnpressed())
			{
				BroadcastSystem::Payload::Touchpad payload = { this, m_vec2TouchpadInitialTouchPoint, touchPoint };
				notify(BroadcastSystem::EVENT::VIVE_TOUCHPAD_UP, &payload);
			}

			// Touchpad touched
			if (component->justTouched())
			{
				m_vec2TouchpadInitialTouchPoint = touchPoint;
				m_vec2TouchpadCurrentTouchPoint = m_vec2TouchpadInitialTouchPoint;

				BroadcastSystem::Payload::Touchpad payload = { this, m_vec2TouchpadInitialTouchPoint, touchPoint };
				notify(BroadcastSystem::EVENT::VIVE_TOUCHPAD_ENGAGE, &payload);
			}

			// Touchpad untouched
			if (component->justUntouched())
			{
				BroadcastSystem::Payload::Touchpad payload = { this, m_vec2TouchpadInitialTouchPoint, touchPoint };
				notify(BroadcastSystem::EVENT::VIVE_TOUCHPAD_DISENGAGE, &payload);

				m_vec2TouchpadInitialTouchPoint = glm::vec2(0.f, 0.f);
				m_vec2TouchpadCurrentTouchPoint = glm::vec2(0.f, 0.f);
			}

			// Touchpad being touched
			if (component->continueTouch())
			{
				m_vec2TouchpadCurrentTouchPoint = touchPoint;

				if (m_vec2TouchpadInitialTouchPoint == glm::vec2(0.f, 0.f))
					m_vec2TouchpadInitialTouchPoint = m_vec2TouchpadCurrentTouchPoint;

				BroadcastSystem::Payload::Touchpad payload = { this, m_vec2TouchpadInitialTouchPoint, touchPoint };
				notify(BroadcastSystem::EVENT::VIVE_TOUCHPAD_TOUCH, &payload);
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

// In model coordinates
glm::vec3 ViveController::getCurrentTouchpadTouchPoint()
{
	return glm::vec3(transformTouchPointToModelCoords(&m_vec2TouchpadCurrentTouchPoint));
}

// In model coordinates
glm::vec3 ViveController::getInitialTouchpadTouchPoint()
{
	return glm::vec3(transformTouchPointToModelCoords(&m_vec2TouchpadInitialTouchPoint));
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

bool ViveController::readyToRender()
{
	return m_Pose.bDeviceIsConnected &&
		m_Pose.bPoseIsValid &&
		m_Pose.eTrackingResult == vr::ETrackingResult::TrackingResult_Running_OK;
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
