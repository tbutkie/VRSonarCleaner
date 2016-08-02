#include "TrackedDevice.h"
#include "ShaderUtils.h"


TrackedDevice::TrackedDevice(vr::TrackedDeviceIndex_t id)
	: m_DeviceID(id)
	, m_pTrackedDeviceToRenderModel(NULL)
	, m_ClassChar(0)
	, m_unControllerTransformProgramID(0)
	, m_glControllerVertBuffer(0)
	, m_uiControllerVertcount(0)
	, m_unControllerVAO(0)
	, m_nControllerMatrixLocation(-1)
	, m_bShow(true)
	, m_bShowAxes(false)
{	

	createShader();
}


TrackedDevice::~TrackedDevice()
{
	if (m_unControllerVAO != 0)
	{
		glDeleteVertexArrays(1, &m_unControllerVAO);
	}
	if (m_unControllerTransformProgramID)
	{
		glDeleteProgram(m_unControllerTransformProgramID);
	}
}

vr::TrackedDeviceIndex_t TrackedDevice::getIndex()
{
	return m_DeviceID;
}

void TrackedDevice::setRenderModel(CGLRenderModel * renderModel)
{
	m_pTrackedDeviceToRenderModel = renderModel;
}

bool TrackedDevice::createShader()
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

	return m_unControllerTransformProgramID != 0;
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

char TrackedDevice::getClassChar()
{
	return m_ClassChar;
}

void TrackedDevice::setClassChar(char classChar)
{
	m_ClassChar = classChar;
}

Matrix4 TrackedDevice::getPose()
{
	return m_mat4Pose;
}

bool TrackedDevice::updatePose(vr::TrackedDevicePose_t pose)
{
	m_Pose = pose;
	m_mat4Pose = ConvertSteamVRMatrixToMatrix4(m_Pose.mDeviceToAbsoluteTracking);

	return m_Pose.bPoseIsValid;
}

void TrackedDevice::renderModel()
{
	if(m_pTrackedDeviceToRenderModel)
		m_pTrackedDeviceToRenderModel->Draw();
}

void TrackedDevice::render(Matrix4 & matVP)
{
	// draw the controller axis lines
	glUseProgram(m_unControllerTransformProgramID);
	glUniformMatrix4fv(m_nControllerMatrixLocation, 1, GL_FALSE, matVP.get());
	glBindVertexArray(m_unControllerVAO);
	glDrawArrays(GL_LINES, 0, m_uiControllerVertcount);
	glBindVertexArray(0);
}

//-----------------------------------------------------------------------------
// Purpose: Converts a SteamVR matrix to our local matrix class
//-----------------------------------------------------------------------------
Matrix4 TrackedDevice::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
{
	Matrix4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	return matrixObj;
}