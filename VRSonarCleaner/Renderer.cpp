#include "Renderer.h"

#include <vector>
#include <shared/glm/glm.hpp>
#include <shared/glm/gtc/type_ptr.hpp>

#include "DebugDrawer.h"
#include "InfoBoxManager.h"

#include "GLSLpreamble.h"

Renderer::Renderer()
	: m_pHMD(NULL)
	, m_pTDM(NULL)
	, m_pLighting(NULL)
	, m_glFrameUBO(0)
	, m_unCompanionWindowVAO(0)
	, m_bShowWireframe(false)
	, m_fNearClip(0.1f)
	, m_fFarClip(50.0f)
{
}

Renderer::~Renderer()
{
	Shutdown();
}

bool Renderer::init(vr::IVRSystem *pHMD, TrackedDeviceManager *pTDM)
{
	m_pHMD = pHMD;
	m_pTDM = pTDM;
	m_pLighting = new LightingSystem();
	// add a directional light and change its ambient coefficient
	m_pLighting->addDirectLight()->ambientCoefficient = 0.5f;
		
	if (SDL_GL_SetSwapInterval(0) < 0)
	{
		printf("%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	glCreateBuffers(1, &m_glFrameUBO);
	glNamedBufferData(m_glFrameUBO, sizeof(FrameUniforms), NULL, GL_STATIC_DRAW); // allocate memory
	glBindBufferRange(GL_UNIFORM_BUFFER, SCENE_UNIFORM_BUFFER_LOCATION, m_glFrameUBO, 0, sizeof(FrameUniforms));

	SetupShaders();
	SetupCameras();
	SetupStereoRenderTargets();

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

void Renderer::addToRenderQueue(RendererSubmission &rs)
{
	m_vRenderQueue.push_back(rs);
}

void Renderer::toggleWireframe()
{
	m_bShowWireframe = !m_bShowWireframe;
}


//-----------------------------------------------------------------------------
// Purpose: Creates all the shaders used by HelloVR SDL
//-----------------------------------------------------------------------------
void Renderer::SetupShaders()
{
	m_Shaders.SetVersion("450");

	m_Shaders.SetPreambleFile("GLSLpreamble.h");

	m_mapShaders["companionWindow"] = m_Shaders.AddProgramFromExts({ "shaders/companionWindow.vert", "shaders/companionWindow.frag" });
	m_mapShaders["lighting"] = m_Shaders.AddProgramFromExts({ "shaders/lighting.vert", "shaders/lighting.frag" });
	m_mapShaders["lightingWireframe"] = m_Shaders.AddProgramFromExts({ "shaders/lighting.vert", "shaders/lightingWF.geom", "shaders/lightingWF.frag" });
	m_mapShaders["debug"] = m_Shaders.AddProgramFromExts({ "shaders/debugDrawer.vert", "shaders/debugDrawer.frag" });
	m_mapShaders["infoBox"] = m_Shaders.AddProgramFromExts({ "shaders/infoBox.vert", "shaders/infoBox.frag" });

	m_pLighting->addShaderToUpdate(m_mapShaders["lighting"]);
	m_pLighting->addShaderToUpdate(m_mapShaders["lightingWireframe"]);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Renderer::SetupStereoRenderTargets()
{
	if (!m_pHMD)
		return false;

	m_pHMD->GetRecommendedRenderTargetSize(&m_nRenderWidth, &m_nRenderHeight);

	// Set viewport for shader uniforms
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, v4Viewport), sizeof(FrameUniforms::v4Viewport), glm::value_ptr(glm::vec4(0, 0, m_nRenderWidth, m_nRenderHeight)));

	CreateFrameBuffer(m_nRenderWidth, m_nRenderHeight, leftEyeDesc);
	CreateFrameBuffer(m_nRenderWidth, m_nRenderHeight, rightEyeDesc);

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Renderer::CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc)
{
	// Create the multisample depth buffer as a render buffer
	glNamedRenderbufferStorageMultisample(framebufferDesc.m_nDepthBufferId, 4, GL_DEPTH_COMPONENT, nWidth, nHeight);
	// Allocate render texture storage
	glTextureStorage2DMultisample(framebufferDesc.m_nRenderTextureId, 4, GL_RGBA8, nWidth, nHeight, true);
	// Allocate resolve texture storage
	glTextureStorage2D(framebufferDesc.m_nResolveTextureId, 1, GL_RGBA8, nWidth, nHeight);

	// Attach depth buffer to render framebuffer 
	glNamedFramebufferRenderbuffer(framebufferDesc.m_nRenderFramebufferId, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	// Attach render texture as color attachment to render framebuffer
	glNamedFramebufferTexture(framebufferDesc.m_nRenderFramebufferId, GL_COLOR_ATTACHMENT0, framebufferDesc.m_nRenderTextureId, 0);

	// Attach resolve texture as color attachment to resolve framebuffer
	glNamedFramebufferTexture(framebufferDesc.m_nResolveFramebufferId, GL_COLOR_ATTACHMENT0, framebufferDesc.m_nResolveTextureId, 0);

	// Set resolve texture parameters
	glTextureParameteri(framebufferDesc.m_nResolveTextureId, GL_TEXTURE_MAX_LEVEL, 0);
	glTextureParameteri(framebufferDesc.m_nResolveTextureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(framebufferDesc.m_nResolveTextureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(framebufferDesc.m_nResolveTextureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(framebufferDesc.m_nResolveTextureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// check FBO statuses
	GLenum renderStatus = glCheckNamedFramebufferStatus(framebufferDesc.m_nRenderFramebufferId, GL_FRAMEBUFFER);
	GLenum resolveStatus = glCheckNamedFramebufferStatus(framebufferDesc.m_nResolveFramebufferId, GL_FRAMEBUFFER);
	if (renderStatus != GL_FRAMEBUFFER_COMPLETE || resolveStatus != GL_FRAMEBUFFER_COMPLETE)
		return false;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::SetupCompanionWindow(int width, int height)
{
	m_nCompanionWindowWidth = width;
	m_nCompanionWindowHeight = height;

	if (!m_pHMD)
		return;

	struct VertexDataWindow
	{
		glm::vec2 position;
		glm::vec2 texCoord;

		VertexDataWindow(const glm::vec2 & pos, const glm::vec2 tex) : position(pos), texCoord(tex) {	}
	};

	std::vector<VertexDataWindow> vVerts;

	// Calculate aspect ratio and use it to letterbox-clip the left eye texture coordinates
	float ar = static_cast<float>(m_nCompanionWindowHeight) / static_cast<float>(m_nCompanionWindowWidth);

	// left eye verts	
	vVerts.push_back(VertexDataWindow(glm::vec2(-1, -1), glm::vec2(0.f, 0.5f - 0.5f * ar)));
	vVerts.push_back(VertexDataWindow(glm::vec2(1, -1), glm::vec2(1.f, 0.5f - 0.5f * ar)));
	vVerts.push_back(VertexDataWindow(glm::vec2(-1, 1), glm::vec2(0.f, 0.5f + 0.5f * ar)));
	vVerts.push_back(VertexDataWindow(glm::vec2(1, 1), glm::vec2(1.f, 0.5f + 0.5f * ar)));
	

	// right eye verts
	//vVerts.push_back(VertexDataWindow(glm::vec2(0, -1), glm::vec2(0, 0)));
	//vVerts.push_back(VertexDataWindow(glm::vec2(1, -1), glm::vec2(1, 0)));
	//vVerts.push_back(VertexDataWindow(glm::vec2(0, 1), glm::vec2(0, 1)));
	//vVerts.push_back(VertexDataWindow(glm::vec2(1, 1), glm::vec2(1, 1)));

	GLushort vIndices[] = { 0, 1, 3,   0, 3, 2 };//,   4, 5, 7,   4, 7, 6 };
	m_uiCompanionWindowIndexSize = _countof(vIndices);

	// Generate/allocate and fill vertex buffer object
	glCreateBuffers(1, &m_glCompanionWindowIDVertBuffer);
	glNamedBufferData(m_glCompanionWindowIDVertBuffer, vVerts.size() * sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW);

	// Generate/allocate and fill index buffer object
	glCreateBuffers(1, &m_glCompanionWindowIDIndexBuffer);
	glNamedBufferData(m_glCompanionWindowIDIndexBuffer, m_uiCompanionWindowIndexSize * sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);
	
	// Define our Vertex Attribute Object
	glGenVertexArrays(1, &m_unCompanionWindowVAO);
	glBindVertexArray(m_unCompanionWindowVAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_glCompanionWindowIDVertBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glCompanionWindowIDIndexBuffer);

		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, position));

		glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
		glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof(VertexDataWindow, texCoord));

	glBindVertexArray(0);

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

	m_vRenderQueue.clear();
	DebugDrawer::getInstance().flushLines();

	// SwapWindow
	{
		SDL_GL_SwapWindow(win);
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::RenderStereoTargets()
{
	glClearColor(0.15f, 0.15f, 0.18f, 1.0f); // nice background color, but not black
	//glClearColor(0.33, 0.39, 0.49, 1.0); //VTT4D background
	glEnable(GL_MULTISAMPLE);

	// Left Eye Render
	glBindFramebuffer(GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);
	RenderScene(vr::Eye_Left);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Right Eye Render
	glBindFramebuffer(GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);
	RenderScene(vr::Eye_Right);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);
	
	// Left Eye Blit
	glBlitNamedFramebuffer(
		leftEyeDesc.m_nRenderFramebufferId,
		leftEyeDesc.m_nResolveFramebufferId,
		0, 0, m_nRenderWidth, m_nRenderHeight,
		0, 0, m_nRenderWidth, m_nRenderHeight,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	// Right Eye Blit
	glBlitNamedFramebuffer(
		rightEyeDesc.m_nRenderFramebufferId,
		rightEyeDesc.m_nResolveFramebufferId,
		0, 0, m_nRenderWidth, m_nRenderHeight, 
		0, 0, m_nRenderWidth, m_nRenderHeight,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::RenderScene(vr::Hmd_Eye nEye)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glm::mat4 thisEyesViewMatrix = (nEye == vr::Eye_Left ? m_mat4eyePoseLeft : m_mat4eyePoseRight) * m_mat4CurrentHMDView;
	glm::mat4 thisEyesProjectionMatrix = (nEye == vr::Eye_Left ? m_mat4ProjectionLeft : m_mat4ProjectionRight);
	glm::mat4 thisEyesViewProjectionMatrix = thisEyesProjectionMatrix * thisEyesViewMatrix;

	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4View), sizeof(FrameUniforms::m4View), glm::value_ptr(thisEyesViewMatrix));
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4Projection), sizeof(FrameUniforms::m4Projection), glm::value_ptr(thisEyesProjectionMatrix));
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4ViewProjection), sizeof(FrameUniforms::m4ViewProjection), glm::value_ptr(thisEyesViewProjectionMatrix));

	m_pLighting->update(thisEyesViewMatrix);

	GLuint* lightingShaderProg = m_bShowWireframe ? m_mapShaders["lightingWireframe"] : m_mapShaders["lighting"];
	// ----- Render Model rendering -----
	if (*lightingShaderProg)
	{
		glUseProgram(*lightingShaderProg);

		if (!m_pHMD->IsInputFocusCapturedByAnotherProcess())
		{
			for (auto &rm : m_mapModelInstances)
			{
				RenderModel *pglRenderModel = findOrLoadRenderModel(rm.first.c_str());

				if (pglRenderModel)
				{
					for (auto const &instancePose : rm.second)
					{
						glUniformMatrix4fv(MODEL_MAT_UNIFORM_LOCATION, 1, GL_FALSE, glm::value_ptr(instancePose));
						pglRenderModel->Draw();
					}
				}
				else
				{
					printf("Unable to load render model %s\n", rm.first.c_str());
				}
			}
		}
	}

	if (*m_mapShaders["infoBox"])
	{
		glUseProgram(*m_mapShaders["infoBox"]);
		InfoBoxManager::getInstance().draw();
	}

	if (*m_mapShaders["debug"])
	{
		glUseProgram(*m_mapShaders["debug"]);
		DebugDrawer::getInstance().render();
	}

	for (auto i : m_vRenderQueue)
	{
		if (*m_mapShaders[i.shaderName])
		{
			glUseProgram(*m_mapShaders[i.shaderName]);
			glUniformMatrix4fv(MODEL_MAT_UNIFORM_LOCATION, 1, GL_FALSE, glm::value_ptr(i.modelToWorldTransform));
			glUniform1f(MATERIAL_SHININESS_UNIFORM_LOCATION, i.specularExponent);

			glBindTextureUnit(DIFFUSE_TEXTURE_BINDING, i.diffuseTex);
			glBindTextureUnit(SPECULAR_TEXTURE_BINDING, i.specularTex);

			glBindVertexArray(i.VAO);
			glDrawElements(i.primitiveType, i.vertCount, GL_UNSIGNED_SHORT, 0);
			glBindVertexArray(0);
		}
	}

	glDisable(GL_BLEND);

	glUseProgram(0);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::RenderCompanionWindow()
{
	if (m_mapShaders["companionWindow"] == NULL)
		return;

	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, m_nCompanionWindowWidth, m_nCompanionWindowHeight);

	glUseProgram(*m_mapShaders["companionWindow"]);
	glBindVertexArray(m_unCompanionWindowVAO);

		// render left eye (first half of index array )
		glBindTextureUnit(DIFFUSE_TEXTURE_BINDING, leftEyeDesc.m_nResolveTextureId);
		glDrawElements(GL_TRIANGLES, m_uiCompanionWindowIndexSize, GL_UNSIGNED_SHORT, 0);

		// render right eye (second half of index array )
		//glBindTextureUnit(DIFFUSE_TEXTURE_BINDING, rightEyeDesc.m_nResolveTextureId);
		//glDrawElements(GL_TRIANGLES, m_uiCompanionWindowIndexSize / 2, GL_UNSIGNED_SHORT, (const void *)(uintptr_t)(m_uiCompanionWindowIndexSize));

	glBindVertexArray(0);
	glUseProgram(0);
}


//-----------------------------------------------------------------------------
// Purpose: Finds a render model we've already loaded or loads a new one
//-----------------------------------------------------------------------------
RenderModel* Renderer::findOrLoadRenderModel(const char *pchRenderModelName)
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
	glDeleteBuffers(1, &m_glCompanionWindowIDVertBuffer);
	glDeleteBuffers(1, &m_glCompanionWindowIDIndexBuffer);
}
