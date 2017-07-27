#include "CMainApplication.h"
#include "DebugDrawer.h"
#include "InfoBoxManager.h"
#include "CloudCollection.h"

#include "ManipulateDataVolumeBehavior.h"
#include "FlowProbe.h"
#include "AdvectionProbe.h"
#include "PointCleanProbe.h"
#include "HolodeckBackground.h"

#include <fstream>
#include <sstream>
#include <string>

#include <ctime>

HolodeckBackground*				g_pHolodeck = NULL;
glm::vec3						g_vec3RoomSize(10.f, 4.f, 6.f);
ManipulateDataVolumeBehavior*	g_pManipulateDataVolumeBehavior = NULL;
FlowProbe*						g_pFlowProbeBehavior = NULL;
AdvectionProbe*					g_pAdvectionProbeBehavior = NULL;
PointCleanProbe*				g_pPointCleanProbeBehavior = NULL;

float							g_fNearClip = 0.001f;
float							g_fFarClip = 1000.f;
const glm::ivec2				g_ivec2DesktopInitialWindowSize(500, 500);
float							g_fDesktopWindowFOV(45.f);


std::vector<BehaviorBase*> g_vpBehaviors;

//-----------------------------------------------------------------------------
// Purpose: OpenGL Debug Callback Function
//-----------------------------------------------------------------------------
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

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
// Purpose:
//-----------------------------------------------------------------------------
void dprintf(const char *fmt, ...)
{
	va_list args;
	char buffer[2048];

	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

	if (g_bPrintf)
		printf("%s", buffer);

	OutputDebugStringA(buffer);
}


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMainApplication::CMainApplication(int argc, char *argv[], int mode)
	: m_bUseVR(false)
	, m_bUseDesktop(false)
	, m_bSonarCleaning(false)
	, m_bFlowVis(false)
	, m_bGreatBayModel(false)
	, m_bShowDesktopFrustum(false)
	, m_bStudyMode(false)
	, m_bDemoMode(false)
	, m_bGLInitialized(false)
	, m_bLeftMouseDown(false)
	, m_bRightMouseDown(false)
	, m_pVRCompanionWindow(NULL)
	, m_pGLContext(NULL)
	, m_pDesktopWindow(NULL)
	, m_pDesktopWindowCursor(NULL)
	, m_pHMD(NULL)
	, m_bVerbose(false)
	, m_bPerf(false)
	, m_fPtHighlightAmt(1.f)
	, m_LastTime(std::chrono::high_resolution_clock::now())
	, m_Arcball(Arcball(false))
	, m_vec3BallEye(glm::vec3(0.f, 0.f, 1.f))
	, m_vec3BallCenter(glm::vec3(0.f))
	, m_vec3BallUp(glm::vec3(0.f, 1.f, 0.f))
	, m_fBallRadius(2.f)
	, m_pLasso(NULL)
{
#if _DEBUG
	m_bDebugOpenGL = true;
#else
	m_bDebugOpenGL = false;
#endif
	
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
		dprintf("Invalid Selection, shutting down...");
		Shutdown();
		break;
	}
};


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMainApplication::~CMainApplication()
{
	// work is done in Shutdown
	dprintf("Shutdown");
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	if (m_bUseVR && !initVR())
	{
		printf("%s - Unable to initialize VR!\n", __FUNCTION__);
		return false;
	}

	if (m_bUseDesktop && !initDesktop())
	{
		printf("%s - Unable to initialize Desktop Mode!\n", __FUNCTION__);
		return false;
	}

	if (m_bSonarCleaning)
	{
		m_pColorScalerTPU = new ColorScaler();
		m_pColorScalerTPU->setColorScale(2);
		m_pColorScalerTPU->setBiValueScale(1);

		m_pClouds = new CloudCollection();
		m_pClouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_1085.txt");
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_528_1324.txt");
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1516.txt");
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1508.txt");
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1500.txt");
		///	clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-148_XL_901_1458.txt");  //TO BIG AND LONG at 90 degree angle to others
		//clouds->loadCloud("H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-148_148_000_2022.txt");	
		m_pClouds->calculateCloudBoundsAndAlign();

		m_pColorScalerTPU->resetBiValueScaleMinMax(m_pClouds->getMinDepthTPU(), m_pClouds->getMaxDepthTPU(), m_pClouds->getMinPositionalTPU(), m_pClouds->getMaxPositionalTPU());

		glm::vec3 wallSize((g_vec3RoomSize.x * 0.9f), (g_vec3RoomSize.y * 0.8f), 0.8f);
		glm::vec3 wallPosition(0.f, (g_vec3RoomSize.y * 0.5f) + (g_vec3RoomSize.y * 0.09f), (g_vec3RoomSize.z * 0.5f) - 0.42f);

		glm::vec3 wallMinCoords(m_pClouds->getXMin(), m_pClouds->getYMin(), m_pClouds->getMinDepth());
		glm::vec3 wallMaxCoords(m_pClouds->getXMax(), m_pClouds->getYMax(), m_pClouds->getMaxDepth());

		glm::vec3 tablePosition;
		glm::vec3 tableSize;

		tablePosition = glm::vec3(0.f, 1.1f, 0.f);
		tableSize = glm::vec3(2.25f, 2.25f, 0.75f);
		m_vec3BallEye = tablePosition + glm::vec3(0.f, 0.f, 1.f) * 3.f;
		m_vec3BallCenter = tablePosition;

		glm::vec3 tableMinCoords(m_pClouds->getCloud(0)->getXMin(), m_pClouds->getCloud(0)->getYMin(), m_pClouds->getCloud(0)->getMinDepth());
		glm::vec3 tableMaxCoords(m_pClouds->getCloud(0)->getXMax(), m_pClouds->getCloud(0)->getYMax(), m_pClouds->getCloud(0)->getMaxDepth());

		tableVolume = new DataVolume(tablePosition, 0, tableSize, tableMinCoords, tableMaxCoords);
		wallVolume = new DataVolume(wallPosition, 1, wallSize, wallMinCoords, wallMaxCoords);

	}
	else if (m_bFlowVis)
	{
		FlowGrid *tempFG;

		if (m_bGreatBayModel)
		{
			tempFG = new FlowGrid("gb.fg", false);
			tempFG->m_fIllustrativeParticleVelocityScale = 0.5f;
		}
		else
		{
			tempFG = new FlowGrid("test.fg", true);
			tempFG->m_fIllustrativeParticleVelocityScale = 0.01f;
		}

		tempFG->setCoordinateScaler(new CoordinateScaler());
		flowVolume = new FlowVolume(tempFG);

		if (m_bGreatBayModel)
			flowVolume->setDimensions(glm::vec3(fmin(g_vec3RoomSize.x, g_vec3RoomSize.z) * 0.5f, fmin(g_vec3RoomSize.x, g_vec3RoomSize.z) * 0.5f, g_vec3RoomSize.y * 0.05f));

		m_vec3BallEye = flowVolume->getPosition() + glm::vec3(0.f, 0.f, 1.f) * 3.f;
		m_vec3BallCenter = flowVolume->getPosition();
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::initGL()
{
	if (m_bGLInitialized)
		return true;

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

bool CMainApplication::initVR()
{
	m_pVRCompanionWindow = createFullscreenWindow(1);

	SDL_GetWindowSize(m_pVRCompanionWindow, &m_nVRCompanionWindowWidth, &m_nVRCompanionWindowHeight);

	if (!m_pGLContext)
		m_pGLContext = SDL_GL_CreateContext(m_pVRCompanionWindow);

	if (m_pGLContext == NULL)
	{
		printf("%s - VR companion window OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}


	if (!initGL())
	{
		printf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
		return false;
	}

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
		printf("Compositor initialization failed. See log file for details\n");
		return false;
	}

	m_pTDM = new TrackedDeviceManager(m_pHMD);

	if (!m_pTDM->init())
	{
		dprintf("Error initializing TrackedDeviceManager\n");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", "Could not initialize the Tracked Device Manager", NULL);
	}

	InfoBoxManager::getInstance().BInit(m_pTDM);
	m_pTDM->attach(&InfoBoxManager::getInstance());

	createVRViews();

	if (m_bSonarCleaning)
	{
		std::string strWindowTitle = "VR Sonar Cleaner | CCOM VisLab";
		SDL_SetWindowTitle(m_pVRCompanionWindow, strWindowTitle.c_str());
	}
	else if (m_bFlowVis)
	{
		std::string strWindowTitle = "VR Flow 4D | CCOM VisLab";
		SDL_SetWindowTitle(m_pVRCompanionWindow, strWindowTitle.c_str());
	}

	if (!m_bUseDesktop)
		g_pHolodeck = new HolodeckBackground(g_vec3RoomSize, 0.25f);

	return true;
}


bool CMainApplication::initDesktop()
{
	//m_pDesktopWindow = createWindow(g_ivec2DesktopInitialWindowSize.x, g_ivec2DesktopInitialWindowSize.y);
	m_pDesktopWindow = createFullscreenWindow(0);
	if (!m_pGLContext)
		m_pGLContext = SDL_GL_CreateContext(m_pDesktopWindow);

	SDL_GetWindowSize(m_pDesktopWindow, &m_ivec2DesktopWindowSize.x, &m_ivec2DesktopWindowSize.y);

	if (!initGL())
	{
		printf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
		return false;
	}

	m_pDesktopWindowCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	SDL_SetCursor(m_pDesktopWindowCursor);
	
	createDesktopView();

	m_pLasso = new LassoTool();

	if (m_bFlowVis)
	{
		m_vec3BallEye = flowVolume->getPosition() + glm::vec3(0.f, 0.f, 1.f) * 3.f;
		m_vec3BallCenter = flowVolume->getPosition();
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::Shutdown()
{
	if (m_bUseVR)
	{
		delete m_pLeftEyeFramebuffer;
		delete m_pRightEyeFramebuffer;

		if (m_pHMD)
		{
			vr::VR_Shutdown();
			m_pHMD = NULL;
		}

		if (m_pTDM)
			delete m_pTDM;

		if (m_pGLContext)
		{
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
			glDebugMessageCallback(nullptr, nullptr);
		}

		if (m_pVRCompanionWindow)
		{
			SDL_DestroyWindow(m_pVRCompanionWindow);
			m_pVRCompanionWindow = NULL;
		}
	}

	fclose(stdout);
	FreeConsole();

	SDL_Quit();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::HandleInput()
{
	SDL_Event sdlEvent;
	bool bRet = false;

	if (m_bUseVR)
		m_pTDM->handleEvents();

	while (SDL_PollEvent(&sdlEvent) != 0)
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

			if ((sdlEvent.key.keysym.mod & KMOD_LCTRL) && sdlEvent.key.keysym.sym == SDLK_v)
			{
				if (!m_bUseVR)
				{
					m_bUseVR = true;
					initVR();
				}
			}

			if (sdlEvent.key.keysym.sym == SDLK_d)
			{
				if ((sdlEvent.key.keysym.mod & KMOD_LCTRL))
				{
					if (!m_bUseDesktop)
					{
						m_bUseDesktop = true;
						initDesktop();
					}
				}
				else
				{
					m_bDemoMode = !m_bDemoMode;

					if (g_pAdvectionProbeBehavior)					
						m_bDemoMode ? g_pAdvectionProbeBehavior->activateDemoMode() : g_pAdvectionProbeBehavior->deactivateDemoMode();

					if (g_pFlowProbeBehavior)
						m_bDemoMode ? g_pFlowProbeBehavior->activateDemoMode() : g_pFlowProbeBehavior->deactivateDemoMode();

					if (g_pPointCleanProbeBehavior)
						m_bDemoMode ? g_pPointCleanProbeBehavior->activateDemoMode() : g_pPointCleanProbeBehavior->deactivateDemoMode();					
				}
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
				if ((sdlEvent.key.keysym.mod & KMOD_LCTRL) && sdlEvent.key.keysym.sym == SDLK_s)
				{
					savePoints();
				}
				//if ((sdlEvent.key.keysym.mod & KMOD_LCTRL) && sdlEvent.key.keysym.sym == SDLK_l)
				//{
				//	loadPoints("test.txt");
				//}
				if (sdlEvent.key.keysym.sym == SDLK_r)
				{
					printf("Pressed r, resetting marks\n");
					m_pClouds->resetMarksInAllClouds();
					//cleaningRoom->resetVolumes();
					wallVolume->resetPositionAndOrientation();
					tableVolume->resetPositionAndOrientation();
				}

				if (sdlEvent.key.keysym.sym == SDLK_g)
				{
					printf("Pressed g, generating fake test cloud\n");
					m_pClouds->clearAllClouds();
					m_pClouds->generateFakeTestCloud(150, 150, 25, 40000);
					m_pClouds->calculateCloudBoundsAndAlign();
					m_pColorScalerTPU->resetBiValueScaleMinMax(m_pClouds->getMinDepthTPU(), m_pClouds->getMaxDepthTPU(), m_pClouds->getMinPositionalTPU(), m_pClouds->getMaxPositionalTPU());
					//cleaningRoom->recalcVolumeBounds();
					glm::vec3 tableMinCoords(m_pClouds->getCloud(0)->getXMin(), m_pClouds->getCloud(0)->getMinDepth(), m_pClouds->getCloud(0)->getYMin());
					glm::vec3 tableMaxCoords(m_pClouds->getCloud(0)->getXMax(), m_pClouds->getCloud(0)->getMaxDepth(), m_pClouds->getCloud(0)->getYMax());

					glm::vec3 wallMinCoords(m_pClouds->getXMin(), m_pClouds->getMinDepth(), m_pClouds->getYMin());
					glm::vec3 wallMaxCoords(m_pClouds->getXMax(), m_pClouds->getMaxDepth(), m_pClouds->getYMax());

					tableVolume->setInnerCoords(tableMinCoords, tableMaxCoords);
					wallVolume->setInnerCoords(wallMinCoords, wallMaxCoords);
				}

				if (sdlEvent.key.keysym.sym == SDLK_SPACE)
				{
					if (m_pLasso->readyToCheck())
					{
						editCleaningTableDesktop();
						m_pLasso->reset();
					}
				}
			}
			else if (m_bFlowVis) //flow
			{
				if (sdlEvent.key.keysym.sym == SDLK_r)
				{
					printf("Pressed r, resetting something...\n");
					flowVolume->resetPositionAndOrientation();
				}

				if (sdlEvent.key.keysym.sym == SDLK_1)
				{
					if (m_bUseVR)
					{
						glm::mat3 matHMD(m_pTDM->getHMDToWorldTransform());
						flowVolume->setDimensions(glm::vec3(1.f, 1.f, 0.1f));
						flowVolume->setPosition(glm::vec3(m_pTDM->getHMDToWorldTransform()[3] - m_pTDM->getHMDToWorldTransform()[2] * 0.5f));

						glm::mat3 matOrientation;
						matOrientation[0] = matHMD[0];
						matOrientation[1] = matHMD[2];
						matOrientation[2] = -matHMD[1];
						flowVolume->setOrientation(glm::quat_cast(matHMD) * glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f)));
					}
				}

				if (sdlEvent.key.keysym.sym == SDLK_2)
				{
					if (m_bUseVR)
					{
						flowVolume->setDimensions(glm::vec3(fmin(g_vec3RoomSize.x, g_vec3RoomSize.z) * 0.9f, fmin(g_vec3RoomSize.x, g_vec3RoomSize.z) * 0.9f, g_vec3RoomSize.y * 0.1f));
						flowVolume->setPosition(glm::vec3(0.f, g_vec3RoomSize.y * 0.1f * 0.5f, 0.f));
						flowVolume->setOrientation(glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)));
					}
				}

				if ((sdlEvent.key.keysym.mod & KMOD_LCTRL) && sdlEvent.key.keysym.sym == SDLK_f)
					printf("FPS: %u\n", m_uiCurrentFPS);
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
					m_Arcball.start(sdlEvent.button.x, sdlEvent.button.y);
					if (m_bRightMouseDown)
					{
						m_pLasso->end();
					}
					m_pLasso->reset();
				}
				if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
				{
					m_bRightMouseDown = true;
					if (!m_bLeftMouseDown)
						m_pLasso->start(sdlEvent.button.x, m_ivec2DesktopWindowSize.y - sdlEvent.button.y);
				}

			}//end mouse down 
			else if (sdlEvent.type == SDL_MOUSEBUTTONUP) //MOUSE UP
			{
				if (sdlEvent.button.button == SDL_BUTTON_LEFT)
				{
					m_bLeftMouseDown = false;
					m_pLasso->reset();
				}
				if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
				{
					m_bRightMouseDown = false;
					m_pLasso->end();
				}

			}//end mouse up
			if (sdlEvent.type == SDL_MOUSEMOTION)
			{
				if (m_bLeftMouseDown)
				{
					m_Arcball.move(sdlEvent.button.x, sdlEvent.button.y);
				}
				if (m_bRightMouseDown && !m_bLeftMouseDown)
				{
					m_pLasso->move(sdlEvent.button.x, m_ivec2DesktopWindowSize.y - sdlEvent.button.y);
				}
			}
			if (sdlEvent.type == SDL_MOUSEWHEEL)
			{
				m_pLasso->reset();
				m_vec3BallEye.z -= ((float)sdlEvent.wheel.y*0.5f);
				if (m_vec3BallEye.z < 0.5f)
					m_vec3BallEye.z = 0.5f;
				if (m_vec3BallEye.z > 10.f)
					m_vec3BallEye.z = 10.f;
			}
		}
	}
	
		
	return bRet;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RunMainLoop()
{
	bool bQuit = false;

	SDL_StartTextInput();

	float fps_interval = 1.0; // sec
	Uint32 fps_lasttime = SDL_GetTicks();
	m_uiCurrentFPS = 0u;
	Uint32 fps_frames = 0;

	std::clock_t start;

	ULONGLONG lastTime = GetTickCount64();

	while (!bQuit)
	{
		ULONGLONG currentTime = GetTickCount64();
		ULONGLONG elapsedTime = currentTime - lastTime;
		ULONGLONG timeBefore, timeAfter;;

		start = std::clock();

		//std::cout << "\tElapsed: " << elapsedTime << "ms\n";
		//std::cout << "+++++++++++++++++++++++++++++++++\n";

		//timeBefore = GetTickCount64();
		bQuit = HandleInput();
		//timeAfter = GetTickCount64();
		//std::cout << "\tHandleInput(): " << timeAfter - timeBefore << "ms\n";

		//timeBefore = GetTickCount64();
		update();
		//timeAfter = GetTickCount64();
		//std::cout << "\tupdate(): " << timeAfter - timeBefore << "ms\n";

		//timeBefore = GetTickCount64();
		drawScene();
		//timeAfter = GetTickCount64();
		//std::cout << "\tdrawScene(): " << timeAfter - timeBefore << "ms\n";

		//timeBefore = GetTickCount64();
		render();
		//timeAfter = GetTickCount64();
		//std::cout << "\trender(): " << timeAfter - timeBefore << "ms\n";

		// Calls WaitGetPoses(), which should occur first thing after rendering
		//timeBefore = GetTickCount64();
		if (m_bUseVR)
			m_pTDM->update();
		//timeAfter = GetTickCount64();
		//std::cout << "\tTrackedDeviceManager::update(): " << timeAfter - timeBefore << "ms\n";

		lastTime = currentTime;

		fps_frames++;
		if (fps_lasttime < SDL_GetTicks() - fps_interval * 1000)
		{
			fps_lasttime = SDL_GetTicks();
			m_uiCurrentFPS = fps_frames;
			fps_frames = 0;
		}

		//std::cout << "FPS: " << m_uiCurrentFPS << std::endl;
		//std::cout << "--------------------------------------------------" << std::endl << std::endl;
	}

	////doesn't help here either
	fclose(stdout);
	FreeConsole();
	
	SDL_StopTextInput();
}


void CMainApplication::update()
{
	if (m_bUseVR)
	{
		if (m_bFlowVis)
		{
			if (m_pTDM->getPrimaryController() && !g_pFlowProbeBehavior)
			{
				g_pFlowProbeBehavior = new FlowProbe(m_pTDM->getPrimaryController(), flowVolume);
				g_vpBehaviors.push_back(g_pFlowProbeBehavior);
			}

			if (m_pTDM->getSecondaryController() && !g_pAdvectionProbeBehavior)
			{
				g_pAdvectionProbeBehavior = new AdvectionProbe(m_pTDM->getSecondaryController(), flowVolume);
				g_vpBehaviors.push_back(g_pAdvectionProbeBehavior);
			}
		}

		if (m_bSonarCleaning)
		{
			if (m_pTDM->getPrimaryController() && !g_pPointCleanProbeBehavior)
			{
				g_pPointCleanProbeBehavior = new PointCleanProbe(m_pTDM->getPrimaryController(), tableVolume, m_pClouds->getCloud(0), m_pHMD);
				g_vpBehaviors.push_back(g_pPointCleanProbeBehavior);
			}
		}

		// Attach grip & scale behavior when both controllers available
		if (m_pTDM->getSecondaryController() && m_pTDM->getPrimaryController() && !g_pManipulateDataVolumeBehavior)
		{
			if (m_bSonarCleaning)
				g_pManipulateDataVolumeBehavior = new ManipulateDataVolumeBehavior(m_pTDM->getSecondaryController(), m_pTDM->getPrimaryController(), tableVolume);
			else if (m_bFlowVis)
				g_pManipulateDataVolumeBehavior = new ManipulateDataVolumeBehavior(m_pTDM->getSecondaryController(), m_pTDM->getPrimaryController(), flowVolume);
			g_vpBehaviors.push_back(g_pManipulateDataVolumeBehavior);
		}

		for (auto const &b : g_vpBehaviors)
			b->update();
	}

	if (m_bUseDesktop)
	{
		m_sviDesktop3DViewInfo.view = glm::lookAt(m_vec3BallEye, m_vec3BallCenter, m_vec3BallUp);

		glm::vec4 vp(0.f, 0.f, static_cast<float>(m_ivec2DesktopWindowSize.x), static_cast<float>(m_ivec2DesktopWindowSize.y));

		m_Arcball.setProjectionMatrix(m_sviDesktop3DViewInfo.projection * m_sviDesktop3DViewInfo.view);
		m_Arcball.setViewport(vp);

		m_Arcball.setZoom(m_fBallRadius, m_vec3BallEye, m_vec3BallUp);
	}

	if (m_bSonarCleaning)
	{
		if (m_bUseVR)
		{

		}

		if (m_bUseDesktop)
			tableVolume->setOrientation(tableVolume->getOriginalOrientation() * glm::quat_cast(m_Arcball.getRotation()));
	}

	if (m_bFlowVis)
	{
		flowVolume->preRenderUpdates();
	}
}

void CMainApplication::drawScene()
{

	for (auto const &b : g_vpBehaviors)
		b->draw();

	if (m_bUseVR)
	{
		m_pTDM->draw();

		InfoBoxManager::getInstance().draw();
	}

	if (m_bUseDesktop)
	{
	}

	if (m_bSonarCleaning)
	{
		if (m_bUseVR)
		{
			if (!m_bUseDesktop)
			{
				wallVolume->drawBBox();
				wallVolume->drawBacking();
				tableVolume->drawBacking();

				//draw wall
				DebugDrawer::getInstance().setTransform(wallVolume->getCurrentDataTransform());
				m_pClouds->drawAllClouds(m_pColorScalerTPU);
			}
			else if (m_bShowDesktopFrustum)
			{
				// get frustum with near plane 1m out from view pos
				glm::mat4 proj = glm::perspective(glm::radians(g_fDesktopWindowFOV), (float)m_ivec2DesktopWindowSize.x / (float)m_ivec2DesktopWindowSize.y, 1.f, g_fFarClip);

				// get world-space points for the viewing plane
				glm::vec3 x0y0 = glm::unProject(glm::vec3(0.f), m_sviDesktop3DViewInfo.view, proj, glm::vec4(0.f, 0.f, m_ivec2DesktopWindowSize.x, m_ivec2DesktopWindowSize.y));
				glm::vec3 x1y0 = glm::unProject(glm::vec3(m_ivec2DesktopWindowSize.x, 0.f, 0.f), m_sviDesktop3DViewInfo.view, proj, glm::vec4(0.f, 0.f, m_ivec2DesktopWindowSize.x, m_ivec2DesktopWindowSize.y));
				glm::vec3 x0y1 = glm::unProject(glm::vec3(0.f, m_ivec2DesktopWindowSize.y, 0.f), m_sviDesktop3DViewInfo.view, proj, glm::vec4(0.f, 0.f, m_ivec2DesktopWindowSize.x, m_ivec2DesktopWindowSize.y));
				glm::vec3 x1y1 = glm::unProject(glm::vec3(m_ivec2DesktopWindowSize.x, m_ivec2DesktopWindowSize.y, 0.f), m_sviDesktop3DViewInfo.view, proj, glm::vec4(0.f, 0.f, m_ivec2DesktopWindowSize.x, m_ivec2DesktopWindowSize.y));
				
				// draw the viewing plane
				DebugDrawer::getInstance().setTransformDefault();
				DebugDrawer::getInstance().drawLine(x0y0, x1y0, glm::vec4(1.f, 0.f, 1.f, 1.f));
				DebugDrawer::getInstance().drawLine(x1y0, x1y1, glm::vec4(1.f, 0.f, 1.f, 1.f));
				DebugDrawer::getInstance().drawLine(x1y1, x0y1, glm::vec4(1.f, 0.f, 1.f, 1.f));
				DebugDrawer::getInstance().drawLine(x0y1, x0y0, glm::vec4(1.f, 0.f, 1.f, 1.f));

				// connect the viewing plane corners to view pos
				DebugDrawer::getInstance().drawLine(m_vec3BallEye, x1y0, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 1.f, 1.f));
				DebugDrawer::getInstance().drawLine(m_vec3BallEye, x1y1, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 1.f, 1.f));
				DebugDrawer::getInstance().drawLine(m_vec3BallEye, x0y1, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 1.f, 1.f));
				DebugDrawer::getInstance().drawLine(m_vec3BallEye, x0y0, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec4(1.f, 0.f, 1.f, 1.f));

				// draw the lasso points, if any
				std::vector<glm::vec3> pts = m_pLasso->getPoints();
				for (int i = 0; i < pts.size(); ++i)
				{
					glm::vec3 pt1 = pts[i];
					glm::vec3 pt2 = pts[(i + 1) % pts.size()];
					DebugDrawer::getInstance().drawLine
					(
						glm::unProject(pt1, m_sviDesktop3DViewInfo.view, proj, glm::vec4(0.f, 0.f, m_ivec2DesktopWindowSize.x, m_ivec2DesktopWindowSize.y)), 
						glm::unProject(pt2, m_sviDesktop3DViewInfo.view, proj, glm::vec4(0.f, 0.f, m_ivec2DesktopWindowSize.x, m_ivec2DesktopWindowSize.y)),
						glm::vec4(0.f, 1.f, 0.f, 1.f)
					);
				}
			}
		}

		if (m_bUseDesktop)
		{
			//draw 2D interface elements
			Renderer::RendererSubmission rs;
			rs.modelToWorldTransform = glm::mat4();
			m_pLasso->prepareForRender(rs);
			Renderer::getInstance().addToUIRenderQueue(rs);
		}

		tableVolume->drawBBox();

		//draw table
		DebugDrawer::getInstance().setTransform(tableVolume->getCurrentDataTransform());
		m_pClouds->getCloud(0)->draw(m_pColorScalerTPU);

	}

	if (m_bFlowVis)
	{
		flowVolume->draw(); // currently draws to debug buffer
		flowVolume->drawBacking();
	}
}

void CMainApplication::render()
{
	if (m_bUseVR)
	{
		SDL_GL_MakeCurrent(m_pVRCompanionWindow, m_pGLContext);

		// Update eye positions using current HMD position
		m_sviLeftEyeInfo.view = m_sviLeftEyeInfo.viewTransform * m_pTDM->getWorldToHMDTransform();
		m_sviRightEyeInfo.view = m_sviRightEyeInfo.viewTransform * m_pTDM->getWorldToHMDTransform();

		Renderer::getInstance().RenderFrame(&m_sviLeftEyeInfo, NULL, m_pLeftEyeFramebuffer);
		Renderer::getInstance().RenderFrame(&m_sviRightEyeInfo, NULL, m_pRightEyeFramebuffer);

		Renderer::getInstance().RenderFullscreenTexture(m_nVRCompanionWindowWidth, m_nVRCompanionWindowHeight, m_pLeftEyeFramebuffer->m_nResolveTextureId, true);

		vr::Texture_t leftEyeTexture = { (void*)m_pLeftEyeFramebuffer->m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::Texture_t rightEyeTexture = { (void*)m_pRightEyeFramebuffer->m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

		vr::VRCompositor()->PostPresentHandoff();
		//std::cout << "Rendering Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;

		SDL_GL_SwapWindow(m_pVRCompanionWindow);
	}

	if (m_bUseDesktop)
	{
		SDL_GL_MakeCurrent(m_pDesktopWindow, m_pGLContext);

		Renderer::getInstance().RenderFrame(&m_sviDesktop3DViewInfo, &m_sviDesktop2DOverlayViewInfo, m_pDesktopFramebuffer);

		Renderer::getInstance().RenderFullscreenTexture(m_ivec2DesktopWindowSize.x, m_ivec2DesktopWindowSize.y, m_pDesktopFramebuffer->m_nResolveTextureId);

		SDL_GL_SwapWindow(m_pDesktopWindow);
	}

	Renderer::getInstance().clearDynamicRenderQueue();
	Renderer::getInstance().clearUIRenderQueue();
	DebugDrawer::getInstance().flushLines();
}


bool fileExists(const std::string &fname)
{
	struct stat buffer;
	return (stat(fname.c_str(), &buffer) == 0);
}

std::string intToString(int i, unsigned int pad_to_magnitude)
{
	if (pad_to_magnitude < 1)
		return std::to_string(i);

	std::string ret;

	int mag = i == 0 ? 0 : (int)log10(i);

	for (int j = pad_to_magnitude - mag; j > 0; --j)
		ret += std::to_string(0);

	ret += std::to_string(i);

	return ret;
}

std::string getTimeString()
{
	time_t t = time(0);   // get time now
	struct tm *now = localtime(&t);

	return 	  /*** DATE ***/
		      intToString(now->tm_year + 1900, 3) + // year
		"-" + intToString(now->tm_mon + 1, 1) +     // month
		"-" + intToString(now->tm_mday, 1) +        // day
		      /*** TIME ***/
		"_" + intToString(now->tm_hour, 1) +        // hour
		"-" + intToString(now->tm_min, 1) +         // minute
		"-" + intToString(now->tm_sec, 1);          // second
}

void CMainApplication::savePoints()
{	
	std::vector<glm::vec3> points = m_pClouds->getCloud(0)->getPointPositions();

	// construct filename
	std::string outFileName("saved_points_" + intToString(points.size(), 0) /*+ "_" + getTimeString()*/ + ".csv");

	// if file exists, keep trying until we find a filename that doesn't already exist
	for (int i = 0; fileExists(outFileName); ++i)
		outFileName = std::string("saved_points_" + intToString(points.size(), 0) + "_" /*+ getTimeString() + "_"*/ + "(" + std::to_string(i + 1) + ")" + ".csv");
	
	std::ofstream outFile;
	outFile.open(outFileName);

	if (outFile.is_open())
	{
		std::cout << "Opened file " << outFileName << " for writing output" << std::endl;
		outFile << "x,y,z,flag" << std::endl;
	}
	else
		std::cout << "Error opening file " << outFileName << " for writing output" << std::endl;

	for (size_t i = 0ull; i < points.size(); ++i)
	{
		outFile << points[i].x << "," << points[i].y << "," << points[i].z << "," << (m_pClouds->getCloud(0)->getPointMark(i) == 1 ? "1" : "0") << std::endl;
	}

	outFile.close();

	std::cout << "File " << outFileName << " successfully written" << std::endl;
}

bool CMainApplication::loadPoints(std::string fileName)
{
	std::ifstream inFile(fileName);

	if (inFile.is_open())
	{
		std::cout << "Opened file " << fileName << " for reading" << std::endl;
	}
	else
	{
		std::cout << "Error opening file " << fileName << " for reading" << std::endl;
		return false;
	}

	std::string line;

	if (!std::getline(inFile, line))
	{
		std::cout << "Empty file; aborting..." << std::endl;
		return false;
	}

	//make sure header is correct before proceeding
	if (line.compare("x,y,z,flag"))
	{
		std::cout << "Unrecognized file header (expected: x,y,z,flag); aborting..." << std::endl;
		return false;
	}
	
	std::vector<glm::vec3> points;
	std::vector<int> flags;

	int lineNo = 2; // already checked header at line 1
	while (std::getline(inFile, line))
	{
		std::istringstream iss(line);
		std::string xStr, yStr, zStr, flagStr;

		if (!std::getline(iss, xStr, ',')
			|| !std::getline(iss, yStr, ',')
			|| !std::getline(iss, zStr, ',')
			|| !std::getline(iss, flagStr, ',')
			)
		{
			std::cout << "Error reading line " << lineNo << " from file " << fileName << std::endl;
			return false;
		}

		points.push_back(glm::vec3(std::stof(xStr), std::stof(yStr), std::stof(zStr)));
		flags.push_back(std::stoi(flagStr));

		lineNo++;
	}

	inFile.close();

	std::cout << "Successfully read " << points.size() << " points from file " << fileName << std::endl;

	return true;
}

bool CMainApplication::editCleaningTableDesktop()
{
	bool hit = false;

	std::vector<glm::vec3> inPts = m_pClouds->getCloud(0)->getPointPositions();

	glm::vec4 vp(0.f, 0.f, static_cast<float>(m_ivec2DesktopWindowSize.x), static_cast<float>(m_ivec2DesktopWindowSize.y));

	for (int i = 0; i < inPts.size(); ++i)
	{
		glm::vec3 in = tableVolume->convertToWorldCoords(inPts[i]);
		glm::vec3 out = glm::project(in, m_sviDesktop3DViewInfo.view, m_sviDesktop3DViewInfo.projection, vp);

		if (m_pLasso->checkPoint(glm::vec2(out)))
		{
			m_pClouds->getCloud(0)->markPoint(i, 1);
			hit = true;
			m_pClouds->getCloud(0)->setRefreshNeeded();
		}
	}

	return hit;
}

SDL_Window * CMainApplication::createFullscreenWindow(int displayIndex)
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
		return false;
	}

	return win;
}

SDL_Window * CMainApplication::createWindow(int width, int height, int displayIndex)
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

void CMainApplication::createVRViews()
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

	if (!Renderer::getInstance().CreateFrameBuffer(m_sviLeftEyeInfo.m_nRenderWidth, m_sviLeftEyeInfo.m_nRenderHeight, *m_pLeftEyeFramebuffer))
		dprintf("Could not create left eye framebuffer!\n");
	if (!Renderer::getInstance().CreateFrameBuffer(m_sviRightEyeInfo.m_nRenderWidth, m_sviRightEyeInfo.m_nRenderHeight, *m_pRightEyeFramebuffer))
		dprintf("Could not create right eye framebuffer!\n");
}

void CMainApplication::createDesktopView()
{
	m_sviDesktop2DOverlayViewInfo.m_nRenderWidth = m_ivec2DesktopWindowSize.x;
	m_sviDesktop2DOverlayViewInfo.m_nRenderHeight = m_ivec2DesktopWindowSize.y;

	m_sviDesktop2DOverlayViewInfo.view = glm::mat4();
	m_sviDesktop2DOverlayViewInfo.projection = glm::ortho(0.f, static_cast<float>(m_sviDesktop2DOverlayViewInfo.m_nRenderWidth), 0.f, static_cast<float>(m_sviDesktop2DOverlayViewInfo.m_nRenderHeight), -1.f, 1.f);


	m_sviDesktop3DViewInfo.m_nRenderWidth = m_ivec2DesktopWindowSize.x;
	m_sviDesktop3DViewInfo.m_nRenderHeight = m_ivec2DesktopWindowSize.y;
	m_sviDesktop3DViewInfo.projection = glm::perspective(glm::radians(g_fDesktopWindowFOV), (float)m_ivec2DesktopWindowSize.x / (float)m_ivec2DesktopWindowSize.y, g_fNearClip, g_fFarClip);

	m_pDesktopFramebuffer = new Renderer::FramebufferDesc();

	if (!Renderer::getInstance().CreateFrameBuffer(m_sviDesktop2DOverlayViewInfo.m_nRenderWidth, m_sviDesktop2DOverlayViewInfo.m_nRenderHeight, *m_pDesktopFramebuffer))
		dprintf("Could not create desktop view framebuffer!\n");	
}
