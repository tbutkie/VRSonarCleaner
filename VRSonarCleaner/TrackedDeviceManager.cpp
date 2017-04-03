#include "TrackedDeviceManager.h"
#include "InfoBoxManager.h"
#include "Renderer.h"
#include <shared/glm/gtc/type_ptr.hpp>
#include <shared/glm/gtc/matrix_transform.hpp>

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
, m_bCursorRadiusResizeMode(false)
, m_bCursorOffsetMode(false)
, m_fCursorRadius(0.05f)
, m_fCursorRadiusMin(0.005f)
, m_fCursorRadiusMax(0.1f)
, m_vec3CursorOffsetDirection(glm::vec3(0.f, 0.f, -1.f))
, m_fCursorOffsetAmount(0.1f)
, m_fCursorOffsetAmountMin(0.1f)
, m_fCursorOffsetAmountMax(1.5f)
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
			printf("Device %u attached. Setting up.\n", event.trackedDeviceIndex);
			setupTrackedDevice(event.trackedDeviceIndex);
			break;
		case vr::VREvent_TrackedDeviceDeactivated:
			printf("Device %u detached.\n", event.trackedDeviceIndex);
			removeTrackedDevice(event.trackedDeviceIndex);
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

bool TrackedDeviceManager::cleaningModeActive()
{
	return m_pPrimaryController && m_pPrimaryController->isTriggerClicked();
}

bool TrackedDeviceManager::getCleaningCursorData(glm::mat4 &thisCursorPose, glm::mat4 &lastCursorPose, float &radius)
{
	if (!m_pPrimaryController || !m_pPrimaryController->poseValid()) return false;
	
	thisCursorPose = m_pPrimaryController->getPose() * glm::translate(glm::mat4(), m_vec3CursorOffsetDirection * m_fCursorOffsetAmount);
	
	lastCursorPose = m_pPrimaryController->getLastPose() * glm::translate(glm::mat4(), m_vec3CursorOffsetDirection * m_fCursorOffsetAmount);

	radius = m_fCursorRadius;

	return true;
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
		if (!m_pHMD->IsTrackedDeviceConnected(nDevice))
		{
			m_rpTrackedDevices[nDevice] = NULL;
			continue;
		}

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

	TrackedDevice* thisDevice;
	
	if (m_pHMD->GetTrackedDeviceClass(unTrackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
		thisDevice = new ViveController(unTrackedDeviceIndex, m_pHMD, m_pRenderModels);
	else
		thisDevice = new TrackedDevice(unTrackedDeviceIndex, m_pHMD, m_pRenderModels);

	thisDevice->BInit();
		
	std::string strRenderModelName = getPropertyString(unTrackedDeviceIndex, vr::Prop_RenderModelName_String);

	thisDevice->setRenderModelName(strRenderModelName);

	std::cout << "Device " << unTrackedDeviceIndex << "'s RenderModel name is " << strRenderModelName << std::endl;

	if (m_pHMD->GetTrackedDeviceClass(unTrackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
	{
		if (!m_pPrimaryController)
			m_pPrimaryController = static_cast<ViveController*>(thisDevice);
		else if (!m_pSecondaryController)
			m_pSecondaryController = static_cast<ViveController*>(thisDevice);

		m_pHMD->GetControllerState(
			unTrackedDeviceIndex,
			&(static_cast<ViveController*>(thisDevice)->m_ControllerState),
			sizeof(static_cast<ViveController*>(thisDevice)->m_ControllerState)
		);

		static_cast<ViveController*>(thisDevice)->m_LastControllerState = static_cast<ViveController*>(thisDevice)->m_ControllerState;

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

					vr::RenderModel_ControllerMode_State_t controllerModeState;
					m_pRenderModels->GetComponentState(
						strRenderModelName.c_str(),
						c.m_strComponentRenderModelName.c_str(),
						&static_cast<ViveController*>(thisDevice)->m_ControllerState,
						&controllerModeState,
						&(c.m_State)
					);

					c.m_LastState = c.m_State;
				}


				c.m_bInitialized = true;

				thisDevice->m_vComponents.push_back(c);

				std::cout << "\t" << (c.m_bHasRenderModel ? "Model -> " : "         ") << i << ": " << c.m_strComponentName << std::endl;
			}
		}

		// attach listeners to controllers
		for (auto &l : m_vpListeners)
			thisDevice->attach(l);
	}

	m_rpTrackedDevices[unTrackedDeviceIndex] = thisDevice;

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

	m_rpTrackedDevices[unTrackedDeviceIndex] = NULL;
}

void TrackedDeviceManager::updateTrackedDeviceRenderModels()
{
	for (uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_rpTrackedDevices[unTrackedDevice] ||
			!m_rpTrackedDevices[unTrackedDevice]->hasRenderModel() ||
			!m_rpTrackedDevices[unTrackedDevice]->poseValid())
			continue;

		size_t nComponents = m_rpTrackedDevices[unTrackedDevice]->m_vComponents.size();

		if (nComponents > 0u)
		{
			for (size_t i = 0; i < nComponents; ++i)
				if (m_rpTrackedDevices[unTrackedDevice]->m_vComponents[i].m_bHasRenderModel &&
					m_rpTrackedDevices[unTrackedDevice]->m_vComponents[i].isVisible())
				{
					glm::mat4 matModel;

					if (m_rpTrackedDevices[unTrackedDevice]->m_vComponents[i].isStatic())
					{
						matModel = m_rpTrackedDevices[unTrackedDevice]->getDeviceToWorldTransform();
					}
					else
					{
						vr::TrackedDevicePose_t p;
						m_pHMD->ApplyTransform(
							&p, 
							&(m_rpTrackedDevices[unTrackedDevice]->m_Pose), 
							&(m_rpTrackedDevices[unTrackedDevice]->m_vComponents[i].m_State.mTrackingToComponentRenderModel)
						);
						matModel = m_rpTrackedDevices[unTrackedDevice]->ConvertSteamVRMatrixToMatrix4(p.mDeviceToAbsoluteTracking);
					}
					Renderer::getInstance().addRenderModelInstance(m_rpTrackedDevices[unTrackedDevice]->m_vComponents[i].m_strComponentRenderModelName.c_str(), matModel);
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
	return m_mat4HMDView;
}

ViveController * TrackedDeviceManager::getPrimaryController()
{
	return m_pPrimaryController;
}

ViveController * TrackedDeviceManager::getSecondaryController()
{
	return m_pSecondaryController;
}

glm::mat4 & TrackedDeviceManager::getPrimaryControllerPose()
{
	if(m_pPrimaryController)
		return m_pPrimaryController->getDeviceToWorldTransform();

	return glm::mat4();
}

glm::mat4 & TrackedDeviceManager::getSecondaryControllerPose()
{
	if (m_pSecondaryController)
		return m_pSecondaryController->getDeviceToWorldTransform();

	return glm::mat4();
}

bool TrackedDeviceManager::attachToPrimaryController(BroadcastSystem::Listener * l)
{
	if (!m_pPrimaryController)
		return false;

	m_pPrimaryController->attach(l);

	return true;
}

bool TrackedDeviceManager::attachToSecondaryController(BroadcastSystem::Listener * l)
{
	if (!m_pSecondaryController)
		return false;

	m_pSecondaryController->attach(l);

	return true;
}

bool TrackedDeviceManager::detachFromPrimaryController(BroadcastSystem::Listener * l)
{
	if (!m_pPrimaryController)
		return false;

	m_pPrimaryController->detach(l);

	return true;
}

bool TrackedDeviceManager::detachFromSecondaryController(BroadcastSystem::Listener * l)
{
	if (!m_pSecondaryController)
		return false;

	m_pSecondaryController->detach(l);

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void TrackedDeviceManager::update()
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
		if (!m_rpTrackedDevices[nDevice])
			continue;

		if (m_rpTrackedDevices[nDevice]->update(poses[nDevice]))
		{
			m_iValidPoseCount++;

			if (m_rpTrackedDevices[nDevice]->getClassChar() == 0)
			{
				switch (m_pHMD->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:		   m_rpTrackedDevices[nDevice]->setClassChar('C'); m_iTrackedControllerCount++; break;
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
	
	if (m_rpTrackedDevices[vr::k_unTrackedDeviceIndex_Hmd]->poseValid())
	{
		m_mat4HMDView = glm::inverse(m_rpTrackedDevices[vr::k_unTrackedDeviceIndex_Hmd]->getDeviceToWorldTransform());

		glm::mat4 HMDtoWorldMat = m_rpTrackedDevices[vr::k_unTrackedDeviceIndex_Hmd]->getDeviceToWorldTransform();
		glm::vec3 HMDpos = glm::vec3(HMDtoWorldMat[3]);
		float widthX, widthZ;
		vr::IVRChaperone* chap = vr::VRChaperone();
		chap->GetPlayAreaSize(&widthX, &widthZ);

		BroadcastSystem::Payload::HMD payload = {
			m_rpTrackedDevices[vr::k_unTrackedDeviceIndex_Hmd],
			HMDtoWorldMat 
		};

		if (abs(HMDpos.x) > widthX || abs(HMDpos.z) > widthZ)
		{
			notify(BroadcastSystem::EVENT::EXIT_PLAY_AREA, &payload);
		}
		else
		{
			notify(BroadcastSystem::EVENT::ENTER_PLAY_AREA, &payload);
		}
	}

	updateTrackedDeviceRenderModels();
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