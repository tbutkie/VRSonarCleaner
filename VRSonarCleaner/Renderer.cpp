#include "Renderer.h"

#include <vector>
#include <numeric>
#include <unordered_map>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/norm.hpp>
#include <lodepng.h>

#include <string>
#include <sstream>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "DebugDrawer.h"
#include "GLSLpreamble.h"
#include "utilities.h"

Renderer::Renderer()
	: m_pLighting(NULL)
	, m_glFrameUBO(0)
	, m_bShowWireframe(false)
	, m_uiFontPointSize(144u)
{
}

Renderer::~Renderer()
{
}

void Renderer::shutdown()
{
}

bool Renderer::init()
{
	m_pLighting = new LightingSystem();
	// add a directional light and change its ambient coefficient
	m_pLighting->addDirectLight(glm::vec4(1.f, -1.f, 1.f, 0.f))->ambientCoefficient = 0.5f;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glCreateBuffers(1, &m_glFrameUBO);
	glNamedBufferData(m_glFrameUBO, sizeof(FrameUniforms), NULL, GL_STATIC_DRAW); // allocate memory
	glBindBufferRange(GL_UNIFORM_BUFFER, SCENE_UNIFORM_BUFFER_LOCATION, m_glFrameUBO, 0, sizeof(FrameUniforms));

	setupShaders();

	setupTextures();

	setupPrimitives();

	setupText();

	glEnable(GL_PROGRAM_POINT_SIZE);

	return true;
}

void Renderer::update()
{
	m_Shaders.UpdatePrograms();
}

void Renderer::addToStaticRenderQueue(RendererSubmission &rs)
{
	if (m_mapTextures[rs.diffuseTexName] == NULL)
	{
		printf("Error: Renderer submission diffuse texture \"%s\" not found\n", rs.diffuseTexName.c_str());
		return;
	}

	if (m_mapTextures[rs.specularTexName] == NULL)
	{
		printf("Error: Renderer submission specular texture \"%s\" not found\n", rs.specularTexName.c_str());
		return;
	}

	GLTexture* diff = m_mapTextures[rs.diffuseTexName];
	GLTexture* spec = m_mapTextures[rs.diffuseTexName];

	if (rs.hasTransparency || diff->hasTransparency() || spec->hasTransparency() || rs.diffuseColor.a < 1.f)
	{
		m_vStaticRenderQueue_Transparency.push_back(rs);
		m_vTransparentRenderQueue.push_back(rs);
	}
	else
		m_vStaticRenderQueue_Opaque.push_back(rs);
}

void Renderer::addToDynamicRenderQueue(RendererSubmission &rs)
{
	if (m_mapTextures[rs.diffuseTexName] == NULL)
	{
		printf("Error: Renderer submission diffuse texture \"%s\" not found\n", rs.diffuseTexName.c_str());
		return;
	}

	if (m_mapTextures[rs.specularTexName] == NULL)
	{
		printf("Error: Renderer submission specular texture \"%s\" not found\n", rs.specularTexName.c_str());
		return;
	}

	GLTexture* diff = m_mapTextures[rs.diffuseTexName];
	GLTexture* spec = m_mapTextures[rs.diffuseTexName];

	if (rs.hasTransparency || diff->hasTransparency() || spec->hasTransparency() || rs.diffuseColor.a < 1.f)
	{
		m_vDynamicRenderQueue_Transparency.push_back(rs);
		m_vTransparentRenderQueue.push_back(rs);
	}
	else
		m_vDynamicRenderQueue_Opaque.push_back(rs);
}

void Renderer::clearDynamicRenderQueue()
{
	m_vDynamicRenderQueue_Opaque.clear();
	m_vDynamicRenderQueue_Transparency.clear();
	m_vTransparentRenderQueue.clear();
	m_vTransparentRenderQueue = m_vStaticRenderQueue_Transparency;
}

void Renderer::addToUIRenderQueue(RendererSubmission & rs)
{
	m_vUIRenderQueue.push_back(rs);
}

void Renderer::clearUIRenderQueue()
{
	m_vUIRenderQueue.clear();
}

void Renderer::showMessage(std::string message, float duration)
{
	m_vMessages.push_back(std::tuple<std::string, float, std::chrono::high_resolution_clock::time_point>(message, duration, std::chrono::high_resolution_clock::now()));
}

void Renderer::setSkybox(std::string right, std::string left, std::string top, std::string bottom, std::string front, std::string back)
{
	std::vector<std::string> faces({right, left, top, bottom, front, back});

	glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_Skybox.texID);

	unsigned int width, height;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		std::vector<unsigned char> image;

		unsigned error = lodepng::decode(image, width, height, faces[i]);

		if (error != 0)
			std::cerr << "error " << error << ": " << lodepng_error_text(error) << std::endl;
		else
		{
			if (i == 0)
			{
				int diffuseMipMapLevels = (int)floor(log2((std::max)(width, height))) + 1;
				glTextureStorage2D(m_Skybox.texID, 1, GL_RGBA8, width, height);
			}
			glTextureSubImage3D(m_Skybox.texID, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
		}
		
	}

	glGenerateTextureMipmap(m_Skybox.texID);

	glTextureParameteri(m_Skybox.texID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_Skybox.texID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_Skybox.texID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_Skybox.texID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_Skybox.texID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

bool Renderer::drawPrimitive(std::string primName, glm::mat4 modelTransform, std::string diffuseTextureName, std::string specularTextureName, float specularExponent)
{
	RendererSubmission rs;
	rs.glPrimitiveType = primName.find("_line") != std::string::npos ? GL_LINES : GL_TRIANGLES;
	rs.shaderName = "lighting";
	rs.modelToWorldTransform = modelTransform;
	rs.VAO = m_glPrimitivesVAO;
	rs.indexByteOffset = getPrimitiveIndexByteOffset(primName);
	rs.indexBaseVertex = getPrimitiveIndexBaseVertex(primName);
	rs.vertCount = getPrimitiveIndexCount(primName);
	rs.indexType = GL_UNSIGNED_SHORT;
	rs.diffuseTexName = diffuseTextureName;
	rs.specularTexName = specularTextureName;
	rs.specularExponent = specularExponent;

	if (primName.find("inverse") != std::string::npos)
		rs.vertWindingOrder = GL_CW;

	addToDynamicRenderQueue(rs);

	return true;
}


bool Renderer::drawPrimitive(std::string primName, glm::mat4 modelTransform, glm::vec4 diffuseColor, glm::vec4 specularColor, float specularExponent)
{
	RendererSubmission rs;
	rs.glPrimitiveType = primName.find("_line") != std::string::npos ? GL_LINES : GL_TRIANGLES;
	rs.shaderName = "lighting";
	rs.modelToWorldTransform = modelTransform;
	rs.VAO = m_glPrimitivesVAO;
	rs.indexByteOffset = getPrimitiveIndexByteOffset(primName);
	rs.indexBaseVertex = getPrimitiveIndexBaseVertex(primName);
	rs.vertCount = getPrimitiveIndexCount(primName);
	rs.indexType = GL_UNSIGNED_SHORT;
	rs.diffuseColor = diffuseColor;
	rs.specularColor = specularColor;
	rs.specularExponent = specularExponent;
	rs.hasTransparency = diffuseColor.a != 1.f;

	if (primName.find("inverse") != std::string::npos)
		rs.vertWindingOrder = GL_CW;

	addToDynamicRenderQueue(rs);

	return true;
}

bool Renderer::drawFlatPrimitive(std::string primName, glm::mat4 modelTransform, glm::vec4 color)
{
	RendererSubmission rs;
	rs.glPrimitiveType = primName.find("_line") != std::string::npos ? GL_LINES : GL_TRIANGLES;
	rs.shaderName = "flat";
	rs.modelToWorldTransform = modelTransform;
	rs.VAO = m_glPrimitivesVAO;
	rs.indexByteOffset = getPrimitiveIndexByteOffset(primName);
	rs.indexBaseVertex = getPrimitiveIndexBaseVertex(primName);
	rs.vertCount = getPrimitiveIndexCount(primName);
	rs.indexType = GL_UNSIGNED_SHORT;
	rs.diffuseColor = color;
	rs.hasTransparency = color.a != 1.f;

	if (primName.find("_inverse") != std::string::npos)
		rs.vertWindingOrder = GL_CW;

	addToDynamicRenderQueue(rs);

	return true;
}

bool Renderer::drawPrimitiveCustom(std::string primName, glm::mat4 modelTransform, std::string shaderName, std::string diffuseTexName, glm::vec4 diffuseColor)
{
	RendererSubmission rs;
	rs.glPrimitiveType = primName.find("_line") != std::string::npos ? GL_LINES : GL_TRIANGLES;
	rs.shaderName = shaderName;
	rs.modelToWorldTransform = modelTransform;
	rs.VAO = m_glPrimitivesVAO;
	rs.indexByteOffset = getPrimitiveIndexByteOffset(primName);
	rs.indexBaseVertex = getPrimitiveIndexBaseVertex(primName);
	rs.vertCount = getPrimitiveIndexCount(primName);
	rs.indexType = GL_UNSIGNED_SHORT;
	rs.diffuseTexName = diffuseTexName;
	rs.diffuseColor = diffuseColor;
	rs.specularTexName = "white";
	rs.specularExponent = 110.f;

	if (primName.find("_inverse") != std::string::npos)
		rs.vertWindingOrder = GL_CW;

	addToDynamicRenderQueue(rs);

	return true;
}

void Renderer::drawConnector(glm::vec3 from, glm::vec3 to, float thickness, glm::vec4 color)
{
	float connectorRadius = thickness * 0.5f;
	glm::vec3 w = to - from;

	glm::vec3 u, v;
	u = glm::cross(glm::vec3(0.f, 1.f, 0.f), w);
	if (glm::length2(u) == 0.f)
		u = glm::cross(glm::vec3(1.f, 0.f, 0.f), w);;
	
	u = glm::normalize(u);
	v = glm::normalize(glm::cross(w, u));

	glm::mat4 trans;
	trans[0] = glm::vec4(u * connectorRadius, 0.f);
	trans[1] = glm::vec4(v * connectorRadius, 0.f);
	trans[2] = glm::vec4(w, 0.f);
	trans[3] = glm::vec4(from, 1.f);
	Renderer::getInstance().drawFlatPrimitive("cylinder", trans, color);
}

void Renderer::toggleWireframe()
{
	m_bShowWireframe = !m_bShowWireframe;
}

GLTexture * Renderer::getTexture(std::string texName)
{
	return m_mapTextures[texName];
}

bool Renderer::addTexture(GLTexture * tex)
{
	if (m_mapTextures[tex->getName()] == NULL)
	{
		m_mapTextures[tex->getName()] = tex;
		return true;
	}
	else
	{
		printf("Error: Adding texture \"%s\" which already exists\n", tex->getName().c_str());
		return false;
	}
}

void Renderer::sortTransparentObjects(glm::vec3 HMDPos)
{
	std::sort(m_vStaticRenderQueue_Transparency.begin(), m_vStaticRenderQueue_Transparency.end(), ObjectSorter(HMDPos));
	std::sort(m_vDynamicRenderQueue_Transparency.begin(), m_vDynamicRenderQueue_Transparency.end(), ObjectSorter(HMDPos));
	std::sort(m_vTransparentRenderQueue.begin(), m_vTransparentRenderQueue.end(), ObjectSorter(HMDPos));
}


//-----------------------------------------------------------------------------
// Purpose: Creates all the shaders used by HelloVR SDL
//-----------------------------------------------------------------------------
void Renderer::setupShaders()
{
	m_Shaders.SetVersion("450");

	m_Shaders.SetPreambleFile("GLSLpreamble.h");

	m_mapShaders["vrwindow"] = m_Shaders.AddProgramFromExts({ "resources/shaders/vrwindow.vert", "resources/shaders/windowtexture.frag" });
	m_mapShaders["desktopwindow"] = m_Shaders.AddProgramFromExts({ "resources/shaders/desktopwindow.vert", "resources/shaders/windowtexture.frag" });
	m_mapShaders["lighting"] = m_Shaders.AddProgramFromExts({ "resources/shaders/lighting.vert", "resources/shaders/lighting.frag" });
	m_mapShaders["lightingWireframe"] = m_Shaders.AddProgramFromExts({ "shaders/lighting.vert", "resources/shaders/lightingWF.geom", "shaders/lightingWF.frag" });
	m_mapShaders["flat"] = m_Shaders.AddProgramFromExts({ "resources/shaders/flat.vert", "resources/shaders/flat.frag" });
	m_mapShaders["debug"] = m_Shaders.AddProgramFromExts({ "resources/shaders/flat.vert", "resources/shaders/flat.frag" });
	m_mapShaders["text"] = m_Shaders.AddProgramFromExts({ "resources/shaders/text.vert", "resources/shaders/text.frag" });
	m_mapShaders["skybox"] = m_Shaders.AddProgramFromExts({ "resources/shaders/skybox.vert", "resources/shaders/skybox.frag" });
	m_mapShaders["instanced"] = m_Shaders.AddProgramFromExts({ "resources/shaders/instanced.vert", "resources/shaders/instanced.frag" });
	m_mapShaders["instanced_lit"] = m_Shaders.AddProgramFromExts({ "resources/shaders/instanced.vert", "resources/shaders/lighting.frag" });

	m_pLighting->addShaderToUpdate(m_mapShaders["lighting"]);
	m_pLighting->addShaderToUpdate(m_mapShaders["lightingWireframe"]);
	m_pLighting->addShaderToUpdate(m_mapShaders["instanced_lit"]);
}

void Renderer::setupTextures()
{
	GLubyte white[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	GLubyte black[4] = { 0x00, 0x00, 0x00, 0xFF };
	GLubyte gray[4] = { 0x80, 0x80, 0x80, 0xFF };

	//glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_BINDING);

	m_mapTextures["white"] = new GLTexture("white", white);
	m_mapTextures["black"] = new GLTexture("black", black);
	m_mapTextures["gray"] = new GLTexture("gray", gray);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Renderer::createFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc)
{
	glCreateRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &framebufferDesc.m_nRenderTextureId);
	glCreateFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
	glCreateTextures(GL_TEXTURE_2D, 1, &framebufferDesc.m_nResolveTextureId);
	glCreateFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);

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

void Renderer::destroyFrameBuffer(FramebufferDesc & framebufferDesc)
{
	glDeleteFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);
	glDeleteTextures(1, &framebufferDesc.m_nResolveTextureId);
	glDeleteFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
	glDeleteTextures(1, &framebufferDesc.m_nRenderTextureId);
	glDeleteRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::renderFrame(SceneViewInfo *sceneView3DInfo, FramebufferDesc *frameBuffer)
{	
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

		glEnable(GL_CULL_FACE);

		glm::mat4 vpMat = sceneView3DInfo->projection * sceneView3DInfo->view;

		glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4View), sizeof(FrameUniforms::m4View), glm::value_ptr(sceneView3DInfo->view));
		glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4Projection), sizeof(FrameUniforms::m4Projection), glm::value_ptr(sceneView3DInfo->projection));
		glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4ViewProjection), sizeof(FrameUniforms::m4ViewProjection), glm::value_ptr(vpMat));

		m_pLighting->update(sceneView3DInfo->view);

		// Opaque objects first while depth buffer writing enabled
		processRenderQueue(m_vStaticRenderQueue_Opaque);
		processRenderQueue(m_vDynamicRenderQueue_Opaque);

		// skybox last
		if (m_Skybox.texID != 0u && m_mapShaders["skybox"] && *m_mapShaders["skybox"])
		{
			glDepthFunc(GL_LEQUAL); 

			glUseProgram(*m_mapShaders["skybox"]);

			glBindVertexArray(m_glPrimitivesVAO);
				glBindTextureUnit(0, m_Skybox.texID);
				glDrawElementsBaseVertex(GL_TRIANGLES, getPrimitiveIndexCount("skybox"), GL_UNSIGNED_SHORT, (GLvoid*)getPrimitiveIndexByteOffset("skybox"), getPrimitiveIndexBaseVertex("skybox"));
			glBindVertexArray(0);

			glDepthFunc(GL_LESS);
		}

		if (m_vTransparentRenderQueue.size() > 0)
		{
			glEnable(GL_BLEND);
			//glDisable(GL_DEPTH_TEST);
			//glDepthMask(GL_FALSE);

			processRenderQueue(m_vTransparentRenderQueue);

			//glEnable(GL_DEPTH_TEST);
			//glDepthMask(GL_TRUE);
			glDisable(GL_BLEND);
		}

		glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	glDisable(GL_MULTISAMPLE);

	// Blit Framebuffer to resolve framebuffer
	glBlitNamedFramebuffer(
		frameBuffer->m_nRenderFramebufferId,
		frameBuffer->m_nResolveFramebufferId,
		0, 0, sceneView3DInfo->m_nRenderWidth, sceneView3DInfo->m_nRenderHeight,
		0, 0, sceneView3DInfo->m_nRenderWidth, sceneView3DInfo->m_nRenderHeight,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);
}

void Renderer::renderUI(SceneViewInfo * sceneViewUIInfo, FramebufferDesc * frameBuffer)
{
	updateUI(glm::ivec2(sceneViewUIInfo->m_nRenderWidth, sceneViewUIInfo->m_nRenderHeight), std::chrono::high_resolution_clock::now());

	glm::mat4 vpMat = sceneViewUIInfo->projection * sceneViewUIInfo->view;

	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, v4Viewport), sizeof(FrameUniforms::v4Viewport), glm::value_ptr(glm::vec4(0, 0, sceneViewUIInfo->m_nRenderWidth, sceneViewUIInfo->m_nRenderHeight)));
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4View), sizeof(FrameUniforms::m4View), glm::value_ptr(sceneViewUIInfo->view));
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4Projection), sizeof(FrameUniforms::m4Projection), glm::value_ptr(sceneViewUIInfo->projection));
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4ViewProjection), sizeof(FrameUniforms::m4ViewProjection), glm::value_ptr(vpMat));

	glViewport(0, 0, sceneViewUIInfo->m_nRenderWidth, sceneViewUIInfo->m_nRenderHeight);

	glEnable(GL_MULTISAMPLE);

	// Render to framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->m_nRenderFramebufferId);
	glDisable(GL_DEPTH_TEST);

	processRenderQueue(m_vUIRenderQueue);

	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	// Blit Framebuffer to resolve framebuffer
	glBlitNamedFramebuffer(
		frameBuffer->m_nRenderFramebufferId,
		frameBuffer->m_nResolveFramebufferId,
		0, 0, sceneViewUIInfo->m_nRenderWidth, sceneViewUIInfo->m_nRenderHeight,
		0, 0, sceneViewUIInfo->m_nRenderWidth, sceneViewUIInfo->m_nRenderHeight,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::renderFullscreenTexture(int width, int height, GLuint textureID, bool textureAspectPortrait)
{
	GLuint* shader;

	if (textureAspectPortrait)
		shader = m_mapShaders["vrwindow"];
	else
		shader = m_mapShaders["desktopwindow"];

	if (shader == NULL)
		return;

	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, width, height);
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, v4Viewport), sizeof(FrameUniforms::v4Viewport), glm::value_ptr(glm::vec4(0, 0, width, height)));

	glUseProgram(*shader);
	glBindVertexArray(m_glPrimitivesVAO);

	// render left eye (first half of index array )
	glBindTextureUnit(DIFFUSE_TEXTURE_BINDING, textureID);
	glDrawElementsBaseVertex(GL_TRIANGLES, m_mapPrimitiveIndexCounts["fullscreenquad"], GL_UNSIGNED_SHORT, (GLvoid*)m_mapPrimitiveIndexByteOffsets["fullscreenquad"], m_mapPrimitiveIndexBaseVertices["fullscreenquad"]);

	glBindVertexArray(0);
	glUseProgram(0);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::renderStereoTexture(int width, int height, GLuint leftEyeTextureID, GLuint rightEyeTextureID)
{
	GLuint* shader = m_mapShaders["desktopwindow"];

	if (shader == NULL)
		return;

	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, width, height);
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, v4Viewport), sizeof(FrameUniforms::v4Viewport), glm::value_ptr(glm::vec4(0, 0, width, height)));

	glUseProgram(*shader);
	glBindVertexArray(m_glPrimitivesVAO);

	glDrawBuffer(GL_BACK_LEFT);

	// render left eye
	glBindTextureUnit(DIFFUSE_TEXTURE_BINDING, leftEyeTextureID);
	glDrawElementsBaseVertex(GL_TRIANGLES, m_mapPrimitiveIndexCounts["fullscreenquad"], GL_UNSIGNED_SHORT, (GLvoid*)m_mapPrimitiveIndexByteOffsets["fullscreenquad"], m_mapPrimitiveIndexBaseVertices["fullscreenquad"]);

	glDrawBuffer(GL_BACK_RIGHT);

	// render left eye (first half of index array )
	glBindTextureUnit(DIFFUSE_TEXTURE_BINDING, rightEyeTextureID);
	glDrawElementsBaseVertex(GL_TRIANGLES, m_mapPrimitiveIndexCounts["fullscreenquad"], GL_UNSIGNED_SHORT, (GLvoid*)m_mapPrimitiveIndexByteOffsets["fullscreenquad"], m_mapPrimitiveIndexBaseVertices["fullscreenquad"]);

	glBindVertexArray(0);
	glUseProgram(0);
}

void Renderer::processRenderQueue(std::vector<RendererSubmission> &renderQueue)
{
	for (auto i : renderQueue)
	{
		if (m_mapShaders[i.shaderName] && *m_mapShaders[i.shaderName])
		{
			glUseProgram(*m_mapShaders[i.shaderName]);
			glUniformMatrix4fv(MODEL_MAT_UNIFORM_LOCATION, 1, GL_FALSE, glm::value_ptr(i.modelToWorldTransform));

			// handle diffuse solid color
			glUniform4fv(DIFFUSE_COLOR_UNIFORM_LOCATION, 1, glm::value_ptr(i.diffuseColor));

			// handle specular solid color
			glUniform4fv(SPECULAR_COLOR_UNIFORM_LOCATION, 1, glm::value_ptr(i.specularColor));
	
			// Handle diffuse texture, if any
			glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_BINDING);
			glBindTextureUnit(DIFFUSE_TEXTURE_BINDING, m_mapTextures[i.diffuseTexName]->getTexture());
			
			// Handle specular texture, if any
			glActiveTexture(GL_TEXTURE0 + SPECULAR_TEXTURE_BINDING);
			glBindTextureUnit(SPECULAR_TEXTURE_BINDING, m_mapTextures[i.specularTexName]->getTexture());

			if (i.specularExponent > 0.f)
				glUniform1f(MATERIAL_SHININESS_UNIFORM_LOCATION, i.specularExponent);

			glFrontFace(i.vertWindingOrder);

			glBindVertexArray(i.VAO);
			if (i.instanced)
				glDrawElementsInstancedBaseVertex(i.glPrimitiveType, i.vertCount, i.indexType, (GLvoid*)i.indexByteOffset, i.instanceCount, i.indexBaseVertex);
			else
				glDrawElementsBaseVertex(i.glPrimitiveType, i.vertCount, i.indexType, (GLvoid*)i.indexByteOffset, i.indexBaseVertex);
			glBindVertexArray(0);
		}
	}
}

bool Renderer::sortByViewDistance(RendererSubmission const & rsLHS, RendererSubmission const & rsRHS, glm::vec3 const & HMDPos)
{
	glm::vec3 lhsPos = rsLHS.transparencySortPosition.w == -1.f ? glm::vec3(rsLHS.modelToWorldTransform[3]) : glm::vec3(rsLHS.transparencySortPosition);
	glm::vec3 rhsPos = rsRHS.transparencySortPosition.w == -1.f ? glm::vec3(rsRHS.modelToWorldTransform[3]) : glm::vec3(rsRHS.transparencySortPosition);

	return glm::length2(lhsPos - HMDPos) > glm::length2(rhsPos - HMDPos);
}


void Renderer::setupPrimitives()
{
	std::vector<PrimVert> verts;
	std::vector<GLushort> inds;

	size_t baseInd = inds.size();
	size_t baseVert = verts.size();
	generateFullscreenQuad(verts, inds);

	for (auto primname : { "fullscreen", "fullscreenquad" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);
	}

	baseInd = inds.size();
	baseVert = verts.size();
	generateDisc(16, verts, inds);

	for (auto primname : { "disc", "circlesprite" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);
		//m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() / 2);

		//m_mapPrimitiveVAOs[primname + std::string("double")] = m_glDiscVAO;
		//m_mapPrimitiveVBOs[primname + std::string("double")] = m_glDiscVBO;
		//m_mapPrimitiveEBOs[primname + std::string("double")] = m_glDiscEBO;
		//m_mapPrimitiveIndexCounts[primname + std::string("double")] = static_cast<GLsizei>(inds.size());
	}

	baseInd = inds.size();
	baseVert = verts.size();
	generateCube(verts, inds);

	for (auto primname : { "cube", "box", "skybox" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);
	}
	
	baseInd = inds.size();
	baseVert = verts.size();
	generatePlane(verts, inds);

	for (auto primname : { "plane", "quad", "sprite" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd) / 2;
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);

		m_mapPrimitiveIndexBaseVertices[primname + std::string("double")] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname + std::string("double")] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname + std::string("double")] = baseInd * sizeof(GLushort);
	}

	baseInd = inds.size();
	baseVert = verts.size();
	generateBBox(verts, inds);

	for (auto primname : { "bbox_lines" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);
	}

	baseInd = inds.size();
	baseVert = verts.size();
	generateIcosphere(3, verts, inds);

	for (auto primname : { "icosphere", "inverse_icosphere", "icosphere_inverse" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);
	}

	baseInd = inds.size();
	baseVert = verts.size();
	generateCylinder(32, verts, inds);

	for (auto primname : { "cylinder", "rod" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);
	}

	baseInd = inds.size();
	baseVert = verts.size();
	generateTorus(1.f, 0.025f, 32, 8, verts, inds);

	for (auto primname : { "torus", "donut", "doughnut" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);
	}

	glCreateBuffers(1, &m_glPrimitivesVBO);
	glCreateBuffers(1, &m_glPrimitivesEBO);
	// Allocate and store buffer data and indices
	glNamedBufferStorage(m_glPrimitivesVBO, verts.size() * sizeof(PrimVert), verts.data(), GL_NONE);
	glNamedBufferStorage(m_glPrimitivesEBO, inds.size() * sizeof(GLushort), inds.data(), GL_NONE);

	glGenVertexArrays(1, &m_glPrimitivesVAO);
	glBindVertexArray(this->m_glPrimitivesVAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->m_glPrimitivesVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glPrimitivesEBO);

		// Set the vertex attribute pointers
		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, p));
		glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
		glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, n));
		glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
		glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, c));
		glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
		glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, t));
	glBindVertexArray(0);
}


void Renderer::generateIcosphere(int recursionLevel, std::vector<PrimVert> &verts, std::vector<GLushort> &inds)
{
	std::vector<glm::vec3> vertices;
	int index = 0;

	auto addVertex = [&](glm::vec3 pt) { vertices.push_back(glm::normalize(pt)); return index++; };

	std::unordered_map<int64_t, GLushort> middlePointIndexCache;

	// create 12 vertices of a icosahedron
	float t = (1.f + sqrt(5.f)) / 2.f;

	addVertex(glm::vec3(-1.f, t, 0.f));
	addVertex(glm::vec3(1.f, t, 0.f));
	addVertex(glm::vec3(-1.f, -t, 0.f));
	addVertex(glm::vec3(1.f, -t, 0.f));

	addVertex(glm::vec3(0.f, -1.f, t));
	addVertex(glm::vec3(0.f, 1.f, t));
	addVertex(glm::vec3(0.f, -1.f, -t));
	addVertex(glm::vec3(0.f, 1.f, -t));

	addVertex(glm::vec3(t, 0.f, -1.f));
	addVertex(glm::vec3(t, 0.f, 1.f));
	addVertex(glm::vec3(-t, 0.f, -1.f));
	addVertex(glm::vec3(-t, 0.f, 1.f));
	
	struct TriangleIndices
	{
		GLushort v1, v2, v3;
	};

	// create 20 triangles of the icosahedron
	std::list<TriangleIndices> faces;

	// 5 faces around point 0
	faces.push_back({ 0, 11, 5 });
	faces.push_back({ 0, 5, 1 });
	faces.push_back({ 0, 1, 7 });
	faces.push_back({ 0, 7, 10 });
	faces.push_back({ 0, 10, 11 });

	// 5 adjacent faces 
	faces.push_back({ 1, 5, 9 });
	faces.push_back({ 5, 11, 4 });
	faces.push_back({ 11, 10, 2 });
	faces.push_back({ 10, 7, 6 });
	faces.push_back({ 7, 1, 8 });

	// 5 faces around point 3
	faces.push_back({ 3, 9, 4 });
	faces.push_back({ 3, 4, 2 });
	faces.push_back({ 3, 2, 6 });
	faces.push_back({ 3, 6, 8 });
	faces.push_back({ 3, 8, 9 });

	// 5 adjacent faces 
	faces.push_back({ 4, 9, 5 });
	faces.push_back({ 2, 4, 11 });
	faces.push_back({ 6, 2, 10 });
	faces.push_back({ 8, 6, 7 });
	faces.push_back({ 9, 8, 1 });


	auto getMiddlePoint = [&](GLushort p1, GLushort p2) {
		// first check if we have it already
		bool firstIsSmaller = p1 < p2;
		int64_t smallerIndex = firstIsSmaller ? p1 : p2;
		int64_t greaterIndex = firstIsSmaller ? p2 : p1;
		int64_t key = (smallerIndex << 32) + greaterIndex;

		// look to see if middle point already computed
		if (middlePointIndexCache.find(key) != middlePointIndexCache.end())
			return middlePointIndexCache[key];

		// not in cache, calculate it
		glm::vec3 point1 = vertices[p1];
		glm::vec3 point2 = vertices[p2];
		glm::vec3 middle = (point1 + point2) / 2.f;

		// add vertex makes sure point is on unit sphere
		GLushort i = addVertex(middle);

		// store it, return index
		middlePointIndexCache[key] = i;
		return i;
	};



	// refine triangles
	for (int i = 0; i < recursionLevel; i++)
	{
		std::list<TriangleIndices> faces2;
		for (auto &tri : faces)
		{
			// replace triangle by 4 triangles
			GLushort a = getMiddlePoint(tri.v1, tri.v2);
			GLushort b = getMiddlePoint(tri.v2, tri.v3);
			GLushort c = getMiddlePoint(tri.v3, tri.v1);

			faces2.push_back({ tri.v1, a, c });
			faces2.push_back({ tri.v2, b, a });
			faces2.push_back({ tri.v3, c, b });
			faces2.push_back({ a, b, c });
		}
		faces = faces2;
	}

	// done, now add triangles to mesh
	for (auto &tri : faces)
	{
		inds.push_back(tri.v1);
		inds.push_back(tri.v2);
		inds.push_back(tri.v3);
	}


	
	for (auto v : vertices)
	{
		PrimVert iv;
		iv.p = v;
		iv.n = v;
		iv.c = glm::vec4(1.f);
		iv.t = glm::vec2(0.5f);

		verts.push_back(iv);
	}
}

void Renderer::generateDisc(int numSegments, std::vector<PrimVert> &verts, std::vector<GLushort> &inds)
{
	size_t baseInd = verts.size();

	glm::vec3 p(0.f, 0.f, 0.f);
	glm::vec3 n(0.f, 0.f, 1.f);
	glm::vec4 c(1.f);
	glm::vec2 t(0.5f, 0.5f);

	verts.push_back(PrimVert({ p, n, c, t }));

	// Front 
	for (float i = 0; i < numSegments; ++i)
	{
		float angle = ((float)i / (float)(numSegments)) * glm::two_pi<float>();

		p = glm::vec3(sin(angle), cos(angle), 0.f);
		t = (glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f;

		verts.push_back(PrimVert({ p, n, c, t }));

		if (i > 0)
		{
			inds.push_back(0); // ctr pt of endcap
			inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);
			inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 2);
		}
	}
	inds.push_back(0);
	inds.push_back(1);
	inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);

	//Back
	//verts.push_back(PrimVert({ glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.5f, 0.5f) }));
	//for (float i = 0; i < numSegments; ++i)
	//{
	//	float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
	//	verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), (glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f }));
	//
	//	if (i > 0)
	//	{
	//		inds.push_back(verts.size() - (i + 2));
	//		inds.push_back(verts.size() - 2);
	//		inds.push_back(verts.size() - 1);
	//	}
	//}
	//inds.push_back(0);
	//inds.push_back(verts.size() - 1);
	//inds.push_back(1);
}

void Renderer::generateCylinder(int numSegments, std::vector<PrimVert> &verts, std::vector<GLushort> &inds)
{
	size_t baseInd = verts.size();

	// Front endcap
	verts.push_back(PrimVert({ glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.5f, 0.5f) }));
	for (int i = 0; i < numSegments; ++i)
	{
		float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
		verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), (glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f }));

		if (i > 0)
		{
			inds.push_back(0);
			inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 2);
			inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);
		}
	}
	inds.push_back(0);
	inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);
	inds.push_back(1);

	// Back endcap
	verts.push_back(PrimVert({ glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(0.5f, 0.5f) }));
	for (int i = 0; i < numSegments; ++i)
	{
		float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
		verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), (glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f }));

		if (i > 0)
		{
			inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - (i + 2)); // ctr pt of endcap
			inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);
			inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 2);
		}
	}
	inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - (numSegments + 1));
	inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - (numSegments));
	inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);

	// Shaft
	for (int i = 0; i < numSegments; ++i)
	{
		float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
		verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 0.f), glm::vec3(sin(angle), cos(angle), 0.f), glm::vec4(1.f), glm::vec2((float)i / (float)(numSegments - 1), 0.f) }));

		verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 1.f), glm::vec3(sin(angle), cos(angle), 0.f), glm::vec4(1.f), glm::vec2((float)i / (float)(numSegments - 1), 1.f) }));

		if (i > 0)
		{
			inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 4);
			inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 3);
			inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 2);

			inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 2);
			inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 3);
			inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);
		}
	}
	inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 2);
	inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - numSegments * 2);
	inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);

	inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - numSegments * 2);
	inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - numSegments * 2 + 1);
	inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);
}


void Renderer::generateTorus(float coreRadius, float meridianRadius, int numCoreSegments, int numMeridianSegments, std::vector<PrimVert> &verts, std::vector<GLushort> &inds)
{
	int nVerts = numCoreSegments * numMeridianSegments;

	for (int i = 0; i < numCoreSegments; i++)
		for (int j = 0; j < numMeridianSegments; j++)
		{
			float u = i / (numCoreSegments - 1.f);
			float v = j / (numMeridianSegments - 1.f);
			float theta = u * 2.f * glm::pi<float>();
			float rho = v * 2.f * glm::pi<float>();
			float x = cos(theta) * (coreRadius + meridianRadius*cos(rho));
			float y = sin(theta) * (coreRadius + meridianRadius*cos(rho));
			float z = meridianRadius*sin(rho);
			float nx = cos(theta)*cos(rho);
			float ny = sin(theta)*cos(rho);
			float nz = sin(rho);
			float s = u;
			float t = v;

			PrimVert currentVert = { glm::vec3(x, y, z), glm::vec3(nx, ny, nz), glm::vec4(1.f), glm::vec2(s, t) };
			verts.push_back(currentVert);

			GLushort uvInd = i * numMeridianSegments + j;
			GLushort uvpInd = i * numMeridianSegments + (j + 1) % numMeridianSegments;
			GLushort umvInd = (((i - 1) % numCoreSegments + numCoreSegments) % numCoreSegments) * numMeridianSegments + j; // true modulo (not C++ remainder operand %) for negative wraparound
			GLushort umvpInd = (((i - 1) % numCoreSegments + numCoreSegments) % numCoreSegments) * numMeridianSegments + (j + 1) % numMeridianSegments;

			inds.push_back(uvInd);   // (u    , v)
			inds.push_back(uvpInd);  // (u    , v + 1)
			inds.push_back(umvInd);  // (u - 1, v)

			inds.push_back(umvInd);  // (u - 1, v)
			inds.push_back(uvpInd);  // (u    , v + 1)
			inds.push_back(umvpInd); // (u - 1, v + 1)
		}
}

void Renderer::generatePlane(std::vector<PrimVert> &verts, std::vector<GLushort> &inds)
{
	// Front face
	verts.push_back(PrimVert({ glm::vec3(-0.5f, -0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
	verts.push_back(PrimVert({ glm::vec3(0.5f, -0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(1.f, 1.f) }));
	verts.push_back(PrimVert({ glm::vec3(0.5f, 0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));
	verts.push_back(PrimVert({ glm::vec3(-0.5f, 0.5f, 0.f), glm::vec3(0.f, 0.f, 1.f),	glm::vec4(1.f), glm::vec2(0.f, 0.f) }));

	inds.push_back(0);
	inds.push_back(1);
	inds.push_back(2);

	inds.push_back(2);
	inds.push_back(3);
	inds.push_back(0);

	// Back face
	verts.push_back(PrimVert({ glm::vec3(0.5f, -0.5f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(1.f, 1.f) }));
	verts.push_back(PrimVert({ glm::vec3(-0.5f, -0.5f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
	verts.push_back(PrimVert({ glm::vec3(-0.5f, 0.5f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.f, 0.f) }));
	verts.push_back(PrimVert({ glm::vec3(0.5f, 0.5f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));

	inds.push_back(4);
	inds.push_back(5);
	inds.push_back(6);

	inds.push_back(6);
	inds.push_back(7);
	inds.push_back(4);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::generateFullscreenQuad(std::vector<PrimVert> &verts, std::vector<GLushort> &inds)
{
	verts.push_back({ glm::vec3(-1.f, -1.f, 0.f), glm::vec3(), glm::vec4(1.f), glm::vec2(0.f, 0.f) });
	verts.push_back({ glm::vec3(1.f, -1.f, 0.f), glm::vec3(), glm::vec4(1.f), glm::vec2(1.f, 0.f) });
	verts.push_back({ glm::vec3(1.f, 1.f, 0.f), glm::vec3(), glm::vec4(1.f), glm::vec2(1.f, 1.f) });
	verts.push_back({ glm::vec3(-1.f, 1.f, 0.f), glm::vec3(), glm::vec4(1.f), glm::vec2(0.f, 1.f) });

	inds.push_back(0);
	inds.push_back(1);
	inds.push_back(2);

	inds.push_back(2);
	inds.push_back(3);
	inds.push_back(0);
}

void Renderer::generateCube(std::vector<PrimVert> &verts, std::vector<GLushort> &inds)
{
	size_t baseVert = verts.size();

	glm::vec3 bboxMin(-0.5f);
	glm::vec3 bboxMax(0.5f);

	// Bottom
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f, -1.f, 0.f), glm::vec4(1.f), glm::vec2(0.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f, -1.f, 0.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f, -1.f, 0.f), glm::vec4(1.f), glm::vec2(1.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f, -1.f, 0.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 4);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 3);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 1);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 3);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 2);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 1);

	// Top
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f, 1.f, 0.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f, 1.f, 0.f), glm::vec4(1.f), glm::vec2(1.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f, 1.f, 0.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f, 1.f, 0.f), glm::vec4(1.f), glm::vec2(0.f) }));
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 4);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 3);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 1);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 3);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 2);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 1);

	// Left
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMin[2]), glm::vec3(-1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(0.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMax[2]), glm::vec3(-1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMax[2]), glm::vec3(-1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(1.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMin[2]), glm::vec3(-1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 4);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 3);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 1);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 3);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 2);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 1);

	// Right
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMax[2]), glm::vec3(1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMax[2]), glm::vec3(1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(0.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMin[2]), glm::vec3(1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMin[2]), glm::vec3(1.f, 0.f, 0.f), glm::vec4(1.f), glm::vec2(1.f) }));
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 4);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 3);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 1);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 3);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 2);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 1);

	// Front
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(1.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(0.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 4);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 3);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 1);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 3);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 2);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 1);

	// Back
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(1.f, 0.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(1.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.f, 1.f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.f) }));
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 4);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 3);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 1);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 3);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 2);
	inds.push_back(static_cast<GLushort>(verts.size() - baseVert) - 1);
}

// Essentially a unit cube wireframe
void Renderer::generateBBox(std::vector<PrimVert> &verts, std::vector<GLushort> &inds)
{
	size_t baseVert = verts.size();

	glm::vec3 bboxMin(-0.5f);
	glm::vec3 bboxMax(0.5f);

	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMin[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMax[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMax[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));
	verts.push_back(PrimVert({ glm::vec3(bboxMin[0], bboxMin[1], bboxMax[2]), glm::vec3(0.f), glm::vec4(1.f), glm::vec2(0.5f) }));

	for (size_t i = 0; i < (12 * 2); ++i)
		inds.push_back(static_cast<GLushort>(i));
}

void Renderer::setupText()
{
	// FreeType
	FT_Library ft;
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	// Load font as face
	FT_Face face;
	if (FT_New_Face(ft, "resources/fonts/arial.ttf", 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

	// Set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, m_uiFontPointSize);

	// Disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Load first 128 characters of ASCII set
	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			glm::ivec2(face->glyph->advance.x, face->glyph->advance.y)
		};
		m_arrCharacters[c] = character;
		char tmp[2];
		tmp[0] = c;
		tmp[1] = '\0';
		//std::cout << "Adding character \'" << tmp << "\' to text renderer..." << std::endl;
		addTexture(new GLTexture(std::string(tmp), face->glyph->bitmap.width, face->glyph->bitmap.rows, texture, true));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);



	// Load Snellen optotype font as face
	{
		if (FT_New_Face(ft, "resources/fonts/sloan.ttf", 0, &face))
			std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

		// Set size to load glyphs as
		FT_Set_Pixel_Sizes(face, 0, m_uiFontPointSize);

		std::vector<FT_Char> sloanLetters = { 'C', 'D', 'H', 'K', 'N', 'O', 'R', 'S', 'V', 'Z', ' ' };

		// Load first 128 characters of ASCII set
		for (auto c : sloanLetters)
		{
			// Load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}
			// Generate texture
			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);
			// Set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// Now store character for later use
			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				glm::ivec2(face->glyph->advance.x, face->glyph->advance.y)
			};
			m_mapSloanCharacters[c] = character;
			char tmp[2];
			tmp[0] = c;
			tmp[1] = '\0';
			//std::cout << "Adding character \'" << std::string(tmp) + "_sloan" << "\' to text renderer..." << std::endl;
			addTexture(new GLTexture(std::string(tmp) + "_sloan", face->glyph->bitmap.width, face->glyph->bitmap.rows, texture, true));
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		// Destroy FreeType once we're finished
		FT_Done_Face(face);
	}

	FT_Done_FreeType(ft);
}

void Renderer::updateUI(glm::ivec2 dims, std::chrono::high_resolution_clock::time_point tick)
{
	unsigned maxLines = 20;
	float msgHeight = static_cast<float>(dims.y) / static_cast<float>(maxLines);
	float fadeIn = 0.1f;
	float fadeOut = 1.f;

	// this function finds expired messages
	auto cleanFunc = [&fadeOut, &tick](std::tuple<std::string, float, std::chrono::high_resolution_clock::time_point> msg)
	{
		float lifetime = (std::get<1>(msg) + fadeOut) * 1000.f;
		float timeAlive = std::chrono::duration<float, std::milli>(tick - std::get<2>(msg)).count();
		return timeAlive > lifetime;
	};

	// delete expired messages using our function
	m_vMessages.erase(
		std::remove_if(
			m_vMessages.begin(),
			m_vMessages.end(),
			cleanFunc),
		m_vMessages.end()
	);

	size_t msgCount = m_vMessages.size();

	for (size_t i = msgCount; i-- > 0; )
	{
		float timeAlive = (std::chrono::duration<float, std::milli>(tick - std::get<2>(m_vMessages[i])).count() / 1000.f);
		float msgTime = std::get<1>(m_vMessages[i]);
		float alpha = 1.f;

		// fade in
		if (timeAlive < fadeIn)
			alpha = timeAlive / fadeIn;

		// fade out
		if (fadeOut > 0.f && timeAlive > msgTime)
			alpha = (fadeOut - (timeAlive - msgTime)) / fadeOut;

		drawUIText(
			std::get<0>(m_vMessages[i]),
			glm::vec4(1.f, 1.f, 1.f, alpha),
			glm::vec3(0.f, dims.y - msgHeight * (msgCount - i - 1), 0.f),
			glm::quat(),
			msgHeight,
			HEIGHT,
			LEFT,
			TOP_LEFT);
	}
}

void Renderer::drawText(std::string text, glm::vec4 color, glm::vec3 pos, glm::quat rot, GLfloat size, TextSizeDim sizeDim, TextAlignment alignment, TextAnchor anchor, bool snellenFont)
{
	float lineSpacing = m_uiFontPointSize * 1.f;

	// cursor origin is at beginning of text baseline
	int cursorDistOnBaseline = 0;
	int maxCursorDist = 0;
	int numLines = 1;
	int firstLineMaxHeight = 0;
	int lastLinePadding = 0;

	std::vector<float> vLineLengths;
	
	// Iterate through all characters to find layout space requirements
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		char charCurrent = *c;

		if (charCurrent == '\n')
		{
			vLineLengths.push_back(static_cast<float>(cursorDistOnBaseline));
			cursorDistOnBaseline = 0;
			numLines++;
			lastLinePadding = 0;
			continue;
		}

		Character* charDesc = snellenFont ? &m_mapSloanCharacters[charCurrent] : &m_arrCharacters[charCurrent];

		if (numLines == 1 && charDesc->Bearing.y > firstLineMaxHeight)
			firstLineMaxHeight = charDesc->Bearing.y;

		int padding = charDesc->Size.y - charDesc->Bearing.y;
		if (padding > lastLinePadding)
			lastLinePadding = padding;

		cursorDistOnBaseline += (charDesc->Advance.x >> 6);
		if (cursorDistOnBaseline > maxCursorDist)
			maxCursorDist = cursorDistOnBaseline;
	}
	vLineLengths.push_back(static_cast<float>(cursorDistOnBaseline));

	glm::vec2 textDims(maxCursorDist, firstLineMaxHeight + (numLines - 1) * lineSpacing + lastLinePadding);
		
	GLfloat scale = size / (sizeDim == WIDTH ? textDims.x : textDims.y);
	
	glm::vec2 anchorPt;

	switch (anchor)
	{
	case Renderer::CENTER_MIDDLE:
		anchorPt = textDims * 0.5f;
		break;
	case Renderer::CENTER_TOP:
		anchorPt = glm::vec2(textDims.x * 0.5f, textDims.y);
		break;
	case Renderer::CENTER_BOTTOM:
		anchorPt = glm::vec2(textDims.x * 0.5f, 0.f);
		break;
	case Renderer::CENTER_LEFT:
		anchorPt = glm::vec2(0.f, textDims.y * 0.5f);
		break;
	case Renderer::CENTER_RIGHT:
		anchorPt = glm::vec2(textDims.x, textDims.y * 0.5f);
		break;
	case Renderer::TOP_LEFT:
		anchorPt = glm::vec2(0.f, textDims.y);
		break;
	case Renderer::TOP_RIGHT:
		anchorPt = glm::vec2(textDims.x, textDims.y);
		break;
	case Renderer::BOTTOM_LEFT:
		anchorPt = glm::vec2(0.f, 0.f);
		break;
	case Renderer::BOTTOM_RIGHT:
		anchorPt = glm::vec2(textDims.x, 0.f);
		break;
	default:
		break;
	}

	glm::vec2 cursor = glm::vec2(0.f, lineSpacing * (numLines - 1) + lastLinePadding) - anchorPt;
	unsigned int lineNum = 0u;

	if (alignment == TextAlignment::RIGHT)
		cursor.x = (textDims.x - vLineLengths[lineNum]) - anchorPt.x;
	else if (alignment == TextAlignment::CENTER)
		cursor.x = (textDims.x - vLineLengths[lineNum]) * 0.5f - anchorPt.x;

	for (c = text.begin(); c != text.end(); c++)
	{
		if (*c == '\n')
		{
			lineNum++;
			if (alignment == TextAlignment::RIGHT)
				cursor.x = (textDims.x - vLineLengths[lineNum]) - anchorPt.x;
			else if (alignment == TextAlignment::CENTER)
				cursor.x = (textDims.x - vLineLengths[lineNum]) * 0.5f - anchorPt.x;
			else
				cursor.x = -anchorPt.x;

			cursor.y -= lineSpacing;
			continue;
		}

		Character *ch = snellenFont ? &m_mapSloanCharacters[*c] : &m_arrCharacters[*c];

		GLfloat xpos = cursor.x + (ch->Bearing.x + 0.5f * ch->Size.x);
		GLfloat ypos = cursor.y + (ch->Bearing.y - 0.5f * ch->Size.y);

		GLfloat w = static_cast<GLfloat>(ch->Size.x);
		GLfloat h = static_cast<GLfloat>(ch->Size.y);

		glm::mat4 trans = glm::scale(glm::translate(glm::mat4(), glm::vec3(xpos, ypos, 0.f)), glm::vec3(w, h, 1.f));

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		cursor.x += (ch->Advance.x >> 6); // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
		
		if (*c == ' ')
			continue;

		std::stringstream diffTexName;
		diffTexName << *c;
		if (snellenFont) diffTexName << "_sloan";

		RendererSubmission rs;
		rs.glPrimitiveType = GL_TRIANGLES;
		rs.shaderName = "text";
		rs.modelToWorldTransform = glm::translate(glm::mat4(), pos) * glm::mat4(rot) * glm::scale(glm::mat4(), glm::vec3(scale, scale, 1.f)) * trans;
		rs.VAO = m_glPrimitivesVAO;
		rs.indexByteOffset = m_mapPrimitiveIndexByteOffsets["quaddouble"];
		rs.indexBaseVertex = m_mapPrimitiveIndexBaseVertices["quaddouble"];
		rs.vertCount = m_mapPrimitiveIndexCounts["quaddouble"];
		rs.indexType = GL_UNSIGNED_SHORT;
		rs.diffuseTexName = diffTexName.str();
		rs.diffuseColor = color;

		addToDynamicRenderQueue(rs);
	}	
}

void Renderer::drawUIText(std::string text, glm::vec4 color, glm::vec3 pos, glm::quat rot, GLfloat size, TextSizeDim sizeDim, TextAlignment alignment, TextAnchor anchor)
{
	float lineSpacing = m_uiFontPointSize * 1.f;

	// cursor origin is at beginning of text baseline
	int cursorDistOnBaseline = 0;
	int maxCursorDist = 0;
	int numLines = 1;
	int firstLineMaxHeight = 0;
	int lastLinePadding = 0;

	std::vector<float> vLineLengths;

	// Iterate through all characters to find layout space requirements
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		if (*c == '\n')
		{
			vLineLengths.push_back(static_cast<float>(cursorDistOnBaseline));
			cursorDistOnBaseline = 0;
			numLines++;
			lastLinePadding = 0;
			continue;
		}

		if (numLines == 1 && m_arrCharacters[*c].Bearing.y > firstLineMaxHeight)
			firstLineMaxHeight = m_arrCharacters[*c].Bearing.y;

		int padding = m_arrCharacters[*c].Size.y - m_arrCharacters[*c].Bearing.y;
		if (padding > lastLinePadding)
			lastLinePadding = padding;

		cursorDistOnBaseline += (m_arrCharacters[*c].Advance.x >> 6);
		if (cursorDistOnBaseline > maxCursorDist)
			maxCursorDist = cursorDistOnBaseline;
	}
	vLineLengths.push_back(static_cast<float>(cursorDistOnBaseline));

	glm::vec2 textDims(maxCursorDist, firstLineMaxHeight + (numLines - 1) * lineSpacing + lastLinePadding);

	GLfloat scale = size / (sizeDim == WIDTH ? textDims.x : textDims.y);

	glm::vec2 anchorPt;

	switch (anchor)
	{
	case Renderer::CENTER_MIDDLE:
		anchorPt = textDims * 0.5f;
		break;
	case Renderer::CENTER_TOP:
		anchorPt = glm::vec2(textDims.x * 0.5f, textDims.y);
		break;
	case Renderer::CENTER_BOTTOM:
		anchorPt = glm::vec2(textDims.x * 0.5f, 0.f);
		break;
	case Renderer::CENTER_LEFT:
		anchorPt = glm::vec2(0.f, textDims.y * 0.5f);
		break;
	case Renderer::CENTER_RIGHT:
		anchorPt = glm::vec2(textDims.x, textDims.y * 0.5f);
		break;
	case Renderer::TOP_LEFT:
		anchorPt = glm::vec2(0.f, textDims.y);
		break;
	case Renderer::TOP_RIGHT:
		anchorPt = glm::vec2(textDims.x, textDims.y);
		break;
	case Renderer::BOTTOM_LEFT:
		anchorPt = glm::vec2(0.f, 0.f);
		break;
	case Renderer::BOTTOM_RIGHT:
		anchorPt = glm::vec2(textDims.x, 0.f);
		break;
	default:
		break;
	}

	glm::vec2 cursor = glm::vec2(0.f, lineSpacing * (numLines - 1) + lastLinePadding) - anchorPt;
	unsigned int lineNum = 0u;

	if (alignment == TextAlignment::RIGHT)
		cursor.x = (textDims.x - vLineLengths[lineNum]) - anchorPt.x;
	else if (alignment == TextAlignment::CENTER)
		cursor.x = (textDims.x - vLineLengths[lineNum]) * 0.5f - anchorPt.x;

	for (c = text.begin(); c != text.end(); c++)
	{
		if (*c == '\n')
		{
			lineNum++;
			if (alignment == TextAlignment::RIGHT)
				cursor.x = (textDims.x - vLineLengths[lineNum]) - anchorPt.x;
			else if (alignment == TextAlignment::CENTER)
				cursor.x = (textDims.x - vLineLengths[lineNum]) * 0.5f - anchorPt.x;
			else
				cursor.x = -anchorPt.x;

			cursor.y -= lineSpacing;
			continue;
		}

		Character ch = m_arrCharacters[*c];

		GLfloat xpos = cursor.x + (ch.Bearing.x + 0.5f * ch.Size.x);
		GLfloat ypos = cursor.y + (ch.Bearing.y - 0.5f * ch.Size.y);

		GLfloat w = static_cast<GLfloat>(ch.Size.x);
		GLfloat h = static_cast<GLfloat>(ch.Size.y);

		glm::mat4 trans = glm::scale(glm::translate(glm::mat4(), glm::vec3(xpos, ypos, 0.f)), glm::vec3(w, h, 1.f));

		RendererSubmission rs;
		rs.glPrimitiveType = GL_TRIANGLES;
		rs.shaderName = "text";
		rs.modelToWorldTransform = glm::translate(glm::mat4(), pos) * glm::mat4(rot) * glm::scale(glm::mat4(), glm::vec3(scale, scale, 1.f)) * trans;
		rs.VAO = m_glPrimitivesVAO;
		rs.indexByteOffset = m_mapPrimitiveIndexByteOffsets["quad"];
		rs.indexBaseVertex = m_mapPrimitiveIndexBaseVertices["quad"];
		rs.vertCount = m_mapPrimitiveIndexCounts["quad"];
		rs.indexType = GL_UNSIGNED_SHORT;
		rs.diffuseTexName = *c;
		rs.diffuseColor = color;

		addToUIRenderQueue(rs);

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		cursor.x += (ch.Advance.x >> 6); // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
}

glm::vec2 Renderer::getTextDimensions(std::string text, float size, TextSizeDim sizeDim)
{
	float lineSpacing = m_uiFontPointSize * 1.f;

	// cursor origin is at beginning of text baseline
	int cursorDistOnBaseline = 0;
	int maxCursorDist = 0;
	int numLines = 1;
	int firstLineMaxHeight = 0;
	int lastLinePadding = 0;

	// Iterate through all characters to find layout space requirements
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		if (*c == '\n')
		{
			cursorDistOnBaseline = 0;
			numLines++;
			lastLinePadding = 0;
			continue;
		}

		if (numLines == 1 && m_arrCharacters[*c].Bearing.y > firstLineMaxHeight)
			firstLineMaxHeight = m_arrCharacters[*c].Bearing.y;

		int padding = m_arrCharacters[*c].Size.y - m_arrCharacters[*c].Bearing.y;
		if (padding > lastLinePadding)
			lastLinePadding = padding;

		cursorDistOnBaseline += (m_arrCharacters[*c].Advance.x >> 6);
		if (cursorDistOnBaseline > maxCursorDist)
			maxCursorDist = cursorDistOnBaseline;
	}
	glm::vec2 textDims(maxCursorDist, firstLineMaxHeight + (numLines - 1) * lineSpacing + lastLinePadding);

	GLfloat scale = size / (sizeDim == WIDTH ? textDims.x : textDims.y);

	return textDims * scale;
}

GLuint Renderer::createInstancedDataBufferVBO(std::vector<glm::vec3>* instancePositions, std::vector<glm::vec4>* instanceColors)
{
	size_t nPoints = instancePositions->size();

	assert(nPoints == instanceColors->size());


	GLuint buffer;
	glCreateBuffers(1, &buffer);

	glNamedBufferStorage(buffer, nPoints * sizeof(glm::vec3) + nPoints * sizeof(glm::vec4), NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData(buffer, 0, nPoints * sizeof(glm::vec3), instancePositions->data());
	glNamedBufferSubData(buffer, nPoints * sizeof(glm::vec3), nPoints * sizeof(glm::vec4), instanceColors->data());

	return buffer;
}

GLuint Renderer::createInstancedPrimitiveVAO(std::string primitiveName, GLuint instanceDataVBO, GLsizei instanceCount, GLsizei instanceStride)
{
	if (!instanceDataVBO)
	{
		ccomutils::dprintf("%s ERROR: Supplied data buffer for creating instanced %s primitives is invalid!\n", __FUNCTION__, primitiveName.c_str());
		return 0;
	}

	// Create  VAO
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glPrimitivesEBO);

		// Describe primitive verts
		glBindBuffer(GL_ARRAY_BUFFER, m_glPrimitivesVBO);
			glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
			glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, p));
			glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
			glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, n));
			glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
			glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, c));
			glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
			glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, t));

		// Describe instanced primitives
		glBindBuffer(GL_ARRAY_BUFFER, instanceDataVBO);
			glEnableVertexAttribArray(INSTANCE_POSITION_ATTRIB_LOCATION);
			glVertexAttribPointer(INSTANCE_POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * instanceStride, (GLvoid*)0);
			glVertexAttribDivisor(INSTANCE_POSITION_ATTRIB_LOCATION, 1);
			glEnableVertexAttribArray(INSTANCE_COLOR_ATTRIB_LOCATION);
			glVertexAttribPointer(INSTANCE_COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) * instanceStride, (GLvoid*)(instanceCount * sizeof(glm::vec3)));
			glVertexAttribDivisor(INSTANCE_COLOR_ATTRIB_LOCATION, 1);
	glBindVertexArray(0);

	return vao;
}

GLuint Renderer::getPrimitiveVAO()
{
	return m_glPrimitivesVAO;
}

GLuint Renderer::getPrimitiveVBO()
{
	return m_glPrimitivesVBO;
}

GLuint Renderer::getPrimitiveEBO()
{
	return m_glPrimitivesEBO;
}

unsigned long long Renderer::getPrimitiveIndexByteOffset(std::string primName)
{
	auto prim = m_mapPrimitiveIndexByteOffsets.find(primName);

	if (prim == m_mapPrimitiveIndexByteOffsets.end())
	{
		std::cerr << "Primitive \"" << primName << "\" not found!" << std::endl;
		return 0;
	}

	return prim->second;
}

GLint Renderer::getPrimitiveIndexBaseVertex(std::string primName)
{
	auto prim = m_mapPrimitiveIndexBaseVertices.find(primName);

	if (prim == m_mapPrimitiveIndexBaseVertices.end())
	{
		std::cerr << "Primitive \"" << primName << "\" not found!" << std::endl;
		return 0;
	}

	return prim->second;
}

GLsizei Renderer::getPrimitiveIndexCount(std::string primName)
{
	auto prim = m_mapPrimitiveIndexCounts.find(primName);

	if (prim == m_mapPrimitiveIndexCounts.end())
	{
		std::cerr << "Primitive \"" << primName << "\" not found!" << std::endl;
		return 0;
	}

	return prim->second;
}