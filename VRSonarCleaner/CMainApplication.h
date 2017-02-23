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
#include "FlowRoom.h"
#include "TrackedDeviceManager.h"
#include "LightingSystem.h"

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
	void RenderFrame();

	bool SetupStereoRenderTargets();
	void SetupDistortion();
	void SetupCameras();

	void RenderStereoTargets();
	void RenderDistortion();
	void RenderScene(vr::Hmd_Eye nEye);

	glm::mat4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
	glm::mat4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);
	glm::mat4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye);

	void savePoints();
	bool loadPoints(std::string fileName);

	bool CreateLensShader();

	void checkForHits();
	void checkForManipulations();

private:

	int mode; //0=Cleaner, 1=Flow

	unsigned int m_uiCurrentFPS;

	bool m_bDebugOpenGL;
	bool m_bVerbose;
	bool m_bPerf;
	bool m_bVblank;
	bool m_bGlFinishHack;

	vr::IVRSystem *m_pHMD;

	TrackedDeviceManager *m_pTDM;

	CleaningRoom* cleaningRoom;
	FlowRoom* flowRoom;
	LightingSystem* m_pLighting;

private: // SDL bookkeeping
	SDL_Window *m_pWindow;
	uint32_t m_nWindowWidth;
	uint32_t m_nWindowHeight;

	SDL_GLContext m_pContext;

private: // OpenGL bookkeeping
	float m_fNearClip;
	float m_fFarClip;
	
	GLuint m_unLensVAO;
	GLuint m_glIDVertBuffer;
	GLuint m_glIDIndexBuffer;
	unsigned int m_uiIndexSize;

	glm::mat4 m_mat4eyePoseLeft;
	glm::mat4 m_mat4eyePoseRight;

	glm::mat4 m_mat4ProjectionCenter;
	glm::mat4 m_mat4ProjectionLeft;
	glm::mat4 m_mat4ProjectionRight;

	struct VertexDataScene
	{
		glm::vec3 position;
		glm::vec2 texCoord;
	};

	struct VertexDataLens
	{
		glm::vec2 position;
		glm::vec2 texCoordRed;
		glm::vec2 texCoordGreen;
		glm::vec2 texCoordBlue;
	};

	GLuint m_unLensProgramID;

	struct FramebufferDesc
	{
		GLuint m_nDepthBufferId;
		GLuint m_nRenderTextureId;
		GLuint m_nRenderFramebufferId;
		GLuint m_nResolveTextureId;
		GLuint m_nResolveFramebufferId;
	};
	FramebufferDesc leftEyeDesc;
	FramebufferDesc rightEyeDesc;

	bool CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc);

	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;
};
