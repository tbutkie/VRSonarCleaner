#include "LassoWindow.h"
#include "ShaderUtils.h"
#include "DebugDrawer.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void LassoWindow::dprintf(const char *fmt, ...)
{
	va_list args;
	char buffer[2048];

	va_start(args, fmt);
	vsprintf_s(buffer, fmt, args);
	va_end(args);
		
	OutputDebugStringA(buffer);

}

static const char *arrow[] = {
	/* width height num_colors chars_per_pixel */
	"    32    32        3            1",
	/* colors */
	"X c #000000",
	". c #ffffff",
	"  c None",
	/* pixels */
	"X                               ",
	"XX                              ",
	"X.X                             ",
	"X..X                            ",
	"X...X                           ",
	"X....X                          ",
	"X.....X                         ",
	"X......X                        ",
	"X.......X                       ",
	"X........X                      ",
	"X.....XXXXX                     ",
	"X..X..X                         ",
	"X.X X..X                        ",
	"XX  X..X                        ",
	"X    X..X                       ",
	"     X..X                       ",
	"      X..X                      ",
	"      X..X                      ",
	"       XX                       ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"                                ",
	"0,0"
};

static SDL_Cursor *init_system_cursor(const char *image[])
{
	int i, row, col;
	Uint8 data[4 * 32];
	Uint8 mask[4 * 32];
	int hot_x, hot_y;

	i = -1;
	for (row = 0; row<32; ++row) {
		for (col = 0; col<32; ++col) {
			if (col % 8) {
				data[i] <<= 1;
				mask[i] <<= 1;
			}
			else {
				++i;
				data[i] = mask[i] = 0;
			}
			switch (image[4 + row][col]) {
			case 'X':
				data[i] |= 0x01;
				mask[i] |= 0x01;
				break;
			case '.':
				mask[i] |= 0x01;
				break;
			case ' ':
				break;
			}
		}
	}
	sscanf(image[4 + row], "%d,%d", &hot_x, &hot_y);
	return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}



//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
LassoWindow::LassoWindow(int argc, char *argv[])
	: m_pWindow(NULL)
	, m_pContext(NULL)
	, m_nWindowWidth(1280)
	, m_nWindowHeight(720)
	, m_unLensProgramID(0)
	, m_bDebugOpenGL(false)
	, m_bVerbose(false)
	, m_bPerf(false)
	, m_bVblank(false)
	, m_bGlFinishHack(true)
	, m_unLensVAO(0)
{
	ballEye.x = 0.0f;
	ballEye.y = 0.0f;
	ballEye.z = 6.0f;
	ballCenter.x = 0.0f;
	ballCenter.y = 0.0f;
	ballCenter.z = 0.0f;
	ballUp.x = 0.0f;
	ballUp.y = 1.0f;
	ballUp.z = 0.0f;
	ballRadius = 2;
	arcball = new Arcball(false);

	leftMouseDown = false;

	dataVolume = new DataVolume(0.f, 0.f, 0.f, 0.f, 4.f, 1.5f, 4.f);
	dataVolume->setInnerCoords(clouds->getCloud(0)->getXMin(), clouds->getCloud(0)->getXMax(), clouds->getCloud(0)->getMinDepth(), clouds->getCloud(0)->getMaxDepth(), clouds->getCloud(0)->getYMin(), clouds->getCloud(0)->getYMax());
}


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
LassoWindow::~LassoWindow()
{
	// work is done in Shutdown
	dprintf("Shutdown");

	//cam = new FocalCamera();
}

void LassoWindow::recalcVolumeBounds()
{
	dataVolume->setInnerCoords(clouds->getCloud(0)->getXMin(), clouds->getCloud(0)->getXMax(), clouds->getCloud(0)->getMinDepth(), clouds->getCloud(0)->getMaxDepth(), clouds->getCloud(0)->getYMin(), clouds->getCloud(0)->getYMax());
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool LassoWindow::BInit()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	
	int nWindowPosX = 8;// 700;
	int nWindowPosY = 30;// 100;
	m_nWindowWidth = 1900;// 1280;
	m_nWindowHeight = 980;// 720;
	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY); //UNCOMMENT AND COMMENT LINE BELOW TO ENABLE FULL OPENGL COMMANDS
																							//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

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

	if (SDL_GL_SetSwapInterval(m_bVblank ? 1 : 0) < 0)
	{
		printf("%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	std::string strWindowTitle = "VR Sonar Cleaner - Lasso Interface | CCOM VisLab";
	SDL_SetWindowTitle(m_pWindow, strWindowTitle.c_str());

	m_fNearClip = 0.1f;
	m_fFarClip = 30.0f;

	if (!BInitGL())
	{
		printf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
		return false;
	}

	//SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW));
	SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR));
	//SDL_SetCursor(init_system_cursor(arrow));

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void APIENTRY DebugCallback2(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	printf("GL Error: %s\n", message);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool LassoWindow::BInitGL()
{
	if (m_bDebugOpenGL)
	{
		glDebugMessageCallback((GLDEBUGPROC)DebugCallback2, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}
		
	cleaningRoom = new CleaningRoom();

	return true;
}



//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void LassoWindow::Shutdown()
{

	if (m_pContext)
	{
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(nullptr, nullptr);
		glDeleteBuffers(1, &m_glIDVertBuffer);
		glDeleteBuffers(1, &m_glIDIndexBuffer);

		if (m_unLensProgramID)
		{
			glDeleteProgram(m_unLensProgramID);
		}

		glDeleteRenderbuffers(1, &leftEyeDesc.m_nDepthBufferId);
		glDeleteTextures(1, &leftEyeDesc.m_nRenderTextureId);
		glDeleteFramebuffers(1, &leftEyeDesc.m_nRenderFramebufferId);
		glDeleteTextures(1, &leftEyeDesc.m_nResolveTextureId);
		glDeleteFramebuffers(1, &leftEyeDesc.m_nResolveFramebufferId);

		glDeleteRenderbuffers(1, &rightEyeDesc.m_nDepthBufferId);
		glDeleteTextures(1, &rightEyeDesc.m_nRenderTextureId);
		glDeleteFramebuffers(1, &rightEyeDesc.m_nRenderFramebufferId);
		glDeleteTextures(1, &rightEyeDesc.m_nResolveTextureId);
		glDeleteFramebuffers(1, &rightEyeDesc.m_nResolveFramebufferId);

		if (m_unLensVAO != 0)
		{
			glDeleteVertexArrays(1, &m_unLensVAO);
		}
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
bool LassoWindow::HandleInput()
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
		else if (sdlEvent.type == SDL_MOUSEBUTTONDOWN) //MOUSE DOWN
		{
			if (sdlEvent.button.button == SDL_BUTTON_LEFT)
			{ 
				leftMouseDown = true;
				arcball->start(sdlEvent.button.x, m_nWindowHeight - sdlEvent.button.y);
			}
			
		}//end mouse down 
		else if (sdlEvent.type == SDL_MOUSEBUTTONUP) //MOUSE UP
		{
			if (sdlEvent.button.button == SDL_BUTTON_LEFT)
			{
				leftMouseDown = false;
			}

		}//end mouse up
		if (sdlEvent.type == SDL_MOUSEMOTION)
		{
			if (leftMouseDown)
			{
				arcball->move(sdlEvent.button.x, m_nWindowHeight - sdlEvent.button.y);
			}
		}
		if (sdlEvent.type == SDL_MOUSEWHEEL)
		{
			ballEye.z -= ((float)sdlEvent.wheel.y*0.5);
			if (ballEye.z  < 0.5)
				ballEye.z = 0.5;
			if (ballEye.z  > 10)
				ballEye.z = 10;

		}
		
	}

	return bRet;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void LassoWindow::RunMainLoop()
{
	bool bQuit = false;

	SDL_StartTextInput();

	while (!bQuit)
	{
		bQuit = HandleInput();
		
		//do interaction check here?

		display();
	}

	////doesn't help here either
	fclose(stdout);
	FreeConsole();

	SDL_StopTextInput();
}

/*
void LassoWindow::checkForHits()
{
	Matrix4 currentCursorPose;
	Matrix4 lastCursorPose;
	float cursorRadius;

	// if editing mode not active, abort
	if (!m_pTDM->getCleaningCursorData(&currentCursorPose, &lastCursorPose, &cursorRadius))
		return;

	// check point cloud for hits
	if (cleaningRoom->checkCleaningTable(currentCursorPose, lastCursorPose, cursorRadius, 10))
		m_pTDM->cleaningHit();
}

void LassoWindow::checkForManipulations()
{
	Matrix4 pose;

	if (m_pTDM->getManipulationData(pose))
		cleaningRoom->gripCleaningTable(&pose);
	else
		cleaningRoom->gripCleaningTable(NULL);
}
*/


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool LassoWindow::CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc)
{
	glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

	glGenTextures(1, &framebufferDesc.m_nRenderTextureId);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	glGenTextures(1, &framebufferDesc.m_nResolveTextureId);
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}



///////////////////////////////////////////////////////////////////////////////////////////////
void LassoWindow::perRenderUpdates()
{

}


///////////////////////////////////////////////////////////////////////////////////////////////
void LassoWindow::display()
{

	// SwapWindow
	{
		SDL_GL_SwapWindow(m_pWindow);
	}

	// Clear
	{
		// We want to make sure the glFinish waits for the entire present to complete, not just the submission
		// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
		glClearColor(0.33, 0.39, 0.49, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	glEnable(GL_DEPTH_TEST);

	float aspect_ratio = (float)m_nWindowWidth / (float)m_nWindowHeight;

	glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0f, aspect_ratio, 1.0f, 50.0f);
	gluLookAt(
		ballEye.x, ballEye.y, ballEye.z,
		ballCenter.x, ballCenter.y, ballCenter.z,
		ballUp.x, ballUp.y, ballUp.z);
	//arcball_setzoom(-ballRadius/abs(ballEye.z), ballEye, ballUp);
	arcball->getProjectionMatrix();
	arcball->getViewport();

	glMatrixMode(GL_MODELVIEW);
	// set up the arcball using the current projection matrix
	glLoadIdentity();

	arcball->setZoom(ballRadius, ballEye, ballUp);
	// now render the regular scene under the arcball rotation about 0,0,0
	// (generally you would want to render everything here)
	arcball->rotate();
	
	dataVolume->drawBBox();

	//draw table
	glPushMatrix();
		dataVolume->activateTransformationMatrix();
		clouds->drawCloud(0);
	glPopMatrix();		
	// Flush and wait for swap.
	if (m_bVblank)
	{
		glFlush();
		glFinish();
	}

	DebugDrawer::getInstance().flushLines();
}


