#include "TrackedDevice.h"
#include "ShaderUtils.h"


TrackedDevice::TrackedDevice(vr::TrackedDeviceIndex_t id, vr::IVRSystem *pHMD, vr::IVRRenderModels *pRenderModels)
	: m_unDeviceID(id)
	, m_pHMD(pHMD)
	, m_pRenderModels(pRenderModels)
	, m_pTrackedDeviceToRenderModel(NULL)
	, m_strRenderModelName("No model name")
	, m_ClassChar(0)
	, m_unTransformProgramID(0)
	, m_glVertBuffer(0)
	, m_uiLineVertcount(0)
	, m_uiTriVertcount(0)
	, m_unVAO(0)
	, m_nMatrixLocation(-1)
	, m_unRenderModelProgramID(0)
	, m_nRenderModelMatrixLocation(-1)
	, m_bShow(true)
	, m_bShowAxes(false)
{	
}


TrackedDevice::~TrackedDevice()
{
	if (m_unVAO != 0)
	{
		glDeleteVertexArrays(1, &m_unVAO);
	}
	if (m_unTransformProgramID)
	{
		glDeleteProgram(m_unTransformProgramID);
	}
	if (m_unRenderModelProgramID)
	{
		glDeleteProgram(m_unRenderModelProgramID);
	}
}

bool TrackedDevice::BInit()
{
	std::string strRenderModelName = getPropertyString(vr::Prop_RenderModelName_String);

	CGLRenderModel *pRenderModel = loadRenderModel(strRenderModelName.c_str());

	if (!pRenderModel)
	{
		std::string sTrackingSystemName = getPropertyString(vr::Prop_TrackingSystemName_String);
		printf("Unable to load render model for tracked device %d (%s.%s)", m_unDeviceID, sTrackingSystemName.c_str(), strRenderModelName.c_str());
		return false;
	}
	else
	{
		m_strRenderModelName = pRenderModel->GetName();

		std::cout << "Device " << m_unDeviceID << "'s RenderModel name is " << m_strRenderModelName << std::endl;
		setRenderModel(pRenderModel);
	}

	createShaders();

	return true;
}

vr::TrackedDeviceIndex_t TrackedDevice::getIndex()
{
	return m_unDeviceID;
}

void TrackedDevice::setRenderModel(CGLRenderModel * renderModel)
{
	m_pTrackedDeviceToRenderModel = renderModel;
	m_strRenderModelName = renderModel->GetName();
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

//-----------------------------------------------------------------------------
// Purpose: Draw all of the line-based controller augmentations
//-----------------------------------------------------------------------------
void TrackedDevice::prepareForRendering()
{
	std::vector<float> vertdataarray;

	m_uiLineVertcount = 0;

	if (!poseValid())
		return;

	const Matrix4 & mat = getPose();

	// Draw Axes
	if (axesActive())
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

			m_uiLineVertcount += 2;
		}
	}

	// Setup the VAO the first time through.
	if (m_unVAO == 0)
	{
		glGenVertexArrays(1, &m_unVAO);
		glBindVertexArray(m_unVAO);

		glGenBuffers(1, &m_glVertBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);

		GLuint stride = 2 * 3 * sizeof(float);
		GLuint offset = 0;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof(Vector3);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray(0);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);

	// set vertex data if we have some
	if (vertdataarray.size() > 0)
	{
		//$ TODO: Use glBufferSubData for this...
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW);
	}
}

void TrackedDevice::render(Matrix4 & matVP)
{
	// draw the controller axis lines
	glUseProgram(m_unTransformProgramID);
	glUniformMatrix4fv(m_nMatrixLocation, 1, GL_FALSE, matVP.get());
	glBindVertexArray(m_unVAO);
	glDrawArrays(GL_LINES, 0, m_uiLineVertcount);
	glDrawArrays(GL_TRIANGLES, m_uiLineVertcount, m_uiTriVertcount);
	glBindVertexArray(0);
}

void TrackedDevice::renderModel(Matrix4 & matVP)
{
	if (!hasRenderModel() || !poseValid())
		return;

	// ----- Render Model rendering -----
	glUseProgram(m_unRenderModelProgramID);

	const Matrix4 & matDeviceToTracking = getPose();
	Matrix4 matMVP = matVP * matDeviceToTracking;
	glUniformMatrix4fv(m_nRenderModelMatrixLocation, 1, GL_FALSE, matMVP.get());

	m_pTrackedDeviceToRenderModel->Draw();	

	glUseProgram(0);
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

//-----------------------------------------------------------------------------
// Purpose: Converts our local matrix class to a SteamVR matrix
//-----------------------------------------------------------------------------
vr::HmdMatrix34_t TrackedDevice::ConvertMatrix4ToSteamVRMatrix(const Matrix4 &matPose)
{
	vr::HmdMatrix34_t matrixObj;

	memset(&matrixObj, 0, sizeof(matrixObj));
	matrixObj.m[0][0] = matPose.get()[0]; matrixObj.m[1][0] = matPose.get()[1]; matrixObj.m[2][0] = matPose.get()[2];
	matrixObj.m[0][1] = matPose.get()[4]; matrixObj.m[1][1] = matPose.get()[5]; matrixObj.m[2][1] = matPose.get()[6];
	matrixObj.m[0][2] = matPose.get()[8]; matrixObj.m[1][2] = matPose.get()[9]; matrixObj.m[2][2] = matPose.get()[10];
	matrixObj.m[0][3] = matPose.get()[12]; matrixObj.m[1][3] = matPose.get()[13]; matrixObj.m[2][3] = matPose.get()[14];

	return matrixObj;
}

bool TrackedDevice::createShaders()
{
	m_unTransformProgramID = CompileGLShader(
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
	m_nMatrixLocation = glGetUniformLocation(m_unTransformProgramID, "matrix");
	if (m_nMatrixLocation == -1)
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

	return m_unRenderModelProgramID != 0
		&& m_unTransformProgramID != 0;
}

//-----------------------------------------------------------------------------
// Purpose: Finds a render model we've already loaded or loads a new one
//-----------------------------------------------------------------------------
CGLRenderModel* TrackedDevice::loadRenderModel(const char *pchRenderModelName)
{
	CGLRenderModel *pRenderModel = NULL;
	
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

	return pRenderModel;
}

//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
std::string TrackedDevice::getPropertyString(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError)
{
	vr::EVRInitError eError = vr::VRInitError_None;

	vr::IVRSystem *pHMD = (vr::IVRSystem *)vr::VR_GetGenericInterface(vr::IVRSystem_Version, &eError);

	uint32_t unRequiredBufferLen = pHMD->GetStringTrackedDeviceProperty(m_unDeviceID, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHMD->GetStringTrackedDeviceProperty(m_unDeviceID, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}

uint32_t TrackedDevice::getPropertyInt32(vr::TrackedDeviceProperty prop, vr::TrackedPropertyError * peError)
{
	vr::EVRInitError eError = vr::VRInitError_None;
	vr::IVRSystem *pHMD = (vr::IVRSystem *)vr::VR_GetGenericInterface(vr::IVRSystem_Version, &eError);
	
	return pHMD->GetInt32TrackedDeviceProperty(m_unDeviceID, prop, peError);;
}
