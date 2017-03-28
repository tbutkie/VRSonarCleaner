#include "LassoWindow.h"
#include "ShaderUtils.h"
#include "DebugDrawer.h"

extern CloudCollection *clouds;

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
	, m_bDebugOpenGL(false)
	, m_bVerbose(false)
	, m_bPerf(false)
	, m_bVblank(false)
	, m_bGlFinishHack(true)
	, leftMouseDown(false)
	, rightMouseDown(false)
{
	ballEye = glm::vec3(0.f, 0.f, 10.f);
	ballCenter = glm::vec3(0.f);
	ballUp = glm::vec3(0.f, -1.f, 0.f);
	ballRadius = 2;
	arcball = new Arcball(false);

	lasso = new LassoTool();

	glm::vec3 pos(0.f, 0.f, 0.f);
	glm::vec3 size(2.f, 0.75f, 2.f);
	glm::vec3 minCoords(clouds->getCloud(0)->getXMin(), clouds->getCloud(0)->getMinDepth(), clouds->getCloud(0)->getYMin());
	glm::vec3 maxCoords(clouds->getCloud(0)->getXMax(), clouds->getCloud(0)->getMaxDepth(), clouds->getCloud(0)->getYMax());
	dataVolume = new DataVolume(pos, 0, size, minCoords, maxCoords);
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
	glm::vec3 minCoords(clouds->getCloud(0)->getXMin(), clouds->getCloud(0)->getMinDepth(), clouds->getCloud(0)->getYMin());
	glm::vec3 maxCoords(clouds->getCloud(0)->getXMax(), clouds->getCloud(0)->getMaxDepth(), clouds->getCloud(0)->getYMax());

	dataVolume->setInnerCoords(minCoords, maxCoords);
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
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY); //UNCOMMENT AND COMMENT LINE BELOW TO ENABLE FULL OPENGL COMMANDS
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

	return true;
}



//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void LassoWindow::Shutdown()
{
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
			}
			if (sdlEvent.key.keysym.sym == SDLK_g)
			{
				printf("Pressed g, generating fake test cloud\n");
				clouds->clearAllClouds();
				clouds->generateFakeTestCloud(150, 150, 25, 40000); 
				clouds->calculateCloudBoundsAndAlign();
			}
			if (sdlEvent.key.keysym.sym == SDLK_SPACE)
			{
				if (lasso->readyToCheck())
				{
					checkForHits();
					lasso->reset();
				}
			}
		}
		else if (sdlEvent.type == SDL_MOUSEBUTTONDOWN) //MOUSE DOWN
		{
			if (sdlEvent.button.button == SDL_BUTTON_LEFT)
			{ 
				leftMouseDown = true;
				arcball->start(sdlEvent.button.x, m_nWindowHeight - sdlEvent.button.y);
				if (rightMouseDown)
				{
					lasso->end();
				}
				lasso->reset();
			}
			if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
			{
				rightMouseDown = true;
				if(!leftMouseDown)
					lasso->start(sdlEvent.button.x, m_nWindowHeight - sdlEvent.button.y);
			}
			
		}//end mouse down 
		else if (sdlEvent.type == SDL_MOUSEBUTTONUP) //MOUSE UP
		{
			if (sdlEvent.button.button == SDL_BUTTON_LEFT)
			{
				leftMouseDown = false;
				lasso->reset();
			}
			if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
			{
				rightMouseDown = false;
				lasso->end();
			}

		}//end mouse up
		if (sdlEvent.type == SDL_MOUSEMOTION)
		{
			if (leftMouseDown)
			{
				arcball->move(sdlEvent.button.x, m_nWindowHeight - sdlEvent.button.y);
			}
			if (rightMouseDown && !leftMouseDown)
			{
				lasso->move(sdlEvent.button.x, m_nWindowHeight - sdlEvent.button.y);
			}
		}
		if (sdlEvent.type == SDL_MOUSEWHEEL)
		{
			lasso->reset();
			ballEye.z -= ((float)sdlEvent.wheel.y*0.5f);
			if (ballEye.z  < 0.5f)
				ballEye.z = 0.5f;
			if (ballEye.z  > 10.f)
				ballEye.z = 10.f;
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


bool LassoWindow::checkForHits()
{
	bool hit = false;
	
	std::vector<glm::vec3> inPts = clouds->getCloud(0)->getPointPositions();

	float aspect_ratio = static_cast<float>(m_nWindowWidth) / static_cast<float>(m_nWindowHeight);

	glm::mat4 projMat = glm::perspective(50.f, aspect_ratio, 1.f, 50.f);
	glm::mat4 viewMat = glm::lookAt(ballEye, ballCenter, ballUp) * arcball->getRotation();
	glm::vec4 vp(0.f, 0.f, static_cast<float>(m_nWindowWidth), static_cast<float>(m_nWindowHeight));
	
	for (int i = 0; i < inPts.size(); ++i)
	{
		glm::vec3 in = dataVolume->convertToWorldCoords(inPts[i]);
		glm::vec3 out = glm::project(in, viewMat, projMat, vp);

		if (lasso->checkPoint(glm::vec2(out)))
		{
			clouds->getCloud(0)->markPoint(i, 1);
			hit = true;
			clouds->getCloud(0)->setRefreshNeeded();
		}
	}

	return hit;
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

	float aspect_ratio = static_cast<float>(m_nWindowWidth) / static_cast<float>(m_nWindowHeight);

	glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);
	
	//draw 2D interface elements
	{
		glm::mat4 projMat = glm::ortho(0.f, static_cast<float>(m_nWindowWidth), 0.f, static_cast<float>(m_nWindowHeight), -1.f, 1.f);

		DebugDrawer::getInstance().setTransformDefault();
		lasso->draw();

		DebugDrawer::getInstance().render(projMat); // no view matrix needed in ortho

		// flush out orthographically-rendered lines
		DebugDrawer::getInstance().flushLines();
	}

	//draw 3D elements
	{
		glm::mat4 projMat = glm::perspective(50.f, aspect_ratio, 1.f, 50.f);
		glm::mat4 viewMat = glm::lookAt(ballEye, ballCenter, ballUp);
		glm::vec4 vp(0.f, 0.f, static_cast<float>(m_nWindowWidth), static_cast<float>(m_nWindowHeight));

		arcball->setProjectionMatrix(projMat * viewMat);
		arcball->setViewport(vp);

		arcball->setZoom(ballRadius, ballEye, ballUp);
		// now render the regular scene under the arcball rotation about 0,0,0
		// (generally you would want to render everything here)
		glm::mat4 rot = arcball->getRotation();

		DebugDrawer::getInstance().setTransformDefault();
		dataVolume->setOrientation(glm::quat_cast(rot));
		dataVolume->drawBBox();

		//draw table
		DebugDrawer::getInstance().setTransform(rot * dataVolume->getCurrentDataTransform());
		clouds->drawCloud(0);

		DebugDrawer::getInstance().render(projMat * viewMat);

		// flush out perspective-rendered lines
		DebugDrawer::getInstance().flushLines();
	}
	// Flush and wait for swap.
	if (m_bVblank)
	{
		glFlush();
		glFinish();
	}
}


