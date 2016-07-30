#include "TrackedDeviceManager.h"

TrackedDeviceManager::TrackedDeviceManager(vr::IVRSystem* pHMD)
: m_pHMD(pHMD)
, m_unControllerTransformProgramID(0)
, m_unRenderModelProgramID(0)
, m_iTrackedControllerCount(0)
, m_iTrackedControllerCount_Last(-1)
, m_iValidPoseCount(0)
, m_iValidPoseCount_Last(-1)
, m_strPoseClasses("")
, m_glControllerVertBuffer(0)
, m_unControllerVAO(0)
, m_nControllerMatrixLocation(-1)
, m_nRenderModelMatrixLocation(-1)
, cursorRadius(0.05f)
, cursorRadiusMin(0.005f)
, cursorRadiusMax(0.1f)
, cursorOffsetDirection(Vector4(0.f, 0.f, -1.f, 0.f))
, cursorOffsetAmount(0.1f)
, cursorOffsetAmountMin(0.1f)
, cursorOffsetAmountMax(1.5f)
{

	// other initialization tasks are done in init
	memset(m_rDevClassChar, 0, sizeof(m_rDevClassChar));
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

	if (m_unControllerVAO != 0)
	{
		glDeleteVertexArrays(1, &m_unControllerVAO);
	}
	if (m_unControllerTransformProgramID)
	{
		glDeleteProgram(m_unControllerTransformProgramID);
	}
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
	
	prepareControllersForRendering();
}
//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------
void TrackedDeviceManager::processVREvent(const vr::VREvent_t & event)
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
		setupRenderModelForTrackedDevice(event.trackedDeviceIndex);
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
		setupRenderModelForTrackedDevice(event.trackedDeviceIndex);
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

float TrackedDeviceManager::getCleaningCursorData(Matrix4 *thisCursorPose, Matrix4 *lastCursorPose)
{
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
		{
			if (m_pHMD->GetTrackedDeviceClass(nDevice) == vr::TrackedDeviceClass_Controller)
			{
				*thisCursorPose = m_rmat4DeviceCursorCurrentPose[nDevice];
				*lastCursorPose = m_rmat4DeviceCursorLastPose[nDevice];
				cursorRadius = this->cursorRadius;
				break;
			}
		}
	}
	return cursorRadius;
}

void TrackedDeviceManager::cleaningHit()
{
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_pHMD->GetTrackedDeviceClass(nDevice) == vr::TrackedDeviceClass_Controller)
		{
			m_pHMD->TriggerHapticPulse(nDevice, 0, 2000);
		}
	}
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



//-----------------------------------------------------------------------------
// Purpose: Draw all of the controllers as X/Y/Z lines
//-----------------------------------------------------------------------------
void TrackedDeviceManager::prepareControllersForRendering()
{
	// don't draw controllers if somebody else has input focus
	if (m_pHMD->IsInputFocusCapturedByAnotherProcess())
		return;

	std::vector<float> vertdataarray;

	m_uiControllerVertcount = 0;
	m_iTrackedControllerCount = 0;

	for (vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice)
	{
		if (!m_pHMD->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		if (m_pHMD->GetTrackedDeviceClass(unTrackedDevice) != vr::TrackedDeviceClass_Controller)
			continue;

		m_iTrackedControllerCount += 1;

		if (!m_rTrackedDevicePose[unTrackedDevice].bPoseIsValid)
			continue;

		const Matrix4 & mat = m_rmat4DevicePose[unTrackedDevice];

		// Draw Axes
		if (m_rpTrackedDevices[unTrackedDevice]->axesActive())
		{
			for (int i = 0; i < 3; ++i)
			{
				Vector3 color(0, 0, 0);
				Vector4 center = mat * Vector4(0, 0, 0, 1);
				Vector4 point(0, 0, 0, 1);
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

				vertdataarray.push_back(point.x);
				vertdataarray.push_back(point.y);
				vertdataarray.push_back(point.z);

				vertdataarray.push_back(color.x);
				vertdataarray.push_back(color.y);
				vertdataarray.push_back(color.z);

				m_uiControllerVertcount += 2;
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
		//	m_uiControllerVertcount += 2;
		//}

		Matrix4 & cursorMat = m_rmat4DeviceCursorCurrentPose[unTrackedDevice];

		// Draw cursor hoop
		if (m_rpTrackedDevices[unTrackedDevice]->cursorActive())
		{
			GLuint num_segments = 64;

			Vector3 color;
			if (m_rpTrackedDevices[unTrackedDevice]->cleaningActive())
				color = Vector3(1.f, 0.f, 0.f);
			else
				color = Vector3(0.8f, 0.8f, 0.2f);

			Vector4 prevVert = cursorMat * Vector4(cursorRadius, 0.f, 0.f, 1.f);
			for (GLuint i = 1; i < num_segments; i++)
			{
				GLfloat theta = 2.f * 3.14159f * static_cast<GLfloat>(i) / static_cast<GLfloat>(num_segments - 1);

				Vector4 circlePt;
				circlePt.x = cursorRadius * cosf(theta);
				circlePt.y = 0.f;
				circlePt.z = cursorRadius * sinf(theta);
				circlePt.w = 1.f;

				Vector4 thisVert = cursorMat * circlePt;

				vertdataarray.push_back(prevVert.x); vertdataarray.push_back(prevVert.y); vertdataarray.push_back(prevVert.z);
				vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

				vertdataarray.push_back(thisVert.x); vertdataarray.push_back(thisVert.y); vertdataarray.push_back(thisVert.z);
				vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

				m_uiControllerVertcount += 2;

				prevVert = thisVert;
			}

			// DISPLAY CURSOR DOT
			//if (!m_rbTrackedDeviceCleaningMode[unTrackedDevice])
			{
				color = Vector3(1.f, 0.f, 0.f);
				if (m_rpTrackedDevices[unTrackedDevice]->cursorActive())
				{
					Vector4 thisCtrPos = cursorMat * Vector4(0.f, 0.f, 0.f, 1.f);
					Vector4 lastCtrPos = m_rmat4DeviceCursorLastPose[unTrackedDevice] * Vector4(0.f, 0.f, 0.f, 1.f);

					vertdataarray.push_back(lastCtrPos.x);
					vertdataarray.push_back(lastCtrPos.y);
					vertdataarray.push_back(lastCtrPos.z);
					vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

					vertdataarray.push_back(thisCtrPos.x);
					vertdataarray.push_back(thisCtrPos.y);
					vertdataarray.push_back(thisCtrPos.z);
					vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

					m_uiControllerVertcount += 2;
				}
			}

			// DISPLAY CONNECTING LINE TO CURSOR
			//if (!m_rbTrackedDeviceCleaningMode[unTrackedDevice])
			{
				color = Vector3(1.f, 1.f, 1.f);
				if (m_rpTrackedDevices[unTrackedDevice]->cursorActive())
				{
					Vector4 controllerCtr = mat * Vector4(0.f, 0.f, 0.f, 1.f);
					Vector4 cursorEdge = cursorMat * Vector4(0.f, 0.f, cursorRadius, 1.f);

					vertdataarray.push_back(cursorEdge.x);
					vertdataarray.push_back(cursorEdge.y);
					vertdataarray.push_back(cursorEdge.z);
					vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

					vertdataarray.push_back(controllerCtr.x);
					vertdataarray.push_back(controllerCtr.y);
					vertdataarray.push_back(controllerCtr.z);
					vertdataarray.push_back(color.x); vertdataarray.push_back(color.y); vertdataarray.push_back(color.z);

					m_uiControllerVertcount += 2;
				}
			}
		}
	}

	// Setup the VAO the first time through.
	if (m_unControllerVAO == 0)
	{
		glGenVertexArrays(1, &m_unControllerVAO);
		glBindVertexArray(m_unControllerVAO);

		glGenBuffers(1, &m_glControllerVertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_glControllerVertBuffer);

		GLuint stride = 2 * 3 * sizeof(float);
		GLuint offset = 0;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof(Vector3);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray(0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_glControllerVertBuffer);

	// set vertex data if we have some
	if (vertdataarray.size() > 0)
	{
		//$ TODO: Use glBufferSubData for this...
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW);
	}
}

void TrackedDeviceManager::renderTrackedDevices(Matrix4 & matVP)
{
	renderControllers(matVP);
	renderDeviceModels(matVP);
}

void TrackedDeviceManager::renderControllers(Matrix4 & matVP)
{
	// draw the controller axis lines
	glUseProgram(m_unControllerTransformProgramID);
	glUniformMatrix4fv(m_nControllerMatrixLocation, 1, GL_FALSE, matVP.get());
	glBindVertexArray(m_unControllerVAO);
	glDrawArrays(GL_LINES, 0, m_uiControllerVertcount);
	glBindVertexArray(0);
}

void TrackedDeviceManager::renderDeviceModels(Matrix4 & matVP)
{
	// ----- Render Model rendering -----
	glUseProgram(m_unRenderModelProgramID);

	for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_rTrackedDeviceToRenderModel[unTrackedDevice])
			continue;

		const vr::TrackedDevicePose_t & pose = m_rTrackedDevicePose[unTrackedDevice];
		if (!pose.bPoseIsValid)
			continue;

		//if (bIsInputCapturedByAnotherProcess && m_pHMD->GetTrackedDeviceClass(unTrackedDevice) == vr::TrackedDeviceClass_Controller)
		//	continue;

		const Matrix4 & matDeviceToTracking = m_rmat4DevicePose[unTrackedDevice];
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

	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	m_iValidPoseCount = 0;
	m_strPoseClasses = "";
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
		{
			m_iValidPoseCount++;
			m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
			m_rmat4DeviceCursorLastPose[nDevice] = m_rmat4DeviceCursorCurrentPose[nDevice];
			m_rmat4DeviceCursorCurrentPose[nDevice] = m_rmat4DevicePose[nDevice] * (Matrix4().identity()).translate(
				Vector3(cursorOffsetDirection.x, cursorOffsetDirection.y, cursorOffsetDirection.z) * cursorOffsetAmount);

			if (m_rDevClassChar[nDevice] == 0)
			{
				switch (m_pHMD->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_Other:             m_rDevClassChar[nDevice] = 'O'; break;
				case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
				default:                                       m_rDevClassChar[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];
		}
	}

	if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd].invert();
	}
}

bool TrackedDeviceManager::createShaders()
{
	m_unControllerTransformProgramID = CompileGLShader(
		"Controller",

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3ColorIn;\n"
		"out vec4 v4Color;\n"
		"void main()\n"
		"{\n"
		"	v4Color.xyz = v3ColorIn; v4Color.a = 1.0;\n"
		"	gl_Position = matrix * position;\n"
		"}\n",

		// fragment shader
		"#version 410\n"
		"in vec4 v4Color;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = v4Color;\n"
		"}\n"
	);
	m_nControllerMatrixLocation = glGetUniformLocation(m_unControllerTransformProgramID, "matrix");
	if (m_nControllerMatrixLocation == -1)
	{
		printf("Unable to find matrix uniform in controller shader\n");
		return false;
	}

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

	return m_unControllerTransformProgramID != 0
		&& m_unRenderModelProgramID != 0;
}

GLuint TrackedDeviceManager::CompileGLShader(const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader)
{
	GLuint unProgramID = glCreateProgram();

	GLuint nSceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(nSceneVertexShader, 1, &pchVertexShader, NULL);
	glCompileShader(nSceneVertexShader);

	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv(nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if (vShaderCompiled != GL_TRUE)
	{
		printf("%s - Unable to compile vertex shader %d!\n", pchShaderName, nSceneVertexShader);
		glDeleteProgram(unProgramID);
		glDeleteShader(nSceneVertexShader);
		return 0;
	}
	glAttachShader(unProgramID, nSceneVertexShader);
	glDeleteShader(nSceneVertexShader); // the program hangs onto this once it's attached

	GLuint  nSceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(nSceneFragmentShader, 1, &pchFragmentShader, NULL);
	glCompileShader(nSceneFragmentShader);

	GLint fShaderCompiled = GL_FALSE;
	glGetShaderiv(nSceneFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
	if (fShaderCompiled != GL_TRUE)
	{
		printf("%s - Unable to compile fragment shader %d!\n", pchShaderName, nSceneFragmentShader);
		glDeleteProgram(unProgramID);
		glDeleteShader(nSceneFragmentShader);
		return 0;
	}

	glAttachShader(unProgramID, nSceneFragmentShader);
	glDeleteShader(nSceneFragmentShader); // the program hangs onto this once it's attached

	glLinkProgram(unProgramID);

	GLint programSuccess = GL_TRUE;
	glGetProgramiv(unProgramID, GL_LINK_STATUS, &programSuccess);
	if (programSuccess != GL_TRUE)
	{
		printf("%s - Error linking program %d!\n", pchShaderName, unProgramID);
		glDeleteProgram(unProgramID);
		return 0;
	}

	glUseProgram(unProgramID);
	glUseProgram(0);

	return unProgramID;
}

//-----------------------------------------------------------------------------
// Purpose: Converts a SteamVR matrix to our local matrix class
//-----------------------------------------------------------------------------
Matrix4 TrackedDeviceManager::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
{
	Matrix4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	return matrixObj;
}