#include "TrackedDevice.h"

#include <shared/glm/gtc/type_ptr.hpp>

TrackedDevice::TrackedDevice(vr::TrackedDeviceIndex_t id, vr::IVRSystem *pHMD, vr::IVRRenderModels * pRenderModels)
	: m_unDeviceID(id)
	, m_pHMD(pHMD)
	, m_pRenderModels(pRenderModels)
	, m_strRenderModelName("No model name")
	, m_bHasRenderModel(false)
	, m_ClassChar(0)
	, m_bShowAxes(false)
	, m_bHidden(false)
{	
}


TrackedDevice::~TrackedDevice()
{
}

bool TrackedDevice::BInit()
{
	return true;
}

vr::TrackedDeviceIndex_t TrackedDevice::getIndex()
{
	return m_unDeviceID;
}

void TrackedDevice::setRenderModelName(std::string renderModelName)
{
	m_strRenderModelName = renderModelName;
	m_bHasRenderModel = true;
}

bool TrackedDevice::hasRenderModel()
{
	return m_bHasRenderModel;
}

bool TrackedDevice::toggleAxes()
{
	return m_bShowAxes = !m_bShowAxes;
}

bool TrackedDevice::axesActive()
{
	return m_bShowAxes;
}

bool TrackedDevice::poseValid()
{
	return m_Pose.bPoseIsValid;
}

glm::mat4 TrackedDevice::getPose()
{
	return ConvertSteamVRMatrixToMatrix4(m_Pose.mDeviceToAbsoluteTracking);
}

char TrackedDevice::getClassChar()
{
	return m_ClassChar;
}

void TrackedDevice::setClassChar(char classChar)
{
	m_ClassChar = classChar;
}

glm::mat4 TrackedDevice::getDeviceToWorldTransform()
{
	return m_mat4DeviceToWorldTransform;
}

bool TrackedDevice::update(vr::TrackedDevicePose_t pose)
{
	m_Pose = pose;
	m_mat4DeviceToWorldTransform = ConvertSteamVRMatrixToMatrix4(m_Pose.mDeviceToAbsoluteTracking);

	return m_Pose.bPoseIsValid;
}

//-----------------------------------------------------------------------------
// Purpose: Converts a SteamVR matrix to our local matrix class
//-----------------------------------------------------------------------------
glm::mat4 TrackedDevice::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
{
	glm::mat4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	return matrixObj;
}

//-----------------------------------------------------------------------------
// Purpose: Converts our local matrix class to a SteamVR matrix
//-----------------------------------------------------------------------------
vr::HmdMatrix34_t TrackedDevice::ConvertMatrix4ToSteamVRMatrix(const glm::mat4 &matPose)
{
	vr::HmdMatrix34_t matrixObj;

	memset(&matrixObj, 0, sizeof(matrixObj));
	matrixObj.m[0][0] = glm::value_ptr(matPose)[0]; matrixObj.m[1][0] = glm::value_ptr(matPose)[1]; matrixObj.m[2][0] = glm::value_ptr(matPose)[2];
	matrixObj.m[0][1] = glm::value_ptr(matPose)[4]; matrixObj.m[1][1] = glm::value_ptr(matPose)[5]; matrixObj.m[2][1] = glm::value_ptr(matPose)[6];
	matrixObj.m[0][2] = glm::value_ptr(matPose)[8]; matrixObj.m[1][2] = glm::value_ptr(matPose)[9]; matrixObj.m[2][2] = glm::value_ptr(matPose)[10];
	matrixObj.m[0][3] = glm::value_ptr(matPose)[12]; matrixObj.m[1][3] = glm::value_ptr(matPose)[13]; matrixObj.m[2][3] = glm::value_ptr(matPose)[14];

	return matrixObj;
}

uint32_t TrackedDevice::getPropertyInt32(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError * peError)
{
	vr::EVRInitError eError = vr::VRInitError_None;
	vr::IVRSystem *pHMD = (vr::IVRSystem *)vr::VR_GetGenericInterface(vr::IVRSystem_Version, &eError);
	
	return pHMD->GetInt32TrackedDeviceProperty(m_unDeviceID, prop, peError);;
}
