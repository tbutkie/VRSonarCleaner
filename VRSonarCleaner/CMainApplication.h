#pragma once

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <gl/glu.h>
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <chrono>

#include "FlowVolume.h"
#include "TrackedDeviceManager.h"
#include "Renderer.h"
#include "arcball.h"
#include "LassoTool.h"
#include "CloudCollection.h"

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

	bool init();
	bool initGL();
	bool initVR();
	bool initDesktop();

	void Shutdown();

	void RunMainLoop();
	bool HandleInput();
	
	void savePoints();
	bool loadPoints(std::string fileName);
	
private:
	unsigned int m_uiCurrentFPS;

	bool m_bDebugOpenGL;
	bool m_bVerbose;
	bool m_bPerf;

	bool m_bGLInitialized;
	bool m_bUseVR;
	bool m_bUseDesktop;
	bool m_bSonarCleaning;
	bool m_bFlowVis;
	bool m_bStudyMode;
	bool m_bGreatBayModel;

	vr::IVRSystem *m_pHMD;

	TrackedDeviceManager *m_pTDM;

	void update();
	void drawScene();
	void render();

	bool editCleaningTableVR(const glm::mat4 & currentCursorPose, const glm::mat4 & lastCursorPose, float radius, bool clearPoints);
	bool editCleaningTableDesktop();
	
	DataVolume* wallVolume;
	DataVolume* tableVolume;
	FlowVolume* flowVolume;

	CloudCollection* m_pClouds;
	ColorScaler* m_pColorScalerTPU;

	//ARCBALL STUFF
	Arcball m_Arcball;
	glm::vec3 m_vec3BallEye;
	glm::vec3 m_vec3BallCenter;
	glm::vec3 m_vec3BallUp;
	float m_fBallRadius;

	LassoTool* m_pLasso;

	std::chrono::time_point<std::chrono::steady_clock> m_LastTime;
	float m_fPtHighlightAmt;

private: // SDL bookkeeping
	SDL_Window* createFullscreenWindow(int displayIndex);
	SDL_Window* createWindow(int width, int height, int displayIndex = 0);

	SDL_Window *m_pVRCompanionWindow;
	int m_nVRCompanionWindowWidth;
	int m_nVRCompanionWindowHeight;
	SDL_GLContext m_pGLContext;

	SDL_Window *m_pDesktopWindow;
	SDL_Cursor *m_pDesktopWindowCursor;
	glm::ivec2 m_ivec2DesktopWindowSize;
	//SDL_GLContext m_pDesktopWindowContext;

	bool m_bLeftMouseDown;
	bool m_bRightMouseDown;

private: // OpenGL bookkeeping
	void createVRViews();
	void createDesktopView();

	Renderer::SceneViewInfo m_sviLeftEyeInfo;
	Renderer::SceneViewInfo m_sviRightEyeInfo;
	Renderer::FramebufferDesc *m_pLeftEyeFramebuffer;
	Renderer::FramebufferDesc *m_pRightEyeFramebuffer;

	Renderer::SceneViewInfo m_sviDesktop2DOverlayViewInfo;
	Renderer::SceneViewInfo m_sviDesktop3DViewInfo;
	Renderer::FramebufferDesc *m_pDesktopFramebuffer;
};
