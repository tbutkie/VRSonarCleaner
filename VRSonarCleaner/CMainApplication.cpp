#include "CMainApplication.h"
#include "DebugDrawer.h"
#include "InfoBoxManager.h"

#include "BehaviorManager.h"
#include "StudyTutorialBehavior.h"
#include "DemoBehavior.h"
#include "SelectAreaBehavior.h"
#include "GrabDataVolumeBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include "CurateStudyDataBehavior.h"
#include "RunStudyBehavior.h"
#include "StudyIntroBehavior.h"
#include "ScaleTutorial.h"
#include "StudyEditTutorial.h"
#include "DesktopCleanBehavior.h"
#include "arcball.h"
#include "LassoTool.h"
#include "SnellenTest.h"

#include "HolodeckBackground.h"

#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <limits>


glm::vec3						g_vec3RoomSize(4.f, 3.f, 3.f);

float							g_fNearClip = 0.001f;
float							g_fFarClip = 10000.f;
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
// Purpose:
//-----------------------------------------------------------------------------
void dprintf(const char *fmt, ...)
{
	va_list args;
	char buffer[2048];

	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);

#ifdef DEBUG
	printf("%s", buffer);
#endif // DEBUG

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
	, m_bMiddleMouseDown(false)
	, m_pVRCompanionWindow(NULL)
	, m_pGLContext(NULL)
	, m_pDesktopWindow(NULL)
	, m_pDesktopWindowCursor(NULL)
	, m_pHMD(NULL)
{	
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

	//if (m_bUseVR)
	//{
	//	vr::VRChaperone()->GetPlayAreaSize(&g_vec3RoomSize.x, &g_vec3RoomSize.z);
	//}

	if (m_bSonarCleaning)
	{
		glm::vec3 wallSize((g_vec3RoomSize.x * 0.9f), (g_vec3RoomSize.y * 0.8f), 0.8f);
		glm::quat wallOrientation(glm::angleAxis(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f)));
		glm::vec3 wallPosition(0.f, (g_vec3RoomSize.y * 0.5f) + (g_vec3RoomSize.y * 0.09f), (g_vec3RoomSize.z * 0.5f) - 0.42f);
		
		m_pWallVolume = new DataVolume(wallPosition, wallOrientation, wallSize);

		glm::vec3 tablePosition = glm::vec3(0.f, 1.f, 0.f);
		glm::quat tableOrientation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
		glm::vec3 tableSize = glm::vec3(2.25f, 2.25f, 0.75f);

		m_pTableVolume = new DataVolume(tablePosition, tableOrientation, tableSize);

		m_Camera.pos = tablePosition + glm::vec3(0.f, 0.f, 1.f) * 3.f;
		m_Camera.lookat = tablePosition;

		m_pColorScalerTPU = new ColorScaler();
		//m_pColorScalerTPU->setColorMode(ColorScaler::Mode::ColorScale_BiValue);
		//m_pColorScalerTPU->setBiValueColorMap(ColorScaler::ColorMap_BiValued::Custom);
		m_pColorScalerTPU->setColorMode(ColorScaler::Mode::ColorScale);
		m_pColorScalerTPU->setColorMap(ColorScaler::ColorMap::Rainbow);

		//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_1085.txt"));
		//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-267_267_528_1324.txt"));
		//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1516.txt"));
		//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1508.txt"));
		//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-149_149_000_1500.txt"));
		//m_vpClouds.push_back(new SonarPointCloud(m_pColorScalerTPU, "H12676_TJ_3101_Reson7125_SV2_400khz_2014_2014-148_148_000_2022.txt"));

		for (auto const &cloud : m_vpClouds)
		{
			m_pTableVolume->add(cloud);
			m_pWallVolume->add(cloud);
		}

		m_vpDataVolumes.push_back(m_pTableVolume);
		m_vpDataVolumes.push_back(m_pWallVolume);

		refreshColorScale(m_pColorScalerTPU, m_vpClouds);

		if (m_bUseDesktop)
		{
			glm::ivec4 vp(0, 0, m_ivec2DesktopWindowSize.x, m_ivec2DesktopWindowSize.y);

			DesktopCleanBehavior *tmp = new DesktopCleanBehavior(m_pTableVolume, &m_sviDesktop3DViewInfo, vp);
			BehaviorManager::getInstance().addBehavior("desktop_edit", tmp);
			tmp->init();
		}
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
		m_pFlowVolume = new FlowVolume(tempFG);

		if (m_bGreatBayModel)
			m_pFlowVolume->setDimensions(glm::vec3(fmin(g_vec3RoomSize.x, g_vec3RoomSize.z) * 0.5f, fmin(g_vec3RoomSize.x, g_vec3RoomSize.z) * 0.5f, g_vec3RoomSize.y * 0.05f));

		m_Camera.pos = m_pFlowVolume->getPosition() + glm::vec3(0.f, 0.f, 1.f) * 3.f;
		m_Camera.lookat = m_pFlowVolume->getPosition();
	}

	m_sviDesktop3DViewInfo.view = glm::lookAt(m_Camera.pos, m_Camera.lookat, m_Camera.up);

	BehaviorManager::getInstance().init();

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

	m_Camera.pos = glm::vec3(0.f, 0.f, 1.f);
	m_Camera.up = glm::vec3(0.f, 1.f, 0.f);

	return true;
}

bool CMainApplication::shutdownVR()
{
	SDL_DestroyWindow(m_pVRCompanionWindow);
	m_pVRCompanionWindow = NULL;

	if (m_pTDM)
	{
		delete m_pTDM;
		m_pTDM = NULL;
	}

	vr::VR_Shutdown();
	m_pHMD = NULL;

	m_bUseVR = false;

	return true;
}

bool CMainApplication::shutdownDesktop()
{
	SDL_DestroyWindow(m_pDesktopWindow);
	m_pDesktopWindow = NULL;

	m_bUseDesktop = false;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::Shutdown()
{
	BehaviorManager::getInstance().shutdown();

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

	DebugDrawer::getInstance().shutdown();
	Renderer::getInstance().shutdown();

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
		ArcBall *arcball = static_cast<ArcBall*>(BehaviorManager::getInstance().getBehavior("arcball"));
		LassoTool *lasso = static_cast<LassoTool*>(BehaviorManager::getInstance().getBehavior("lasso"));

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

			if (sdlEvent.key.keysym.sym == SDLK_f)
			{
				using namespace std::experimental::filesystem::v1;
				auto herePath = current_path();
				std::cout << "Current directory: " << herePath << std::endl;
				for (directory_iterator it(herePath); it != directory_iterator(); ++it)
					if(is_regular_file(*it))
						std::cout << (*it) << std::endl;
			}

			if (sdlEvent.key.keysym.sym == SDLK_l)
			{
				if (m_bUseVR)
				{
					if (!BehaviorManager::getInstance().getBehavior("harvestpoints"))
						BehaviorManager::getInstance().addBehavior("harvestpoints", new SelectAreaBehavior(m_pTDM, m_pWallVolume, m_pTableVolume));
					if (!BehaviorManager::getInstance().getBehavior("grab"))
						BehaviorManager::getInstance().addBehavior("grab", new GrabDataVolumeBehavior(m_pTDM, m_pTableVolume));
					if (!BehaviorManager::getInstance().getBehavior("scale"))
						BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pTableVolume));
				}
				else m_pWallVolume->setVisible(false);

				using namespace std::experimental::filesystem::v1;

				//path dataset("south_santa_rosa");
				//path dataset("santa_cruz_south");
				path dataset("santa_cruz_basin");

				auto basePath = current_path().append(path("data"));
				std::cout << "Base study data directory: " << basePath << std::endl;

				auto acceptsPath = path(basePath).append(path("accept"));
				auto rejectsPath = path(basePath).append(path("reject"));

				std::vector<SonarPointCloud*> tmpPointCloudCollection;

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
							tmpPointCloudCollection.push_back(tmp);
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
				//			tmpPointCloudCollection.push_back(tmp);
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
			if (sdlEvent.key.keysym.sym == SDLK_b)
			{
				m_pTableVolume->setVisible(false);
				m_pWallVolume->setVisible(false);
				BehaviorManager::getInstance().clearBehaviors();
				SnellenTest *st = new SnellenTest(m_pTDM);
				BehaviorManager::getInstance().addBehavior("test", st);
				st->init();
			}
			if (sdlEvent.key.keysym.sym == SDLK_t)
			{
				BehaviorManager::getInstance().addBehavior("Tutorial", new StudyTutorialBehavior(m_pTDM, m_pTableVolume, m_pWallVolume));
			}
			if (sdlEvent.key.keysym.sym == SDLK_y)
			{
				m_bDemoMode = true;
				BehaviorManager::getInstance().clearBehaviors();
				BehaviorManager::getInstance().addBehavior("Demo", new DemoBehavior(m_pTDM, m_pTableVolume, m_pWallVolume));
			}
			if (sdlEvent.key.keysym.sym == SDLK_u)
			{
				BehaviorManager::getInstance().addBehavior("GetStudyData", new CurateStudyDataBehavior(m_pTDM, m_pTableVolume, m_pWallVolume));
			}
			if (sdlEvent.key.keysym.sym == SDLK_i)
			{
				m_pWallVolume->setVisible(false);
				m_pTableVolume->setVisible(false);
				BehaviorManager::getInstance().clearBehaviors();
				RunStudyBehavior *rsb = new RunStudyBehavior(m_pTDM);
				BehaviorManager::getInstance().addBehavior("Study", rsb);
				rsb->init();
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

						m_sviDesktop3DViewInfo.view = glm::lookAt(m_Camera.pos, m_Camera.lookat, m_Camera.up);

						arcball->reset();
					}
				}

				if (sdlEvent.key.keysym.sym == SDLK_g)
				{
					printf("Pressed g, generating fake test cloud\n");
					//m_pClouds->generateFakeTestCloud(150, 150, 25, 40000);
					//m_pColorScalerTPU->resetBiValueScaleMinMax(m_pClouds->getMinDepthTPU(), m_pClouds->getMaxDepthTPU(), m_pClouds->getMinPositionalTPU(), m_pClouds->getMaxPositionalTPU());
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
					m_pFlowVolume->resetPositionAndOrientation();
				}

				if (sdlEvent.key.keysym.sym == SDLK_1)
				{
					if (m_bUseVR)
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
					if (m_bUseVR)
					{
						m_pFlowVolume->setDimensions(glm::vec3(fmin(g_vec3RoomSize.x, g_vec3RoomSize.z) * 0.9f, fmin(g_vec3RoomSize.x, g_vec3RoomSize.z) * 0.9f, g_vec3RoomSize.y * 0.1f));
						m_pFlowVolume->setPosition(glm::vec3(0.f, g_vec3RoomSize.y * 0.1f * 0.5f, 0.f));
						m_pFlowVolume->setOrientation(glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)));
					}
				}
			}

			if ((sdlEvent.key.keysym.mod & KMOD_LCTRL) && sdlEvent.key.keysym.sym == SDLK_f)
			{
				std::cout << "Frame Time: " << m_msFrameTime.count() << "ms" << std::endl;
				std::cout << "\t" << m_msInputHandleTime.count() << "ms\tInput Handling" << std::endl;
				std::cout << "\t" << m_msUpdateTime.count() << "ms\tState Update" << std::endl;
				std::cout << "\t" << m_msDrawTime.count() << "ms\tScene Drawing" << std::endl;
				std::cout << "\t" << m_msRenderTime.count() << "ms\tRendering" << std::endl;
				if (m_bUseVR)
					std::cout << "\t" << m_msVRUpdateTime.count() << "ms\tVR System Update" << std::endl;
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
						arcball->beginDrag(glm::vec2(sdlEvent.button.x, m_ivec2DesktopWindowSize.y - sdlEvent.button.y));

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
						lasso->start(sdlEvent.button.x, m_ivec2DesktopWindowSize.y - sdlEvent.button.y);
				}
				if (sdlEvent.button.button == SDL_BUTTON_MIDDLE)
				{
					if (lasso && m_bRightMouseDown)
					{
						lasso->end();
					}
					m_bMiddleMouseDown = true;

					if (arcball)
						arcball->translate(glm::vec2(sdlEvent.button.x, m_ivec2DesktopWindowSize.y - sdlEvent.button.y));
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
						arcball->drag(glm::vec2(sdlEvent.button.x, m_ivec2DesktopWindowSize.y - sdlEvent.button.y));
				}
				if (m_bRightMouseDown && !m_bLeftMouseDown)
				{
					if (lasso)
						lasso->move(sdlEvent.button.x, m_ivec2DesktopWindowSize.y - sdlEvent.button.y);
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

				m_sviDesktop3DViewInfo.view = glm::lookAt(m_Camera.pos, m_Camera.lookat, m_Camera.up);

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
	
		
	return bRet;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RunMainLoop()
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

	////doesn't help here either
	fclose(stdout);
	FreeConsole();
	
	SDL_StopTextInput();
}


void CMainApplication::update()
{
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

	if (m_bFlowVis)
	{
		m_pFlowVolume->preRenderUpdates();
	}
}

void CMainApplication::drawScene()
{
	BehaviorManager::getInstance().draw();

	if (m_bSonarCleaning)
	{
		if (m_bUseVR)
		{
			if (m_bShowDesktopFrustum)
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

		if (m_bUseDesktop)
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

		for (auto &dv : m_vpDataVolumes)
		{
			if (!dv->isVisible()) continue;

			glm::mat4 trans;
			
			if (m_bUseDesktop)
				trans = glm::inverse(m_sviDesktop3DViewInfo.view);

			if (m_bUseVR)
				trans = m_pTDM->getHMDToWorldTransform();

			dv->drawVolumeBacking(trans, glm::vec4(0.15f, 0.21f, 0.31f, 1.f), 1.f);
			dv->drawBBox(glm::vec4(0.f, 0.f, 0.f, 1.f), 0.f);
			//dv->drawAxes();

			//draw table
			Renderer::RendererSubmission rs;
			rs.glPrimitiveType = GL_POINTS;
			rs.shaderName = "flat";
			rs.indexType = GL_UNSIGNED_INT;

			for (auto &cloud : dv->getDatasets())
			{
				rs.VAO = dv == m_pWallVolume ? static_cast<SonarPointCloud*>(cloud)->getPreviewVAO() : static_cast<SonarPointCloud*>(cloud)->getVAO();
				rs.vertCount = dv == m_pWallVolume ? static_cast<SonarPointCloud*>(cloud)->getPreviewPointCount() : static_cast<SonarPointCloud*>(cloud)->getPointCount();
				rs.modelToWorldTransform = dv->getCurrentDataTransform(cloud);
				Renderer::getInstance().addToDynamicRenderQueue(rs);
			}
		}
	}

	if (m_bFlowVis)
	{
		m_pFlowVolume->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), glm::vec4(0.15f, 0.21f, 0.31f, 1.f), 1.f);
		m_pFlowVolume->drawBBox(glm::vec4(0.f, 0.f, 0.f, 1.f), 0.f);

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

void CMainApplication::render()
{
	if (m_bUseVR)
	{
		SDL_GL_MakeCurrent(m_pVRCompanionWindow, m_pGLContext);

		// Update eye positions using current HMD position
		m_sviLeftEyeInfo.view = m_sviLeftEyeInfo.viewTransform * m_pTDM->getWorldToHMDTransform();
		m_sviRightEyeInfo.view = m_sviRightEyeInfo.viewTransform * m_pTDM->getWorldToHMDTransform();

		Renderer::getInstance().sortTransparentObjects(glm::vec3(m_pTDM->getHMDToWorldTransform()[3]));

		Renderer::getInstance().RenderFrame(&m_sviLeftEyeInfo, NULL, m_pLeftEyeFramebuffer);
		Renderer::getInstance().RenderFrame(&m_sviRightEyeInfo, NULL, m_pRightEyeFramebuffer);

		Renderer::getInstance().RenderFullscreenTexture(m_nVRCompanionWindowWidth, m_nVRCompanionWindowHeight, m_pLeftEyeFramebuffer->m_nResolveTextureId, true);

		vr::Texture_t leftEyeTexture = { (void*)m_pLeftEyeFramebuffer->m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::Texture_t rightEyeTexture = { (void*)m_pRightEyeFramebuffer->m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

		//vr::VRCompositor()->PostPresentHandoff();
		//std::cout << "Rendering Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
		
		//glFinish();
		
		SDL_GL_SwapWindow(m_pVRCompanionWindow);

		//glClearColor(0, 0, 0, 1);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glFlush();
		//glFinish();
	}

	if (m_bUseDesktop)
	{
		SDL_GL_MakeCurrent(m_pDesktopWindow, m_pGLContext);

		Renderer::getInstance().sortTransparentObjects(glm::vec3(glm::inverse(m_sviDesktop3DViewInfo.view)[3]));

		Renderer::getInstance().RenderFrame(&m_sviDesktop3DViewInfo, &m_sviDesktop2DOverlayViewInfo, m_pDesktopFramebuffer);

		Renderer::getInstance().RenderFullscreenTexture(m_ivec2DesktopWindowSize.x, m_ivec2DesktopWindowSize.y, m_pDesktopFramebuffer->m_nResolveTextureId);

		SDL_GL_SwapWindow(m_pDesktopWindow);
	}

	Renderer::getInstance().clearDynamicRenderQueue();
	Renderer::getInstance().clearUIRenderQueue();
	DebugDrawer::getInstance().flushLines();
}

void CMainApplication::refreshColorScale(ColorScaler * colorScaler, std::vector<SonarPointCloud*> clouds)
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
