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
	Engine(bool demo, bool vr, bool renderToHMD, bool stereoContext, int displayIndex, float displayDiagInches);
	virtual ~Engine();

	bool init();

	void Shutdown();

	void RunMainLoop();
	
private:
	// Initialization Parameters
	bool m_bDemoMode;
	bool m_bUseVR;
	bool m_bRenderToHMD;
	bool m_bStereoOpenGL;
	int m_nDisplayIndex;
	float m_fDisplayDiagonalInches;

	std::chrono::duration<double, std::milli> m_msFrameTime, m_msInputHandleTime, m_msUpdateTime, m_msDrawTime, m_msRenderTime;

	bool m_bGLInitialized;
	bool m_bShowDesktopFrustum;
	bool m_bShowDiagnostics;

	vr::IVRSystem *m_pHMD;

	TrackedDeviceManager *m_pTDM;

	bool initVR();

	bool handleInput();

	void update();
	void drawScene();
	void render();

	void shutdownVR();

	Scene* m_pCurrentScene;

private:
};
