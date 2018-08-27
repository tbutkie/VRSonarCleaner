#include "Engine.h"
#include "DebugDrawer.h"
#include "InfoBoxManager.h"

#include "BehaviorManager.h"
#include "StudyTutorialBehavior.h"
#include "DemoBehavior.h"
#include "SelectAreaBehavior.h"
#include "GrabObjectBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include "CurateStudyDataBehavior.h"
#include "RunStudyBehavior.h"
#include "StudyIntroBehavior.h"
#include "ScaleTutorial.h"
#include "StudyEditTutorial.h"
#include "DesktopCleanBehavior.h"
#include "StudyTrialDesktopBehavior.h"
#include "arcball.h"
#include "LassoTool.h"
#include "SnellenTest.h"
#include "CloudEditControllerTutorial.h"
#include "FlowProbe.h"
#include "HairyFlowProbe.h"
#include "FlowFieldCurator.h"
#include "DebugProbe.h"

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
	: m_bUseVR(false)
	, m_bUseDesktop(false)
	, m_bSonarCleaning(false)
	, m_bFlowVis(false)
	, m_bGreatBayModel(false)
	, m_bShowDesktopFrustum(false)
	, m_bStudyMode(false)
	, m_bDemoMode(false)
	, m_bShowDiagnostics(false)
	, m_bGLInitialized(false)
	, m_bLeftMouseDown(false)
	, m_bRightMouseDown(false)
	, m_bMiddleMouseDown(false)
	, m_pCurrentScene(NULL)
	, m_pWindow(NULL)
	, m_pWindowCursor(NULL)
	, m_pGLContext(NULL)
	, m_pWindowFramebuffer(NULL)
	, m_pLeftEyeFramebuffer(NULL)
	, m_pRightEyeFramebuffer(NULL)
	, m_pHMD(NULL)
	, m_bInitialColorRefresh(false)
{
	int mode = 3;

	switch (mode)
	{
	case 1:
		m_bUseVR = true;
		m_bSonarCleaning = true;
		break;
	case 2:
		m_bUseVR = true;
		m_bSonarCleaning = true;
		m_bStudyMode = true;
		break;
	case 3:
		m_bUseVR = true;
		m_bFlowVis = true;
		break;
	case 4:
		m_bUseVR = true;
		m_bFlowVis = true;
		m_bGreatBayModel = true;
		break;
	case 5:
		m_bUseVR = true;
		m_bFlowVis = true;
		m_bStudyMode = true;
		break;
	case 6:
		m_bUseDesktop = true;
		m_bSonarCleaning = true;
		break;
	case 7:
		m_bUseDesktop = true;
		m_bSonarCleaning = true;
		m_bStudyMode = true;
		break;
	case 8:
		m_bUseDesktop = true;
		m_bFlowVis = true;
		break;
	case 9:
		m_bUseDesktop = true;
		m_bFlowVis = true;
		m_bGreatBayModel = true;
		break;
	default:
		utils::dprintf("Invalid Selection, shutting down...");
		Shutdown();
		break;
	}
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

	if (m_bUseVR && !initVR())
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

	if (m_pCurrentScene)
		m_pCurrentScene->init();

	if (m_bSonarCleaning)
	{
		std::string strWindowTitle = "VR Sonar Cleaner | CCOM VisLab";
		SDL_SetWindowTitle(m_pWindow, strWindowTitle.c_str());

		glm::vec3 wallSize(10.f, (g_vec3RoomSize.y * 0.9f), 0.5f);
		glm::quat wallOrientation(glm::angleAxis(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)));
		glm::vec3 wallPosition(0.f, g_vec3RoomSize.y * 0.5f, g_vec3RoomSize.z);
		
		m_pWallVolume = new DataVolume(wallPosition, wallOrientation, wallSize);
		m_pWallVolume->setBackingColor(glm::vec4(0.15f, 0.21f, 0.31f, 1.f));

		glm::vec3 tablePosition = glm::vec3(0.f, 1.f, -g_vec3RoomSize.z);
		glm::quat tableOrientation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
		glm::vec3 tableSize = glm::vec3(1.5f, 1.5f, 0.5f);

		m_pTableVolume = new DataVolume(tablePosition, tableOrientation, tableSize);
		m_pTableVolume->setBackingColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
		m_pTableVolume->setFrameColor(glm::vec4(1.f));

		m_Camera.pos = tablePosition + glm::vec3(0.f, 0.f, 1.f) * 3.f;
		m_Camera.lookat = tablePosition;

		{
			//if (!BehaviorManager::getInstance().getBehavior("harvestpoints"))
			//	BehaviorManager::getInstance().addBehavior("harvestpoints", new SelectAreaBehavior(m_pTDM, m_pWallVolume, m_pTableVolume));
			if (!BehaviorManager::getInstance().getBehavior("grab"))
				BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pTableVolume));
			if (!BehaviorManager::getInstance().getBehavior("scale"))
				BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pTableVolume));

			BehaviorManager::getInstance().addBehavior("pointclean", new PointCleanProbe(m_pTDM, m_pTableVolume, m_pHMD));
		}

		m_pColorScalerTPU = new ColorScaler();
		m_pColorScalerTPU->setColorMode(ColorScaler::Mode::ColorScale_BiValue);
		m_pColorScalerTPU->setBiValueColorMap(ColorScaler::ColorMap_BiValued::Custom);
		//m_pColorScalerTPU->setColorMode(ColorScaler::Mode::ColorScale);
		//m_pColorScalerTPU->setColorMap(ColorScaler::ColorMap::Rainbow);

		m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "resources/data/sonar/demo/H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_1085.txt", SonarPointCloud::SONAR_FILETYPE::CARIS));
		//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "resources/data/sonar/demo/H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_528_1324.txt", SonarPointCloud::SONAR_FILETYPE::CARIS));
		//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "resources/data/sonar/demo/H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1516.txt", SonarPointCloud::SONAR_FILETYPE::CARIS));
		//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "resources/data/sonar/demo/H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1508.txt", SonarPointCloud::SONAR_FILETYPE::CARIS));
		//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "resources/data/sonar/demo/H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1500.txt", SonarPointCloud::SONAR_FILETYPE::CARIS));
		//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "resources/data/sonar/demo/H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-148_148_000_2022.txt", SonarPointCloud::SONAR_FILETYPE::CARIS));
		
		
		//using namespace std::experimental::filesystem::v1;
		//
		////path dataset("south_santa_rosa");
		////path dataset("santa_cruz_south");
		//path dataset("santa_cruz_basin");
		//
		//auto basePath = current_path().append(path("resources/data/sonar/nautilus"));
		//
		//auto acceptsPath = path(basePath).append(path("accept"));
		//
		//for (directory_iterator it(acceptsPath.append(dataset)); it != directory_iterator(); ++it)
		//{
		//	if (is_regular_file(*it))
		//	{
		//		if (std::find_if(m_vpClouds.begin(), m_vpClouds.end(), [&it](SonarPointCloud* &pc) { return pc->getName() == (*it).path().string(); }) == m_vpClouds.end())
		//		{
		//			SonarPointCloud* tmp = new SonarPointCloud(m_pColorScalerTPU, (*it).path().string(), SonarPointCloud::QIMERA);
		//			m_vpClouds.push_back(tmp);
		//		}
		//	}
		//}

		for (auto const &cloud : m_vpClouds)
		{
			m_pWallVolume->add(cloud);
			m_pTableVolume->add(cloud);
		}
			
		m_vpDataVolumes.push_back(m_pTableVolume);
		m_vpDataVolumes.push_back(m_pWallVolume);

		if (m_bUseDesktop)
		{
			glm::ivec4 vp(0, 0, m_ivec2WindowSize.x, m_ivec2WindowSize.y);

			DesktopCleanBehavior *tmp = new DesktopCleanBehavior(m_pTableVolume, &m_sviWindow3DInfo, vp);
			BehaviorManager::getInstance().addBehavior("desktop_edit", tmp);
			tmp->init();
		}
	}
	else if (m_bFlowVis)
	{
		std::string strWindowTitle = "VR Flow 4D | CCOM VisLab";
		SDL_SetWindowTitle(m_pWindow, strWindowTitle.c_str());

		std::vector<std::string> flowGrids;
		
		if (m_bGreatBayModel)
		{
			flowGrids.push_back("resources/data/flowgrid/gb.fg");
			m_pFlowVolume = new FlowVolume(flowGrids, false);
			m_pFlowVolume->setDimensions(glm::vec3(fmin(g_vec3RoomSize.x, g_vec3RoomSize.z) * 0.5f, fmin(g_vec3RoomSize.x, g_vec3RoomSize.z) * 0.5f, g_vec3RoomSize.y * 0.05f));
			m_pFlowVolume->setParticleVelocityScale(0.5f);
		}
		else
		{
			flowGrids.push_back("resources/data/flowgrid/test.fg");
			m_pFlowVolume = new FlowVolume(flowGrids, true);
			m_pFlowVolume->setParticleVelocityScale(0.01f);

		}

		m_pFlowVolume->setBackingColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
		m_pFlowVolume->setFrameColor(glm::vec4(1.f));


		m_Camera.pos = m_pFlowVolume->getPosition() + glm::vec3(0.f, 0.f, 1.f) * 3.f;
		m_Camera.lookat = m_pFlowVolume->getPosition();

		{
			BehaviorManager::getInstance().addBehavior("flowcurator", new FlowFieldCurator(m_pTDM, m_pFlowVolume));
			BehaviorManager::getInstance().addBehavior("flowprobe", new FlowProbe(m_pTDM, m_pFlowVolume));
			//BehaviorManager::getInstance().addBehavior("debugprobe", new DebugProbe(m_pTDM, m_pFlowVolume));

			if (!BehaviorManager::getInstance().getBehavior("grab"))
				BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pFlowVolume));
			if (!BehaviorManager::getInstance().getBehavior("scale"))
				BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pFlowVolume));

		}
	}

	m_sviWindow3DInfo.view = glm::lookAt(m_Camera.pos, m_Camera.lookat, m_Camera.up);

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

	m_Camera.pos = glm::vec3(0.f, 0.f, 1.f);
	m_Camera.up = glm::vec3(0.f, 1.f, 0.f);

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

	m_bUseVR = false;
}

void Engine::shutdownDesktop()
{
	SDL_DestroyWindow(m_pWindow);
	m_pWindow = NULL;

	m_bUseDesktop = false;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Engine::Shutdown()
{
	BehaviorManager::getInstance().shutdown();

	for (auto fb : { m_pWindowFramebuffer , m_pLeftEyeFramebuffer, m_pRightEyeFramebuffer })
		if (fb)
			Renderer::getInstance().destroyFrameBuffer(*fb);

	DebugDrawer::getInstance().shutdown();

	Renderer::getInstance().shutdown();

	if (m_pGLContext)
	{
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(nullptr, nullptr);
	}

	if (m_bUseVR)
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

	if (m_bUseVR)
		m_pTDM->handleEvents();

	while (SDL_PollEvent(&sdlEvent) != 0)
	{
		ArcBall *arcball = static_cast<ArcBall*>(BehaviorManager::getInstance().getBehavior("arcball"));
		LassoTool *lasso = static_cast<LassoTool*>(BehaviorManager::getInstance().getBehavior("lasso"));


		if (m_bStudyMode)
		{
			if (sdlEvent.type == SDL_KEYDOWN)
			{
				if ((sdlEvent.key.keysym.mod & KMOD_LSHIFT) && sdlEvent.key.keysym.sym == SDLK_ESCAPE
					|| sdlEvent.key.keysym.sym == SDLK_q)
				{
					bRet = true;
				}
		
				if (m_bSonarCleaning)
				{
					if (sdlEvent.key.keysym.sym == SDLK_r)
					{
						if (arcball)
						{
							std::stringstream ss;
		
							ss << "View Reset" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
		
							DataLogger::getInstance().logMessage(ss.str());
		
							m_Camera.pos = m_pTableVolume->getPosition() + glm::vec3(0.f, 0.f, 1.f) * 3.f;
							m_Camera.lookat = m_pTableVolume->getPosition();
		
							m_sviWindow3DInfo.view = glm::lookAt(m_Camera.pos, m_Camera.lookat, m_Camera.up);
		
							arcball->reset();
						}
					}
					
					if (sdlEvent.key.keysym.sym == SDLK_RETURN)
					{
						RunStudyBehavior *study = static_cast<RunStudyBehavior*>(BehaviorManager::getInstance().getBehavior("Desktop Study"));
		
						if (study)
							study->next();
					}
		
					if (sdlEvent.key.keysym.sym == SDLK_SPACE)
					{
						DesktopCleanBehavior *desktopEdit = static_cast<DesktopCleanBehavior*>(BehaviorManager::getInstance().getBehavior("desktop_edit"));
		
						if (desktopEdit)
							desktopEdit->activate();
					}
		
		
					if (sdlEvent.key.keysym.sym == SDLK_KP_1)
					{
						m_pWallVolume->setVisible(false);
						m_pTableVolume->setVisible(false);
						BehaviorManager::getInstance().clearBehaviors();
						RunStudyBehavior *rsb = new RunStudyBehavior(m_pTDM, false);
						BehaviorManager::getInstance().addBehavior("Standing Study", rsb);
						rsb->init();
					}
		
					if (sdlEvent.key.keysym.sym == SDLK_KP_2)
					{
						m_pWallVolume->setVisible(false);
						m_pTableVolume->setVisible(false);
						BehaviorManager::getInstance().clearBehaviors();
						RunStudyBehavior *rsb = new RunStudyBehavior(m_pTDM, true);
						BehaviorManager::getInstance().addBehavior("Sitting Study", rsb);
						rsb->init();
					}
					if (sdlEvent.key.keysym.sym == SDLK_KP_3)
					{
						m_pWallVolume->setVisible(false);
						m_pTableVolume->setVisible(false);
						BehaviorManager::getInstance().clearBehaviors();
						RunStudyBehavior *rsb = new RunStudyBehavior(&m_sviWindow3DInfo, glm::ivec4(0, 0, m_ivec2WindowSize.x, m_ivec2WindowSize.y), &m_Camera);
						BehaviorManager::getInstance().addBehavior("Desktop Study", rsb);
						rsb->init();
					}
				}
			}
		
			// MOUSE
			if (m_bUseDesktop)
			{
				if (sdlEvent.type == SDL_MOUSEBUTTONDOWN) //MOUSE DOWN
				{
					if (sdlEvent.button.button == SDL_BUTTON_LEFT)
					{
						m_bLeftMouseDown = true;
		
						if (arcball)
							arcball->beginDrag(glm::vec2(sdlEvent.button.x, m_ivec2WindowSize.y - sdlEvent.button.y));
		
						if (lasso)
						{
							if (m_bRightMouseDown)
								lasso->end();
		
							lasso->reset();
						}
		
					}
					if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
					{
						m_bRightMouseDown = true;
						if (lasso && !m_bLeftMouseDown)
							lasso->start(sdlEvent.button.x, m_ivec2WindowSize.y - sdlEvent.button.y);
					}
					if (sdlEvent.button.button == SDL_BUTTON_MIDDLE)
					{
						if (lasso && m_bRightMouseDown)
						{
							lasso->end();
						}
						m_bMiddleMouseDown = true;
		
						if (arcball)
							arcball->translate(glm::vec2(sdlEvent.button.x, m_ivec2WindowSize.y - sdlEvent.button.y));
					}
		
				}//end mouse down 
				else if (sdlEvent.type == SDL_MOUSEBUTTONUP) //MOUSE UP
				{
					if (sdlEvent.button.button == SDL_BUTTON_LEFT)
					{
						m_bLeftMouseDown = false;
		
						if (arcball)
							arcball->endDrag();
		
						if (lasso)
							lasso->reset();
					}
					if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
					{
						m_bRightMouseDown = false;
		
						if (lasso)
							lasso->end();
					}
					if (sdlEvent.button.button == SDL_BUTTON_MIDDLE)
					{
						m_bMiddleMouseDown = false;
					}
		
				}//end mouse up
				if (sdlEvent.type == SDL_MOUSEMOTION)
				{
					if (m_bLeftMouseDown)
					{
						if (arcball)
							arcball->drag(glm::vec2(sdlEvent.button.x, m_ivec2WindowSize.y - sdlEvent.button.y));
					}
					if (m_bRightMouseDown && !m_bLeftMouseDown)
					{
						if (lasso)
							lasso->move(sdlEvent.button.x, m_ivec2WindowSize.y - sdlEvent.button.y);
					}
				}
				if (sdlEvent.type == SDL_MOUSEWHEEL)
				{
					if (lasso)
						lasso->reset();
		
					glm::vec3 eyeForward = glm::normalize(m_Camera.lookat - m_Camera.pos);
					m_Camera.pos += eyeForward * ((float)sdlEvent.wheel.y*0.1f);
		
					float newLen = glm::length(m_Camera.lookat - m_Camera.pos);
		
					if (newLen < 0.1f)
						m_Camera.pos = m_Camera.lookat - eyeForward * 0.1f;
					if (newLen > 10.f)
						m_Camera.pos = m_Camera.lookat - eyeForward * 10.f;
		
					m_sviWindow3DInfo.view = glm::lookAt(m_Camera.pos, m_Camera.lookat, m_Camera.up);
		
					if (DataLogger::getInstance().logging())
					{
						std::stringstream ss;
		
						ss << "Camera Zoom" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
						ss << "\t";
						ss << "cam-pos:\"" << m_Camera.pos.x << "," << m_Camera.pos.y << "," << m_Camera.pos.z << "\"";
						ss << ";";
						ss << "cam-look:\"" << m_Camera.lookat.x << "," << m_Camera.lookat.y << "," << m_Camera.lookat.z << "\"";
						ss << ";";
						ss << "cam-up:\"" << m_Camera.up.x << "," << m_Camera.up.y << "," << m_Camera.up.z << "\"";
		
						DataLogger::getInstance().logMessage(ss.str());
					}
				}
			}
		}
		else
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
				
				if (sdlEvent.key.keysym.sym == SDLK_l)
				{
					if (m_bUseVR)
					{
						if (!BehaviorManager::getInstance().getBehavior("harvestpoints"))
							BehaviorManager::getInstance().addBehavior("harvestpoints", new SelectAreaBehavior(m_pTDM, m_pWallVolume, m_pTableVolume));
						if (!BehaviorManager::getInstance().getBehavior("grab"))
							BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pTableVolume));
						if (!BehaviorManager::getInstance().getBehavior("scale"))
							BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pTableVolume));
					}
					else 
						m_pWallVolume->setVisible(false);
				
					using namespace std::experimental::filesystem::v1;
				
					//path dataset("south_santa_rosa");
					//path dataset("santa_cruz_south");
					path dataset("santa_cruz_basin");
				
					auto basePath = current_path().append(path("resources/data/sonar/nautilus"));
					std::cout << "Base data directory: " << basePath << std::endl;
				
					auto acceptsPath = path(basePath).append(path("accept"));
					auto rejectsPath = path(basePath).append(path("reject"));
				
					for (directory_iterator it(acceptsPath.append(dataset)); it != directory_iterator(); ++it)
					{
						if (is_regular_file(*it))
						{
							if (std::find_if(m_vpClouds.begin(), m_vpClouds.end(), [&it](SonarPointCloud* &pc) { return pc->getName() == (*it).path().string(); }) == m_vpClouds.end())
							{
								SonarPointCloud* tmp = new SonarPointCloud(m_pColorScalerTPU, (*it).path().string(), SonarPointCloud::QIMERA);
								m_vpClouds.push_back(tmp);
								m_pTableVolume->add(tmp);
								m_pWallVolume->add(tmp);
								break;
							}
						}
					}
				
					//for (directory_iterator it(rejectsPath.append(dataset)); it != directory_iterator(); ++it)
					//{
					//	if (is_regular_file(*it))
					//	{
					//		if (std::find_if(m_vpClouds.begin(), m_vpClouds.end(), [&it](SonarPointCloud* &pc) { return pc->getName() == (*it).path().string(); }) == m_vpClouds.end())
					//		{
					//			SonarPointCloud* tmp = new SonarPointCloud(m_pColorScalerTPU, (*it).path().string(), SonarPointCloud::QIMERA);
					//			m_vpClouds.push_back(tmp);
					//			m_pTableVolume->add(tmp);
					//			m_pWallVolume->add(tmp);
					//			break;
					//		}
					//	}
					//}
				
					refreshColorScale(m_pColorScalerTPU, m_vpClouds);
				}
				
				if ((sdlEvent.key.keysym.mod & KMOD_LCTRL) && sdlEvent.key.keysym.sym == SDLK_v)
				{
					if (!m_bUseVR)
					{
						m_bUseVR = true;
						initVR();
					}
				}
				if (sdlEvent.key.keysym.sym == SDLK_KP_ENTER)
				{
					m_pTableVolume->setVisible(false);
					m_pWallVolume->setVisible(false);
					BehaviorManager::getInstance().clearBehaviors();
					SnellenTest *st = new SnellenTest(m_pTDM, 10.f);
					BehaviorManager::getInstance().addBehavior("snellen", st);
					st->init();
				}
				if (sdlEvent.key.keysym.sym == SDLK_UP)
				{
					SnellenTest *snellen = static_cast<SnellenTest*>(BehaviorManager::getInstance().getBehavior("snellen"));
				
					if (snellen)
					{
						snellen->setVisualAngle(snellen->getVisualAngle() + 1.f);
						snellen->newTest();
					}
				}
				if (sdlEvent.key.keysym.sym == SDLK_DOWN)
				{
					SnellenTest *snellen = static_cast<SnellenTest*>(BehaviorManager::getInstance().getBehavior("snellen"));
				
					if (snellen)
					{
						float angle = snellen->getVisualAngle() - 1.f;
						if (angle < 1.f) angle = 1.f;
						snellen->setVisualAngle(angle);
						snellen->newTest();
					}
				}
				
				
				if (sdlEvent.key.keysym.sym == SDLK_y)
				{
					m_bDemoMode = true;
					m_pTableVolume->setVisible(false);
					m_pWallVolume->setVisible(false);
					BehaviorManager::getInstance().clearBehaviors();
					CloudEditControllerTutorial *cet = new CloudEditControllerTutorial(m_pTDM);
					BehaviorManager::getInstance().addBehavior("Demo", cet);
					cet->init();
				}
				if (sdlEvent.key.keysym.sym == SDLK_u)
				{
					BehaviorManager::getInstance().addBehavior("GetStudyData", new CurateStudyDataBehavior(m_pTDM, m_pTableVolume, m_pWallVolume));
				}
				
				if (sdlEvent.key.keysym.sym == SDLK_KP_0)
				{
					m_pWallVolume->setVisible(false);
					m_pTableVolume->setVisible(false);
					BehaviorManager::getInstance().clearBehaviors();
					BehaviorManager::getInstance().addBehavior("Tutorial", new StudyTutorialBehavior(m_pTDM, m_pTableVolume, m_pWallVolume));
				}
				
				if (sdlEvent.key.keysym.sym == SDLK_KP_PERIOD)
				{
					m_pWallVolume->setVisible(false);
					m_pTableVolume->setVisible(false);
					BehaviorManager::getInstance().clearBehaviors();
					StudyTrialDesktopBehavior  *stdb = new StudyTrialDesktopBehavior(&m_sviWindow3DInfo, glm::ivec4(0.f, 0.f, m_ivec2WindowSize.x, m_ivec2WindowSize.y), &m_Camera, "tutorial_points.csv", "demo");
					BehaviorManager::getInstance().addBehavior("Tutorial", stdb);
					stdb->init();
				}

				if (sdlEvent.key.keysym.sym == SDLK_d)
				{
					
					static_cast<ProbeBehavior*>(BehaviorManager::getInstance().getBehavior("pointclean"))->activateDemoMode();

					//if ((sdlEvent.key.keysym.mod & KMOD_LCTRL))
					//{
					//	if (!m_bUseDesktop)
					//	{
					//		m_bUseDesktop = true;
					//		initDesktop();
					//	}
					//}
				}

				if (sdlEvent.key.keysym.sym == SDLK_x)
				{
					if ((sdlEvent.key.keysym.mod & KMOD_LCTRL))
					{
						if (m_bUseVR)
						{
							shutdownVR();
						}
					}
				}
				
				if (sdlEvent.key.keysym.sym == SDLK_z)
				{
					if ((sdlEvent.key.keysym.mod & KMOD_LCTRL))
					{
						if (m_bUseDesktop)
						{
							shutdownDesktop();
						}
					}
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

				if (m_bSonarCleaning)
				{
					if (sdlEvent.key.keysym.sym == SDLK_r)
					{
						printf("Pressed r, resetting marks\n");

						if (!m_bStudyMode)
						{
							for (auto &cloud : m_vpClouds)
								cloud->resetAllMarks();
						}

						for (auto &dv : m_vpDataVolumes)
							dv->resetPositionAndOrientation();

						if (arcball)
						{
							m_Camera.pos = m_pTableVolume->getPosition() + glm::vec3(0.f, 0.f, 1.f) * 3.f;
							m_Camera.lookat = m_pTableVolume->getPosition();
						
							m_sviWindow3DInfo.view = glm::lookAt(m_Camera.pos, m_Camera.lookat, m_Camera.up);
						
							arcball->reset();
						}
					}

					if (sdlEvent.key.keysym.sym == SDLK_g)
					{
						printf("Pressed g, generating fake test cloud\n");
						//m_pClouds->generateFakeTestCloud(150, 150, 25, 40000);
						//m_pColorScalerTPU->resetBiValueScaleMinMax(m_pClouds->getMinDepthTPU(), m_pClouds->getMaxDepthTPU(), m_pClouds->getMinPositionalTPU(), m_pClouds->getMaxPositionalTPU());
					}
					
					if (sdlEvent.key.keysym.sym == SDLK_RETURN)
					{
						RunStudyBehavior *study = static_cast<RunStudyBehavior*>(BehaviorManager::getInstance().getBehavior("Desktop Study"));
					
						if (study)
							study->next();
					}
					
					if (sdlEvent.key.keysym.sym == SDLK_SPACE)
					{
						DesktopCleanBehavior *desktopEdit = static_cast<DesktopCleanBehavior*>(BehaviorManager::getInstance().getBehavior("desktop_edit"));
					
						if (desktopEdit)
							desktopEdit->activate();
					}
				}
				else if (m_bFlowVis) //flow
				{
					if (sdlEvent.key.keysym.sym == SDLK_r)
					{
						printf("Pressed r, resetting something...\n");
						if (m_pFlowVolume)
							m_pFlowVolume->resetPositionAndOrientation();
					}
				
					if (sdlEvent.key.keysym.sym == SDLK_1)
					{
						if (m_bUseVR && m_pFlowVolume)
						{
							glm::mat3 matHMD(m_pTDM->getHMDToWorldTransform());
							m_pFlowVolume->setDimensions(glm::vec3(1.f, 1.f, 0.1f));
							m_pFlowVolume->setPosition(glm::vec3(m_pTDM->getHMDToWorldTransform()[3] - m_pTDM->getHMDToWorldTransform()[2] * 0.5f));
				
							glm::mat3 matOrientation;
							matOrientation[0] = matHMD[0];
							matOrientation[1] = matHMD[2];
							matOrientation[2] = -matHMD[1];
							m_pFlowVolume->setOrientation(glm::quat_cast(matHMD) * glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f)));
						}
					}
				
					if (sdlEvent.key.keysym.sym == SDLK_2)
					{
						if (m_bUseVR && m_pFlowVolume)
						{
							m_pFlowVolume->setDimensions(glm::vec3(fmin(g_vec3RoomSize.x, g_vec3RoomSize.z) * 0.9f, fmin(g_vec3RoomSize.x, g_vec3RoomSize.z) * 0.9f, g_vec3RoomSize.y * 0.1f));
							m_pFlowVolume->setPosition(glm::vec3(0.f, g_vec3RoomSize.y * 0.1f * 0.5f, 0.f));
							m_pFlowVolume->setOrientation(glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)));
						}
					}
				}

				if ((sdlEvent.key.keysym.mod & KMOD_LCTRL) && sdlEvent.key.keysym.sym == SDLK_f)
				{
					m_bShowDiagnostics = !m_bShowDiagnostics;
				}
			}


			//MOUSE
			if (m_bUseDesktop)
			{
				if (sdlEvent.type == SDL_MOUSEBUTTONDOWN) //MOUSE DOWN
				{
					if (sdlEvent.button.button == SDL_BUTTON_LEFT)
					{
						m_bLeftMouseDown = true;
			
						if (arcball)
							arcball->beginDrag(glm::vec2(sdlEvent.button.x, m_ivec2WindowSize.y - sdlEvent.button.y));
			
						if (lasso)
						{
							if (m_bRightMouseDown)
								lasso->end();
			
							lasso->reset();
						}
			
					}
					if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
					{
						m_bRightMouseDown = true;
						if (lasso && !m_bLeftMouseDown)
							lasso->start(sdlEvent.button.x, m_ivec2WindowSize.y - sdlEvent.button.y);
					}
					if (sdlEvent.button.button == SDL_BUTTON_MIDDLE)
					{
						if (lasso && m_bRightMouseDown)
						{
							lasso->end();
						}
						m_bMiddleMouseDown = true;
			
						if (arcball)
							arcball->translate(glm::vec2(sdlEvent.button.x, m_ivec2WindowSize.y - sdlEvent.button.y));
					}
			
				}//end mouse down 
				else if (sdlEvent.type == SDL_MOUSEBUTTONUP) //MOUSE UP
				{
					if (sdlEvent.button.button == SDL_BUTTON_LEFT)
					{
						m_bLeftMouseDown = false;
			
						if (arcball)
							arcball->endDrag();
			
						if (lasso)
							lasso->reset();
					}
					if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
					{
						m_bRightMouseDown = false;
			
						if (lasso)
							lasso->end();
					}
					if (sdlEvent.button.button == SDL_BUTTON_MIDDLE)
					{
						m_bMiddleMouseDown = false;
					}
			
				}//end mouse up
				if (sdlEvent.type == SDL_MOUSEMOTION)
				{
					if (m_bLeftMouseDown)
					{
						if (arcball)
							arcball->drag(glm::vec2(sdlEvent.button.x, m_ivec2WindowSize.y - sdlEvent.button.y));
					}
					if (m_bRightMouseDown && !m_bLeftMouseDown)
					{
						if (lasso)
							lasso->move(sdlEvent.button.x, m_ivec2WindowSize.y - sdlEvent.button.y);
					}
				}
				if (sdlEvent.type == SDL_MOUSEWHEEL)
				{
					if (lasso)
						lasso->reset();
			
					glm::vec3 eyeForward = glm::normalize(m_Camera.lookat - m_Camera.pos);
					m_Camera.pos += eyeForward * ((float)sdlEvent.wheel.y*0.1f);
			
					float newLen = glm::length(m_Camera.lookat - m_Camera.pos);
			
					if (newLen < 0.1f)
						m_Camera.pos = m_Camera.lookat - eyeForward * 0.1f;
					if (newLen > 10.f)
						m_Camera.pos = m_Camera.lookat - eyeForward * 10.f;
			
					m_sviWindow3DInfo.view = glm::lookAt(m_Camera.pos, m_Camera.lookat, m_Camera.up);
			
					if (DataLogger::getInstance().logging())
					{
						std::stringstream ss;
			
						ss << "Camera Zoom" << "\t" << DataLogger::getInstance().getTimeSinceLogStartString();
						ss << "\t";
						ss << "cam-pos:\"" << m_Camera.pos.x << "," << m_Camera.pos.y << "," << m_Camera.pos.z << "\"";
						ss << ";";
						ss << "cam-look:\"" << m_Camera.lookat.x << "," << m_Camera.lookat.y << "," << m_Camera.lookat.z << "\"";
						ss << ";";
						ss << "cam-up:\"" << m_Camera.up.x << "," << m_Camera.up.y << "," << m_Camera.up.z << "\"";
			
						DataLogger::getInstance().logMessage(ss.str());
					}
				}
			}
		}
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

		if (m_bUseVR)
		{
			a = clock::now();
			m_pTDM->update();
			m_msVRUpdateTime = clock::now() - a;
		}
	}
}


void Engine::update()
{
	Renderer::getInstance().update();

	BehaviorManager::getInstance().update();

	if (m_bUseDesktop)
	{
		
	}

	if (m_bSonarCleaning)
	{
		for (auto &cloud : m_vpClouds)
			cloud->update();

		for (auto &dv : m_vpDataVolumes)
			dv->update();
	}

	if (m_bFlowVis && m_pFlowVolume)
	{
		m_pFlowVolume->update();
	}
}

void Engine::drawScene()
{
	BehaviorManager::getInstance().draw();

	if (m_bShowDiagnostics)
	{
		std::stringstream ss;
		ss.precision(2);

		ss << std::fixed << "Total Frame Time: " << m_msFrameTime.count() << "ms" << std::endl;
		ss << std::fixed << "Input Handling: " << m_msInputHandleTime.count() << "ms" << std::endl;
		ss << std::fixed << "State Update: " << m_msUpdateTime.count() << "ms" << std::endl;
		ss << std::fixed << "Scene Drawing: " << m_msDrawTime.count() << "ms" << std::endl;
		ss << std::fixed << "Rendering: " << m_msRenderTime.count() << "ms" << std::endl;
		if (m_bUseVR)
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

	if (m_bSonarCleaning)
	{
		if (m_bUseVR)
		{
			Renderer::getInstance().drawPrimitive("plane", glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)) * glm::scale(glm::mat4(), glm::vec3(g_vec3RoomSize.x * 2.f, g_vec3RoomSize.z * 2.f, 1.f)), glm::vec4(0.2f, 0.2f, 0.2f, 1.f), glm::vec4(1.f), 132.f);
			
			if (m_bShowDesktopFrustum)
			{
				// get frustum with near plane 1m out from view pos
				glm::mat4 proj = glm::perspective(glm::radians(g_fDesktopWindowFOV), (float)m_ivec2WindowSize.x / (float)m_ivec2WindowSize.y, 1.f, g_fFarClip);

				// get world-space points for the viewing plane
				glm::vec3 x0y0 = glm::unProject(glm::vec3(0.f), m_sviWindow3DInfo.view, proj, glm::vec4(0.f, 0.f, m_ivec2WindowSize.x, m_ivec2WindowSize.y));
				glm::vec3 x1y0 = glm::unProject(glm::vec3(m_ivec2WindowSize.x, 0.f, 0.f), m_sviWindow3DInfo.view, proj, glm::vec4(0.f, 0.f, m_ivec2WindowSize.x, m_ivec2WindowSize.y));
				glm::vec3 x0y1 = glm::unProject(glm::vec3(0.f, m_ivec2WindowSize.y, 0.f), m_sviWindow3DInfo.view, proj, glm::vec4(0.f, 0.f, m_ivec2WindowSize.x, m_ivec2WindowSize.y));
				glm::vec3 x1y1 = glm::unProject(glm::vec3(m_ivec2WindowSize.x, m_ivec2WindowSize.y, 0.f), m_sviWindow3DInfo.view, proj, glm::vec4(0.f, 0.f, m_ivec2WindowSize.x, m_ivec2WindowSize.y));
				
				// draw the viewing plane
				DebugDrawer::getInstance().setTransformDefault();
				DebugDrawer::getInstance().drawLine(x0y0, x1y0, glm::vec4(1.f, 0.f, 1.f, 1.f));
				DebugDrawer::getInstance().drawLine(x1y0, x1y1, glm::vec4(1.f, 0.f, 1.f, 1.f));
				DebugDrawer::getInstance().drawLine(x1y1, x0y1, glm::vec4(1.f, 0.f, 1.f, 1.f));
				DebugDrawer::getInstance().drawLine(x0y1, x0y0, glm::vec4(1.f, 0.f, 1.f, 1.f));

				// connect the viewing plane corners to view pos
				DebugDrawer::getInstance().drawLine(m_Camera.pos, x1y0, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 1.f, 1.f));
				DebugDrawer::getInstance().drawLine(m_Camera.pos, x1y1, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 1.f, 1.f));
				DebugDrawer::getInstance().drawLine(m_Camera.pos, x0y1, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 1.f, 1.f));
				DebugDrawer::getInstance().drawLine(m_Camera.pos, x0y0, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 1.f, 1.f));

				// draw the lasso points, if any
				//std::vector<glm::vec3> pts = m_pLasso->getPoints();
				//for (int i = 0; i < pts.size(); ++i)
				//{
				//	glm::vec3 pt1 = pts[i];
				//	glm::vec3 pt2 = pts[(i + 1) % pts.size()];
				//	DebugDrawer::getInstance().drawLine
				//	(
				//		glm::unProject(pt1, m_sviDesktop3DViewInfo.view, proj, glm::vec4(0.f, 0.f, m_ivec2DesktopWindowSize.x, m_ivec2DesktopWindowSize.y)), 
				//		glm::unProject(pt2, m_sviDesktop3DViewInfo.view, proj, glm::vec4(0.f, 0.f, m_ivec2DesktopWindowSize.x, m_ivec2DesktopWindowSize.y)),
				//		glm::vec4(0.f, 1.f, 0.f, 1.f)
				//	);
				//}
			}
		}

		if (m_bUseDesktop && !m_bStudyMode)
		{
			std::stringstream ss;
			ss.precision(2);

			ss << std::fixed << m_msFrameTime.count() << "ms/frame | " << 1.f / std::chrono::duration_cast<std::chrono::duration<float>>(m_msFrameTime).count() << "fps";

			Renderer::getInstance().drawUIText(
				ss.str(),
				glm::vec4(1.f),
				glm::vec3(0.f),
				glm::quat(),
				20.f,
				Renderer::HEIGHT,
				Renderer::CENTER,
				Renderer::BOTTOM_LEFT
			);
		}

		bool unloadedData = false;

		for (auto &dv : m_vpDataVolumes)
		{
			if (!dv->isVisible()) continue;

			glm::mat4 trans;
			
			if (m_bUseDesktop)
				trans = glm::inverse(m_sviWindow3DInfo.view);

			if (m_bUseVR)
				trans = m_pTDM->getHMDToWorldTransform();

			dv->drawVolumeBacking(trans, 1.f);
			dv->drawBBox(0.f);
			//dv->drawAxes();

			//draw table
			Renderer::RendererSubmission rs;
			rs.glPrimitiveType = GL_TRIANGLES;
			rs.shaderName = "instanced";
			rs.indexType = GL_UNSIGNED_SHORT;
			rs.indexByteOffset = Renderer::getInstance().getPrimitiveIndexByteOffset("disc");
			rs.indexBaseVertex = Renderer::getInstance().getPrimitiveIndexBaseVertex("disc");
			rs.vertCount = Renderer::getInstance().getPrimitiveIndexCount("disc");
			rs.instanced = true;
			rs.specularExponent = 32.f;
			//rs.diffuseColor = glm::vec4(1.f, 1.f, 1.f, 0.5f);
			//rs.diffuseTexName = "resources/images/circle.png";

			for (auto &cloud : dv->getDatasets())
			{
				if (!static_cast<SonarPointCloud*>(cloud)->ready())
				{
					unloadedData = true;
					continue;
				}

				rs.VAO = dv == m_pWallVolume ? static_cast<SonarPointCloud*>(cloud)->getPreviewVAO() : static_cast<SonarPointCloud*>(cloud)->getVAO();
				rs.modelToWorldTransform = dv->getTransformDataset(cloud);
				rs.instanceCount = dv == m_pWallVolume ? static_cast<SonarPointCloud*>(cloud)->getPreviewPointCount() : static_cast<SonarPointCloud*>(cloud)->getPointCount();
				Renderer::getInstance().addToDynamicRenderQueue(rs);
			}
		}

		if (!unloadedData && !m_bInitialColorRefresh)
		{
			refreshColorScale(m_pColorScalerTPU, m_vpClouds);
			m_bInitialColorRefresh = true;
		}
	}

	if (m_bFlowVis && m_pFlowVolume)
	{
		m_pFlowVolume->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), 1.f);
		m_pFlowVolume->drawBBox(0.f);

		m_pFlowVolume->draw();
	}

	if (m_bUseVR)
	{
		m_pTDM->draw();

		InfoBoxManager::getInstance().draw();
	}

	if (m_bUseDesktop)
	{
	}

	// MUST be run last to xfer previous debug draw calls to opengl buffers
	DebugDrawer::getInstance().draw();
}

void Engine::render()
{
	if (m_bUseVR)
	{
		SDL_GL_MakeCurrent(m_pWindow, m_pGLContext);

		// Update eye positions using current HMD position
		m_sviLeftEyeInfo.view = m_sviLeftEyeInfo.viewTransform * m_pTDM->getWorldToHMDTransform();
		m_sviRightEyeInfo.view = m_sviRightEyeInfo.viewTransform * m_pTDM->getWorldToHMDTransform();

		Renderer::getInstance().sortTransparentObjects(glm::vec3(m_pTDM->getHMDToWorldTransform()[3]));

		Renderer::getInstance().renderFrame(&m_sviLeftEyeInfo, m_pLeftEyeFramebuffer);
		Renderer::getInstance().renderFrame(&m_sviRightEyeInfo, m_pRightEyeFramebuffer);

		GLint srcX0 = m_sviLeftEyeInfo.m_nRenderWidth / 2 - m_sviWindowUIInfo.m_nRenderWidth / 2;
		GLint srcX1 = srcX0 + m_sviWindowUIInfo.m_nRenderWidth;
		GLint srcY0 = m_sviLeftEyeInfo.m_nRenderHeight / 2 - m_sviWindowUIInfo.m_nRenderHeight / 2;
		GLint srcY1 = srcY0 + m_sviWindowUIInfo.m_nRenderHeight;

		glBlitNamedFramebuffer(
			m_pLeftEyeFramebuffer->m_nRenderFramebufferId,
			m_pWindowFramebuffer->m_nRenderFramebufferId,
			srcX0, srcY0, srcX1, srcY1,
			0, 0, m_sviWindowUIInfo.m_nRenderWidth, m_sviWindowUIInfo.m_nRenderHeight,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
			GL_NEAREST);

		Renderer::getInstance().renderUI(&m_sviWindowUIInfo, m_pWindowFramebuffer);

		Renderer::getInstance().renderFullscreenTexture(
			m_ivec2WindowSize.x,
			m_ivec2WindowSize.y,
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

	if (m_bUseDesktop)
	{
		SDL_GL_MakeCurrent(m_pWindow, m_pGLContext);

		Renderer::getInstance().sortTransparentObjects(glm::vec3(glm::inverse(m_sviWindow3DInfo.view)[3]));

		Renderer::getInstance().renderFrame(&m_sviWindow3DInfo, m_pWindowFramebuffer);

		Renderer::getInstance().renderFullscreenTexture(m_ivec2WindowSize.x, m_ivec2WindowSize.y, m_pWindowFramebuffer->m_nResolveTextureId);

		SDL_GL_SwapWindow(m_pWindow);
	}

	Renderer::getInstance().clearDynamicRenderQueue();
	Renderer::getInstance().clearUIRenderQueue();
	DebugDrawer::getInstance().flushLines();
}

void Engine::refreshColorScale(ColorScaler * colorScaler, std::vector<SonarPointCloud*> clouds)
{
	if (clouds.size() == 0ull)
		return;

	float minDepthTPU = (*std::min_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcDepthTPUMinCompare))->getMinDepthTPU();
	float maxDepthTPU = (*std::max_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcDepthTPUMaxCompare))->getMaxDepthTPU();

	float minPosTPU = (*std::min_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcPosTPUMinCompare))->getMinPositionalTPU();
	float maxPosTPU = (*std::max_element(clouds.begin(), clouds.end(), SonarPointCloud::s_funcPosTPUMaxCompare))->getMaxPositionalTPU();

	colorScaler->resetMinMaxForColorScale(m_pTableVolume->getMinDataBound().z, m_pTableVolume->getMaxDataBound().z);
	colorScaler->resetBiValueScaleMinMax(minDepthTPU, maxDepthTPU, minPosTPU, maxPosTPU);

	// apply new color scale
	for (auto &cloud : clouds)
		cloud->resetAllMarks();
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

void Engine::createVRViews()
{
	uint32_t renderWidth, renderHeight;
	m_pHMD->GetRecommendedRenderTargetSize(&renderWidth, &renderHeight);

	m_sviLeftEyeInfo.m_nRenderWidth = renderWidth;
	m_sviLeftEyeInfo.m_nRenderHeight = renderHeight;
	m_sviLeftEyeInfo.viewTransform = glm::inverse(m_pTDM->getHMDEyeToHeadTransform(vr::Eye_Left));
	m_sviLeftEyeInfo.projection = m_pTDM->getHMDEyeProjection(vr::Eye_Left, g_fNearClip, g_fFarClip);

	m_sviRightEyeInfo.m_nRenderWidth = renderWidth;
	m_sviRightEyeInfo.m_nRenderHeight = renderHeight;
	m_sviRightEyeInfo.viewTransform = glm::inverse(m_pTDM->getHMDEyeToHeadTransform(vr::Eye_Right));
	m_sviRightEyeInfo.projection = m_pTDM->getHMDEyeProjection(vr::Eye_Right, g_fNearClip, g_fFarClip);

	m_pLeftEyeFramebuffer = new Renderer::FramebufferDesc();
	m_pRightEyeFramebuffer = new Renderer::FramebufferDesc();

	if (!Renderer::getInstance().createFrameBuffer(m_sviLeftEyeInfo.m_nRenderWidth, m_sviLeftEyeInfo.m_nRenderHeight, *m_pLeftEyeFramebuffer))
		utils::dprintf("Could not create left eye framebuffer!\n");
	if (!Renderer::getInstance().createFrameBuffer(m_sviRightEyeInfo.m_nRenderWidth, m_sviRightEyeInfo.m_nRenderHeight, *m_pRightEyeFramebuffer))
		utils::dprintf("Could not create right eye framebuffer!\n");
}

void Engine::createDesktopView()
{
	m_sviWindowUIInfo.m_nRenderWidth = m_ivec2WindowSize.x;
	m_sviWindowUIInfo.m_nRenderHeight = m_ivec2WindowSize.y;

	m_sviWindowUIInfo.view = glm::mat4();
	m_sviWindowUIInfo.projection = glm::ortho(0.f, static_cast<float>(m_sviWindowUIInfo.m_nRenderWidth), 0.f, static_cast<float>(m_sviWindowUIInfo.m_nRenderHeight), -1.f, 1.f);

	m_sviWindow3DInfo.m_nRenderWidth = m_ivec2WindowSize.x;
	m_sviWindow3DInfo.m_nRenderHeight = m_ivec2WindowSize.y;
	m_sviWindow3DInfo.projection = glm::perspective(glm::radians(g_fDesktopWindowFOV), (float)m_ivec2WindowSize.x / (float)m_ivec2WindowSize.y, g_fNearClip, g_fFarClip);

	m_pWindowFramebuffer = new Renderer::FramebufferDesc();

	if (!Renderer::getInstance().createFrameBuffer(m_ivec2WindowSize.x, m_ivec2WindowSize.y, *m_pWindowFramebuffer))
		utils::dprintf("Could not create desktop view framebuffer!\n");	
}
