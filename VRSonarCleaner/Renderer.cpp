#include "Renderer.h"

#include <vector>
#include <numeric>
#include <unordered_map>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/norm.hpp>
#include <lodepng.h>

#include <string>
#include <sstream>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "GLSLpreamble.h"
#include "utilities.h"

#include "PrimitivesFactory.h"

float	g_fDefaultNearClip = 0.01f;
float	g_fDefaultFarClip = 100.f;
float	g_fDefaultFOV(45.f);

//-----------------------------------------------------------------------------
// Purpose: OpenGL Debug Callback Function
//-----------------------------------------------------------------------------
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131184 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}


Renderer::Renderer()
	: m_pLighting(NULL)
	, m_glFrameUBO(0)
	, m_bShowWireframe(false)
	, m_uiFontPointSize(144u)
	, m_tpStart(std::chrono::high_resolution_clock::now())
	, m_bShowSkybox(true)
	, m_bStereoWindow(false)
	, m_bStereoRender(true)
	, m_vec4ClearColor(glm::vec4(0.15f, 0.15f, 0.18f, 1.f))
	, m_sviMonoInfo(&m_sviLeftEyeInfo)
{
}

Renderer::~Renderer()
{
}

void Renderer::shutdown()
{
	for (auto fb : { m_pWindowFramebuffer , m_pLeftEyeFramebuffer, m_pRightEyeFramebuffer })
		if (fb)
			Renderer::getInstance().destroyFrameBuffer(*fb);

	m_pMonoFramebuffer = NULL;

#if _DEBUG
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
	glDebugMessageCallback(nullptr, nullptr);
#endif

	SDL_DestroyWindow(m_pWindow);
}

void Renderer::setStereoRenderSize(glm::ivec2 res)
{
	for (auto &fb : { m_pLeftEyeFramebuffer, m_pRightEyeFramebuffer })
	{
		if (fb)
		{
			destroyFrameBuffer(*fb);
			delete fb;
		}
	}

	m_pLeftEyeFramebuffer = new Renderer::FramebufferDesc();
	m_pRightEyeFramebuffer = new Renderer::FramebufferDesc();
	
	if (!createFrameBuffer(res.x, res.y, *m_pLeftEyeFramebuffer))
		utils::dprintf("Could not create left eye framebuffer!\n");
	if (!createFrameBuffer(res.x, res.y, *m_pRightEyeFramebuffer))
		utils::dprintf("Could not create right eye framebuffer!\n");

	m_pMonoFramebuffer = m_pLeftEyeFramebuffer;
}

bool Renderer::init(bool stereoRender, bool stereoContext)
{
	m_bStereoRender = stereoRender;
	m_bStereoWindow = stereoContext;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		utils::dprintf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	int nDisps = SDL_GetNumVideoDisplays();

	for (int i = 0; i < nDisps; ++i)
	{
		int nModes = SDL_GetNumDisplayModes(i);

		SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };

		for (int j = 0; j < nModes; ++j)
		{
			SDL_GetDisplayMode(i, j, &mode);
			SDL_Log("SDL_GetDisplayMode(%i, %i, &mode):\t\t%i bpp\t%i x %i\t@%iHz",
				i, j, SDL_BITSPERPIXEL(mode.format), mode.w, mode.h, mode.refresh_rate);
		}
	}

	m_pWindow = createFullscreenWindow(1, stereoContext);

	SDL_GetWindowSize(m_pWindow, &m_ivec2WindowSize.x, &m_ivec2WindowSize.y);
	
	m_pGLContext = SDL_GL_CreateContext(m_pWindow);

	if (stereoContext)
	{
		int val;
		SDL_GL_GetAttribute(SDL_GL_STEREO, &val);
		SDL_Log("SDL_GL_STEREO: %i", val);
		if (!val)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Stereo OpenGL Context Failure", "A stereoscopic OpenGL context was requested, but could not be made.", NULL);
			return false;
		}
	}

	if (m_pGLContext == NULL)
	{
		utils::dprintf("%s - VR companion window OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR));

	// Set V-Sync
	if (SDL_GL_SetSwapInterval(0) < 0)
		printf("%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError());

	glewExperimental = GL_TRUE;
	GLenum nGlewError = glewInit();
	if (nGlewError != GLEW_OK)
	{
		printf("%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString(nGlewError));
		return false;
	}

	glGetError(); // to clear the error caused deep in GLEW

#if _DEBUG
	glDebugMessageCallback((GLDEBUGPROC)DebugCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

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

	createDesktopView();

	return true;
}

void Renderer::update()
{
	m_Shaders.UpdatePrograms();
}

glm::vec4 Renderer::getClearColor()
{
	return m_vec4ClearColor;
}

void Renderer::setClearColor(glm::vec4 color)
{
	m_vec4ClearColor = color;
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

void Renderer::setSkyboxTransform(glm::mat4 trans)
{
	m_Skybox.transform = trans;
}

glm::mat4 Renderer::getSkyboxTransform()
{
	return m_Skybox.transform;
}

void Renderer::setWindowTitle(std::string title)
{
	SDL_SetWindowTitle(m_pWindow, title.c_str());
}

Renderer::SceneViewInfo * Renderer::getMonoInfo()
{
	return &(*m_sviMonoInfo);
}

Renderer::SceneViewInfo * Renderer::getLeftEyeInfo()
{
	return &m_sviLeftEyeInfo;
}

Renderer::FramebufferDesc * Renderer::getMonoFrameBuffer()
{
	return m_pMonoFramebuffer;
}

Renderer::FramebufferDesc * Renderer::getLeftEyeFrameBuffer()
{
	return m_pLeftEyeFramebuffer;
}

Renderer::SceneViewInfo * Renderer::getRightEyeInfo()
{
	return &m_sviRightEyeInfo;
}

Renderer::FramebufferDesc * Renderer::getRightEyeFrameBuffer()
{
	return m_pRightEyeFramebuffer;
}

Renderer::SceneViewInfo * Renderer::getWindow3DViewInfo()
{
	return &m_sviWindow3DInfo;
}

Renderer::SceneViewInfo * Renderer::getWindowUIInfo()
{
	return &m_sviWindowUIInfo;
}

Renderer::Camera* Renderer::getCamera()
{
	return &m_WindowCamera;
}

glm::ivec2 Renderer::getUIRenderSize()
{
	return m_ivec2WindowSize;
}

glm::ivec2 Renderer::getPresentationWindowSize()
{
	return m_ivec2WindowSize;
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

void Renderer::drawDirectedPrimitive(std::string primname, glm::vec3 from, glm::vec3 to, float thickness, glm::vec4 color)
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
	Renderer::getInstance().drawFlatPrimitive(primname, trans, color);
}

void Renderer::drawDirectedPrimitiveLit(std::string primname, glm::vec3 from, glm::vec3 to, float thickness, glm::vec4 diffColor, glm::vec4 specColor, float specExp)
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
	Renderer::getInstance().drawPrimitive(primname, trans, diffColor, specColor, specExp);
}
	

void Renderer::drawPointerLit(glm::vec3 from, glm::vec3 to, float thickness, glm::vec4 baseColor, glm::vec4 shaftColor, glm::vec4 arrowColor, glm::vec4 specColor, float specExp)
{
	glm::vec3 connectorVec(to - from);
	Renderer::getInstance().drawPrimitive("icosphere", glm::translate(glm::mat4(), from) * glm::scale(glm::mat4(), glm::vec3(thickness * 0.75f)), baseColor);
	Renderer::getInstance().drawDirectedPrimitiveLit("cylinder", from, to - connectorVec * 0.1f - glm::normalize(connectorVec) * thickness, thickness, shaftColor, specColor, specExp);
	Renderer::getInstance().drawDirectedPrimitiveLit("cone", to - connectorVec * 0.1f - glm::normalize(connectorVec) * thickness, to, thickness * 2.f, arrowColor, specColor, specExp);
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

void Renderer::toggleSkybox()
{
	m_bShowSkybox = !m_bShowSkybox;
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
	m_mapShaders["streamline"] = m_Shaders.AddProgramFromExts({ "resources/shaders/streamline.vert", "resources/shaders/streamline.frag" });

	m_pLighting->addShaderToUpdate(m_mapShaders["lighting"]);
	m_pLighting->addShaderToUpdate(m_mapShaders["lightingWireframe"]);
	m_pLighting->addShaderToUpdate(m_mapShaders["instanced_lit"]);
	m_pLighting->addShaderToUpdate(m_mapShaders["streamline"]);
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

	glClearColor(m_vec4ClearColor.r, m_vec4ClearColor.g, m_vec4ClearColor.b, m_vec4ClearColor.a); // nice background color, but not black
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
		if (m_bShowSkybox && m_Skybox.texID != 0u && m_mapShaders["skybox"] && *m_mapShaders["skybox"])
		{
			glFrontFace(GL_CCW);

			glDepthFunc(GL_LEQUAL);

			glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4View), sizeof(FrameUniforms::m4View), glm::value_ptr(sceneView3DInfo->view * m_Skybox.transform));
			glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4ViewProjection), sizeof(FrameUniforms::m4ViewProjection), glm::value_ptr(vpMat * m_Skybox.transform));
		
			glUseProgram(*m_mapShaders["skybox"]);
		
			glBindVertexArray(m_glPrimitivesVAO);
				glBindTextureUnit(0, m_Skybox.texID);
				glDrawElementsBaseVertex(GL_TRIANGLES, getPrimitiveIndexCount("skybox"), GL_UNSIGNED_SHORT, (GLvoid*)getPrimitiveIndexByteOffset("skybox"), getPrimitiveIndexBaseVertex("skybox"));
			glBindVertexArray(0);
		
			glDepthFunc(GL_LESS);
		}

		if (m_vTransparentRenderQueue.size() > 0)
		{
			glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4View), sizeof(FrameUniforms::m4View), glm::value_ptr(sceneView3DInfo->view));
			glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, m4ViewProjection), sizeof(FrameUniforms::m4ViewProjection), glm::value_ptr(vpMat));

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
void Renderer::renderFullscreenTexture(int x, int y, int width, int height, GLuint textureID, bool textureAspectPortrait)
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
	glViewport(x, y, width, height);
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, v4Viewport), sizeof(FrameUniforms::v4Viewport), glm::value_ptr(glm::vec4(x, y, width, height)));

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
	GLuint* shader = m_mapShaders["vrwindow"];

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


void Renderer::render()
{
	auto tick = std::chrono::high_resolution_clock::now();
	float secsElapsed = std::chrono::duration<float>(tick - m_tpStart).count();
	glNamedBufferSubData(m_glFrameUBO, offsetof(FrameUniforms, fGlobalTime), sizeof(FrameUniforms::fGlobalTime), &secsElapsed);

	if (m_bStereoRender)
	{
		// VR Render
		// We don't draw the UI layer to the VR left/right eye textures, only to presentation windows on the PC display
		Renderer::getInstance().renderFrame(&m_sviLeftEyeInfo, m_pLeftEyeFramebuffer);
		Renderer::getInstance().renderFrame(&m_sviRightEyeInfo, m_pRightEyeFramebuffer);		
	}
	else
	{
		Renderer::getInstance().renderFrame(&(*m_sviMonoInfo), m_pMonoFramebuffer);
	}

	if (m_bStereoWindow) //DESKTOP STEREO RENDER
	{
		// FOR LEFT EYE
		// Bind left eye framebuffer render buffer to read/write
		// Draw UI Layer to left eye render target and resolve to left eye texture
		// Bind Read Framebuffer to resolved left eye texture
		// Bind Draw Framebuffer to window/default
		// Set glDrawBuffer to GL_BACK_LEFT
		// Render to full-screen quad
		// Repeat for right eye, setting glDrawBuffer to GL_BACK_RIGHT


		// Render UI to left and right eye frames
		Renderer::getInstance().renderUI(&m_sviWindowUIInfo, m_pLeftEyeFramebuffer);
		Renderer::getInstance().renderUI(&m_sviWindowUIInfo, m_pRightEyeFramebuffer);

		Renderer::getInstance().renderStereoTexture(m_ivec2WindowSize.x, m_ivec2WindowSize.y, m_pLeftEyeFramebuffer->m_nResolveTextureId, m_pRightEyeFramebuffer->m_nResolveTextureId);
	}
	else //DESKTOP NON-STEREO
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_pWindowFramebuffer->m_nRenderFramebufferId);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLint srcX0, srcX1, srcY0, srcY1;
		GLint dstX0, dstX1, dstY0, dstY1;

		if (m_sviWindowUIInfo.m_nRenderWidth < m_sviMonoInfo->m_nRenderWidth)
		{
			srcX0 = m_sviMonoInfo->m_nRenderWidth / 2 - m_sviWindowUIInfo.m_nRenderWidth / 2;
			srcX1 = srcX0 + m_sviWindowUIInfo.m_nRenderWidth;
			dstX0 = 0;
			dstX1 = m_sviWindowUIInfo.m_nRenderWidth;
		}
		else
		{
			srcX0 = 0;
			srcX1 = m_sviMonoInfo->m_nRenderWidth;

			dstX0 = m_sviWindowUIInfo.m_nRenderWidth / 2 - m_sviMonoInfo->m_nRenderWidth / 2;;
			dstX1 = dstX0 + m_sviMonoInfo->m_nRenderWidth;
		}

		if (m_sviWindowUIInfo.m_nRenderHeight < m_sviMonoInfo->m_nRenderHeight)
		{
			srcY0 = m_sviMonoInfo->m_nRenderHeight / 2 - m_sviWindowUIInfo.m_nRenderHeight / 2;
			srcY1 = srcY0 + m_sviWindowUIInfo.m_nRenderHeight;
			dstY0 = 0;
			dstY1 = m_sviWindowUIInfo.m_nRenderHeight;
		}
		else
		{
			srcY0 = 0;
			srcY1 = m_sviMonoInfo->m_nRenderHeight;

			dstY0 = m_sviWindowUIInfo.m_nRenderHeight / 2 - m_sviMonoInfo->m_nRenderHeight / 2;;
			dstY1 = dstY0 + m_sviMonoInfo->m_nRenderHeight;
		}

		glBlitNamedFramebuffer(
			m_pMonoFramebuffer->m_nRenderFramebufferId,
			m_pWindowFramebuffer->m_nRenderFramebufferId,
			srcX0, srcY0, srcX1, srcY1,
			dstX0, dstY0, dstX1, dstY1,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
			GL_NEAREST);

		Renderer::getInstance().renderUI(&m_sviWindowUIInfo, m_pWindowFramebuffer);

		Renderer::getInstance().renderFullscreenTexture(
			0, 0, m_sviWindowUIInfo.m_nRenderWidth, m_sviWindowUIInfo.m_nRenderHeight,
			m_pWindowFramebuffer->m_nResolveTextureId,
			true
		);
	}
}

void Renderer::swapAndClear()
{
	SDL_GL_SwapWindow(m_pWindow);
	clearDynamicRenderQueue();
	clearUIRenderQueue();
}

bool Renderer::sortByViewDistance(RendererSubmission const & rsLHS, RendererSubmission const & rsRHS, glm::vec3 const & HMDPos)
{
	glm::vec3 lhsPos = rsLHS.transparencySortPosition.w == -1.f ? glm::vec3(rsLHS.modelToWorldTransform[3]) : glm::vec3(rsLHS.transparencySortPosition);
	glm::vec3 rhsPos = rsRHS.transparencySortPosition.w == -1.f ? glm::vec3(rsRHS.modelToWorldTransform[3]) : glm::vec3(rsRHS.transparencySortPosition);

	return glm::length2(lhsPos - HMDPos) > glm::length2(rhsPos - HMDPos);
}


SDL_Window * Renderer::createFullscreenWindow(int displayIndex, bool stereo)
{
	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY ); //UNCOMMENT AND COMMENT LINE BELOW TO ENABLE FULL OPENGL COMMANDS
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
#if _DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
	SDL_Rect displayBounds;

	if (SDL_GetDisplayBounds(displayIndex, &displayBounds) < 0)
	{
		displayIndex = 0;
		SDL_GetDisplayBounds(displayIndex, &displayBounds);
	}

	if (stereo)
	{
		SDL_GL_SetAttribute(SDL_GL_STEREO, 1);	
	}

	SDL_Window* win = SDL_CreateWindow("CCOM VR", displayBounds.x, displayBounds.y, displayBounds.w, displayBounds.h, unWindowFlags);

	if (win == NULL)
		printf("%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());

	if (stereo)
	{
		SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };
		
		SDL_GetDisplayMode(displayIndex, 0, &mode);
		SDL_Log("SDL_GetDisplayMode(%i, %i, &mode):\t\t%i bpp\t%i x %i\t@%iHz",
			displayIndex, 0, SDL_BITSPERPIXEL(mode.format), mode.w, mode.h, mode.refresh_rate);

		mode.refresh_rate = 120;

		SDL_SetWindowDisplayMode(win, &mode);
	}

	return win;
}


SDL_Window * Renderer::createWindow(int width, int height, int displayIndex, bool stereo)
{
	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY ); //UNCOMMENT AND COMMENT LINE BELOW TO ENABLE FULL OPENGL COMMANDS
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
#if _DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	SDL_Window* win = SDL_CreateWindow("CCOM VR", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, unWindowFlags);

	if (win == NULL)
	{
		printf("%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	return win;
}

void Renderer::createDesktopView()
{
	m_sviWindowUIInfo.m_nRenderWidth = m_ivec2WindowSize.x;
	m_sviWindowUIInfo.m_nRenderHeight = m_ivec2WindowSize.y;

	m_sviWindowUIInfo.view = glm::mat4();
	m_sviWindowUIInfo.projection = glm::ortho(0.f, static_cast<float>(m_sviWindowUIInfo.m_nRenderWidth), 0.f, static_cast<float>(m_sviWindowUIInfo.m_nRenderHeight), -1.f, 1.f);
	m_sviWindowUIInfo.viewport = glm::ivec4(0, 0, m_ivec2WindowSize.x, m_ivec2WindowSize.y);

	m_sviWindow3DInfo.m_nRenderWidth = m_ivec2WindowSize.x;
	m_sviWindow3DInfo.m_nRenderHeight = m_ivec2WindowSize.y;
	m_sviWindow3DInfo.projection = glm::perspective(glm::radians(g_fDefaultFOV), (float)m_ivec2WindowSize.x / (float)m_ivec2WindowSize.y, g_fDefaultNearClip, g_fDefaultFarClip);
	m_sviWindow3DInfo.viewport = glm::ivec4(0, 0, m_ivec2WindowSize.x, m_ivec2WindowSize.y);

	if (!m_pWindowFramebuffer)
		m_pWindowFramebuffer = new FramebufferDesc();

	if (!createFrameBuffer(m_ivec2WindowSize.x, m_ivec2WindowSize.y, *m_pWindowFramebuffer))
		utils::dprintf("Could not create desktop view framebuffer!\n");
}


void Renderer::setWindowToDisplay(int displayIndex)
{
	SDL_Rect displayBounds;

	SDL_GetDisplayBounds(displayIndex, &displayBounds);

	SDL_SetWindowPosition(m_pWindow, displayBounds.x, displayBounds.y);
	SDL_SetWindowSize(m_pWindow, displayBounds.w, displayBounds.h);

	m_ivec2WindowSize = glm::ivec2(displayBounds.w, displayBounds.h);

	destroyFrameBuffer(*m_pWindowFramebuffer);
	createDesktopView();
}


void Renderer::setupPrimitives()
{
	std::vector<PrimVert> verts;
	std::vector<GLushort> inds;

	size_t baseInd = inds.size();
	size_t baseVert = verts.size();
	PrimitiveFactory::generateFullscreenQuad(verts, inds);

	for (auto primname : { "fullscreen", "fullscreenquad" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);
	}

	baseInd = inds.size();
	baseVert = verts.size();
	PrimitiveFactory::generateDisc(16, verts, inds);

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
	PrimitiveFactory::generateCube(verts, inds);

	for (auto primname : { "cube", "box", "skybox" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);
	}
	
	baseInd = inds.size();
	baseVert = verts.size();
	PrimitiveFactory::generatePlane(verts, inds);

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
	PrimitiveFactory::generateBBox(verts, inds);

	for (auto primname : { "bbox_lines" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);
	}

	baseInd = inds.size();
	baseVert = verts.size();
	PrimitiveFactory::generateIcosphere(3, verts, inds);

	for (auto primname : { "icosphere", "inverse_icosphere", "icosphere_inverse" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);
	}

	baseInd = inds.size();
	baseVert = verts.size();
	PrimitiveFactory::generateCylinder(32, verts, inds);

	for (auto primname : { "cylinder", "rod" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);
	}

	baseInd = inds.size();
	baseVert = verts.size();
	PrimitiveFactory::generateFacetedCone(16, verts, inds);

	for (auto primname : { "cone_faceted", "faceted_cone", "flat_cone", "cone_flat" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);
	}

	baseInd = inds.size();
	baseVert = verts.size();
	PrimitiveFactory::generateSmoothCone(32, verts, inds);

	for (auto primname : { "cone", "smooth_cone", "cone_smooth" })
	{
		m_mapPrimitiveIndexBaseVertices[primname] = static_cast<GLint>(baseVert);
		m_mapPrimitiveIndexCounts[primname] = static_cast<GLsizei>(inds.size() - baseInd);
		m_mapPrimitiveIndexByteOffsets[primname] = baseInd * sizeof(GLushort);
	}

	baseInd = inds.size();
	baseVert = verts.size();
	PrimitiveFactory::generateTorus(1.f, 0.025f, 32, 8, verts, inds);

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

void Renderer::drawFrustum(SceneViewInfo const * svi)
{
	// get world-space points for the viewing plane
	glm::vec3 x0y0 = glm::unProject(glm::vec3(0.f), svi->view, svi->projection, glm::vec4(0.f, 0.f, svi->m_nRenderWidth, svi->m_nRenderHeight));
	glm::vec3 x1y0 = glm::unProject(glm::vec3(svi->m_nRenderWidth, 0.f, 0.f), svi->view, svi->projection, glm::vec4(0.f, 0.f, svi->m_nRenderWidth, svi->m_nRenderHeight));
	glm::vec3 x0y1 = glm::unProject(glm::vec3(0.f, svi->m_nRenderHeight, 0.f), svi->view, svi->projection, glm::vec4(0.f, 0.f, svi->m_nRenderWidth, svi->m_nRenderHeight));
	glm::vec3 x1y1 = glm::unProject(glm::vec3(svi->m_nRenderWidth, svi->m_nRenderHeight, 0.f), svi->view, svi->projection, glm::vec4(0.f, 0.f, svi->m_nRenderWidth, svi->m_nRenderHeight));

	// draw the viewing plane
	drawDirectedPrimitiveLit("cylinder", x0y0, x1y0, 0.01f, glm::vec4(1.f, 0.f, 1.f, 1.f));
	drawDirectedPrimitiveLit("cylinder", x1y0, x1y1, 0.01f, glm::vec4(1.f, 0.f, 1.f, 1.f));
	drawDirectedPrimitiveLit("cylinder", x1y1, x0y1, 0.01f, glm::vec4(1.f, 0.f, 1.f, 1.f));
	drawDirectedPrimitiveLit("cylinder", x0y1, x0y0, 0.01f, glm::vec4(1.f, 0.f, 1.f, 1.f));
	
	glm::vec3 viewPos = glm::vec3(glm::inverse(svi->view)[3]);

	// connect the viewing plane corners to view pos
	drawDirectedPrimitiveLit("cylinder", viewPos, x1y0, 0.01f, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 1.f, 1.f));
	drawDirectedPrimitiveLit("cylinder", viewPos, x1y1, 0.01f, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 1.f, 1.f));
	drawDirectedPrimitiveLit("cylinder", viewPos, x0y1, 0.01f, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 1.f, 1.f));
	drawDirectedPrimitiveLit("cylinder", viewPos, x0y0, 0.01f, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 1.f, 1.f));
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
		utils::dprintf("%s ERROR: Supplied data buffer for creating instanced %s primitives is invalid!\n", __FUNCTION__, primitiveName.c_str());
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