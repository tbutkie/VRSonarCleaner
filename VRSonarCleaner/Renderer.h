#pragma once

#include <GL/glew.h>
#include <openvr.h>
#include <SDL.h>
#include "LightingSystem.h"
#include "TrackedDeviceManager.h"

class Renderer
{
public:
	struct FramebufferDesc
	{
		GLuint m_nDepthBufferId;
		GLuint m_nRenderTextureId;
		GLuint m_nRenderFramebufferId;
		GLuint m_nResolveTextureId;
		GLuint m_nResolveFramebufferId;
	};

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

public:	
	// Singleton instance access
	static Renderer& getInstance()
	{
		static Renderer s_instance;
		return s_instance;
	}
	
	bool init(vr::IVRSystem *pHMD, TrackedDeviceManager *pTDM, LightingSystem *pLS);

	void RenderFrame(SDL_Window *win, glm::mat4 &HMDPose);

	void Shutdown();

private:
	Renderer();
	~Renderer();

	bool CreateLensShader();

	bool CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc);

	void SetupCameras();
	bool SetupStereoRenderTargets();
	void SetupDistortion();

	glm::mat4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
	glm::mat4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);

	void RenderStereoTargets();
	void RenderScene(vr::Hmd_Eye nEye);
	void RenderDistortion();

private:
	vr::IVRSystem *m_pHMD;
	TrackedDeviceManager *m_pTDM;
	LightingSystem* m_pLighting;

	bool m_bVblank;
	bool m_bGlFinishHack;

	GLuint m_unLensProgramID;

	uint32_t m_nWindowWidth;
	uint32_t m_nWindowHeight;
	FramebufferDesc leftEyeDesc;
	FramebufferDesc rightEyeDesc;
	
	GLuint m_unLensVAO;
	GLuint m_glIDVertBuffer;
	GLuint m_glIDIndexBuffer;
	unsigned int m_uiIndexSize;

	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;
	float m_fNearClip;
	float m_fFarClip;

	glm::mat4 m_mat4eyePoseLeft;
	glm::mat4 m_mat4eyePoseRight;

	glm::mat4 m_mat4ProjectionLeft;
	glm::mat4 m_mat4ProjectionRight;

	glm::mat4 m_mat4CurrentHMDPose;

public:
	// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
	Renderer(Renderer const&) = delete;
	void operator=(Renderer const&) = delete;
};

