#include "TrackedDeviceManager.h"
#include "ShaderUtils.h"
#include "InfoBoxManager.h"
#include <shared/glm/gtc/type_ptr.hpp>

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
, m_unRenderModelProgramID(0)
, m_nRenderModelMatrixLocation(-1)
{
}


TrackedDeviceManager::~TrackedDeviceManager()
{
	if (m_unRenderModelProgramID)
	{
		glDeleteProgram(m_unRenderModelProgramID);
	}

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

	m_unRenderModelProgramID = CompileGLShader(
		"render model",

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3NormalIn;\n"
		"layout(location = 2) in vec2 v2TexCoordsIn;\n"
		"out vec2 v2TexCoord;\n"
		"void main()\n"
		"{\n"
		"	v2TexCoord = v2TexCoordsIn;\n"
		"	gl_Position = matrix * vec4(position.xyz, 1);\n"
		"}\n",

		//fragment shader
		"#version 410 core\n"
		"uniform sampler2D diffuse;\n"
		"in vec2 v2TexCoord;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = texture( diffuse, v2TexCoord);\n"
		"}\n"

	);
	m_nRenderModelMatrixLocation = glGetUniformLocation(m_unRenderModelProgramID, "matrix");
	if (m_nRenderModelMatrixLocation == -1)
	{
		printf("Unable to find matrix uniform in render model shader\n");
		return false;
	}

	return m_unRenderModelProgramID != 0;
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
		removeTrackedDevice(event.trackedDeviceIndex);
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

void TrackedDeviceManager::attach(BroadcastSystem::Listener * obs)
{
	m_vpListeners.push_back(obs);

	if (m_pEditController)
		m_pEditController->attach(obs);

	if (m_pManipController)
		m_pManipController->attach(obs);
}

void TrackedDeviceManager::detach(BroadcastSystem::Listener * obs)
{
	m_vpListeners.erase(std::remove(m_vpListeners.begin(), m_vpListeners.end(), obs), m_vpListeners.end());

	if (m_pEditController)
		m_pEditController->detach(obs);

	if (m_pManipController)
		m_pManipController->detach(obs);
}

bool TrackedDeviceManager::cleaningModeActive()
{
	return m_pEditController && m_pEditController->cleaningActive();
}

bool TrackedDeviceManager::getCleaningCursorData(glm::mat4 *thisCursorPose, glm::mat4 *lastCursorPose, float *radius)
{
	if (!m_pEditController || !m_pEditController->poseValid()) return false;
	
	m_pEditController->getCursorPoses(thisCursorPose, lastCursorPose);
	*radius = m_pEditController->getCursorRadius();

	return true;
}

bool TrackedDeviceManager::getManipulationData(glm::mat4 &controllerPose)
{	
	if (m_pManipController && m_pManipController->poseValid() && m_pManipController->isTriggerClicked())
	{
		controllerPose = m_pManipController->getDeviceToWorldTransform();
		return true;
	}

	return false;
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

	CGLRenderModel *pRenderModel = findOrLoadRenderModel(strRenderModelName.c_str());

	if (!pRenderModel)
	{
		std::string sTrackingSystemName = getPropertyString(unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String);
		printf("Unable to load render model for tracked device %d (%s.%s)\n", unTrackedDeviceIndex, sTrackingSystemName.c_str(), strRenderModelName.c_str());
		return false;
	}
	else
	{
		m_rpTrackedDevices[unTrackedDeviceIndex]->setRenderModel(pRenderModel);
		std::cout << "Device " << unTrackedDeviceIndex << "'s RenderModel name is " << strRenderModelName << std::endl;

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
					std::string sComponentRenderModelName = pchBuffer2;

					CGLRenderModel *pComponentRenderModel = findOrLoadRenderModel(pchBuffer2);
					delete[] pchBuffer2;

					c.m_pComponentRenderModel = pComponentRenderModel;
					c.m_bHasRenderModel = true;
				}

				c.m_bInitialized = true;

				m_rpTrackedDevices[unTrackedDeviceIndex]->m_vComponents.push_back(c);

				std::cout << "\t" << (c.m_bHasRenderModel ? "Model -> " : "         ") << i << ": " << c.m_strComponentName << std::endl;
			}
		}
	}


	if (m_pHMD->GetTrackedDeviceClass(unTrackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
	{
		ViveController *thisController = NULL;

		if (!m_pEditController)
		{
			m_pEditController = new EditingController(unTrackedDeviceIndex, m_pHMD, m_pRenderModels);
			m_pEditController->BInit();
			thisController = m_pEditController;
		}
		else if (!m_pManipController)
		{
			m_pManipController = new ViveController(unTrackedDeviceIndex, m_pHMD, m_pRenderModels);
			m_pManipController->BInit();
			thisController = m_pManipController;
		}

		//thisController->attach(&InfoBoxManager::getInstance());

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
		if (m_rpTrackedDevices[unTrackedDeviceIndex]->getIndex() == m_pEditController->getIndex())
		{
			m_pEditController->detach(&InfoBoxManager::getInstance());

			for (auto &l : m_vpListeners)
				m_pEditController->detach(l);

			delete m_pEditController;
			m_pEditController = NULL;
		}
		else if (m_rpTrackedDevices[unTrackedDeviceIndex]->getIndex() == m_pManipController->getIndex())
		{
			m_pManipController->detach(&InfoBoxManager::getInstance());

			for (auto &l : m_vpListeners)
				m_pManipController->detach(l);

			delete m_pManipController;
			m_pManipController = NULL;
		}
	}

}

//-----------------------------------------------------------------------------
// Purpose: Finds a render model we've already loaded or loads a new one
//-----------------------------------------------------------------------------
CGLRenderModel* TrackedDeviceManager::findOrLoadRenderModel(const char *pchRenderModelName)
{
	// check model cache for existing model
	CGLRenderModel *pRenderModel = m_mapModelCache[std::string(pchRenderModelName)];

	// found model in the cache, so return it
	if (pRenderModel)
	{
		printf("Found existing render model for %s\n", pchRenderModelName);
		return pRenderModel;
	}
	else
		printf("No existing render model found for %s; Loading...\n", pchRenderModelName);

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

	vr::VRRenderModels()->FreeRenderModel(pModel);
	vr::VRRenderModels()->FreeTexture(pTexture);

	m_mapModelCache[std::string(pchRenderModelName)] = pRenderModel;

	return pRenderModel;
}


void TrackedDeviceManager::renderTrackedDevices(glm::mat4 & matVP)
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
		}

		if (m_pManipController && m_pManipController->getIndex() == unTrackedDevice)
		{
			m_pManipController->render(matVP);
		}

		// ----- Render Model rendering -----
		glUseProgram(m_unRenderModelProgramID);

		size_t nComponents = m_rpTrackedDevices[unTrackedDevice]->m_vComponents.size();

		if (m_rpTrackedDevices[unTrackedDevice]->m_vComponents.size() > 0u)
		{

			for (size_t i = 0; i < nComponents; ++i)
				if (m_rpTrackedDevices[unTrackedDevice]->m_vComponents[i].m_pComponentRenderModel && 
					m_rpTrackedDevices[unTrackedDevice]->m_vComponents[i].m_bVisible)
				{
					glm::mat4 matMVP;

					if (!m_rpTrackedDevices[unTrackedDevice]->m_vComponents[i].m_bStatic)
					{
						vr::TrackedDevicePose_t p;
						m_pHMD->ApplyTransform(&p, &m_rpTrackedDevices[unTrackedDevice]->m_Pose, &(m_rpTrackedDevices[unTrackedDevice]->m_vComponents[i].m_mat3PoseTransform));
						matMVP = matVP * m_rpTrackedDevices[unTrackedDevice]->ConvertSteamVRMatrixToMatrix4(p.mDeviceToAbsoluteTracking);
					}
					else
					{
						matMVP = matVP * m_rpTrackedDevices[unTrackedDevice]->getDeviceToWorldTransform();
					}

					glUniformMatrix4fv(m_nRenderModelMatrixLocation, 1, GL_FALSE, glm::value_ptr(matMVP));
					m_rpTrackedDevices[unTrackedDevice]->m_vComponents[i].m_pComponentRenderModel->Draw();
				}
		}
		else
		{
			const glm::mat4 &matDeviceToTracking = m_rpTrackedDevices[unTrackedDevice]->getDeviceToWorldTransform();
			glm::mat4 matMVP = matVP * matDeviceToTracking;
			glUniformMatrix4fv(m_nRenderModelMatrixLocation, 1, GL_FALSE, glm::value_ptr(matMVP));

			m_rpTrackedDevices[unTrackedDevice]->getRenderModel()->Draw();
		}

		glUseProgram(0);
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

glm::mat4 & TrackedDeviceManager::getHMDPose()
{
	return m_mat4HMDPose;
}

glm::mat4 & TrackedDeviceManager::getEditControllerPose()
{
	if(m_pEditController)
		return m_pEditController->getDeviceToWorldTransform();

	return glm::mat4();
}

glm::mat4 & TrackedDeviceManager::getManipControllerPose()
{
	if (m_pManipController)
		return m_pManipController->getDeviceToWorldTransform();

	return glm::mat4();
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
	m_iTrackedControllerCount = 0;
	m_strPoseClasses = "";
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_rpTrackedDevices[nDevice]->updateDeviceToWorldTransform(poses[nDevice]))
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
	
	if(m_pEditController)
		m_pEditController->updatePose(poses[m_pEditController->getIndex()]);

	if (m_pManipController)
		m_pManipController->updatePose(poses[m_pManipController->getIndex()]);

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
}

//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
std::string TrackedDeviceManager::getPropertyString(vr::TrackedDeviceIndex_t deviceID, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError)
{
	vr::EVRInitError eError = vr::VRInitError_None;

	vr::IVRSystem *pHMD = (vr::IVRSystem *)vr::VR_GetGenericInterface(vr::IVRSystem_Version, &eError);

	uint32_t unRequiredBufferLen = pHMD->GetStringTrackedDeviceProperty(deviceID, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHMD->GetStringTrackedDeviceProperty(deviceID, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}