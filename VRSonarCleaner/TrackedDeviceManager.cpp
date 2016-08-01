#include "TrackedDeviceManager.h"
#include "ShaderUtils.h"

TrackedDeviceManager::TrackedDeviceManager(vr::IVRSystem* pHMD)
: m_pHMD(pHMD)
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
	for (std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++)
	{
		delete (*i);
	}
	m_vecRenderModels.clear();
	
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
		delete m_rpTrackedDevices[nDevice];


	if (m_unRenderModelProgramID)
	{
		glDeleteProgram(m_unRenderModelProgramID);
	}
}

void TrackedDeviceManager::init()
{
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
		m_rpTrackedDevices[nDevice] = new TrackedDevice(nDevice);

	setupRenderModels();
	createShaders();
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
		setupRenderModelForTrackedDevice(event.trackedDeviceIndex);
		printf("Device %u attached. Setting up render model.\n", event.trackedDeviceIndex);
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
		vr::VRControllerState_t state;
		if (!m_pHMD->GetControllerState(event.trackedDeviceIndex, &state))
			return;

		if (m_pEditController && m_pEditController->getIndex() == event.trackedDeviceIndex)
		{
			m_pEditController->processControllerEvent(event, state);
		}

		if (m_pManipController && m_pManipController->getIndex() == event.trackedDeviceIndex)
		{
			m_pManipController->processControllerEvent(event, state);
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
	{
		vr::VRControllerState_t state;
		if (m_pHMD->GetControllerState(m_pEditController->getIndex(), &state))
			m_pEditController->updateState(&state);
	}

	if (m_pManipController)
	{
		vr::VRControllerState_t state;
		if (m_pHMD->GetControllerState(m_pManipController->getIndex(), &state))
			m_pManipController->updateState(&state);
	}
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
void TrackedDeviceManager::setupRenderModels()
{
	memset(m_rTrackedDeviceToRenderModel, 0, sizeof(m_rTrackedDeviceToRenderModel));

	if (!m_pHMD)
		return;

	for (uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_pHMD->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		setupRenderModelForTrackedDevice(unTrackedDevice);
	}

}

//-----------------------------------------------------------------------------
// Purpose: Finds a render model we've already loaded or loads a new one
//-----------------------------------------------------------------------------
CGLRenderModel* TrackedDeviceManager::findOrLoadRenderModel(const char *pchRenderModelName)
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
void TrackedDeviceManager::setupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex)
{
	if (unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount)
		return;

	// try to find a model we've already set up
	std::string sRenderModelName = getTrackedDeviceString(unTrackedDeviceIndex, vr::Prop_RenderModelName_String);

	CGLRenderModel *pRenderModel = findOrLoadRenderModel(sRenderModelName.c_str());

	if (!pRenderModel)
	{
		std::string sTrackingSystemName = getTrackedDeviceString(unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String);
		printf("Unable to load render model for tracked device %d (%s.%s)", unTrackedDeviceIndex, sTrackingSystemName.c_str(), sRenderModelName.c_str());
	}
	else
	{
		m_rpTrackedDevices[unTrackedDeviceIndex]->setRenderModel(pRenderModel);
		m_rTrackedDeviceToRenderModel[unTrackedDeviceIndex] = pRenderModel;
	}

	if (m_pHMD->GetTrackedDeviceClass(unTrackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
	{
		if (!m_pEditController)
		{
			m_pEditController = new ViveController(unTrackedDeviceIndex);

			if (pRenderModel)
				m_pEditController->setRenderModel(pRenderModel);
		}
		else if(!m_pManipController)
		{
			m_pManipController = new ViveController(unTrackedDeviceIndex);

			if (pRenderModel)
				m_pManipController->setRenderModel(pRenderModel);
		}

	}
}

//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
std::string TrackedDeviceManager::getTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError)
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
		if (!m_rTrackedDeviceToRenderModel[unTrackedDevice])
			continue;

		if (!m_rpTrackedDevices[unTrackedDevice]->poseValid())
			continue;

		//if (bIsInputCapturedByAnotherProcess && m_pHMD->GetTrackedDeviceClass(unTrackedDevice) == vr::TrackedDeviceClass_Controller)
		//	continue;

		const Matrix4 & matDeviceToTracking = m_rpTrackedDevices[unTrackedDevice]->getPose();
		Matrix4 matMVP = matVP * matDeviceToTracking;
		glUniformMatrix4fv(m_nRenderModelMatrixLocation, 1, GL_FALSE, matMVP.get());

		m_rTrackedDeviceToRenderModel[unTrackedDevice]->Draw();
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