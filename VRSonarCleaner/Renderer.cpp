#include "Renderer.h"

#include <vector>
#include <shared/glm/glm.hpp>
#include <shared/glm/gtc/type_ptr.hpp>

#include "DebugDrawer.h"

#include "GLSLpreamble.h"

Renderer::Renderer()
	: m_pLighting(NULL)
	, m_glFrameUBO(0)
	, m_glFullscreenTextureVAO(0)
	, m_bShowWireframe(false)
{
}

Renderer::~Renderer()
{
	Shutdown();
}

void Renderer::Shutdown()
{
	glDeleteBuffers(1, &m_glFullscreenTextureVBO);
	glDeleteBuffers(1, &m_glFullscreenTextureEBO);
}

bool Renderer::init()
{
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

	setupShaders();

	setupFullscreenTexture();

	return true;
}

void Renderer::addToStaticRenderQueue(RendererSubmission &rs)
{
	m_vStaticRenderQueue.push_back(rs);
}

void Renderer::addToDynamicRenderQueue(RendererSubmission &rs)
{
	m_vDynamicRenderQueue.push_back(rs);
}

void Renderer::clearDynamicRenderQueue()
{
	m_vDynamicRenderQueue.clear();
}

void Renderer::addToUIRenderQueue(RendererSubmission & rs)
{
	m_vUIRenderQueue.push_back(rs);
}

void Renderer::clearUIRenderQueue()
{
	m_vUIRenderQueue.clear();
}

void Renderer::toggleWireframe()
{
	m_bShowWireframe = !m_bShowWireframe;
}


//-----------------------------------------------------------------------------
// Purpose: Creates all the shaders used by HelloVR SDL
//-----------------------------------------------------------------------------
void Renderer::setupShaders()
{
	m_Shaders.SetVersion("450");

	m_Shaders.SetPreambleFile("GLSLpreamble.h");

	m_mapShaders["fullscreenTexture"] = m_Shaders.AddProgramFromExts({ "shaders/fullscreentexture.vert", "shaders/fullscreentexture.frag" });
	m_mapShaders["lighting"] = m_Shaders.AddProgramFromExts({ "shaders/lighting.vert", "shaders/lighting.frag" });
	m_mapShaders["lightingWireframe"] = m_Shaders.AddProgramFromExts({ "shaders/lighting.vert", "shaders/lightingWF.geom", "shaders/lightingWF.frag" });
	m_mapShaders["flat"] = m_Shaders.AddProgramFromExts({ "shaders/flat.vert", "shaders/flat.frag" });
	m_mapShaders["debug"] = m_Shaders.AddProgramFromExts({ "shaders/flat.vert", "shaders/flat.frag" });
	m_mapShaders["infoBox"] = m_Shaders.AddProgramFromExts({ "shaders/infoBox.vert", "shaders/infoBox.frag" });

	m_pLighting->addShaderToUpdate(m_mapShaders["lighting"]);
	m_pLighting->addShaderToUpdate(m_mapShaders["lightingWireframe"]);
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
void Renderer::RenderFrame(SceneViewInfo *sceneView3DInfo, SceneViewInfo *sceneViewUIInfo, FramebufferDesc *frameBuffer)
{
	m_Shaders.UpdatePrograms();
	
	// Set viewport for and send it as a shader uniform
	glViewport(0, 0, sceneView3DInfo->m_nRenderWidth, sceneView3DInfo->m_nRenderHeight);
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, v4Viewport), sizeof(FrameUniforms::v4Viewport), glm::value_ptr(glm::vec4(0, 0, sceneView3DInfo->m_nRenderWidth, sceneView3DInfo->m_nRenderHeight)));

	glClearColor(0.15f, 0.15f, 0.18f, 1.0f); // nice background color, but not black
												//glClearColor(0.33, 0.39, 0.49, 1.0); //VTT4D background
	glEnable(GL_MULTISAMPLE);

	// Render to framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->m_nRenderFramebufferId);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glm::mat4 vpMat = sceneView3DInfo->projection * sceneView3DInfo->view;

		glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4View), sizeof(FrameUniforms::m4View), glm::value_ptr(sceneView3DInfo->view));
		glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4Projection), sizeof(FrameUniforms::m4Projection), glm::value_ptr(sceneView3DInfo->projection));
		glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4ViewProjection), sizeof(FrameUniforms::m4ViewProjection), glm::value_ptr(vpMat));

		m_pLighting->update(sceneView3DInfo->view);


		if (*m_mapShaders["debug"])
		{
			glUseProgram(*m_mapShaders["debug"]);
			glUniformMatrix4fv(MODEL_MAT_UNIFORM_LOCATION, 1, GL_FALSE, glm::value_ptr(glm::mat4()));
			DebugDrawer::getInstance().render();
		}

		// STATIC OBJECTS
		processRenderQueue(m_vStaticRenderQueue);

		// DYNAMIC OBJECTS
		processRenderQueue(m_vDynamicRenderQueue);

		// UI ELEMENTS
		if (sceneViewUIInfo)
		{
			vpMat = sceneViewUIInfo->projection * sceneViewUIInfo->view;

			glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4View), sizeof(FrameUniforms::m4View), glm::value_ptr(sceneViewUIInfo->view));
			glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4Projection), sizeof(FrameUniforms::m4Projection), glm::value_ptr(sceneViewUIInfo->projection));
			glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4ViewProjection), sizeof(FrameUniforms::m4ViewProjection), glm::value_ptr(vpMat));

			glDisable(GL_DEPTH_TEST);
			processRenderQueue(m_vUIRenderQueue);
		}

		glDisable(GL_BLEND);

		glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glDisable(GL_MULTISAMPLE);

	// Blit Framebuffer to screen
	glBlitNamedFramebuffer(
		frameBuffer->m_nRenderFramebufferId,
		frameBuffer->m_nResolveFramebufferId,
		0, 0, sceneView3DInfo->m_nRenderWidth, sceneView3DInfo->m_nRenderHeight,
		0, 0, sceneView3DInfo->m_nRenderWidth, sceneView3DInfo->m_nRenderHeight,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);
}

void Renderer::RenderUI(SceneViewInfo * sceneViewInfo, FramebufferDesc * frameBuffer)
{
}

void Renderer::processRenderQueue(std::vector<RendererSubmission> &renderQueue)
{
	for (auto i : renderQueue)
	{
		if (m_mapShaders[i.shaderName] && *m_mapShaders[i.shaderName])
		{
			glUseProgram(*m_mapShaders[i.shaderName]);
			glUniformMatrix4fv(MODEL_MAT_UNIFORM_LOCATION, 1, GL_FALSE, glm::value_ptr(i.modelToWorldTransform));

			if (i.specularExponent > 0.f)
				glUniform1f(MATERIAL_SHININESS_UNIFORM_LOCATION, i.specularExponent);

			if (i.diffuseTex > 0u)
				glBindTextureUnit(DIFFUSE_TEXTURE_BINDING, i.diffuseTex);
			if (i.specularTex > 0u)
				glBindTextureUnit(SPECULAR_TEXTURE_BINDING, i.specularTex);

			glBindVertexArray(i.VAO);
			glDrawElements(i.primitiveType, i.vertCount, i.indexType, 0);
			glBindVertexArray(0);
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::setupFullscreenTexture()
{
	glm::vec2 vVerts[4];

	// left eye verts	
	vVerts[0] = glm::vec2(-1, -1);
	vVerts[1] = glm::vec2(1, -1);
	vVerts[2] = glm::vec2(-1, 1);
	vVerts[3] = glm::vec2(1, 1);

	// right eye verts
	//vVerts.push_back(VertexDataWindow(glm::vec2(0, -1), glm::vec2(0, 0)));
	//vVerts.push_back(VertexDataWindow(glm::vec2(1, -1), glm::vec2(1, 0)));
	//vVerts.push_back(VertexDataWindow(glm::vec2(0, 1), glm::vec2(0, 1)));
	//vVerts.push_back(VertexDataWindow(glm::vec2(1, 1), glm::vec2(1, 1)));

	GLushort vIndices[] = { 0, 1, 3,   0, 3, 2 };//,   4, 5, 7,   4, 7, 6 };
	m_uiCompanionWindowVertCount = _countof(vIndices);

	// Generate/allocate and fill vertex buffer object
	glCreateBuffers(1, &m_glFullscreenTextureVBO);
	glNamedBufferData(m_glFullscreenTextureVBO, sizeof(vVerts) * sizeof(glm::vec2), &vVerts[0], GL_STATIC_DRAW);

	// Generate/allocate and fill index buffer object
	glCreateBuffers(1, &m_glFullscreenTextureEBO);
	glNamedBufferData(m_glFullscreenTextureEBO, m_uiCompanionWindowVertCount * sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW);

	// Define our Vertex Attribute Object
	glGenVertexArrays(1, &m_glFullscreenTextureVAO);
	glBindVertexArray(m_glFullscreenTextureVAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_glFullscreenTextureVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glFullscreenTextureEBO);

		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);

	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::RenderFullscreenTexture(int width, int height, GLuint textureID)
{
	if (m_mapShaders["fullscreenTexture"] == NULL)
		return;

	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, width, height); 
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, v4Viewport), sizeof(FrameUniforms::v4Viewport), glm::value_ptr(glm::vec4(0, 0, width, height)));

	glUseProgram(*m_mapShaders["fullscreenTexture"]);
	glBindVertexArray(m_glFullscreenTextureVAO);

	// render left eye (first half of index array )
	glBindTextureUnit(DIFFUSE_TEXTURE_BINDING, textureID);
	glDrawElements(GL_TRIANGLES, m_uiCompanionWindowVertCount, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}
