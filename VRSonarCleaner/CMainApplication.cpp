#include "CMainApplication.h"
#include "ShaderUtils.h"
#include "DebugDrawer.h"
#include "InfoBoxManager.h"

#include <fstream>
#include <sstream>
#include <string>

#include <ctime>

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
CMainApplication::CMainApplication(int argc, char *argv[], int Mode)
	: m_pWindow(NULL)
	, m_pContext(NULL)
	, m_nWindowWidth(1280)
	, m_nWindowHeight(720)
	, m_pHMD(NULL)
	, m_bVerbose(false)
	, m_bPerf(false)
{
#if _DEBUG
	m_bDebugOpenGL = true;
#else
	m_bDebugOpenGL = false;
#endif

	mode = Mode;

	for (int i = 1; i < argc; i++)
	{
		if (!stricmp(argv[i], "-gldebug"))
		{
			m_bDebugOpenGL = true;
		}
		else if (!stricmp(argv[i], "-verbose"))
		{
			m_bVerbose = true;
		}
		//else if (!stricmp(argv[i], "-novblank"))
		//{
		//	m_bVblank = false;
		//}
		//else if (!stricmp(argv[i], "-noglfinishhack"))
		//{
		//	m_bGlFinishHack = false;
		//}
		else if (!stricmp(argv[i], "-noprintf"))
		{
			g_bPrintf = false;
		}
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
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::BInit()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
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

	m_pTDM = new TrackedDeviceManager(m_pHMD);

	int nWindowPosX = 10;// 700;
	int nWindowPosY = 30;// 100;
	m_nWindowWidth = 1660;// 1280;
	m_nWindowHeight = 980;// 720;
	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY ); //UNCOMMENT AND COMMENT LINE BELOW TO ENABLE FULL OPENGL COMMANDS
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	if (m_bDebugOpenGL)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	m_pWindow = SDL_CreateWindow("hellovr_sdl", nWindowPosX, nWindowPosY, m_nWindowWidth, m_nWindowHeight, unWindowFlags);
	if (m_pWindow == NULL)
	{
		printf("%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	m_pContext = SDL_GL_CreateContext(m_pWindow);
	if (m_pContext == NULL)
	{
		printf("%s - OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	glewExperimental = GL_TRUE;
	GLenum nGlewError = glewInit();
	if (nGlewError != GLEW_OK)
	{
		printf("%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString(nGlewError));
		return false;
	}
	glGetError(); // to clear the error caused deep in GLEW

	if (mode == 0)
	{
		std::string strWindowTitle = "VR Sonar Cleaner | CCOM VisLab";
		SDL_SetWindowTitle(m_pWindow, strWindowTitle.c_str());
	}
	else
	{
		std::string strWindowTitle = "VR Flow 4D | CCOM VisLab";
		SDL_SetWindowTitle(m_pWindow, strWindowTitle.c_str());
	}

	// 		m_MillisecondsTimer.start(1, this);
	// 		m_SecondsTimer.start(1000, this);

	if (!BInitGL())
	{
		printf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
		return false;
	}

	if (!BInitCompositor())
	{
		printf("%s - Failed to initialize VR Compositor!\n", __FUNCTION__);
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
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
bool CMainApplication::BInitGL()
{
	if (m_bDebugOpenGL)
	{
		glDebugMessageCallback((GLDEBUGPROC)DebugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	if (!m_pTDM->BInit())
	{
		dprintf("Error initializing TrackedDeviceManager\n");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", "Could not get render model interface", NULL);
	}

	InfoBoxManager::getInstance().BInit(m_pTDM);
	m_pTDM->attach(&InfoBoxManager::getInstance());
	
	m_pLighting = new LightingSystem();
	m_pLighting->addDirectLight();

	if (!Renderer::getInstance().init(m_pHMD, m_pTDM, m_pLighting))
		return false;

	if (mode == 0)
	{
		cleaningRoom = new CleaningRoom();
	}
	else if (mode == 1)
	{
		flowRoom = new FlowRoom();
		m_pTDM->attach(flowRoom);
	}


	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::BInitCompositor()
{
	vr::EVRInitError peError = vr::VRInitError_None;

	if (!vr::VRCompositor())
	{
		printf("Compositor initialization failed. See log file for details\n");
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::Shutdown()
{
	if (m_pHMD)
	{
		vr::VR_Shutdown();
		m_pHMD = NULL;
	}

	if (m_pTDM)
		delete m_pTDM;

	if (m_pContext)
	{
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(nullptr, nullptr);
	}

	
	fclose(stdout);
	FreeConsole();

	if (m_pWindow)
	{
		SDL_DestroyWindow(m_pWindow);
		m_pWindow = NULL;
	}

	SDL_Quit();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::HandleInput()
{
	SDL_Event sdlEvent;
	bool bRet = false;

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
			
			if (mode == 0) //cleaning
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
					clouds->resetMarksInAllClouds();
					cleaningRoom->resetVolumes();
				}

				if (sdlEvent.key.keysym.sym == SDLK_g)
				{
					printf("Pressed g, generating fake test cloud\n");
					clouds->clearAllClouds();
					clouds->generateFakeTestCloud(150, 150, 25, 40000);
					clouds->calculateCloudBoundsAndAlign();
					cleaningRoom->recalcVolumeBounds();
				}
			}//end if mode==0
			else if (mode == 1) //flow
			{
				if (sdlEvent.key.keysym.sym == SDLK_r)
				{
					printf("Pressed r, resetting something...\n");
					flowRoom->reset();
				}

				if (sdlEvent.key.keysym.sym == SDLK_f)
					printf("FPS: %u\n", m_uiCurrentFPS);
			}
			
			if (sdlEvent.key.keysym.sym == SDLK_l)
			{
				/*
				printf("Controller Locations:\n");
				// Process SteamVR controller state
				for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
				{
					printf("Controller %d\n", unDevice);										
				}
				*/
			}
		}
	}
	
	m_pTDM->handleEvents();
	
	return bRet;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RunMainLoop()
{
	bool bQuit = false;

	SDL_StartTextInput();
	SDL_ShowCursor(SDL_DISABLE);

	float fps_interval = 1.0; // sec
	Uint32 fps_lasttime = SDL_GetTicks();
	m_uiCurrentFPS = 0u;
	Uint32 fps_frames = 0;

	std::clock_t start;

	while (!bQuit)
	{
		start = std::clock();

		//std::cout << "--------------------------------------------------" << std::endl;

		bQuit = HandleInput();

		m_pTDM->updateTrackedDevices();

		if (mode == 0)
		{
			glm::mat4 currentCursorPose;
			glm::mat4 lastCursorPose;
			float cursorRadius;

			// if editing controller not available or pose isn't valid, abort
			if (!m_pTDM->getCleaningCursorData(currentCursorPose, lastCursorPose, cursorRadius))
				return;

			// check point cloud for hits
			//if (cleaningRoom->checkCleaningTable(currentCursorPose, lastCursorPose, cursorRadius, 10))
			if (cleaningRoom->editCleaningTable(currentCursorPose, lastCursorPose, cursorRadius, m_pTDM->cleaningModeActive()))
				m_pTDM->cleaningHit();

			glm::mat4 ctrlPose;
			if (m_pTDM->getManipulationData(ctrlPose))
				cleaningRoom->gripCleaningTable(ctrlPose);
			else
				cleaningRoom->releaseCleaningTable();

			cleaningRoom->draw(); // currently draws to debug buffer
		}

		if (mode == 1)
		{
			//grip rotate if needed
			glm::mat4 ctrlPose;
			if (m_pTDM->getManipulationData(ctrlPose))
				flowRoom->gripModel(ctrlPose);
			else
				flowRoom->releaseModel();

			flowRoom->preRenderUpdates();

			flowRoom->draw(); // currently draws to debug buffer
		}

		m_pLighting->updateLightingUniforms();		
		
		//std::cout << "FlowRoom Update Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
		//start = std::clock();

		Renderer::getInstance().RenderFrame(m_pWindow, m_pTDM->getHMDPose());

		Renderer::getInstance().resetRenderModelInstances();

		//std::cout << "Rendering Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;

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
	std::vector<glm::vec3> points = clouds->getCloud(0)->getPointPositions();

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
		outFile << points[i].x << "," << points[i].y << "," << points[i].z << "," << (clouds->getCloud(0)->getPointMark(i) == 1 ? "1" : "0") << std::endl;
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
