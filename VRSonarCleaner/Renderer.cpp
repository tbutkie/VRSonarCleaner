#include "Renderer.h"

#include <vector>
#include <numeric>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/norm.hpp>

#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "DebugDrawer.h"
#include "Icosphere.h"
#include "GLSLpreamble.h"

Renderer::Renderer()
	: m_pLighting(NULL)
	, m_glFrameUBO(0)
	, m_glFullscreenTextureVAO(0)
	, m_bShowWireframe(false)
	, m_uiFontPointSize(144u)
{
}

Renderer::~Renderer()
{
}

void Renderer::shutdown()
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

	setupTextures();

	setupPrimitives();

	setupFullscreenQuad();

	setupText();

	return true;
}

void Renderer::addToStaticRenderQueue(RendererSubmission &rs)
{
	if (m_mapTextures[rs.diffuseTexName] == NULL)
	{
		printf("Error: Renderer submission diffuse texture \"%s\" not found\n", rs.diffuseTexName);
		return;
	}

	if (m_mapTextures[rs.specularTexName] == NULL)
	{
		printf("Error: Renderer submission specular texture \"%s\" not found\n", rs.specularTexName);
		return;
	}

	GLTexture* diff = m_mapTextures[rs.diffuseTexName];
	GLTexture* spec = m_mapTextures[rs.diffuseTexName];

	if (rs.hasTransparency || diff->hasTransparency() || spec->hasTransparency() || rs.diffuseColor.a < 1.f || rs.specularColor.a < 1.f)
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
		printf("Error: Renderer submission diffuse texture \"%s\" not found\n", rs.diffuseTexName);
		return;
	}

	if (m_mapTextures[rs.specularTexName] == NULL)
	{
		printf("Error: Renderer submission specular texture \"%s\" not found\n", rs.specularTexName);
		return;
	}

	GLTexture* diff = m_mapTextures[rs.diffuseTexName];
	GLTexture* spec = m_mapTextures[rs.diffuseTexName];

	if (rs.hasTransparency || diff->hasTransparency() || spec->hasTransparency() || rs.diffuseColor.a < 1.f || rs.specularColor.a < 1.f)
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

bool Renderer::drawPrimitive(std::string primName, glm::mat4 modelTransform, std::string diffuseTextureName, std::string specularTextureName, float specularExponent)
{
	if (m_mapPrimitives.find(primName) == m_mapPrimitives.end())
		return false;

	RendererSubmission rs;
	rs.glPrimitiveType = GL_TRIANGLES;
	rs.shaderName = "lighting";
	rs.modelToWorldTransform = modelTransform;
	rs.VAO = m_mapPrimitives[primName].first;
	rs.vertCount = m_mapPrimitives[primName].second;
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
	if (m_mapPrimitives.find(primName) == m_mapPrimitives.end())
		return false;

	RendererSubmission rs;
	rs.glPrimitiveType = GL_TRIANGLES;
	rs.shaderName = "lighting";
	rs.modelToWorldTransform = modelTransform;
	rs.VAO = m_mapPrimitives[primName].first;
	rs.vertCount = m_mapPrimitives[primName].second;
	rs.indexType = GL_UNSIGNED_SHORT;
	rs.diffuseColor = diffuseColor;
	rs.specularColor = specularColor;
	rs.specularExponent = specularExponent;

	if (primName.find("inverse") != std::string::npos)
		rs.vertWindingOrder = GL_CW;

	addToDynamicRenderQueue(rs);

	return true;
}

bool Renderer::drawFlatPrimitive(std::string primName, glm::mat4 modelTransform, glm::vec4 color)
{
	if (m_mapPrimitives.find(primName) == m_mapPrimitives.end())
		return false;


	RendererSubmission rs;
	rs.glPrimitiveType = primName.find("_line") != std::string::npos ? GL_LINES : GL_TRIANGLES;
	rs.shaderName = "flat";
	rs.modelToWorldTransform = modelTransform;
	rs.VAO = m_mapPrimitives[primName].first;
	rs.vertCount = m_mapPrimitives[primName].second;
	rs.indexType = GL_UNSIGNED_SHORT;
	rs.diffuseColor = color;

	if (primName.find("_inverse") != std::string::npos)
		rs.vertWindingOrder = GL_CW;

	addToDynamicRenderQueue(rs);

	return true;
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
		printf("Error: Adding texture \"%s\" which already exists\n", tex->getName());
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

	m_mapShaders["vrwindow"] = m_Shaders.AddProgramFromExts({ "shaders/vrwindow.vert", "shaders/windowtexture.frag" });
	m_mapShaders["desktopwindow"] = m_Shaders.AddProgramFromExts({ "shaders/desktopwindow.vert", "shaders/windowtexture.frag" });
	m_mapShaders["lighting"] = m_Shaders.AddProgramFromExts({ "shaders/lighting.vert", "shaders/lighting.frag" });
	m_mapShaders["lightingWireframe"] = m_Shaders.AddProgramFromExts({ "shaders/lighting.vert", "shaders/lightingWF.geom", "shaders/lightingWF.frag" });
	m_mapShaders["flat"] = m_Shaders.AddProgramFromExts({ "shaders/flat.vert", "shaders/flat.frag" });
	m_mapShaders["debug"] = m_Shaders.AddProgramFromExts({ "shaders/flat.vert", "shaders/flat.frag" });
	m_mapShaders["solid"] = m_Shaders.AddProgramFromExts({ "shaders/solid.vert", "shaders/flat.frag" });
	m_mapShaders["text"] = m_Shaders.AddProgramFromExts({ "shaders/text.vert", "shaders/text.frag" });

	m_pLighting->addShaderToUpdate(m_mapShaders["lighting"]);
	m_pLighting->addShaderToUpdate(m_mapShaders["lightingWireframe"]);
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

		glEnable(GL_CULL_FACE);

		glm::mat4 vpMat = sceneView3DInfo->projection * sceneView3DInfo->view;

		glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4View), sizeof(FrameUniforms::m4View), glm::value_ptr(sceneView3DInfo->view));
		glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4Projection), sizeof(FrameUniforms::m4Projection), glm::value_ptr(sceneView3DInfo->projection));
		glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4ViewProjection), sizeof(FrameUniforms::m4ViewProjection), glm::value_ptr(vpMat));

		m_pLighting->update(sceneView3DInfo->view);

		// Opaque objects first while depth buffer writing enabled
		processRenderQueue(m_vStaticRenderQueue_Opaque);
		processRenderQueue(m_vDynamicRenderQueue_Opaque);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		processRenderQueue(m_vTransparentRenderQueue);

		// UI ELEMENTS
		if (sceneViewUIInfo)
		{
			glDisable(GL_DEPTH_TEST);
			RenderUI(sceneViewUIInfo, frameBuffer);
			glEnable(GL_DEPTH_TEST);
		}

		glDisable(GL_BLEND);

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

void Renderer::RenderUI(SceneViewInfo * sceneViewInfo, FramebufferDesc * frameBuffer)
{
	glm::mat4 vpMat = sceneViewInfo->projection * sceneViewInfo->view;

	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4View), sizeof(FrameUniforms::m4View), glm::value_ptr(sceneViewInfo->view));
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4Projection), sizeof(FrameUniforms::m4Projection), glm::value_ptr(sceneViewInfo->projection));
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4ViewProjection), sizeof(FrameUniforms::m4ViewProjection), glm::value_ptr(vpMat));

	processRenderQueue(m_vUIRenderQueue);
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
			glDrawElements(i.glPrimitiveType, i.vertCount, i.indexType, 0);
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
	generateIcosphere(4);
	generateCylinder(32);
	generateTorus(1.f, 0.025f, 32, 8);
	generatePlane();
	generateBBox();
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::setupFullscreenQuad()
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
	glNamedBufferStorage(m_glFullscreenTextureVBO, sizeof(vVerts) * sizeof(glm::vec2), &vVerts[0], GL_NONE);

	// Generate/allocate and fill index buffer object
	glCreateBuffers(1, &m_glFullscreenTextureEBO);
	glNamedBufferStorage(m_glFullscreenTextureEBO, m_uiCompanionWindowVertCount * sizeof(GLushort), &vIndices[0], GL_NONE);

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


void Renderer::generateIcosphere(int recursionLevel)
{
	Icosphere ico(recursionLevel);

	m_glIcosphereVAO = ico.getVAO();

	m_mapPrimitives["icosphere"] = m_mapPrimitives["inverse_icosphere"] = std::make_pair(m_glIcosphereVAO, ico.getIndices().size());
}


void Renderer::generateTorus(float coreRadius, float meridianRadius, int numCoreSegments, int numMeridianSegments)
{
	int nVerts = numCoreSegments * numMeridianSegments;

	std::vector<PrimVert> verts;
	std::vector<GLushort> inds;

	for (int i = 0; i < numCoreSegments; i++)
		for (int j = 0; j < numMeridianSegments; j++)
		{
			float u = i / (numCoreSegments - 1.f);
			float v = j / (numMeridianSegments - 1.f);
			float theta = u * 2.f * M_PI;
			float rho = v * 2.f * M_PI;
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

	glGenVertexArrays(1, &m_glTorusVAO);
	glGenBuffers(1, &m_glTorusVBO);
	glGenBuffers(1, &m_glTorusEBO);

	glBindVertexArray(this->m_glTorusVAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->m_glTorusVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glTorusEBO);

		// Set the vertex attribute pointers
		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*) offsetof(PrimVert, p));
		glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
		glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*) offsetof(PrimVert, n));
		glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
		glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, c));
		glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
		glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, t));
	glBindVertexArray(0);

	// Allocate and store buffer data and indices
	glNamedBufferStorage(m_glTorusVBO, verts.size() * sizeof(PrimVert), &verts[0], GL_NONE);
	glNamedBufferStorage(m_glTorusEBO, inds.size() * sizeof(GLushort), &inds[0], GL_NONE);

	m_mapPrimitives["torus"] = std::make_pair(m_glTorusVAO, inds.size());
}

void Renderer::generateCylinder(int numSegments)
{
	std::vector<PrimVert> verts;
	std::vector<GLushort> inds;

	// Front endcap
	verts.push_back(PrimVert({ glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.5f, 0.5f) }));
	for (float i = 0; i < numSegments; ++i)
	{
		float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
		verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), (glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f }));

		if (i > 0)
		{
			inds.push_back(0);
			inds.push_back(verts.size() - 2);
			inds.push_back(verts.size() - 1);
		}
	}
	inds.push_back(0);
	inds.push_back(verts.size() - 1);
	inds.push_back(1);

	// Back endcap
	verts.push_back(PrimVert({ glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(0.5f, 0.5f) }));
	for (float i = 0; i < numSegments; ++i)
	{
		float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
		verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), (glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f }));

		if (i > 0)
		{
			inds.push_back(verts.size() - (i + 2)); // ctr pt of endcap
			inds.push_back(verts.size() - 1);
			inds.push_back(verts.size() - 2);
		}
	}
	inds.push_back(verts.size() - (numSegments + 1));
	inds.push_back(verts.size() - (numSegments));
	inds.push_back(verts.size() - 1);

	// Shaft
	for (float i = 0; i < numSegments; ++i)
	{
		float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
		verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 0.f), glm::vec3(sin(angle), cos(angle), 0.f), glm::vec4(1.f), glm::vec2((float)i / (float)(numSegments - 1), 0.f) }));

		verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 1.f), glm::vec3(sin(angle), cos(angle), 0.f), glm::vec4(1.f), glm::vec2((float)i / (float)(numSegments - 1), 1.f) }));

		if (i > 0)
		{
			inds.push_back(verts.size() - 4);
			inds.push_back(verts.size() - 3);
			inds.push_back(verts.size() - 2);

			inds.push_back(verts.size() - 2);
			inds.push_back(verts.size() - 3);
			inds.push_back(verts.size() - 1);
		}
	}
	inds.push_back(verts.size() - 2);
	inds.push_back(verts.size() - numSegments * 2);
	inds.push_back(verts.size() - 1);

	inds.push_back(verts.size() - numSegments * 2);
	inds.push_back(verts.size() - numSegments * 2 + 1);
	inds.push_back(verts.size() - 1);

	glGenVertexArrays(1, &m_glCylinderVAO);
	glGenBuffers(1, &m_glCylinderVBO);
	glGenBuffers(1, &m_glCylinderEBO);

	// Setup VAO
	glBindVertexArray(this->m_glCylinderVAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->m_glCylinderVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glCylinderEBO);

		// Set the vertex attribute pointers
		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*) offsetof(PrimVert, p));
		glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
		glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*) offsetof(PrimVert, n));
		glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
		glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*) offsetof(PrimVert, c));
		glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
		glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*) offsetof(PrimVert, t));
	glBindVertexArray(0);

	// Allocate buffer data
	glNamedBufferStorage(m_glCylinderVBO, verts.size() * sizeof(PrimVert), &verts[0], GL_NONE);
	// Element array buffer
	glNamedBufferStorage(m_glCylinderEBO, inds.size() * sizeof(GLushort), &inds[0], GL_NONE);

	m_mapPrimitives["cylinder"] = std::make_pair(m_glCylinderVAO, inds.size());
}

void Renderer::generatePlane()
{
	std::vector<PrimVert> verts;
	std::vector<GLushort> inds;

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

	glGenVertexArrays(1, &m_glPlaneVAO);
	glGenBuffers(1, &m_glPlaneVBO);
	glGenBuffers(1, &m_glPlaneEBO);

	// Setup VAO
	glBindVertexArray(this->m_glPlaneVAO);
		// Load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->m_glPlaneVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glPlaneEBO);

		// Set the vertex attribute pointers
		glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
		glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*) offsetof(PrimVert, p));
		glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
		glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*) offsetof(PrimVert, n));
		glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
		glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*) offsetof(PrimVert, c));
		glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
		glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*) offsetof(PrimVert, t));
	glBindVertexArray(0);

	// Alloc buffer and store data
	glNamedBufferStorage(m_glPlaneVBO, verts.size() * sizeof(PrimVert), &verts[0], GL_NONE);
	// Element array buffer
	glNamedBufferStorage(m_glPlaneEBO, inds.size() * sizeof(GLushort), &inds[0], GL_NONE);

	m_mapPrimitives["plane"] = m_mapPrimitives["quad"] = std::make_pair(m_glPlaneVAO, 6); // one sided
	m_mapPrimitives["planedouble"] = m_mapPrimitives["quaddouble"] = std::make_pair(m_glPlaneVAO, 12); // two sided
}


// Essentially a unit cube wireframe
void Renderer::generateBBox()
{
	std::vector<PrimVert> verts;

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

	std::vector<GLushort> inds(12 * 2);
	std::iota(inds.begin(), inds.end(), 0u);

	glGenVertexArrays(1, &m_glBBoxVAO);
	glGenBuffers(1, &m_glBBoxVBO);
	glGenBuffers(1, &m_glBBoxEBO);

	// Setup VAO
	glBindVertexArray(m_glBBoxVAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, m_glBBoxVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glBBoxEBO);

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

	// Alloc buffer and store data
	glNamedBufferStorage(m_glBBoxVBO, verts.size() * sizeof(PrimVert), &verts[0], GL_NONE);
	// Element array buffer
	glNamedBufferStorage(m_glBBoxEBO, inds.size() * sizeof(GLushort), &inds[0], GL_NONE);

	m_mapPrimitives["bbox_lines"] = std::make_pair(m_glBBoxVAO, inds.size()); 
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
	if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face))
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
		//std::cout << "Adding character \'" << std::string((const char*)&c) << "\' to text renderer..." << std::endl;
		addTexture(new GLTexture(std::string(tmp), face->glyph->bitmap.width, face->glyph->bitmap.rows, texture, true));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	// Configure VAO/VBO for texture quads
	glGenVertexArrays(1, &m_glTextVAO);
	glGenBuffers(1, &m_glTextVBO);
	glBindVertexArray(m_glTextVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_glTextVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Renderer::drawText(std::string text, GLfloat width_meters, glm::vec4 color, TextAnchor anchor)
{
	float lineSpacing = m_uiFontPointSize * 1.f;

	// cursor origin is at beginning of text baseline
	glm::vec2 cursor(0.f);
	glm::vec2 textDims(0.f);
	GLuint numLines = 1u;
	
	// Iterate through all characters to find layout space requirements
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		if (*c == '\n')
		{
			cursor.x = 0.f;
			numLines++;
			continue;
		}

		cursor.x += (m_arrCharacters[*c].Advance.x >> 6);
		if (cursor.x > textDims.x)
			textDims.x = cursor.x;
	}
	textDims.y = numLines * lineSpacing;
		
	GLfloat scale = width_meters / textDims.x;
	
	glm::vec2 anchorPt;

	switch (anchor)
	{
	case Renderer::CENTER:
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

	cursor = glm::vec2(0.f, -m_uiFontPointSize) * scale;

	for (c = text.begin(); c != text.end(); c++)
	{
		if (*c == '\n')
		{
			if (cursor.x > textDims.x)
				textDims.x = cursor.x;

			cursor.x = 0.f;
			cursor.y -= m_uiFontPointSize * scale;
			continue;
		}

		Character ch = m_arrCharacters[*c];

		glm::vec2 anchorToCursorOffset(0.f);

		GLfloat xpos = (cursor.x + (ch.Bearing.x + 0.5f * ch.Size.x) - anchorToCursorOffset.x) * scale;
		GLfloat ypos = (cursor.y + (ch.Bearing.y - 0.5f * ch.Size.y) - anchorToCursorOffset.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;

		glm::mat4 trans = glm::translate(glm::mat4(), glm::vec3(xpos, ypos, 0.f)) * glm::scale(glm::mat4(), glm::vec3(w, h, 1.f));


		RendererSubmission rs;
		rs.glPrimitiveType = GL_TRIANGLES;
		rs.shaderName = "text";
		rs.modelToWorldTransform = glm::translate(glm::mat4(), glm::vec3(0.f, 1.f, 0.f)) * trans;
		rs.VAO = m_mapPrimitives["quaddouble"].first;
		rs.vertCount = m_mapPrimitives["quaddouble"].second;
		rs.indexType = GL_UNSIGNED_SHORT;
		rs.diffuseTexName = *c;
		rs.diffuseColor = color;

		addToStaticRenderQueue(rs);

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		cursor.x += (ch.Advance.x >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}	
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Renderer::RenderFullscreenTexture(int width, int height, GLuint textureID, bool textureAspectPortrait)
{
	GLuint* shader;

	if (textureAspectPortrait)
		shader = m_mapShaders["vrwindow"];
	else
		shader = m_mapShaders["desktopwindow"];

	if (shader == NULL)
		return;

	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, width, height); 
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, v4Viewport), sizeof(FrameUniforms::v4Viewport), glm::value_ptr(glm::vec4(0, 0, width, height)));

	glUseProgram(*shader);
	glBindVertexArray(m_glFullscreenTextureVAO);

	// render left eye (first half of index array )
	glBindTextureUnit(DIFFUSE_TEXTURE_BINDING, textureID);
	glDrawElements(GL_TRIANGLES, m_uiCompanionWindowVertCount, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}
