#pragma once

#include <GL/glew.h>
#include <openvr.h>
#include <SDL.h>
#include "CGLRenderModel.h"
#include "LightingSystem.h"
#include "TrackedDeviceManager.h"
#include "shaderset.h"

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

	struct VertexDataWindow
	{
		glm::vec2 position;
		glm::vec2 texCoord;

		VertexDataWindow(const glm::vec2 & pos, const glm::vec2 tex) : position(pos), texCoord(tex) {	}
	};

public:	
	// Singleton instance access
	static Renderer& getInstance()
	{
		static Renderer s_instance;
		return s_instance;
	}
	
	bool init(vr::IVRSystem *pHMD, TrackedDeviceManager *pTDM, LightingSystem *pLS);

	void addRenderModelInstance(const char* name, glm::mat4 instancePose);
	void resetRenderModelInstances();

	void RenderFrame(SDL_Window *win, glm::mat4 &HMDView);

	void Shutdown();

private:
	Renderer();
	~Renderer();
	
	bool CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc);

	void SetupShaders();
	void SetupCameras();
	bool SetupStereoRenderTargets();
	void SetupCompanionWindow();
	
	CGLRenderModel* findOrLoadRenderModel(const char *pchRenderModelName);

	glm::mat4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
	glm::mat4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);

	void RenderStereoTargets();
	void RenderScene(vr::Hmd_Eye nEye);
	void RenderCompanionWindow();

private:
	vr::IVRSystem *m_pHMD;
	TrackedDeviceManager *m_pTDM;
	LightingSystem* m_pLighting;

	ShaderSet m_Shaders;

	std::map<std::string, CGLRenderModel*> m_mapModelCache;
	std::map<std::string, std::vector<glm::mat4>> m_mapModelInstances;

	bool m_bVblank;
	bool m_bGlFinishHack;

	GLuint* m_punCompanionWindowProgramID;
	GLuint* m_punRenderModelProgramID;
	GLuint* m_punDebugDrawerProgramID;

	int m_nCompanionWindowWidth;
	int m_nCompanionWindowHeight;
	FramebufferDesc leftEyeDesc;
	FramebufferDesc rightEyeDesc;
	
	GLuint m_unCompanionWindowVAO;
	GLuint m_glCompanionWindowIDVertBuffer;
	GLuint m_glCompanionWindowIDIndexBuffer;
	unsigned int m_uiCompanionWindowIndexSize;

	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;
	float m_fNearClip;
	float m_fFarClip;

	glm::mat4 m_mat4eyePoseLeft;
	glm::mat4 m_mat4eyePoseRight;

	glm::mat4 m_mat4ProjectionLeft;
	glm::mat4 m_mat4ProjectionRight;

	glm::mat4 m_mat4CurrentHMDView;

public:
	// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
	Renderer(Renderer const&) = delete;
	void operator=(Renderer const&) = delete;
};

