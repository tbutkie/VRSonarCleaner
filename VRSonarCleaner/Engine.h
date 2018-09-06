#pragma once

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <gl/glu.h>
#include <chrono>
#include <vector>

#include <openvr.h>

#include "TrackedDeviceManager.h"
#include "Renderer.h"
#include "SceneBase.h"

#include <glm.hpp>

//-----------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
class Engine
{
public:
	Engine();
	virtual ~Engine();

	bool init();

	void Shutdown();

	void RunMainLoop();
	
private:
	std::chrono::duration<double, std::milli> m_msFrameTime, m_msInputHandleTime, m_msUpdateTime, m_msVRUpdateTime, m_msDrawTime, m_msRenderTime;

	bool m_bGLInitialized;
	bool m_bGreatBayModel;
	bool m_bShowDesktopFrustum;
	bool m_bDemoMode;
	bool m_bShowDiagnostics;

	vr::IVRSystem *m_pHMD;

	TrackedDeviceManager *m_pTDM;

	bool initGL();
	bool initVR();
	bool initDesktop();

	bool HandleInput();

	void update();
	void drawScene();
	void render();

	void shutdownVR();
	void shutdownDesktop();

	Scene* m_pCurrentScene;

private: // SDL bookkeeping
	SDL_Window* createFullscreenWindow(int displayIndex);
	SDL_Window* createWindow(int width, int height, int displayIndex = 0);
	void setWindowToDisplay(SDL_Window* win, int displayIndex);

	SDL_Window *m_pWindow;
	SDL_Cursor *m_pWindowCursor;
	glm::ivec2 m_ivec2WindowSize;
	SDL_GLContext m_pGLContext;

private:
};
