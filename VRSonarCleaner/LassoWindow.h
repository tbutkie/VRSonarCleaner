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

#include "../shared/lodepng.h"
#include "../shared/Matrices.h"
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
	
	//ARCBALL STUFF
	// scene parameters
	glm::vec3 ballEye;
	glm::vec3 ballCenter;
	glm::vec3 ballUp;
	float ballRadius;
	
	DataVolume *dataVolume;

	bool leftMouseDown;

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

	Matrix4 m_mat4eyePosLeft;
	Matrix4 m_mat4eyePosRight;

	Matrix4 m_mat4ProjectionCenter;
	Matrix4 m_mat4ProjectionLeft;
	Matrix4 m_mat4ProjectionRight;

	struct VertexDataScene
	{
		Vector3 position;
		Vector2 texCoord;
	};

	struct VertexDataLens
	{
		Vector2 position;
		Vector2 texCoordRed;
		Vector2 texCoordGreen;
		Vector2 texCoordBlue;
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
