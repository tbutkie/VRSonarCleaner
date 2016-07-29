#include "TrackedDeviceManager.h"

TrackedDeviceManager::TrackedDeviceManager(vr::IVRSystem* pHMD) : m_pHMD(pHMD)
{
}


TrackedDeviceManager::~TrackedDeviceManager()
{
}

void TrackedDeviceManager::Init()
{
	SetupRenderModels();
}

void TrackedDeviceManager::handleEvents()
{
	// Process SteamVR events
	vr::VREvent_t event;
	while (m_pHMD->PollNextEvent(&event, sizeof(event)))
	{
		ProcessVREvent(event);
	}
	
	updateControllerStates();
}
//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------
void TrackedDeviceManager::ProcessVREvent(const vr::VREvent_t & event)
{
	if (m_pHMD->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
	{
		processControllerEvent(event);
		return;
	}

	switch (event.eventType)
	{
	case vr::VREvent_TrackedDeviceActivated:
	{
		SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
		printf("Device %u attached. Setting up render model.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceDeactivated:
	{
		printf("Device %u detached.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceUpdated:
	{
		printf("Device %u updated.\n", event.trackedDeviceIndex);
	}
	break;
	default:
		;//dprintf("Device %u uncaught event %u.\n", event.trackedDeviceIndex, event.eventType);
	}
}

void TrackedDeviceManager::processControllerEvent(const vr::VREvent_t & event)
{
	vr::VRControllerState_t state;
	if (!m_pHMD->GetControllerState(event.trackedDeviceIndex, &state))
		return;

	switch (event.eventType)
	{
	case vr::VREvent_TrackedDeviceActivated:
	{
		SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
		printf("Controller (device %u) attached. Setting up render model.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceDeactivated:
	{
		printf("Controller (device %u) detached.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceUpdated:
	{
		printf("Controller (device %u) updated.\n", event.trackedDeviceIndex);
	}
	break;
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
			m_rpTrackedDevices[event.trackedDeviceIndex]->toggleAxes();
			m_pHMD->TriggerHapticPulse(event.trackedDeviceIndex, 0, 2000);
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
			m_rpTrackedDevices[event.trackedDeviceIndex]->touchpadInitialTouch(state.rAxis[0].x, state.rAxis[0].y);
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
			m_rpTrackedDevices[event.trackedDeviceIndex]->touchpadUntouched();
		}
	}
	break;
	default:
		printf("Controller (device %u) uncaught event %u.\n", event.trackedDeviceIndex, event.eventType);
	}
}

void TrackedDeviceManager::updateControllerStates()
{
	// If controller is engaged in interaction, update interaction vars
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		if (m_pHMD->GetTrackedDeviceClass(unDevice) != vr::TrackedDeviceClass_Controller)
			continue;

		vr::VRControllerState_t state;
		if (!m_pHMD->GetControllerState(unDevice, &state)) continue;

		m_rpTrackedDevices[unDevice]->updateState(&state);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
void TrackedDeviceManager::SetupRenderModels()
{
	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i)	
		m_rpTrackedDevices[i]->setRenderModel(0);

	if (!m_pHMD)
		return;

	for (uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_pHMD->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		SetupRenderModelForTrackedDevice(unTrackedDevice);
	}

}

//-----------------------------------------------------------------------------
// Purpose: Finds a render model we've already loaded or loads a new one
//-----------------------------------------------------------------------------
CGLRenderModel* TrackedDeviceManager::FindOrLoadRenderModel(const char *pchRenderModelName)
{
	CGLRenderModel *pRenderModel = NULL;
	for (std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++)
	{
		if (!stricmp((*i)->GetName().c_str(), pchRenderModelName))
		{
			pRenderModel = *i;
			break;
		}
	}

	// load the model if we didn't find one
	if (!pRenderModel)
	{
		vr::RenderModel_t *pModel;
		vr::EVRRenderModelError error;
		while (1)
		{
			error = vr::VRRenderModels()->LoadRenderModel_Async(pchRenderModelName, &pModel);
			if (error != vr::VRRenderModelError_Loading)
				break;

			::Sleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			printf("Unable to load render model %s - %s\n", pchRenderModelName, vr::VRRenderModels()->GetRenderModelErrorNameFromEnum(error));
			return NULL; // move on to the next tracked device
		}

		vr::RenderModel_TextureMap_t *pTexture;
		while (1)
		{
			error = vr::VRRenderModels()->LoadTexture_Async(pModel->diffuseTextureId, &pTexture);
			if (error != vr::VRRenderModelError_Loading)
				break;

			::Sleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			printf("Unable to load render texture id:%d for render model %s\n", pModel->diffuseTextureId, pchRenderModelName);
			vr::VRRenderModels()->FreeRenderModel(pModel);
			return NULL; // move on to the next tracked device
		}

		pRenderModel = new CGLRenderModel(pchRenderModelName);
		if (!pRenderModel->BInit(*pModel, *pTexture))
		{
			printf("Unable to create GL model from render model %s\n", pchRenderModelName);
			delete pRenderModel;
			pRenderModel = NULL;
		}
		else
		{
			m_vecRenderModels.push_back(pRenderModel);
		}
		vr::VRRenderModels()->FreeRenderModel(pModel);
		vr::VRRenderModels()->FreeTexture(pTexture);
	}
	return pRenderModel;
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL a Render Model for a single tracked device
//-----------------------------------------------------------------------------
void TrackedDeviceManager::SetupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex)
{
	if (unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount)
		return;

	// try to find a model we've already set up
	std::string sRenderModelName = GetTrackedDeviceString(unTrackedDeviceIndex, vr::Prop_RenderModelName_String);

	CGLRenderModel *pRenderModel = FindOrLoadRenderModel(sRenderModelName.c_str());

	if (!pRenderModel)
	{
		std::string sTrackingSystemName = GetTrackedDeviceString(unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String);
		printf("Unable to load render model for tracked device %d (%s.%s)", unTrackedDeviceIndex, sTrackingSystemName.c_str(), sRenderModelName.c_str());
	}
	else
	{
		m_rpTrackedDevices[unTrackedDeviceIndex]->setRenderModel(pRenderModel);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
std::string TrackedDeviceManager::GetTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
{
	uint32_t unRequiredBufferLen = m_pHMD->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = m_pHMD->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}