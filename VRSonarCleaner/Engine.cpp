#include "Engine.h"
#include "InfoBoxManager.h"

#include "BehaviorManager.h"
#include "DataLogger.h"
#include "FlowScene.h"
#include "SonarScene.h"

#include "HolodeckBackground.h"
#include "utilities.h"

#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <limits>


glm::vec3						g_vec3RoomSize(1.f, 3.f, 1.f);

float							g_fNearClip = 0.01f;
float							g_fFarClip = 100.f;
const glm::ivec2				g_ivec2DesktopInitialWindowSize(500, 500);
float							g_fDesktopWindowFOV(45.f);

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


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
Engine::Engine()
	: m_bGreatBayModel(false)
	, m_bShowDesktopFrustum(false)
	, m_bDemoMode(false)
	, m_bShowDiagnostics(false)
	, m_bGLInitialized(false)
	, m_pCurrentScene(NULL)
	, m_pWindow(NULL)
	, m_pWindowCursor(NULL)
	, m_pGLContext(NULL)
	, m_pWindowFramebuffer(NULL)
	, m_pLeftEyeFramebuffer(NULL)
	, m_pRightEyeFramebuffer(NULL)
	, m_pHMD(NULL)
{
};


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
Engine::~Engine()
{
	// work is done in Shutdown
	utils::dprintf("Shutdown");
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Engine::init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		utils::dprintf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	m_pWindow = createFullscreenWindow(1);

	SDL_GetWindowSize(m_pWindow, &m_ivec2WindowSize.x, &m_ivec2WindowSize.y);

	m_pGLContext = SDL_GL_CreateContext(m_pWindow);

	if (m_pGLContext == NULL)
	{
		utils::dprintf("%s - VR companion window OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	if (!initGL())
	{
		utils::dprintf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
		return false;
	}

	if (!initDesktop())
	{
		utils::dprintf("%s - Unable to initialize desktop window!\n", __FUNCTION__);
		return false;
	}

	if (!initVR())
	{
		utils::dprintf("%s - Unable to initialize VR!\n", __FUNCTION__);
		return false;
	}

	BehaviorManager::getInstance().init();

	Renderer::getInstance().setSkybox(
		"resources/images/skybox/sea/right.png",
		"resources/images/skybox/sea/left.png",
		"resources/images/skybox/sea/top.png",
		"resources/images/skybox/sea/bottom.png",
		"resources/images/skybox/sea/front.png",
		"resources/images/skybox/sea/back.png"
	);

	if (false)
	{
		m_pCurrentScene = new SonarScene(m_pWindow, m_pTDM);
	}
	else
	{
		m_pCurrentScene = new FlowScene(m_pWindow, m_pTDM);
	}

	m_pCurrentScene->init();

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Engine::initGL()
{
	if (m_bGLInitialized)
		return true;

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

	if (!Renderer::getInstance().init())
		return false;

	m_bGLInitialized = true;

	return true;
}

bool Engine::initVR()
{
	// Loading the SteamVR Runtime
	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);

	if (eError != vr::VRInitError_None)
	{
		m_pHMD = NULL;
		char buf[1024];
		sprintf_s(buf, sizeof(buf), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL);
		return false;
	}

	if (!vr::VRCompositor())
	{
		utils::dprintf("Compositor initialization failed. See log file for details\n");
		return false;
	}

	vr::VRChaperone()->GetPlayAreaSize(&g_vec3RoomSize.x, &g_vec3RoomSize.z);
	utils::dprintf("Play bounds %fx%f\n", g_vec3RoomSize.x, g_vec3RoomSize.z);

	m_pTDM = new TrackedDeviceManager(m_pHMD);

	if (!m_pTDM->init())
	{
		utils::dprintf("Error initializing TrackedDeviceManager\n");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", "Could not initialize the Tracked Device Manager", NULL);
	}

	InfoBoxManager::getInstance().BInit(m_pTDM);

	createVRViews();

	return true;
}


bool Engine::initDesktop()
{
	m_pWindowCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	SDL_SetCursor(m_pWindowCursor);
	
	createDesktopView();

	return true;
}

void Engine::shutdownVR()
{
	if (m_pTDM)
	{
		delete m_pTDM;
		m_pTDM = NULL;
	}

	vr::VR_Shutdown();
	m_pHMD = NULL;
}

void Engine::shutdownDesktop()
{
	SDL_DestroyWindow(m_pWindow);
	m_pWindow = NULL;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Engine::Shutdown()
{
	BehaviorManager::getInstance().shutdown();

	DebugDrawer::getInstance().shutdown();

	Renderer::getInstance().shutdown();

	if (m_pGLContext)
	{
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(nullptr, nullptr);
	}

	shutdownVR();

	shutdownDesktop();

	SDL_Quit();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Engine::HandleInput()
{
	SDL_Event sdlEvent;
	bool bRet = false;

	if (m_pTDM)
		m_pTDM->handleEvents();

	while (SDL_PollEvent(&sdlEvent) != 0)
	{


		if (sdlEvent.type == SDL_KEYDOWN && sdlEvent.key.keysym.mod & KMOD_CTRL)
		{
			if (sdlEvent.key.keysym.sym == SDLK_0)
				setWindowToDisplay(m_pWindow, 0);
			if (sdlEvent.key.keysym.sym == SDLK_1)
				setWindowToDisplay(m_pWindow, 1);
		}

		
		{
			if (sdlEvent.type == SDL_QUIT)
			{
				bRet = true;
			}
			else if (sdlEvent.type == SDL_KEYDOWN)
			{
				if (sdlEvent.key.keysym.sym == SDLK_ESCAPE
					|| sdlEvent.key.keysym.sym == SDLK_q)
				{
					bRet = true;
				}

				if (sdlEvent.key.keysym.sym == SDLK_j)
				{
					Renderer::getInstance().showMessage("This is a test!");
				}
				
				if (sdlEvent.key.keysym.sym == SDLK_f)
				{
					//using namespace std::experimental::filesystem::v1;
					//auto herePath = current_path();
					//std::cout << "Current directory: " << herePath << std::endl;
					//for (directory_iterator it(herePath); it != directory_iterator(); ++it)
					//	if (is_regular_file(*it))
					//		std::cout << (*it) << std::endl;
				}

				
				if (sdlEvent.key.keysym.sym == SDLK_c)
				{
					std::cout << DataLogger::getInstance().getTimeSinceLogStartString() << "\n";
				}
				
				if ((sdlEvent.key.keysym.mod & KMOD_LCTRL) && sdlEvent.key.keysym.sym == SDLK_w)
				{
					Renderer::getInstance().toggleWireframe();
				}
				
				if (!(sdlEvent.key.keysym.mod & KMOD_LCTRL) && sdlEvent.key.keysym.sym == SDLK_f)
				{
					m_bShowDesktopFrustum = !m_bShowDesktopFrustum;
				}

				if ((sdlEvent.key.keysym.mod & KMOD_LCTRL) && sdlEvent.key.keysym.sym == SDLK_f)
				{
					m_bShowDiagnostics = !m_bShowDiagnostics;
				}
			}


			
		}

		m_pCurrentScene->processSDLEvent(sdlEvent);
	}
	
		
	return bRet;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Engine::RunMainLoop()
{
	bool bQuit = false;

	SDL_StartTextInput();

	using clock = std::chrono::high_resolution_clock;

	clock::time_point start, lastTime;

	start = lastTime = clock::now();

	while (!bQuit)
	{
		auto currentTime = clock::now();
		m_msFrameTime = currentTime - lastTime;
		lastTime = currentTime;

		auto a = clock::now();
		bQuit = HandleInput();
		m_msInputHandleTime = clock::now() - a;

		a = clock::now();
		update();
		m_msUpdateTime = clock::now() - a;

		a = clock::now();
		drawScene();
		m_msDrawTime = clock::now() - a;

		a = clock::now();
		render();
		m_msRenderTime = clock::now() - a;

		a = clock::now();
		m_pTDM->update();
		m_msVRUpdateTime = clock::now() - a;
	}
}


void Engine::update()
{
	Renderer::getInstance().update();

	BehaviorManager::getInstance().update();

	m_pCurrentScene->update();
}

void Engine::drawScene()
{
	if (m_bShowDiagnostics)
	{
		std::stringstream ss;
		ss.precision(2);

		ss << std::fixed << "Total Frame Time: " << m_msFrameTime.count() << "ms" << std::endl;
		ss << std::fixed << "Input Handling: " << m_msInputHandleTime.count() << "ms" << std::endl;
		ss << std::fixed << "State Update: " << m_msUpdateTime.count() << "ms" << std::endl;
		ss << std::fixed << "Scene Drawing: " << m_msDrawTime.count() << "ms" << std::endl;
		ss << std::fixed << "Rendering: " << m_msRenderTime.count() << "ms" << std::endl;
		ss << std::fixed << "OpenVR Update: " << m_msVRUpdateTime.count() << "ms" << std::endl;

		Renderer::getInstance().drawUIText(
			ss.str(),
			glm::vec4(1.f),
			glm::vec3(0.f),
			glm::quat(),
			static_cast<GLfloat>(m_sviWindowUIInfo.m_nRenderHeight / 4),
			Renderer::HEIGHT,
			Renderer::LEFT,
			Renderer::BOTTOM_LEFT
		);
	}

	BehaviorManager::getInstance().draw();
	
	m_pTDM->draw();

	InfoBoxManager::getInstance().draw();

	m_pCurrentScene->draw();

	// MUST be run last to xfer previous debug draw calls to opengl buffers
	DebugDrawer::getInstance().draw();
}

void Engine::render()
{
	// VR
	{
		SDL_GL_MakeCurrent(m_pWindow, m_pGLContext);

		// Update eye positions using current HMD position
		m_sviLeftEyeInfo.view = m_sviLeftEyeInfo.viewTransform * m_pTDM->getWorldToHMDTransform();
		m_sviRightEyeInfo.view = m_sviRightEyeInfo.viewTransform * m_pTDM->getWorldToHMDTransform();

		Renderer::getInstance().sortTransparentObjects(glm::vec3(m_pTDM->getHMDToWorldTransform()[3]));

		Renderer::getInstance().renderFrame(&m_sviLeftEyeInfo, m_pLeftEyeFramebuffer);
		Renderer::getInstance().renderFrame(&m_sviRightEyeInfo, m_pRightEyeFramebuffer);

		glBindFramebuffer(GL_FRAMEBUFFER, m_pWindowFramebuffer->m_nRenderFramebufferId);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLint srcX0, srcX1, srcY0, srcY1;
		GLint dstX0, dstX1, dstY0, dstY1;

		if (m_sviWindowUIInfo.m_nRenderWidth < m_sviLeftEyeInfo.m_nRenderWidth)
		{
			srcX0 = m_sviLeftEyeInfo.m_nRenderWidth / 2 - m_sviWindowUIInfo.m_nRenderWidth / 2;
			srcX1 = srcX0 + m_sviWindowUIInfo.m_nRenderWidth;
			dstX0 = 0;
			dstX1 = m_sviWindowUIInfo.m_nRenderWidth;
		}
		else
		{
			srcX0 = 0;
			srcX1 = m_sviLeftEyeInfo.m_nRenderWidth;

			dstX0 = m_sviWindowUIInfo.m_nRenderWidth / 2 - m_sviLeftEyeInfo.m_nRenderWidth / 2;;
			dstX1 = dstX0 + m_sviLeftEyeInfo.m_nRenderWidth;
		}

		if (m_sviWindowUIInfo.m_nRenderHeight < m_sviLeftEyeInfo.m_nRenderHeight)
		{
			srcY0 = m_sviLeftEyeInfo.m_nRenderHeight / 2 - m_sviWindowUIInfo.m_nRenderHeight / 2;
			srcY1 = srcY0 + m_sviWindowUIInfo.m_nRenderHeight;
			dstY0 = 0;
			dstY1 = m_sviWindowUIInfo.m_nRenderHeight;
		}
		else
		{
			srcY0 = 0;
			srcY1 = m_sviLeftEyeInfo.m_nRenderHeight;

			dstY0 = m_sviWindowUIInfo.m_nRenderHeight / 2 - m_sviLeftEyeInfo.m_nRenderHeight / 2;;
			dstY1 = dstY0 + m_sviLeftEyeInfo.m_nRenderHeight;
		}

		glBlitNamedFramebuffer(
			m_pLeftEyeFramebuffer->m_nRenderFramebufferId,
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

		vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)(m_pLeftEyeFramebuffer->m_nResolveTextureId), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)m_pRightEyeFramebuffer->m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

		//vr::VRCompositor()->PostPresentHandoff();
		//std::cout << "Rendering Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
		
		//glFinish();
		
		SDL_GL_SwapWindow(m_pWindow);

		//glClearColor(0, 0, 0, 1);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glFlush();
		//glFinish();
	}

	//DESKTOP
	{
		SDL_GL_MakeCurrent(m_pWindow, m_pGLContext);

		Renderer::getInstance().sortTransparentObjects(glm::vec3(glm::inverse(m_sviWindow3DInfo.view)[3]));

		Renderer::getInstance().renderFrame(&m_sviWindow3DInfo, m_pWindowFramebuffer);

		Renderer::getInstance().renderFullscreenTexture(0, 0, m_ivec2WindowSize.x, m_ivec2WindowSize.y, m_pWindowFramebuffer->m_nResolveTextureId);

		SDL_GL_SwapWindow(m_pWindow);
	}

	Renderer::getInstance().clearDynamicRenderQueue();
	Renderer::getInstance().clearUIRenderQueue();
	DebugDrawer::getInstance().flushLines();
}

SDL_Window * Engine::createFullscreenWindow(int displayIndex)
{
	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY ); //UNCOMMENT AND COMMENT LINE BELOW TO ENABLE FULL OPENGL COMMANDS
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
#if _DEBUG
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
	SDL_Rect displayBounds;

	if (SDL_GetDisplayBounds(displayIndex, &displayBounds) < 0)
		SDL_GetDisplayBounds(0, &displayBounds);

	SDL_Window* win = SDL_CreateWindow("CCOM VR", displayBounds.x, displayBounds.y, displayBounds.w, displayBounds.h, unWindowFlags);

	if (win == NULL)
	{
		printf("%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
	}

	return win;
}

SDL_Window * Engine::createWindow(int width, int height, int displayIndex)
{
	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY ); //UNCOMMENT AND COMMENT LINE BELOW TO ENABLE FULL OPENGL COMMANDS
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
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

void Engine::setWindowToDisplay(SDL_Window * win, int displayIndex)
{
	SDL_Rect displayBounds;

	SDL_GetDisplayBounds(displayIndex, &displayBounds);

	SDL_SetWindowPosition(win, displayBounds.x, displayBounds.y);
	SDL_SetWindowSize(win, displayBounds.w, displayBounds.h);

	m_ivec2WindowSize = glm::ivec2(displayBounds.w, displayBounds.h);

	Renderer::getInstance().destroyFrameBuffer(*m_pWindowFramebuffer);
	createDesktopView();
}
