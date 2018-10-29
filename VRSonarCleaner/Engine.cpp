#include "Engine.h"
#include "InfoBoxManager.h"

#include "BehaviorManager.h"
#include "DataLogger.h"
#include "FlowScene.h"
#include "SonarScene.h"
#include "StudyTrialScene.h"

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
	if (!Renderer::getInstance().init())
	{
		utils::dprintf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
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
		m_pCurrentScene = new SonarScene(m_pTDM);
	}
	else
	{
		m_pCurrentScene = new StudyTrialScene(m_pTDM);
	}

	m_pCurrentScene->init();

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
	
	uint32_t renderWidth, renderHeight;
	m_pHMD->GetRecommendedRenderTargetSize(&renderWidth, &renderHeight);

	Renderer::SceneViewInfo* leftSVI = Renderer::getInstance().getLeftEyeInfo();
	Renderer::SceneViewInfo* rightSVI = Renderer::getInstance().getRightEyeInfo();

	leftSVI->m_nRenderWidth = renderWidth;
	leftSVI->m_nRenderHeight = renderHeight;
	leftSVI->viewTransform = glm::inverse(m_pTDM->getHMDEyeToHeadTransform(vr::Eye_Left));
	leftSVI->projection = m_pTDM->getHMDEyeProjection(vr::Eye_Left, g_fNearClip, g_fFarClip);
	leftSVI->viewport = glm::ivec4(0, 0, renderWidth, renderHeight);

	rightSVI->m_nRenderWidth = renderWidth;
	rightSVI->m_nRenderHeight = renderHeight;
	rightSVI->viewTransform = glm::inverse(m_pTDM->getHMDEyeToHeadTransform(vr::Eye_Right));
	rightSVI->projection = m_pTDM->getHMDEyeProjection(vr::Eye_Right, g_fNearClip, g_fFarClip);
	rightSVI->viewport = glm::ivec4(0, 0, renderWidth, renderHeight);

	Renderer::getInstance().setStereoRenderSize(glm::ivec2(renderWidth, renderHeight));

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


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Engine::Shutdown()
{
	BehaviorManager::getInstance().shutdown();

	Renderer::getInstance().shutdown();

	shutdownVR();

	SDL_Quit();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool Engine::handleInput()
{
	SDL_Event sdlEvent;
	bool bRet = false;

	if (m_pTDM)
	{
		m_pTDM->update();
		m_pTDM->handleEvents();
	}

	while (SDL_PollEvent(&sdlEvent) != 0)
	{


		if (sdlEvent.type == SDL_KEYDOWN && sdlEvent.key.keysym.mod & KMOD_CTRL)
		{
			if (sdlEvent.key.keysym.sym == SDLK_0)
				Renderer::getInstance().setWindowToDisplay(0);
			if (sdlEvent.key.keysym.sym == SDLK_1)
				Renderer::getInstance().setWindowToDisplay(1);
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
		bQuit = handleInput();
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

		ss << std::fixed << "Total Frame Time: " << m_msFrameTime.count() << "ms | " << 1.f / std::chrono::duration_cast<std::chrono::duration<float>>(m_msFrameTime).count() << "fps" << std::endl;
		ss << std::fixed << "Input Handling: " << m_msInputHandleTime.count() << "ms" << std::endl;
		ss << std::fixed << "State Update: " << m_msUpdateTime.count() << "ms" << std::endl;
		ss << std::fixed << "Scene Drawing: " << m_msDrawTime.count() << "ms" << std::endl;
		ss << std::fixed << "Rendering: " << m_msRenderTime.count() << "ms" << std::endl;

		Renderer::getInstance().drawUIText(
			ss.str(),
			glm::vec4(1.f),
			glm::vec3(0.f),
			glm::quat(),
			static_cast<GLfloat>(Renderer::getInstance().getUIRenderSize().y / 4),
			Renderer::HEIGHT,
			Renderer::LEFT,
			Renderer::BOTTOM_LEFT
		);
	}		std::stringstream ss;

	BehaviorManager::getInstance().draw();
	
	m_pTDM->draw();

	InfoBoxManager::getInstance().draw();

	m_pCurrentScene->draw();
}

void Engine::render()
{
	// Update eye positions using current HMD position
	Renderer::SceneViewInfo* le = Renderer::getInstance().getLeftEyeInfo();
	Renderer::SceneViewInfo* re = Renderer::getInstance().getRightEyeInfo();
	le->view = le->viewTransform * m_pTDM->getWorldToHMDTransform();
	re->view = re->viewTransform * m_pTDM->getWorldToHMDTransform();

	Renderer::getInstance().sortTransparentObjects(glm::vec3(m_pTDM->getHMDToWorldTransform()[3]));

	Renderer::getInstance().render();

	vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)Renderer::getInstance().getLeftEyeFrameBuffer()->m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)Renderer::getInstance().getRightEyeFrameBuffer()->m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

	Renderer::getInstance().swapAndClear();
}
