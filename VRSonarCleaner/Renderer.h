#pragma once

#include <GL/glew.h>
#include <SDL.h>
#include "LightingSystem.h"
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
		GLenum			indexType;
		std::string		shaderName;
		GLuint			diffuseTex;
		GLuint			specularTex;
		float			specularExponent;
		glm::mat4		modelToWorldTransform;

		RendererSubmission()
			: primitiveType(GL_NONE)
			, VAO(0)
			, vertCount(0)
			, indexType(GL_UNSIGNED_SHORT)
			, shaderName("")
			, diffuseTex(0)
			, specularTex(0)
			, specularExponent(0.f)
			, modelToWorldTransform(glm::mat4())
		{}
	};

	struct SceneViewInfo {
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 viewTransform; // for e.g. transforming from head to eye space;
		uint32_t m_nRenderWidth;
		uint32_t m_nRenderHeight;
	};

public:	
	// Singleton instance access
	static Renderer& getInstance()
	{
		static Renderer s_instance;
		return s_instance;
	}
	
	bool init();

	bool CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc);


	void addToStaticRenderQueue(RendererSubmission &rs);
	void addToDynamicRenderQueue(RendererSubmission &rs);
	void clearDynamicRenderQueue();
	void addToUIRenderQueue(RendererSubmission &rs);
	void clearUIRenderQueue();

	void toggleWireframe();

	void RenderFrame(SceneViewInfo *sceneViewInfo, SceneViewInfo *sceneViewUIInfo, FramebufferDesc *frameBuffer);
	void RenderUI(SceneViewInfo *sceneViewInfo, FramebufferDesc *frameBuffer);
	void RenderFullscreenTexture(int width, int height, GLuint textureID, bool textureAspectPortrait = false);

	void Shutdown();

private:
	Renderer();
	~Renderer();
	
	void setupShaders();
	
	void setupFullscreenTexture();

	void processRenderQueue(std::vector<RendererSubmission> &renderQueue);

private:
	LightingSystem* m_pLighting;

	ShaderSet m_Shaders;

	std::vector<RendererSubmission> m_vStaticRenderQueue;
	std::vector<RendererSubmission> m_vDynamicRenderQueue;
	std::vector<RendererSubmission> m_vUIRenderQueue;

	bool m_bShowWireframe;

	std::map<std::string, GLuint*> m_mapShaders;

	GLuint m_glFrameUBO;
	
	GLuint m_glFullscreenTextureVAO;
	GLuint m_glFullscreenTextureVBO;
	GLuint m_glFullscreenTextureEBO;
	unsigned int m_uiCompanionWindowVertCount;

public:
	// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
	Renderer(Renderer const&) = delete;
	void operator=(Renderer const&) = delete;
};

