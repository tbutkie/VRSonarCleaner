#pragma once
#pragma once

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <gl/glu.h>
#include <stdio.h>
#include <string>
#include <cstdlib>

#include "CleaningRoom.h"
#include "CloudCollection.h"
//#include "FocalCamera.h"
#include "DataVolume.h"
#include "arcball.h"
#include "LassoTool.h"

#include "../shared/lodepng.h"
#include "../shared/pathtools.h"


class LassoWindow
{
public:
	LassoWindow(int argc, char *argv[]);
	virtual ~LassoWindow();

	void dprintf(const char *fmt, ...);

	bool BInit();
	bool BInitGL();

	void Shutdown();

	void recalcVolumeBounds();

	void RunMainLoop();
	bool HandleInput();

	void display();
		
	CleaningRoom* cleaningRoom;

private:
	bool checkForHits();
	
	//ARCBALL STUFF
	// scene parameters
	Arcball *arcball;
	glm::vec3 ballEye;
	glm::vec3 ballCenter;
	glm::vec3 ballUp;
	float ballRadius;

	LassoTool *lasso;
	
	DataVolume *dataVolume;

	bool leftMouseDown;
	bool rightMouseDown;

	bool m_bDebugOpenGL;
	bool m_bVerbose;
	bool m_bPerf;
	bool m_bVblank;
	bool m_bGlFinishHack;

private: // SDL bookkeeping
	SDL_Window *m_pWindow;
	uint32_t m_nWindowWidth;
	uint32_t m_nWindowHeight;

	SDL_GLContext m_pContext;

private: // OpenGL bookkeeping
	float m_fNearClip;
	float m_fFarClip;
};
