#pragma once

#include <SDL.h>
#include <GL/glew.h>
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

	bool initVR();

	bool HandleInput();

	void update();
	void drawScene();
	void render();

	void shutdownVR();

	Scene* m_pCurrentScene;

private:
};
