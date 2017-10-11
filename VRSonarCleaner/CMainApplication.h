#pragma once

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <gl/glu.h>
#include <chrono>
#include <vector>

#include <openvr.h>

#include "FlowVolume.h"
#include "TrackedDeviceManager.h"
#include "Renderer.h"
#include "arcball.h"
#include "LassoTool.h"
#include "SonarPointCloud.h"
#include "ColorScaler.h"

#include <shared/glm/glm.hpp>

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
	
private:
	std::chrono::duration<double, std::milli> m_msFrameTime, m_msInputHandleTime, m_msUpdateTime, m_msVRUpdateTime, m_msDrawTime, m_msRenderTime;

	bool m_bGLInitialized;
	bool m_bUseVR;
	bool m_bUseDesktop;
	bool m_bSonarCleaning;
	bool m_bFlowVis;
	bool m_bStudyMode;
	bool m_bGreatBayModel;
	bool m_bShowDesktopFrustum;
	bool m_bDemoMode;

	vr::IVRSystem *m_pHMD;

	TrackedDeviceManager *m_pTDM;

	void update();
	void drawScene();
	void render();

	bool editCleaningTableDesktop();

	void refreshColorScale(ColorScaler* colorScaler, std::vector<SonarPointCloud*> clouds);
	
	DataVolume* m_pWallVolume;
	DataVolume* m_pTableVolume;
	FlowVolume* m_pFlowVolume;

	std::vector<SonarPointCloud*> m_vpClouds;
	std::vector<DataVolume*> m_vpDataVolumes;
	ColorScaler* m_pColorScalerTPU;

	//ARCBALL STUFF
	Arcball m_Arcball;
	glm::vec3 m_vec3BallEye;
	glm::vec3 m_vec3BallCenter;
	glm::vec3 m_vec3BallUp;
	float m_fBallRadius;

	LassoTool* m_pLasso;

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
	bool m_bMiddleMouseDown;

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
