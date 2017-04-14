#include "CMainApplication.h"
#include "DebugDrawer.h"
#include "InfoBoxManager.h"
#include "CloudCollection.h"

#include "ManipulateDataVolumeBehavior.h"
#include "FlowProbe.h"
#include "AdvectionProbe.h"
#include "HolodeckBackground.h"

#include <fstream>
#include <sstream>
#include <string>

#include <ctime>

extern CloudCollection *clouds;

HolodeckBackground*				g_pHolodeck = NULL;
glm::vec3						g_vec3RoomSize(10.f, 4.f, 6.f);
ManipulateDataVolumeBehavior*	g_pManipulateDataVolumeBehavior = NULL;
FlowProbe*						g_pFlowProbeBehavior = NULL;
AdvectionProbe*					g_pAdvectionProbeBehavior = NULL;

std::vector<BehaviorBase*> g_vpBehaviors;

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
	, m_pHMD(NULL)
	, m_bVerbose(false)
	, m_bPerf(false)
	, m_fPtHighlightAmt(1.f)
	, m_LastTime(std::chrono::high_resolution_clock::now())
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

	int numDisplays = SDL_GetNumVideoDisplays();

	SDL_Rect displayBounds;

	if (numDisplays > 1)
		SDL_GetDisplayBounds(1, &displayBounds);
	else
		SDL_GetDisplayBounds(0, &displayBounds);

	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY ); //UNCOMMENT AND COMMENT LINE BELOW TO ENABLE FULL OPENGL COMMANDS
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	if (m_bDebugOpenGL)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	m_pWindow = SDL_CreateWindow("hellovr_sdl", displayBounds.x, displayBounds.y, displayBounds.w, displayBounds.h, unWindowFlags);
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

	if (!Renderer::getInstance().init(m_pHMD, m_pTDM))
		return false;

	g_pHolodeck = new HolodeckBackground(g_vec3RoomSize, 0.25f);

	if (mode == 0)
	{
		//cleaningRoom = new CleaningRoom(g_vec3RoomSize);

		glm::vec3 wallSize((g_vec3RoomSize.x * 0.9f), 0.8f, (g_vec3RoomSize.y * 0.8f));
		glm::vec3 wallPosition(0.f, (g_vec3RoomSize.y * 0.5f) + (g_vec3RoomSize.y * 0.09f), (g_vec3RoomSize.z * 0.5f) - 0.42f);

		glm::vec3 wallMinCoords(clouds->getXMin(), clouds->getMinDepth(), clouds->getYMin());
		glm::vec3 wallMaxCoords(clouds->getXMax(), clouds->getMaxDepth(), clouds->getYMax());

		glm::vec3 tablePosition(0.f, 1.1f, 0.f);
		glm::vec3 tableSize(2.25f, 0.75f, 2.25f);

		glm::vec3 tableMinCoords(clouds->getCloud(0)->getXMin(), clouds->getCloud(0)->getMinDepth(), clouds->getCloud(0)->getYMin());
		glm::vec3 tableMaxCoords(clouds->getCloud(0)->getXMax(), clouds->getCloud(0)->getMaxDepth(), clouds->getCloud(0)->getYMax());

		tableVolume = new DataVolume(tablePosition, 0, tableSize, tableMinCoords, tableMaxCoords);
		wallVolume = new DataVolume(wallPosition, 1, wallSize, wallMinCoords, wallMaxCoords);

	}
	else if (mode == 1)
	{
		FlowGrid *tempFG = new FlowGrid("test.fg");
		tempFG->setCoordinateScaler(new CoordinateScaler());
		flowVolume = new FlowVolume(tempFG);
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

			if ((sdlEvent.key.keysym.mod & KMOD_LCTRL) && sdlEvent.key.keysym.sym == SDLK_w)
			{
				Renderer::getInstance().toggleWireframe();
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
					//cleaningRoom->resetVolumes();
					wallVolume->resetPositionAndOrientation();
					tableVolume->resetPositionAndOrientation();
				}

				if (sdlEvent.key.keysym.sym == SDLK_g)
				{
					printf("Pressed g, generating fake test cloud\n");
					clouds->clearAllClouds();
					clouds->generateFakeTestCloud(150, 150, 25, 40000);
					clouds->calculateCloudBoundsAndAlign();
					//cleaningRoom->recalcVolumeBounds();
					glm::vec3 tableMinCoords(clouds->getCloud(0)->getXMin(), clouds->getCloud(0)->getMinDepth(), clouds->getCloud(0)->getYMin());
					glm::vec3 tableMaxCoords(clouds->getCloud(0)->getXMax(), clouds->getCloud(0)->getMaxDepth(), clouds->getCloud(0)->getYMax());

					glm::vec3 wallMinCoords(clouds->getXMin(), clouds->getMinDepth(), clouds->getYMin());
					glm::vec3 wallMaxCoords(clouds->getXMax(), clouds->getMaxDepth(), clouds->getYMax());

					tableVolume->setInnerCoords(tableMinCoords, tableMaxCoords);
					wallVolume->setInnerCoords(wallMinCoords, wallMaxCoords);
				}
			}//end if mode==0
			else if (mode == 1) //flow
			{
				if (sdlEvent.key.keysym.sym == SDLK_r)
				{
					printf("Pressed r, resetting something...\n");
					flowVolume->resetPositionAndOrientation();
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

		if (mode == 1)
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

		// Attach grip & scale behavior when both controllers available
		if (m_pTDM->getSecondaryController() && m_pTDM->getPrimaryController() && !g_pManipulateDataVolumeBehavior)
		{
			g_pManipulateDataVolumeBehavior = new ManipulateDataVolumeBehavior(m_pTDM->getSecondaryController(), m_pTDM->getPrimaryController(), (mode == 0 ? tableVolume : flowVolume));
			g_vpBehaviors.push_back(g_pManipulateDataVolumeBehavior);
		}

		m_pTDM->update();

		for (auto const &b : g_vpBehaviors)
			b->update();

		for (auto const &b : g_vpBehaviors)
			b->draw();

		g_pHolodeck->draw();

		if (mode == 0)
		{
			glm::mat4 currentCursorPose;
			glm::mat4 lastCursorPose;
			float cursorRadius;

			// if editing controller not available or pose isn't valid, abort
			if (m_pTDM->getCleaningCursorData(currentCursorPose, lastCursorPose, cursorRadius))
			{
				// check point cloud for hits
				//if (cleaningRoom->checkCleaningTable(currentCursorPose, lastCursorPose, cursorRadius, 10))
				if (editCleaningTable(currentCursorPose, lastCursorPose, cursorRadius, m_pTDM->cleaningModeActive()))
					m_pTDM->cleaningHit();
			}

			//cleaningRoom->draw(); // currently draws to debug buffer
			//draw debug
			wallVolume->drawBBox();
			wallVolume->drawBacking();
			wallVolume->drawAxes();
			tableVolume->drawBBox();
			tableVolume->drawBacking();
			tableVolume->drawAxes();


			//draw table
			DebugDrawer::getInstance().setTransform(tableVolume->getCurrentDataTransform());
			clouds->getCloud(0)->draw();

			//draw wall
			DebugDrawer::getInstance().setTransform(wallVolume->getCurrentDataTransform());
			clouds->drawAllClouds();
		}

		if (mode == 1)
		{
			flowVolume->preRenderUpdates();

			flowVolume->draw(); // currently draws to debug buffer
		}
		
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


// This code taken from http://www.flipcode.com/archives/Fast_Point-In-Cylinder_Test.shtml
// with credit to Greg James @ Nvidia
float cylTest(const glm::vec4 & pt1, const glm::vec4 & pt2, float lengthsq, float radius_sq, const glm::vec3 & testpt)
{
	float dx, dy, dz;	// vector d  from line segment point 1 to point 2
	float pdx, pdy, pdz;	// vector pd from point 1 to test point
	float dot, dsq;

	dx = pt2.x - pt1.x;	// translate so pt1 is origin.  Make vector from
	dy = pt2.y - pt1.y;     // pt1 to pt2.  Need for this is easily eliminated
	dz = pt2.z - pt1.z;

	pdx = testpt.x - pt1.x;		// vector from pt1 to test point.
	pdy = testpt.y - pt1.y;
	pdz = testpt.z - pt1.z;

	// Dot the d and pd vectors to see if point lies behind the 
	// cylinder cap at pt1.x, pt1.y, pt1.z

	dot = pdx * dx + pdy * dy + pdz * dz;

	// If dot is less than zero the point is behind the pt1 cap.
	// If greater than the cylinder axis line segment length squared
	// then the point is outside the other end cap at pt2.

	if (dot < 0.f || dot > lengthsq)
		return(-1.f);
	else
	{
		dsq = (pdx*pdx + pdy*pdy + pdz*pdz) - dot*dot / lengthsq;

		if (dsq > radius_sq)
			return(-1.f);
		else
			return(dsq);		// return distance squared to axis
	}
}


bool CMainApplication::editCleaningTable(const glm::mat4 & currentCursorPose, const glm::mat4 & lastCursorPose, float radius, bool clearPoints)
{
	glm::mat4 mat4CurrentVolumeXform = tableVolume->getCurrentDataTransform();
	glm::mat4 mat4LastVolumeXform = tableVolume->getLastDataTransform();

	if (mat4LastVolumeXform == glm::mat4())
		mat4LastVolumeXform = mat4CurrentVolumeXform;

	glm::vec3 vec3CurrentCursorPos = glm::vec3(currentCursorPose[3]);
	glm::vec3 vec3LastCursorPos = glm::vec3((mat4CurrentVolumeXform * glm::inverse(mat4LastVolumeXform) * lastCursorPose)[3]);

	bool performCylTest = true;
	if (vec3CurrentCursorPos == vec3LastCursorPos) performCylTest = false;

	float cyl_len_sq = (vec3CurrentCursorPos.x - vec3LastCursorPos.x) * (vec3CurrentCursorPos.x - vec3LastCursorPos.x) +
		(vec3CurrentCursorPos.y - vec3LastCursorPos.y) * (vec3CurrentCursorPos.y - vec3LastCursorPos.y) +
		(vec3CurrentCursorPos.z - vec3LastCursorPos.z) * (vec3CurrentCursorPos.z - vec3LastCursorPos.z);

	// CLOCK UPDATE
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_LastTime);
	m_LastTime = std::chrono::high_resolution_clock::now();

	float blink_rate_ms = 250.f;

	float delta = static_cast<float>(elapsed_ms.count()) / blink_rate_ms;
	m_fPtHighlightAmt = fmodf(m_fPtHighlightAmt + delta, 1.f);

	// POINTS CHECK
	bool anyHits = false;
	bool pointsRefresh = false;

	std::vector<glm::vec3> points = clouds->getCloud(0)->getPointPositions();

	for (size_t i = 0ull; i < points.size(); ++i)
	{
		//skip already marked points
		if (clouds->getCloud(0)->getPointMark(i) == 1)
			continue;

		glm::vec3 thisPt = glm::vec3(mat4CurrentVolumeXform * glm::vec4(points[i].x, points[i].y, points[i].z, 1.f));

		// fast point-in-AABB failure test
		if (thisPt.x < (std::min)(vec3CurrentCursorPos.x, vec3LastCursorPos.x) - radius ||
			thisPt.x >(std::max)(vec3CurrentCursorPos.x, vec3LastCursorPos.x) + radius ||
			thisPt.y < (std::min)(vec3CurrentCursorPos.y, vec3LastCursorPos.y) - radius ||
			thisPt.y >(std::max)(vec3CurrentCursorPos.y, vec3LastCursorPos.y) + radius ||
			thisPt.z < (std::min)(vec3CurrentCursorPos.z, vec3LastCursorPos.z) - radius ||
			thisPt.z >(std::max)(vec3CurrentCursorPos.z, vec3LastCursorPos.z) + radius)
		{
			if (clouds->getCloud(0)->getPointMark(i) != 0)
			{
				clouds->getCloud(0)->markPoint(i, 0);
				pointsRefresh = true;
			}
			continue;
		}

		float radius_sq = radius * radius;
		float current_dist_sq = (thisPt.x - vec3CurrentCursorPos.x) * (thisPt.x - vec3CurrentCursorPos.x) +
			(thisPt.y - vec3CurrentCursorPos.y) * (thisPt.y - vec3CurrentCursorPos.y) +
			(thisPt.z - vec3CurrentCursorPos.z) * (thisPt.z - vec3CurrentCursorPos.z);

		if (current_dist_sq <= radius_sq ||
			(performCylTest && cylTest(glm::vec4(vec3CurrentCursorPos.x, vec3CurrentCursorPos.y, vec3CurrentCursorPos.z, 1.f),
				glm::vec4(vec3LastCursorPos.x, vec3LastCursorPos.y, vec3LastCursorPos.z, 1.f),
				cyl_len_sq,
				radius_sq,
				glm::vec3(thisPt.x, thisPt.y, thisPt.z)) >= 0)
			)
		{
			if (clearPoints)
			{
				anyHits = true;
				clouds->getCloud(0)->markPoint(i, 1);
			}
			else
				clouds->getCloud(0)->markPoint(i, 100.f + 100.f * m_fPtHighlightAmt);

			pointsRefresh = true;
		}
		else if (clouds->getCloud(0)->getPointMark(i) != 0)
		{
			clouds->getCloud(0)->markPoint(i, 0);
			pointsRefresh = true;
		}
	}

	if (pointsRefresh)
		clouds->getCloud(0)->setRefreshNeeded();

	return anyHits;
}