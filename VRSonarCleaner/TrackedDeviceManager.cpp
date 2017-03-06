#include "TrackedDeviceManager.h"
#include "ShaderUtils.h"
#include "InfoBoxManager.h"
#include "Renderer.h"
#include <shared/glm/gtc/type_ptr.hpp>

TrackedDeviceManager::TrackedDeviceManager(vr::IVRSystem* pHMD)
: m_pHMD(pHMD)
, m_pRenderModels(NULL)
, m_pPrimaryController(NULL)
, m_pSecondaryController(NULL)
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
		if (m_rpTrackedDevices[nDevice])
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
		switch (event.eventType)
		{
		case vr::VREvent_TrackedDeviceActivated:
			setupTrackedDevice(event.trackedDeviceIndex);
			printf("Device %u attached. Setting up.\n", event.trackedDeviceIndex);
			break;
		case vr::VREvent_TrackedDeviceDeactivated:
			removeTrackedDevice(event.trackedDeviceIndex);
			printf("Device %u detached.\n", event.trackedDeviceIndex);
			break;
		case vr::VREvent_TrackedDeviceUpdated:
			printf("Device %u updated.\n", event.trackedDeviceIndex);
			break;
		default:
			// This is where uncaught events go for now.
			break;
		}
	}
}

void TrackedDeviceManager::attach(BroadcastSystem::Listener * obs)
{
	m_vpListeners.push_back(obs);

	if (m_pPrimaryController)
		m_pPrimaryController->attach(obs);

	if (m_pSecondaryController)
		m_pSecondaryController->attach(obs);
}

void TrackedDeviceManager::detach(BroadcastSystem::Listener * obs)
{
	m_vpListeners.erase(std::remove(m_vpListeners.begin(), m_vpListeners.end(), obs), m_vpListeners.end());

	if (m_pPrimaryController)
		m_pPrimaryController->detach(obs);

	if (m_pSecondaryController)
		m_pSecondaryController->detach(obs);
}

bool TrackedDeviceManager::cleaningModeActive()
{
	return m_pPrimaryController && m_pPrimaryController->cleaningActive();
}

bool TrackedDeviceManager::getCleaningCursorData(glm::mat4 &thisCursorPose, glm::mat4 &lastCursorPose, float &radius)
{
	if (!m_pPrimaryController || !m_pPrimaryController->poseValid()) return false;
	
	m_pPrimaryController->getCursorPoses(thisCursorPose, lastCursorPose);
	radius = m_pPrimaryController->getCursorRadius();

	return true;
}

bool TrackedDeviceManager::getManipulationData(glm::mat4 &controllerPose)
{	
	if (m_pSecondaryController && m_pSecondaryController->poseValid() && m_pSecondaryController->isTriggerClicked())
	{
		controllerPose = m_pSecondaryController->getDeviceToWorldTransform();
		return true;
	}

	return false;
}

void TrackedDeviceManager::cleaningHit()
{
	m_pHMD->TriggerHapticPulse(m_pPrimaryController->getIndex(), 0, 2000);
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

		if (!setupTrackedDevice(nDevice))
			printf("Unable to setup tracked device %d\n", nDevice);
	}

}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL a Render Model for a single tracked device
//-----------------------------------------------------------------------------
bool TrackedDeviceManager::setupTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex)
{
	if (unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount)
		return false;

	m_rpTrackedDevices[unTrackedDeviceIndex]->BInit();
		
	std::string strRenderModelName = getPropertyString(unTrackedDeviceIndex, vr::Prop_RenderModelName_String);

	m_rpTrackedDevices[unTrackedDeviceIndex]->setRenderModelName(strRenderModelName);

	std::cout << "Device " << unTrackedDeviceIndex << "'s RenderModel name is " << strRenderModelName << std::endl;

	if (m_pHMD->GetTrackedDeviceClass(unTrackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
	{
		ViveController *thisController = NULL;

		if (!m_pPrimaryController)
		{
			m_pPrimaryController = new EditingController(unTrackedDeviceIndex, m_pHMD, m_pRenderModels);
			m_pPrimaryController->BInit();
			m_pPrimaryController->setRenderModelName(strRenderModelName);
			thisController = m_pPrimaryController;
		}
		else if (!m_pSecondaryController)
		{
			m_pSecondaryController = new ViveController(unTrackedDeviceIndex, m_pHMD, m_pRenderModels);
			m_pSecondaryController->BInit();
			m_pSecondaryController->setRenderModelName(strRenderModelName);
			thisController = m_pSecondaryController;
		}

		// Check if there are model components
		uint32_t nModelComponents = m_pRenderModels->GetComponentCount(strRenderModelName.c_str());

		// Loop over model components and add them to the tracked device
		for (uint32_t i = 0; i < nModelComponents; ++i)
		{
			TrackedDevice::TrackedDeviceComponent c;
			c.m_unComponentIndex = i;

			uint32_t unRequiredBufferLen = m_pRenderModels->GetComponentName(strRenderModelName.c_str(), i, NULL, 0);
			if (unRequiredBufferLen == 0)
			{
				printf("Controller [device %d] component %d index out of range.\n", unTrackedDeviceIndex, i);
			}
			else
			{
				char *pchBuffer1 = new char[unRequiredBufferLen];
				unRequiredBufferLen = m_pRenderModels->GetComponentName(strRenderModelName.c_str(), i, pchBuffer1, unRequiredBufferLen);
				c.m_strComponentName = pchBuffer1;
				delete[] pchBuffer1;

				unRequiredBufferLen = m_pRenderModels->GetComponentRenderModelName(strRenderModelName.c_str(), c.m_strComponentName.c_str(), NULL, 0);
				if (unRequiredBufferLen == 0)
					c.m_bHasRenderModel = false;
				else
				{
					char *pchBuffer2 = new char[unRequiredBufferLen];
					unRequiredBufferLen = m_pRenderModels->GetComponentRenderModelName(strRenderModelName.c_str(), c.m_strComponentName.c_str(), pchBuffer2, unRequiredBufferLen);
					c.m_strComponentRenderModelName = pchBuffer2;

					delete[] pchBuffer2;

					c.m_bHasRenderModel = true;
				}

				c.m_bInitialized = true;

				thisController->m_vComponents.push_back(c);

				std::cout << "\t" << (c.m_bHasRenderModel ? "Model -> " : "         ") << i << ": " << c.m_strComponentName << std::endl;
			}
		}

		for (auto &l : m_vpListeners)
			thisController->attach(l);
	}

	return true;
}

void TrackedDeviceManager::removeTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex)
{
	if (unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount)
		return;

	if (m_pHMD->GetTrackedDeviceClass(unTrackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
	{
		if (m_rpTrackedDevices[unTrackedDeviceIndex]->getIndex() == m_pPrimaryController->getIndex())
		{
			m_pPrimaryController->detach(&InfoBoxManager::getInstance());

			for (auto &l : m_vpListeners)
				m_pPrimaryController->detach(l);

			delete m_pPrimaryController;
			m_pPrimaryController = NULL;
		}
		else if (m_rpTrackedDevices[unTrackedDeviceIndex]->getIndex() == m_pSecondaryController->getIndex())
		{
			m_pSecondaryController->detach(&InfoBoxManager::getInstance());

			for (auto &l : m_vpListeners)
				m_pSecondaryController->detach(l);

			delete m_pSecondaryController;
			m_pSecondaryController = NULL;
		}
	}

}

void TrackedDeviceManager::updateTrackedDeviceRenderModels()
{
	for (uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_rpTrackedDevices[unTrackedDevice]->hasRenderModel())
			continue;

		if (!m_rpTrackedDevices[unTrackedDevice]->poseValid())
			continue;

		//if (bIsInputCapturedByAnotherProcess && m_pHMD->GetTrackedDeviceClass(unTrackedDevice) == vr::TrackedDeviceClass_Controller)
		//	continue;
		ViveController* thisController = NULL;
		if (m_pPrimaryController && m_pPrimaryController->getIndex() == unTrackedDevice)
		{
			thisController = m_pPrimaryController;
		}

		if (m_pSecondaryController && m_pSecondaryController->getIndex() == unTrackedDevice)
		{
			thisController = m_pSecondaryController;
		}

		if (thisController && thisController->m_vComponents.size() > 0u)
		{
			size_t nComponents = thisController->m_vComponents.size();

			for (size_t i = 0; i < nComponents; ++i)
				if (thisController->m_vComponents[i].m_bHasRenderModel &&
					thisController->m_vComponents[i].m_bVisible)
				{
					glm::mat4 matModel;

					if (thisController->m_vComponents[i].m_bStatic)
					{
						matModel = thisController->getDeviceToWorldTransform();
					}
					else
					{
						vr::TrackedDevicePose_t p;
						m_pHMD->ApplyTransform(&p, &thisController->m_Pose, &(thisController->m_vComponents[i].m_mat3PoseTransform));
						matModel = thisController->ConvertSteamVRMatrixToMatrix4(p.mDeviceToAbsoluteTracking);
					}
					Renderer::getInstance().addRenderModelInstance(thisController->m_vComponents[i].m_strComponentRenderModelName.c_str(), matModel);
				}
		}
		else // render model without components
		{
			glm::mat4 matModel = m_rpTrackedDevices[unTrackedDevice]->getDeviceToWorldTransform();

			Renderer::getInstance().addRenderModelInstance(m_rpTrackedDevices[unTrackedDevice]->m_strRenderModelName.c_str(), matModel);
		}
	}
}

glm::mat4 & TrackedDeviceManager::getHMDPose()
{
	return m_mat4HMDPose;
}

glm::mat4 & TrackedDeviceManager::getEditControllerPose()
{
	if(m_pPrimaryController)
		return m_pPrimaryController->getDeviceToWorldTransform();

	return glm::mat4();
}

glm::mat4 & TrackedDeviceManager::getManipControllerPose()
{
	if (m_pSecondaryController)
		return m_pSecondaryController->getDeviceToWorldTransform();

	return glm::mat4();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void TrackedDeviceManager::updateTrackedDevices()
{
	if (!m_pHMD)
		return;

	vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
	vr::VRCompositor()->WaitGetPoses(poses, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	m_iValidPoseCount = 0;
	m_iTrackedControllerCount = 0;
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
				case vr::TrackedDeviceClass_Controller:        m_rpTrackedDevices[nDevice]->setClassChar('C'); m_iTrackedControllerCount++; break;
				case vr::TrackedDeviceClass_HMD:               m_rpTrackedDevices[nDevice]->setClassChar('H'); break;
				case vr::TrackedDeviceClass_Invalid:           m_rpTrackedDevices[nDevice]->setClassChar('I'); break;
				case vr::TrackedDeviceClass_GenericTracker:    m_rpTrackedDevices[nDevice]->setClassChar('G'); break;
				case vr::TrackedDeviceClass_TrackingReference: m_rpTrackedDevices[nDevice]->setClassChar('T'); break;
				default:                                       m_rpTrackedDevices[nDevice]->setClassChar('?'); break;
				}
			}
			m_strPoseClasses += m_rpTrackedDevices[nDevice]->getClassChar();
		}
	}

	// Spew out the controller and pose count whenever they change.
	if (m_iTrackedControllerCount != m_iTrackedControllerCount_Last || m_iValidPoseCount != m_iValidPoseCount_Last)
	{
		m_iValidPoseCount_Last = m_iValidPoseCount;
		m_iTrackedControllerCount_Last = m_iTrackedControllerCount;

		printf("PoseCount:%d(%s) Controllers:%d\n", m_iValidPoseCount, m_strPoseClasses.c_str(), m_iTrackedControllerCount);
	}
	
	if (m_pPrimaryController)
	{
		m_pPrimaryController->updatePose(poses[m_pPrimaryController->getIndex()]);
		m_pPrimaryController->updateControllerState();

		// don't draw controllers if somebody else has input focus
		if (!m_pHMD->IsInputFocusCapturedByAnotherProcess())
		{
			m_pPrimaryController->prepareForRendering();
		}
	}

	if (m_pSecondaryController)
	{
		m_pSecondaryController->updatePose(poses[m_pSecondaryController->getIndex()]);
		m_pSecondaryController->updateControllerState();

		// don't draw controllers if somebody else has input focus
		if (!m_pHMD->IsInputFocusCapturedByAnotherProcess())
		{
			m_pSecondaryController->prepareForRendering();
		}
	}

	if (m_rpTrackedDevices[vr::k_unTrackedDeviceIndex_Hmd]->poseValid())
	{
		m_mat4HMDPose = glm::inverse(m_rpTrackedDevices[vr::k_unTrackedDeviceIndex_Hmd]->getDeviceToWorldTransform());

		glm::mat4 tempMat = m_rpTrackedDevices[vr::k_unTrackedDeviceIndex_Hmd]->getDeviceToWorldTransform();
		glm::vec3 HMDpos = glm::vec3(tempMat[3]);
		float widthX, widthZ;
		vr::IVRChaperone* chap = vr::VRChaperone();
		chap->GetPlayAreaSize(&widthX, &widthZ);
		if (abs(HMDpos.x) > widthX || abs(HMDpos.z) > widthZ)
			notify(m_rpTrackedDevices[vr::k_unTrackedDeviceIndex_Hmd], BroadcastSystem::EVENT::EXIT_PLAY_AREA);
		else
			notify(m_rpTrackedDevices[vr::k_unTrackedDeviceIndex_Hmd], BroadcastSystem::EVENT::ENTER_PLAY_AREA);
	}

	updateTrackedDeviceRenderModels();
}

void TrackedDeviceManager::renderControllerCustomizations(glm::mat4 * matVP)
{
	if (m_pPrimaryController)
		m_pPrimaryController->render(*matVP);

	if (m_pSecondaryController)
		m_pSecondaryController->render(*matVP);
}

//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
std::string TrackedDeviceManager::getPropertyString(vr::TrackedDeviceIndex_t deviceID, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError)
{
	vr::EVRInitError eError = vr::VRInitError_None;
	
	uint32_t unRequiredBufferLen = m_pHMD->GetStringTrackedDeviceProperty(deviceID, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = m_pHMD->GetStringTrackedDeviceProperty(deviceID, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}