#pragma once

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <gl/glu.h>
#include <stdio.h>
#include <string>
#include <cstdlib>

#include "CGLRenderModel.h"
#include "CleaningRoom.h"
#include "FlowVolume.h"
#include "TrackedDeviceManager.h"
#include "LightingSystem.h"
#include "Renderer.h"

#include <openvr.h>

#include "../shared/lodepng.h"
#include "../shared/pathtools.h"

#include <shared/glm/glm.hpp>

static bool g_bPrintf = true;

//-----------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
class CMainApplication
{
public:
	CMainApplication(int argc, char *argv[], int Mode);
	virtual ~CMainApplication();

	bool BInit();
	bool BInitGL();
	bool BInitCompositor();

	void Shutdown();

	void RunMainLoop();
	bool HandleInput();
	
	void savePoints();
	bool loadPoints(std::string fileName);
	
private:

	int mode; //0=Cleaner, 1=Flow

	unsigned int m_uiCurrentFPS;

	bool m_bDebugOpenGL;
	bool m_bVerbose;
	bool m_bPerf;

	vr::IVRSystem *m_pHMD;

	TrackedDeviceManager *m_pTDM;

	CleaningRoom* cleaningRoom;
	FlowVolume* flowVolume;
	LightingSystem* m_pLighting;

private: // SDL bookkeeping
	SDL_Window *m_pWindow;
	uint32_t m_nWindowWidth;
	uint32_t m_nWindowHeight;

	SDL_GLContext m_pContext;

private: // OpenGL bookkeeping
};
