#include "TrackedDeviceManager.h"
#include "ShaderUtils.h"

TrackedDeviceManager::TrackedDeviceManager(vr::IVRSystem* pHMD)
: m_pHMD(pHMD)
, m_pRenderModels(NULL)
, m_pEditController(NULL)
, m_pManipController(NULL)
, m_unRenderModelProgramID(0)
, m_iTrackedControllerCount(0)
, m_iTrackedControllerCount_Last(-1)
, m_iValidPoseCount(0)
, m_iValidPoseCount_Last(-1)
, m_strPoseClasses("")
, m_nRenderModelMatrixLocation(-1)
{
}


TrackedDeviceManager::~TrackedDeviceManager()
{	
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
		delete m_rpTrackedDevices[nDevice];


	if (m_unRenderModelProgramID)
	{
		glDeleteProgram(m_unRenderModelProgramID);
	}
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
	createShaders();

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
// Purpose: Processes a single VR event
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
	else if (m_pHMD->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
	{
		if (m_pEditController && m_pEditController->getIndex() == event.trackedDeviceIndex)
		{
			m_pEditController->update(&event);
		}

		if (m_pManipController && m_pManipController->getIndex() == event.trackedDeviceIndex)
		{
			m_pManipController->update(&event);
		}

		return;		
	}
	else
	{
		; // This is where uncaught events go for now
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

bool TrackedDeviceManager::getCleaningCursorData(Matrix4 *thisCursorPose, Matrix4 *lastCursorPose, float *radius)
{
	if (!m_pEditController) return false;

	float cursorRadius = 0.f;
	bool cleaningModeActive = false;

	if (m_pEditController->poseValid())
	{
		m_pEditController->getCursorPoses(thisCursorPose, lastCursorPose);
		*radius = m_pEditController->getCursorRadius();
		cleaningModeActive = m_pEditController->cleaningActive();
	}

	return cleaningModeActive;
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
		m_rpTrackedDevices[nDevice] = new TrackedDevice(nDevice);

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
			m_pEditController = new EditingController(unTrackedDeviceIndex);
			thisController = m_pEditController;
		}
		else if(!m_pManipController)
		{
			m_pManipController = new ViveController(unTrackedDeviceIndex);
			thisController = m_pManipController;
		}
		
		if (pRenderModel && thisController)
		{
			thisController->setRenderModel(pRenderModel);

			const char* pchRenderName = pRenderModel->GetName().c_str();

			uint32_t nModelComponents = m_pRenderModels->GetComponentCount(pchRenderName);

			for (uint32_t i = 0; i < nModelComponents; ++i)
			{
				uint32_t unRequiredBufferLen = m_pRenderModels->GetComponentName(pRenderModel->GetName().c_str(), i, NULL, 0);
				if (unRequiredBufferLen == 0)
					continue;

				char *pchBuffer1 = new char[unRequiredBufferLen];
				unRequiredBufferLen = m_pRenderModels->GetComponentName(pchRenderName, i, pchBuffer1, unRequiredBufferLen);
				std::string sComponentName = pchBuffer1;
				delete[] pchBuffer1;

				bool hasRenderModel = true;
				unRequiredBufferLen = m_pRenderModels->GetComponentRenderModelName(pchRenderName, sComponentName.c_str(), NULL, 0);
				if (unRequiredBufferLen == 0)
					hasRenderModel = false;
				else
				{
					char *pchBuffer2 = new char[unRequiredBufferLen];
					unRequiredBufferLen = m_pRenderModels->GetComponentRenderModelName(pchRenderName, sComponentName.c_str(), pchBuffer2, unRequiredBufferLen);
					std::string sComponentRenderModelName = pchBuffer2;
					delete[] pchBuffer2;

					CGLRenderModel *pComponentRenderModel = findOrLoadRenderModel(sComponentRenderModelName.c_str());
					thisController->addComponentRenderModel(i, sComponentName, pComponentRenderModel);
				}

				std::cout << "\t" << (hasRenderModel ? "M -> " : "     ") << i << ": " << sComponentName << std::endl;
			}
		}
	}
}

void TrackedDeviceManager::renderTrackedDevices(Matrix4 & matVP)
{
	if (m_pEditController) m_pEditController->render(matVP);
	if (m_pManipController) m_pManipController->render(matVP);
	renderDeviceModels(matVP);
}

void TrackedDeviceManager::renderDeviceModels(Matrix4 & matVP)
{
	// ----- Render Model rendering -----
	glUseProgram(m_unRenderModelProgramID);

	for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_rpTrackedDevices[unTrackedDevice]->hasRenderModel())
			continue;

		if (!m_rpTrackedDevices[unTrackedDevice]->poseValid())
			continue;

		//if (bIsInputCapturedByAnotherProcess && m_pHMD->GetTrackedDeviceClass(unTrackedDevice) == vr::TrackedDeviceClass_Controller)
		//	continue;

		uint32_t nComponents = m_pEditController ? m_pEditController->getComponentCount() : 0;

		if (nComponents == 0)
		{
			const Matrix4 & matDeviceToTracking = m_rpTrackedDevices[unTrackedDevice]->getPose();
			Matrix4 matMVP = matVP * matDeviceToTracking;
			glUniformMatrix4fv(m_nRenderModelMatrixLocation, 1, GL_FALSE, matMVP.get());

			m_rpTrackedDevices[unTrackedDevice]->renderModel();
		}
		else
		{
			for (uint32_t i = 0; i < nComponents; ++i)
			{
				const Matrix4 & matDeviceToTracking = m_pEditController->getPose();
				const Matrix4 & matComponentToDevice = m_pEditController->getComponentPose(i);
				Matrix4 matMVP = matVP * matDeviceToTracking;
				glUniformMatrix4fv(m_nRenderModelMatrixLocation, 1, GL_FALSE, matMVP.get());

				m_pEditController->renderModel();
			}
		}
	}

	glUseProgram(0);
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

bool TrackedDeviceManager::createShaders()
{
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