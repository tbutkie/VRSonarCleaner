#pragma once

#include <GL/glew.h>
#include <SDL.h>
#include <GLTexture.h>
#include <gtc/quaternion.hpp>
#include <chrono>
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
		GLenum			glPrimitiveType;
		GLuint			VAO;
		int				vertCount;
		GLenum			indexType;
		std::string		shaderName;
		GLenum			vertWindingOrder;
		glm::vec4		diffuseColor;
		glm::vec4		specularColor;
		std::string		diffuseTexName;
		std::string		specularTexName;
		float			specularExponent;
		bool			hasTransparency;
		glm::vec4		transparencySortPosition;
		glm::mat4		modelToWorldTransform;

		RendererSubmission()
			: glPrimitiveType(GL_NONE)
			, VAO(0)
			, vertCount(0)
			, indexType(GL_UNSIGNED_SHORT)
			, shaderName("")
			, vertWindingOrder(GL_CCW)
			, diffuseColor(glm::vec4(1.f))
			, specularColor(glm::vec4(1.f))
			, diffuseTexName("white")
			, specularTexName("white")
			, specularExponent(0.f)
			, hasTransparency(false)
			, transparencySortPosition(glm::vec4(0.f, 0.f, 0.f, -1.f))
			, modelToWorldTransform(glm::mat4())
		{}
	};

	class ObjectSorter {
		glm::vec3 _HMDPos;
	public:
		ObjectSorter(glm::vec3 HMDPos) : _HMDPos(HMDPos) {}
		bool operator()(RendererSubmission lhs, RendererSubmission rhs) const {
			return sortByViewDistance(lhs, rhs, _HMDPos);
		}
	};

	struct SceneViewInfo {
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 viewTransform; // for e.g. transforming from head to eye space;
		uint32_t m_nRenderWidth;
		uint32_t m_nRenderHeight;
	};

	struct Camera {
		glm::vec3 pos;
		glm::vec3 lookat;
		glm::vec3 up;
	};

public:	
	// Singleton instance access
	static Renderer& getInstance()
	{
		static Renderer s_instance;
		return s_instance;
	}

	enum TextAlignment {
		LEFT,
		CENTER,
		RIGHT
	};

	/* 
	   *--------*--------*
	   |				 |
	   |				 |
	   *		*		 *
	   |				 |
	   |				 |
	   *--------*--------*
	   Where * = an anchor point for the text box
	*/
	enum TextAnchor {
		CENTER_MIDDLE,
		CENTER_TOP,
		CENTER_BOTTOM,
		CENTER_LEFT,
		CENTER_RIGHT,
		TOP_LEFT,
		TOP_RIGHT,
		BOTTOM_LEFT,
		BOTTOM_RIGHT
	};

	enum TextSizeDim {
		WIDTH,
		HEIGHT
	};
	
	bool init();

	bool CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc);

	void addToStaticRenderQueue(RendererSubmission &rs);
	void addToDynamicRenderQueue(RendererSubmission &rs);
	void clearDynamicRenderQueue();
	void addToUIRenderQueue(RendererSubmission &rs);
	void clearUIRenderQueue();

	void showMessage(std::string message, float duration = 5.f);

	bool drawPrimitive(std::string primName, glm::mat4 modelTransform, std::string diffuseTextureName, std::string specularTextureName, float specularExponent);
	bool drawPrimitive(std::string primName, glm::mat4 modelTransform, glm::vec4 diffuseColor, glm::vec4 specularColor, float specularExponent);
	bool drawFlatPrimitive(std::string primName, glm::mat4 modelTransform, glm::vec4 color);
	bool drawPrimitiveCustom(std::string primName, glm::mat4 modelTransform, std::string shaderName, std::string diffuseTexName = "white", glm::vec4 diffuseColor = glm::vec4(1.f));
	void drawConnector(glm::vec3 from, glm::vec3 to, float thickness, glm::vec4 color);
	void drawText(std::string text, glm::vec4 color, glm::vec3 pos, glm::quat rot, GLfloat size, TextSizeDim sizeDim, TextAlignment alignment = TextAlignment::CENTER, TextAnchor anchor = TextAnchor::CENTER_MIDDLE, bool snellenFont = false);
	void drawUIText(std::string text, glm::vec4 color, glm::vec3 pos, glm::quat rot, GLfloat size, TextSizeDim sizeDim, TextAlignment alignment = TextAlignment::CENTER, TextAnchor anchor = TextAnchor::CENTER_MIDDLE);
	glm::vec2 getTextDimensions(std::string text, float size, TextSizeDim sizeDim);

	GLuint getPrimitiveVAO(std::string primName);
	GLsizei getPrimitiveIndexCount(std::string primName);

	static glm::mat4 getBillBoardTransform(const glm::vec3 &pos, const glm::vec3 &viewPos, const glm::vec3 &up, bool lockToUpVector);

	static glm::mat4 getUnprojectionMatrix(glm::mat4 &proj, glm::mat4 &view, glm::mat4 &model, glm::ivec4 &vp);

	void toggleWireframe();

	GLTexture* getTexture(std::string texName);
	bool addTexture(GLTexture* tex);

	void sortTransparentObjects(glm::vec3 HMDPos);

	void RenderFrame(SceneViewInfo *sceneViewInfo, SceneViewInfo *sceneViewUIInfo, FramebufferDesc *frameBuffer);
	void RenderUI(SceneViewInfo *sceneViewInfo, FramebufferDesc *frameBuffer);
	void RenderFullscreenTexture(int width, int height, GLuint textureID, bool textureAspectPortrait = false);
	void RenderStereoTexture(int width, int height, GLuint leftEyeTextureID, GLuint rightEyeTextureID);


	void shutdown();

private:
	Renderer();
	~Renderer();
	
	void setupShaders();

	void setupTextures();
	
	void setupFullscreenQuad();

	void setupPrimitives();
	void generateIcosphere(int recursionLevel);
	void generateTorus(float coreRadius, float meridianRadius, int numCoreSegments, int numMeridianSegments);
	void generateCylinder(int numSegments);
	void generatePlane();
	void generateCube();
	void generateBBox();

	void setupText();

	void processRenderQueue(std::vector<RendererSubmission> &renderQueue);

	static bool sortByViewDistance(RendererSubmission const &rsLHS, RendererSubmission const &rsRHS, glm::vec3 const &HMDPos);

private:
	struct PrimVert {
		glm::vec3 p; // point
		glm::vec3 n; // normal
		glm::vec4 c; // color
		glm::vec2 t; // texture coord
	};

	struct Character {
		GLuint TextureID;   // ID handle of the glyph texture
		glm::ivec2 Size;    // Size of glyph
		glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
		glm::ivec2 Advance;    // Horizontal offset to advance to next glyph
	};

	Character m_arrCharacters[128];
	std::map<char, Character> m_mapSloanCharacters;
	unsigned int m_uiFontPointSize;

	LightingSystem* m_pLighting;

	ShaderSet m_Shaders;

	std::vector<RendererSubmission> m_vStaticRenderQueue_Opaque;
	std::vector<RendererSubmission> m_vStaticRenderQueue_Transparency;
	std::vector<RendererSubmission> m_vDynamicRenderQueue_Opaque;
	std::vector<RendererSubmission> m_vDynamicRenderQueue_Transparency;
	std::vector<RendererSubmission> m_vTransparentRenderQueue;
	std::vector<RendererSubmission> m_vUIRenderQueue;

	bool m_bShowWireframe;

	std::map<std::string, GLuint*> m_mapShaders;

	std::map<std::string, std::pair<GLuint, GLsizei>> m_mapPrimitives;

	std::map<std::string, GLTexture*> m_mapTextures; // holds a flag for texture with transparency

	std::vector<std::tuple<std::string, float, std::chrono::high_resolution_clock::time_point>> m_vMessages;

	GLuint m_glIcosphereVAO, m_glIcosphereVBO, m_glIcosphereEBO;
	GLuint m_glTorusVAO, m_glTorusVBO, m_glTorusEBO;
	GLuint m_glCylinderVAO, m_glCylinderVBO, m_glCylinderEBO;
	GLuint m_glPlaneVAO, m_glPlaneVBO, m_glPlaneEBO;
	GLuint m_glCubeVAO, m_glCubeVBO, m_glCubeEBO;
	GLuint m_glBBoxVAO, m_glBBoxVBO, m_glBBoxEBO;

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

