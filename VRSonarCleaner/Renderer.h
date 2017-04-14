#pragma once

#include <GL/glew.h>
#include <openvr.h>
#include <SDL.h>
#include "RenderModel.h"
#include "LightingSystem.h"
#include "TrackedDeviceManager.h"
#include "shaderset.h"

struct FrameUniforms {
	glm::vec4 v4Viewport;
	glm::mat4 m4View;
	glm::mat4 m4Projection;
	glm::mat4 m4ViewProjection;
};

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

		FramebufferDesc()
		{
			glCreateRenderbuffers(1, &m_nDepthBufferId);
			glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &m_nRenderTextureId);
			glCreateFramebuffers(1, &m_nRenderFramebufferId);
			glCreateTextures(GL_TEXTURE_2D, 1, &m_nResolveTextureId);
			glCreateFramebuffers(1, &m_nResolveFramebufferId);
		}

		~FramebufferDesc()
		{
			glDeleteFramebuffers(1, &m_nResolveFramebufferId);
			glDeleteTextures(1, &m_nResolveTextureId);
			glDeleteFramebuffers(1, &m_nRenderFramebufferId);
			glDeleteTextures(1, &m_nRenderTextureId);
			glDeleteRenderbuffers(1, &m_nDepthBufferId);
		}
	};

	struct RendererSubmission
	{
		GLenum			primitiveType;
		GLuint			VAO;
		int				vertCount;
		std::string		shaderName;
		GLuint			diffuseTex;
		GLuint			specularTex;
		float			specularExponent;
		glm::mat4		modelToWorldTransform;
	};

public:	
	// Singleton instance access
	static Renderer& getInstance()
	{
		static Renderer s_instance;
		return s_instance;
	}
	
	bool init(vr::IVRSystem *pHMD, TrackedDeviceManager *pTDM);

	void addRenderModelInstance(const char* name, glm::mat4 instancePose);
	void resetRenderModelInstances();

	void addToRenderQueue(RendererSubmission &rs);

	void toggleWireframe();

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
	
	RenderModel* findOrLoadRenderModel(const char *pchRenderModelName);

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

	std::vector<RendererSubmission> m_vRenderQueue;

	std::map<std::string, RenderModel*> m_mapModelCache;
	std::map<std::string, std::vector<glm::mat4>> m_mapModelInstances;

	bool m_bVblank;
	bool m_bGlFinishHack;
	bool m_bShowWireframe;

	std::map<std::string, GLuint*> m_mapShaders;

	GLuint m_glFrameUBO;

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

