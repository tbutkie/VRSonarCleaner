#include "Renderer.h"

#include <vector>
#include <shared/glm/glm.hpp>
#include <shared/glm/gtc/type_ptr.hpp>

#include "DebugDrawer.h"
#include "InfoBoxManager.h"
#include "ShaderUtils.h"

#include "preamble.glsl"

Renderer::Renderer()
	: m_pHMD(NULL)
	, m_pTDM(NULL)
	, m_pLighting(NULL)
	, m_unRenderModelProgramID(0)
	, m_nRenderModelmatMVPLocation(-1)
	, m_nRenderModelmatMVLocation(-1)
	, m_nRenderModelvec3LightDirLocation(-1)
	, m_punCompanionWindowProgramID(NULL)
	, m_unCompanionWindowVAO(0)
	, m_bVblank(false)
	, m_bGlFinishHack(true)
	, m_fNearClip(0.1f)
	, m_fFarClip(30.0f)
{
}


Renderer::~Renderer()
{
	Shutdown();
}

bool Renderer::init(vr::IVRSystem *pHMD, TrackedDeviceManager *pTDM, LightingSystem *pLS)
{
	m_pHMD = pHMD;
	m_pTDM = pTDM;
	m_pLighting = pLS;
	
	if (SDL_GL_SetSwapInterval(m_bVblank ? 1 : 0) < 0)
	{
		printf("%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	m_Shaders.SetVersion("450");

	m_Shaders.SetPreambleFile("preamble.glsl");

	if (!CreateCompanionWindowShader() ||
		!CreateRenderModelShader())
		return false;

	SetupCameras();
	SetupStereoRenderTargets();
	SetupCompanionWindow();

	return true;
}

void Renderer::addRenderModelInstance(const char * name, glm::mat4 instancePose)
{
	m_mapModelInstances[std::string(name)].push_back(instancePose);
}

void Renderer::resetRenderModelInstances()
{
	for (auto &rm : m_mapModelInstances)
		rm.second.clear();
}


//-----------------------------------------------------------------------------
// Purpose: Creates all the shaders used by HelloVR SDL
//-----------------------------------------------------------------------------
bool Renderer::CreateCompanionWindowShader()
{
	m_punCompanionWindowProgramID = m_Shaders.AddProgramFromExts({ "shaders/companionWindow.vert", "shaders/companionWindow.frag" });
	
	return m_punCompanionWindowProgramID != NULL;
}

bool Renderer::CreateRenderModelShader()
{
	m_unRenderModelProgramID = CompileGLShader(
		"render model",

		// vertex shader
		"#version 410\n"
		//"#include \"preamble.glsl\"\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3NormalIn;\n"
		"layout(location = 2) in vec2 v2TexCoordsIn;\n"
		"uniform mat4 matMVP;\n"
		"uniform mat4 matMV;\n"
		"out vec3 v3Normal;\n"
		"out vec2 v2TexCoord;\n"
		"void main()\n"
		"{\n"
		"	v3Normal = normalize(mat3(matMV) * v3NormalIn);\n"
		"	v2TexCoord = v2TexCoordsIn;\n"
		"	gl_Position = matMVP * vec4(position.xyz, 1);\n"
		"}\n", 

		//fragment shader
		"#version 410 core\n"
		"uniform vec3 lightDir;\n"
		"uniform sampler2D diffuse;\n"
		"in vec3 v3Normal;\n"
		"in vec2 v2TexCoord;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = max(dot(v3Normal, lightDir), 0.0) * texture( diffuse, v2TexCoord);\n"
		"}\n"

	);

	m_nRenderModelmatMVPLocation = glGetUniformLocation(m_unRenderModelProgramID, "matMVP");
	if (m_nRenderModelmatMVPLocation == -1)
	{
		printf("Unable to find MVP matrix uniform in render model shader\n");
		return false;
	}

	m_nRenderModelmatMVLocation = glGetUniformLocation(m_unRenderModelProgramID, "matMV");
	if (m_nRenderModelmatMVLocation == -1)
	{
		printf("Unable to find modelview matrix uniform in render model shader\n");
		return false;
	}

	m_nRenderModelvec3LightDirLocation = glGetUniformLocation(m_unRenderModelProgramID, "lightDir");
	if (m_nRenderModelvec3LightDirLocation == -1)
	{
		printf("Unable to find light direction uniform in render model shader\n");
		return false;
	}

	return m_unRenderModelProgramID != 0;
}



//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Renderer::CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc)
{
	glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

	glGenTextures(1, &framebufferDesc.m_nRenderTextureId);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	glGenTextures(1, &framebufferDesc.m_nResolveTextureId);
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Renderer::SetupStereoRenderTargets()
{
	if (!m_pHMD)
		return false;

	m_pHMD->GetRecommendedRenderTargetSize(&m_nRenderWidth, &m_nRenderHeight);

	CreateFrameBuffer(m_nRenderWidth, m_nRenderHeight, leftEyeDesc);
	CreateFrameBuffer(m_nRenderWidth, m_nRenderHeight, rightEyeDesc);

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::SetupCompanionWindow()
{
	if (!m_pHMD)
		return;

	std::vector<VertexDataWindow> vVerts;

	// left eye verts
	vVerts.push_back(VertexDataWindow(glm::vec2(-1, -1), glm::vec2(0, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(0, -1), glm::vec2(1, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(-1, 1), glm::vec2(0, 1)));
	vVerts.push_back(VertexDataWindow(glm::vec2(0, 1), glm::vec2(1, 1)));

	// right eye verts
	vVerts.push_back(VertexDataWindow(glm::vec2(0, -1), glm::vec2(0, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(1, -1), glm::vec2(1, 0)));
	vVerts.push_back(VertexDataWindow(glm::vec2(0, 1), glm::vec2(0, 1)));
	vVerts.push_back(VertexDataWindow(glm::vec2(1, 1), glm::vec2(1, 1)));

	GLushort vIndices[] = { 0, 1, 3,   0, 3, 2,   4, 5, 7,   4, 7, 6 };
	m_uiCompanionWindowIndexSize = _countof(vIndices);

	glGenVertexArrays(1, &m_unCompanionWindowVAO);
	glBindVertexArray(m_unCompanionWindowVAO);

	glGenBuffers(1, &m_glCompanionWindowIDVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_glCompanionWindowIDVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, vVerts.size() * sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW);

	glGenBuffers(1, &m_glCompanionWindowIDIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glCompanionWindowIDIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_uiCompanionWindowIndexSize * sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
	glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, position));

	glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
	glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, texCoord));

	glBindVertexArray(0);

	glDisableVertexAttribArray(POSITION_ATTRIB_LOCATION);
	glDisableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::SetupCameras()
{
	m_mat4ProjectionLeft = GetHMDMatrixProjectionEye(vr::Eye_Left);
	m_mat4ProjectionRight = GetHMDMatrixProjectionEye(vr::Eye_Right);
	m_mat4eyePoseLeft = GetHMDMatrixPoseEye(vr::Eye_Left);
	m_mat4eyePoseRight = GetHMDMatrixPoseEye(vr::Eye_Right);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::RenderFrame(SDL_Window *win, glm::mat4 &HMDView)
{
	m_mat4CurrentHMDView = HMDView;
	SDL_GetWindowSize(win, &m_nCompanionWindowWidth, &m_nCompanionWindowHeight);

	m_Shaders.UpdatePrograms();

	// for now as fast as possible
	if (m_pHMD)
	{
		RenderStereoTargets();
		RenderCompanionWindow();

		vr::Texture_t leftEyeTexture = { (void*)leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::Texture_t rightEyeTexture = { (void*)rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
	}

	if (m_bVblank && m_bGlFinishHack)
	{
		//$ HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
		// happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
		// appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
		// 1/29/2014 mikesart
		glFinish();
	}

	// SwapWindow
	{
		SDL_GL_SwapWindow(win);
	}

	// Clear
	{
		// We want to make sure the glFinish waits for the entire present to complete, not just the submission
		// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Flush and wait for swap.
	if (m_bVblank)
	{
		glFlush();
		glFinish();
	}

	DebugDrawer::getInstance().flushLines();
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::RenderStereoTargets()
{
	//glClearColor(0.15f, 0.15f, 0.18f, 1.0f); // nice background color, but not black
	glClearColor(0.33, 0.39, 0.49, 1.0); //VTT4D background
	glEnable(GL_MULTISAMPLE);

	// Left Eye
	glBindFramebuffer(GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);
	RenderScene(vr::Eye_Left);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.m_nResolveFramebufferId);

	glBlitFramebuffer(0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glEnable(GL_MULTISAMPLE);

	// Right Eye
	glBindFramebuffer(GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);
	RenderScene(vr::Eye_Right);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.m_nResolveFramebufferId);

	glBlitFramebuffer(0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::RenderScene(vr::Hmd_Eye nEye)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	glm::mat4 thisEyesViewMatrix = (nEye == vr::Eye_Left ? m_mat4eyePoseLeft : m_mat4eyePoseRight) * m_mat4CurrentHMDView;
	glm::mat4 thisEyesViewProjectionMatrix = (nEye == vr::Eye_Left ? m_mat4ProjectionLeft : m_mat4ProjectionRight) * thisEyesViewMatrix;
	
	m_pTDM->renderControllerCustomizations(&thisEyesViewProjectionMatrix);

	m_pLighting->updateView(thisEyesViewMatrix);

	if (!m_pHMD->IsInputFocusCapturedByAnotherProcess())
	{
		// ----- Render Model rendering -----
		glUseProgram(m_unRenderModelProgramID);

		for (auto &rm : m_mapModelInstances)
		{
			CGLRenderModel *pglRenderModel = findOrLoadRenderModel(rm.first.c_str());

			if (pglRenderModel)
			{
				for (auto const &instancePose : rm.second)
				{
					glUniformMatrix4fv(m_nRenderModelmatMVPLocation, 1, GL_FALSE, glm::value_ptr(thisEyesViewProjectionMatrix * instancePose));
					glUniformMatrix4fv(m_nRenderModelmatMVLocation, 1, GL_FALSE, glm::value_ptr(thisEyesViewMatrix * instancePose));
					glUniform3fv(m_nRenderModelvec3LightDirLocation, 1, glm::value_ptr(glm::normalize(glm::mat3(thisEyesViewMatrix) * glm::vec3(1.f))));
					pglRenderModel->Draw();
				}
			}
			else
			{
				printf("Unable to load render model %s\n", rm.first.c_str());
			}
		}

		glUseProgram(0);
	}
	
	InfoBoxManager::getInstance().render(glm::value_ptr(thisEyesViewProjectionMatrix));

	// DEBUG DRAWER RENDER CALL
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	DebugDrawer::getInstance().render(thisEyesViewProjectionMatrix);
	glDisable(GL_BLEND);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::RenderCompanionWindow()
{
	if (m_punCompanionWindowProgramID == NULL)
		return;

	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, m_nCompanionWindowWidth, m_nCompanionWindowHeight);

	glBindVertexArray(m_unCompanionWindowVAO);
	glUseProgram(*m_punCompanionWindowProgramID);

	// render left eye (first half of index array )
	glBindTexture(GL_TEXTURE_2D, leftEyeDesc.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDrawElements(GL_TRIANGLES, m_uiCompanionWindowIndexSize / 2, GL_UNSIGNED_SHORT, 0);

	// render right eye (second half of index array )
	glBindTexture(GL_TEXTURE_2D, rightEyeDesc.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDrawElements(GL_TRIANGLES, m_uiCompanionWindowIndexSize / 2, GL_UNSIGNED_SHORT, (const void *)(uintptr_t)(m_uiCompanionWindowIndexSize));

	glBindVertexArray(0);
	glUseProgram(0);
}


//-----------------------------------------------------------------------------
// Purpose: Finds a render model we've already loaded or loads a new one
//-----------------------------------------------------------------------------
CGLRenderModel* Renderer::findOrLoadRenderModel(const char *pchRenderModelName)
{
	// check model cache for existing model
	CGLRenderModel *pRenderModel = m_mapModelCache[std::string(pchRenderModelName)];

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

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
glm::mat4 Renderer::GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
{
	if (!m_pHMD)
		return glm::mat4();

	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, m_fNearClip, m_fFarClip);

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
glm::mat4 Renderer::GetHMDMatrixPoseEye(vr::Hmd_Eye nEye)
{
	if (!m_pHMD)
		return glm::mat4();

	vr::HmdMatrix34_t matEyeToHead = m_pHMD->GetEyeToHeadTransform(nEye);
	glm::mat4 matrixObj(
		matEyeToHead.m[0][0], matEyeToHead.m[1][0], matEyeToHead.m[2][0], 0.f,
		matEyeToHead.m[0][1], matEyeToHead.m[1][1], matEyeToHead.m[2][1], 0.f,
		matEyeToHead.m[0][2], matEyeToHead.m[1][2], matEyeToHead.m[2][2], 0.f,
		matEyeToHead.m[0][3], matEyeToHead.m[1][3], matEyeToHead.m[2][3], 1.f
	);

	return glm::inverse(matrixObj);
}

void Renderer::Shutdown()
{
	if (m_unRenderModelProgramID)
	{
		glDeleteProgram(m_unRenderModelProgramID);
	}

	glDeleteBuffers(1, &m_glCompanionWindowIDVertBuffer);
	glDeleteBuffers(1, &m_glCompanionWindowIDIndexBuffer);

	if (m_punCompanionWindowProgramID != NULL)
	{
		glDeleteProgram(*m_punCompanionWindowProgramID);
	}

	glDeleteRenderbuffers(1, &leftEyeDesc.m_nDepthBufferId);
	glDeleteTextures(1, &leftEyeDesc.m_nRenderTextureId);
	glDeleteFramebuffers(1, &leftEyeDesc.m_nRenderFramebufferId);
	glDeleteTextures(1, &leftEyeDesc.m_nResolveTextureId);
	glDeleteFramebuffers(1, &leftEyeDesc.m_nResolveFramebufferId);

	glDeleteRenderbuffers(1, &rightEyeDesc.m_nDepthBufferId);
	glDeleteTextures(1, &rightEyeDesc.m_nRenderTextureId);
	glDeleteFramebuffers(1, &rightEyeDesc.m_nRenderFramebufferId);
	glDeleteTextures(1, &rightEyeDesc.m_nResolveTextureId);
	glDeleteFramebuffers(1, &rightEyeDesc.m_nResolveFramebufferId);

	if (m_unCompanionWindowVAO != 0)
	{
		glDeleteVertexArrays(1, &m_unCompanionWindowVAO);
	}
}
