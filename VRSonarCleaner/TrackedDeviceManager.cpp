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

bool TrackedDeviceManager::init()
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

void TrackedDeviceManager::hideBaseStations(bool hidden)
{
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
		if (m_pHMD->IsTrackedDeviceConnected(nDevice) && m_rpTrackedDevices[nDevice]->getClassChar() == 'T')
			m_rpTrackedDevices[nDevice]->m_bHidden = hidden;
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
	{
		thisDevice = new ViveController(unTrackedDeviceIndex, m_pHMD, m_pRenderModels);

		if (!m_pPrimaryController)
			m_pPrimaryController = static_cast<ViveController*>(thisDevice);
		else if (!m_pSecondaryController)
			m_pSecondaryController = static_cast<ViveController*>(thisDevice);
	}
	else
	{
		thisDevice = new TrackedDevice(unTrackedDeviceIndex, m_pHMD, m_pRenderModels);
	}

	thisDevice->BInit();	
	
	m_rpTrackedDevices[unTrackedDeviceIndex] = thisDevice;

	if (thisDevice->getClassChar() == 'C')
	{
		// attach listeners to controller
		for (auto &l : m_vpListeners)
			thisDevice->attach(l);
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

	m_rpTrackedDevices[unTrackedDeviceIndex] = NULL;
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

			if (m_rpTrackedDevices[nDevice]->getClassChar() == 'C')
				m_iTrackedControllerCount++;
			
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
		m_mat4HMDToWorldTransform = m_rpTrackedDevices[vr::k_unTrackedDeviceIndex_Hmd]->getDeviceToWorldTransform();
		m_mat4WorldToHMDTransform = glm::inverse(m_mat4HMDToWorldTransform);

		//glm::mat4 HMDtoWorldMat = m_rpTrackedDevices[vr::k_unTrackedDeviceIndex_Hmd]->getDeviceToWorldTransform();
		//glm::vec3 HMDpos = glm::vec3(HMDtoWorldMat[3]);
		//float widthX, widthZ;
		//vr::VRChaperone()->GetPlayAreaSize(&widthX, &widthZ);
		//
		//BroadcastSystem::Payload::HMD payload = {
		//	m_rpTrackedDevices[vr::k_unTrackedDeviceIndex_Hmd],
		//	HMDtoWorldMat
		//};
		//
		//if (abs(HMDpos.x) > widthX || abs(HMDpos.z) > widthZ)
		//{
		//	notify(BroadcastSystem::EVENT::EXIT_PLAY_AREA, &payload);
		//}
		//else
		//{
		//	notify(BroadcastSystem::EVENT::ENTER_PLAY_AREA, &payload);
		//}
	}
}


void TrackedDeviceManager::draw()
{
	for (uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_rpTrackedDevices[unTrackedDevice] ||
			m_rpTrackedDevices[unTrackedDevice]->m_bHidden ||
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

					RenderModel* rm = findOrLoadRenderModel(m_rpTrackedDevices[unTrackedDevice]->m_vComponents[i].m_strComponentRenderModelName.c_str());
					
					Renderer::RendererSubmission rs;
					rs.shaderName = "lighting";
					rs.VAO = rm->getVAO();
					rs.vertCount = rm->getVertexCount();
					rs.primitiveType = GL_TRIANGLES;
					rs.diffuseTex = rm->getDiffuseTexture();
					rs.specularTex = rm->getSpecularTexture();
					rs.specularExponent = rm->getMaterialShininess();
					rs.modelToWorldTransform = matModel;
					Renderer::getInstance().addToDynamicRenderQueue(rs);
				}
		}
		else // render model without components
		{
			glm::mat4 matModel = m_rpTrackedDevices[unTrackedDevice]->getDeviceToWorldTransform();

			RenderModel* rm = findOrLoadRenderModel(m_rpTrackedDevices[unTrackedDevice]->m_strRenderModelName.c_str());

			Renderer::RendererSubmission rs;
			rs.shaderName = "lighting";
			rs.VAO = rm->getVAO();
			rs.vertCount = rm->getVertexCount();
			rs.primitiveType = GL_TRIANGLES;
			rs.diffuseTex = rm->getDiffuseTexture();
			rs.specularTex = rm->getSpecularTexture();
			rs.specularExponent = rm->getMaterialShininess();
			rs.modelToWorldTransform = matModel;
			Renderer::getInstance().addToDynamicRenderQueue(rs);
		}
	}
}

glm::mat4 & TrackedDeviceManager::getWorldToHMDTransform()
{
	return m_mat4WorldToHMDTransform;
}

glm::mat4 & TrackedDeviceManager::getHMDToWorldTransform()
{
	return m_mat4HMDToWorldTransform;
}

// Returns vr::k_unTrackedDeviceIndexInvalid if not found
uint32_t TrackedDeviceManager::getDeviceComponentID(uint32_t deviceID, std::string componentName)
{
	for (auto c : m_rpTrackedDevices[deviceID]->m_vComponents)
		if (c.m_strComponentName.compare(componentName) == 0)
			return c.m_unComponentIndex;

	return vr::k_unTrackedDeviceIndexInvalid;
}

glm::mat4 TrackedDeviceManager::getDeviceComponentPose(uint32_t deviceID, uint32_t componentID)
{
	vr::TrackedDevicePose_t p;
	m_pHMD->ApplyTransform(
		&p,
		&(m_rpTrackedDevices[deviceID]->m_Pose),
		&(m_rpTrackedDevices[deviceID]->m_vComponents[componentID].m_State.mTrackingToComponentLocal)
	);
	return m_rpTrackedDevices[deviceID]->ConvertSteamVRMatrixToMatrix4(p.mDeviceToAbsoluteTracking);
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


//-----------------------------------------------------------------------------
// Purpose: Finds a render model we've already loaded or loads a new one
//-----------------------------------------------------------------------------
RenderModel* TrackedDeviceManager::findOrLoadRenderModel(const char *pchRenderModelName)
{
	// check model cache for existing model
	RenderModel *pRenderModel = m_mapModelCache[std::string(pchRenderModelName)];

	// found model in the cache, so return it
	if (pRenderModel)
	{
		//printf("Found existing render model for %s\n", pchRenderModelName);
		return pRenderModel;
	}

	vr::RenderModel_t *pModel;
	vr::EVRRenderModelError error;
	while (1)
	{
		error = vr::VRRenderModels()->LoadRenderModel_Async(pchRenderModelName, &pModel);
		if (error != vr::VRRenderModelError_Loading)
			break;

		Sleep(1);
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

	pRenderModel = new RenderModel(pchRenderModelName);
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

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
glm::mat4 TrackedDeviceManager::getHMDEyeProjection(vr::Hmd_Eye nEye, float nearClipPlane, float farClipPlane)
{
	if (!m_pHMD)
		return glm::mat4();

	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, nearClipPlane, farClipPlane);

	return glm::mat4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
glm::mat4 TrackedDeviceManager::getHMDEyeToHeadTransform(vr::Hmd_Eye nEye)
{
	if (!m_pHMD)
		return glm::mat4();

	vr::HmdMatrix34_t matEyeToHead = m_pHMD->GetEyeToHeadTransform(nEye);

	return glm::mat4(
		matEyeToHead.m[0][0], matEyeToHead.m[1][0], matEyeToHead.m[2][0], 0.f,
		matEyeToHead.m[0][1], matEyeToHead.m[1][1], matEyeToHead.m[2][1], 0.f,
		matEyeToHead.m[0][2], matEyeToHead.m[1][2], matEyeToHead.m[2][2], 0.f,
		matEyeToHead.m[0][3], matEyeToHead.m[1][3], matEyeToHead.m[2][3], 1.f
	);
}