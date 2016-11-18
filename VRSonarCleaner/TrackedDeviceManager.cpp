#include "TrackedDeviceManager.h"
#include "ShaderUtils.h"
#include "InfoBoxManager.h"

TrackedDeviceManager::TrackedDeviceManager(vr::IVRSystem* pHMD)
: m_pHMD(pHMD)
, m_pRenderModels(NULL)
, m_pEditController(NULL)
, m_pManipController(NULL)
, m_iTrackedControllerCount(0)
, m_iTrackedControllerCount_Last(-1)
, m_iValidPoseCount(0)
, m_iValidPoseCount_Last(-1)
, m_strPoseClasses("")
{
}


TrackedDeviceManager::~TrackedDeviceManager()
{	
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
		delete m_rpTrackedDevices[nDevice];
}

bool TrackedDeviceManager::BInit()
{
	vr::EVRInitError eError = vr::VRInitError_None;

	m_pRenderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);

	if (!m_pRenderModels)
	{
		m_pHMD = NULL;
		vr::VR_Shutdown();

		char buf[1024];
		sprintf_s(buf, sizeof(buf), "Unable to get render model interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		return false;
	}

	initDevices();

	return true;
}

void TrackedDeviceManager::handleEvents()
{
	// Process SteamVR events
	vr::VREvent_t event;
	while (m_pHMD->PollNextEvent(&event, sizeof(event)))
	{
		processVREvent(event);
	}
	
	updateControllerStates();
	// don't draw controllers if somebody else has input focus
	if (!m_pHMD->IsInputFocusCapturedByAnotherProcess())
	{
		if (m_pEditController && m_pHMD->IsTrackedDeviceConnected(m_pEditController->getIndex()))
			m_pEditController->prepareForRendering();

		if (m_pManipController && m_pHMD->IsTrackedDeviceConnected(m_pManipController->getIndex()))
			m_pManipController->prepareForRendering();
	}
}
//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event. Controller events are ignored; instead,
//          the controller states are updated (if they've changed) every frame
//-----------------------------------------------------------------------------
void TrackedDeviceManager::processVREvent(const vr::VREvent_t & event)
{
	if (event.eventType == vr::VREvent_TrackedDeviceActivated)
	{
		setupTrackedDevice(event.trackedDeviceIndex);
		printf("Device %u attached. Setting up.\n", event.trackedDeviceIndex);
	}
	else if (event.eventType == vr::VREvent_TrackedDeviceDeactivated)
	{
		printf("Device %u detached.\n", event.trackedDeviceIndex);
	}
	else if (event.eventType == vr::VREvent_TrackedDeviceUpdated)
	{
		printf("Device %u updated.\n", event.trackedDeviceIndex);
	}
	else
	{
		; // This is where uncaught events go for now.
	}
}

void TrackedDeviceManager::updateControllerStates()
{
	// If controller is engaged in interaction, update interaction vars
	if (m_pEditController)
		m_pEditController->update();
	
	if (m_pManipController)
		m_pManipController->update();
}

bool TrackedDeviceManager::cleaningModeActive()
{
	return m_pEditController && m_pEditController->cleaningActive();
}

bool TrackedDeviceManager::getCleaningCursorData(Matrix4 *thisCursorPose, Matrix4 *lastCursorPose, float *radius)
{
	if (!m_pEditController || !m_pEditController->poseValid()) return false;
	
	m_pEditController->getCursorPoses(thisCursorPose, lastCursorPose);
	*radius = m_pEditController->getCursorRadius();

	return true;
}

Matrix4* TrackedDeviceManager::getManipulationData()
{	
	if (m_pManipController && m_pManipController->poseValid() && m_pManipController->isTriggerClicked())
	{
		return &m_pManipController->getPose();
	}

	return NULL;
}

void TrackedDeviceManager::cleaningHit()
{
	m_pHMD->TriggerHapticPulse(m_pEditController->getIndex(), 0, 2000);
}

//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
void TrackedDeviceManager::initDevices()
{
	if (!m_pHMD)
		return;
	
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		m_rpTrackedDevices[nDevice] = new TrackedDevice(nDevice, m_pHMD, m_pRenderModels);

		if (!m_pHMD->IsTrackedDeviceConnected(nDevice))
			continue;

		setupTrackedDevice(nDevice);
	}

}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL a Render Model for a single tracked device
//-----------------------------------------------------------------------------
void TrackedDeviceManager::setupTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex)
{
	if (unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount)
		return;
	
	m_rpTrackedDevices[unTrackedDeviceIndex]->BInit();

	if (m_pHMD->GetTrackedDeviceClass(unTrackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
	{
		ViveController *thisController = NULL;

		if (!m_pEditController)
		{
			m_pEditController = new EditingController(unTrackedDeviceIndex, m_pHMD, m_pRenderModels);
			m_pEditController->BInit();
			thisController = m_pEditController;
		}
		else if(!m_pManipController)
		{
			m_pManipController = new ViveController(unTrackedDeviceIndex, m_pHMD, m_pRenderModels);
			m_pManipController->BInit();
			thisController = m_pManipController;
		}

		thisController->attach(&InfoBoxManager::getInstance());
	}
}

void TrackedDeviceManager::renderTrackedDevices(Matrix4 & matVP)
{
	for (uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_rpTrackedDevices[unTrackedDevice]->hasRenderModel())
			continue;

		if (!m_rpTrackedDevices[unTrackedDevice]->poseValid())
			continue;

		//if (bIsInputCapturedByAnotherProcess && m_pHMD->GetTrackedDeviceClass(unTrackedDevice) == vr::TrackedDeviceClass_Controller)
		//	continue;

		if (m_pEditController && m_pEditController->getIndex() == unTrackedDevice)
		{
			m_pEditController->render(matVP);
			m_pEditController->renderModel(matVP);
			continue;
		}

		if (m_pManipController && m_pManipController->getIndex() == unTrackedDevice)
		{
			m_pManipController->render(matVP);
			m_pManipController->renderModel(matVP);
			continue;
		}

		m_rpTrackedDevices[unTrackedDevice]->renderModel(matVP);
	}
}

void TrackedDeviceManager::postRenderUpdate()
{
	// Spew out the controller and pose count whenever they change.
	if (m_iTrackedControllerCount != m_iTrackedControllerCount_Last || m_iValidPoseCount != m_iValidPoseCount_Last)
	{
		m_iValidPoseCount_Last = m_iValidPoseCount;
		m_iTrackedControllerCount_Last = m_iTrackedControllerCount;

		printf("PoseCount:%d(%s) Controllers:%d\n", m_iValidPoseCount, m_strPoseClasses.c_str(), m_iTrackedControllerCount);
	}

	UpdateHMDMatrixPose();
}

Matrix4 & TrackedDeviceManager::getHMDPose()
{
	return m_mat4HMDPose;
}

Matrix4 & TrackedDeviceManager::getEditControllerPose()
{
	if(m_pEditController)
		return m_pEditController->getPose();

	return Matrix4().identity();
}

Matrix4 & TrackedDeviceManager::getManipControllerPose()
{
	if (m_pEditController)
		return m_pManipController->getPose();

	return Matrix4().identity();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void TrackedDeviceManager::UpdateHMDMatrixPose()
{
	if (!m_pHMD)
		return;

	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
	vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	m_iValidPoseCount = 0;
	m_strPoseClasses = "";
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_rpTrackedDevices[nDevice]->updatePose(poses[nDevice]))
		{
			m_iValidPoseCount++;

			if (m_rpTrackedDevices[nDevice]->getClassChar() == 0)
			{
				switch (m_pHMD->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:        m_rpTrackedDevices[nDevice]->setClassChar('C'); break;
				case vr::TrackedDeviceClass_HMD:               m_rpTrackedDevices[nDevice]->setClassChar('H'); break;
				case vr::TrackedDeviceClass_Invalid:           m_rpTrackedDevices[nDevice]->setClassChar('I'); break;
				case vr::TrackedDeviceClass_Other:             m_rpTrackedDevices[nDevice]->setClassChar('O'); break;
				case vr::TrackedDeviceClass_TrackingReference: m_rpTrackedDevices[nDevice]->setClassChar('T'); break;
				default:                                       m_rpTrackedDevices[nDevice]->setClassChar('?'); break;
				}
			}
			m_strPoseClasses += m_rpTrackedDevices[nDevice]->getClassChar();
		}
	}
	
	if(m_pEditController)
		m_pEditController->updatePose(poses[m_pEditController->getIndex()]);

	if (m_pManipController)
		m_pManipController->updatePose(poses[m_pManipController->getIndex()]);

	if (m_rpTrackedDevices[vr::k_unTrackedDeviceIndex_Hmd]->poseValid())
	{
		m_mat4HMDPose = m_rpTrackedDevices[vr::k_unTrackedDeviceIndex_Hmd]->getPose().invert();
	}
}