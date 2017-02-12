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
	bool BInitCompositor();

	void Shutdown();

	void recalcVolumeBounds();

	void RunMainLoop();
	bool HandleInput();
	void RenderFrame();

	void perRenderUpdates();

	void display();
		
	CleaningRoom* cleaningRoom;

	//FocalCamera *cam;

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

	GLuint m_unLensVAO;
	GLuint m_glIDVertBuffer;
	GLuint m_glIDIndexBuffer;
	unsigned int m_uiIndexSize;

	glm::mat4 m_mat4eyePosLeft;
	glm::mat4 m_mat4eyePosRight;

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
