#pragma once

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <gl/glu.h>
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
	float fGlobalTime; // in seconds
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
	};

	struct RendererSubmission
	{
		GLenum				glPrimitiveType;
		GLuint				VAO;
		GLsizei				vertCount;
		unsigned long long	indexByteOffset;
		GLint				indexBaseVertex;
		GLenum				indexType;
		std::string			shaderName;
		GLenum				vertWindingOrder;
		glm::vec4			diffuseColor;
		glm::vec4			specularColor;
		std::string			diffuseTexName;
		std::string			specularTexName;
		float				specularExponent;
		bool				hasTransparency;
		bool				instanced;
		GLsizei				instanceCount;
		glm::vec4			transparencySortPosition;
		glm::mat4			modelToWorldTransform;

		RendererSubmission()
			: glPrimitiveType(GL_NONE)
			, VAO(0)
			, vertCount(0)
			, indexByteOffset(0ull)
			, indexBaseVertex(0)
			, indexType(GL_UNSIGNED_SHORT)
			, shaderName("")
			, vertWindingOrder(GL_CCW)
			, diffuseColor(glm::vec4(1.f))
			, specularColor(glm::vec4(1.f))
			, diffuseTexName("white")
			, specularTexName("white")
			, specularExponent(0.f)
			, hasTransparency(false)
			, instanced(false)
			, instanceCount(0u)
			, transparencySortPosition(glm::vec4(0.f, 0.f, 0.f, -1.f))
			, modelToWorldTransform(glm::mat4())
		{}
	};

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
		glm::ivec4 viewport;
		float nearClip;
		float farClip;
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

	void setStereoRenderSize(glm::ivec2 res);
	void setMonoRenderSize(glm::ivec2 res);
	
	bool init(bool stereoRender = false, bool stereoContext = false);
	void update();

	glm::vec4 getClearColor();
	void setClearColor(glm::vec4 color);

	void addToStaticRenderQueue(RendererSubmission &rs);
	void addToDynamicRenderQueue(RendererSubmission &rs);
	void clearDynamicRenderQueue();
	void addToUIRenderQueue(RendererSubmission &rs);
	void clearUIRenderQueue();

	void showMessage(std::string message, float duration = 5.f);

	void setSkybox(std::string right, std::string left, std::string top, std::string bottom, std::string front, std::string back);
	void setSkyboxTransform(glm::mat4 trans);
	glm::mat4 getSkyboxTransform();

	void setWindowTitle(std::string title);
	void setWindowToDisplay(int displayIndex);

	SceneViewInfo* getMonoInfo();
	FramebufferDesc* getMonoFrameBuffer();
	SceneViewInfo* getLeftEyeInfo();
	FramebufferDesc* getLeftEyeFrameBuffer();
	SceneViewInfo* getRightEyeInfo();
	FramebufferDesc* getRightEyeFrameBuffer();
	SceneViewInfo* getWindow3DViewInfo();
	SceneViewInfo* getWindowUIInfo();
	Camera* getCamera();

	glm::ivec2 getUIRenderSize();
	glm::ivec2 getPresentationWindowSize();

	bool drawPrimitive(std::string primName, glm::mat4 modelTransform, std::string diffuseTextureName, std::string specularTextureName = "white", float specularExponent = 32.f);
	bool drawPrimitive(std::string primName, glm::mat4 modelTransform, glm::vec4 diffuseColor, glm::vec4 specularColor = glm::vec4(1.f), float specularExponent = 32.f);
	bool drawFlatPrimitive(std::string primName, glm::mat4 modelTransform, glm::vec4 color);
	bool drawPrimitiveCustom(std::string primName, glm::mat4 modelTransform, std::string shaderName, std::string diffuseTexName = "white", glm::vec4 diffuseColor = glm::vec4(1.f));
	void drawDirectedPrimitive(std::string primname, glm::vec3 from, glm::vec3 to, float thickness, glm::vec4 color);
	void drawDirectedPrimitiveLit(std::string primname, glm::vec3 from, glm::vec3 to, float thickness, glm::vec4 diffColor, glm::vec4 specColor = glm::vec4(1.f), float specExp = 32.f);
	void drawPointerLit(glm::vec3 from, glm::vec3 to, float thickness, glm::vec4 baseColor, glm::vec4 shaftColor, glm::vec4 arrowColor, glm::vec4 specColor = glm::vec4(1.f), float specExp = 32.f);
	void drawText(std::string text, glm::vec4 color, glm::vec3 pos, glm::quat rot, GLfloat size, TextSizeDim sizeDim, TextAlignment alignment = TextAlignment::CENTER, TextAnchor anchor = TextAnchor::CENTER_MIDDLE, bool snellenFont = false);
	void drawUIText(std::string text, glm::vec4 color, glm::vec3 pos, glm::quat rot, GLfloat size, TextSizeDim sizeDim, TextAlignment alignment = TextAlignment::CENTER, TextAnchor anchor = TextAnchor::CENTER_MIDDLE);
	glm::vec2 getTextDimensions(std::string text, float size, TextSizeDim sizeDim);

	void drawFrustum(SceneViewInfo const * svi);

	GLuint createInstancedDataBufferVBO(std::vector<glm::vec3> *instancePositions, std::vector<glm::vec4> *instanceColors);
	GLuint createInstancedPrimitiveVAO(std::string primitiveName, GLuint instanceDataVBO, GLsizei instanceCount, GLsizei instanceStride = 1);

	GLuint getPrimitiveVAO();
	GLuint getPrimitiveVBO();
	GLuint getPrimitiveEBO();
	unsigned long long getPrimitiveIndexByteOffset(std::string primName);
	GLint getPrimitiveIndexBaseVertex(std::string primName);
	GLsizei getPrimitiveIndexCount(std::string primName);

	void toggleWireframe();

	bool addShader(std::string shaderName, std::vector<std::string> shaderSources, bool lighting = false);
	GLuint* getShader(std::string name);

	GLTexture* getTexture(std::string texName);
	bool addTexture(GLTexture* tex);

	void toggleSkybox();

	float getElapsedSeconds();
	float getElapsedMilliseconds();

	uint64_t getRenderedFrameCount();

	void render();
	void swapAndClear();

	void sortTransparentObjects(glm::vec3 HMDPos);

	void shutdown();

private:
	Renderer();
	~Renderer();

	SDL_Window* createFullscreenWindow(int displayIndex, bool stereo = false);
	SDL_Window* createWindow(int width, int height, int displayIndex = 0, bool stereo = false);

	bool createFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc);
	void destroyFrameBuffer(FramebufferDesc &framebufferDesc);

	void createDesktopView();
	
	void setupShaders();

	void setupTextures();

	void setupPrimitives();

	void setupText();
	void updateUI(glm::ivec2 dims, std::chrono::high_resolution_clock::time_point tick);

	void processRenderQueue(std::vector<RendererSubmission> &renderQueue);

	void renderFrame(SceneViewInfo *sceneView3DInfo, FramebufferDesc *frameBuffer);
	void renderUI(SceneViewInfo *sceneViewUIInfo, FramebufferDesc *frameBuffer);
	void renderFullscreenTexture(int x, int y, int width, int height, GLuint textureID, bool textureAspectPortrait = false);
	void renderStereoTexture(int width, int height, GLuint leftEyeTextureID, GLuint rightEyeTextureID);

	static bool sortByViewDistance(RendererSubmission const &rsLHS, RendererSubmission const &rsRHS, glm::vec3 const &HMDPos);

private:

	struct Skybox {
		std::string right, left, top, bottom, front, back;
		GLuint texID;
		glm::mat4 transform;

		Skybox() : texID(0) {}
	};

	bool m_bShowSkybox;
	bool m_bStereoWindow; // render to a quad-buffered OpenGL context
	bool m_bStereoRender; // render two views per loop

	struct Character {
		GLuint TextureID;   // ID handle of the glyph texture
		glm::ivec2 Size;    // Size of glyph
		glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
		glm::ivec2 Advance;    // Horizontal offset to advance to next glyph
	};

	std::chrono::high_resolution_clock::time_point m_tpStart;
	std::chrono::high_resolution_clock::time_point m_tpRender;

	uint64_t m_nFrameCount;

	SDL_Window *m_pWindow;
	SDL_Cursor *m_pWindowCursor;
	SDL_GLContext m_pGLContext;

	glm::ivec2 m_ivec2WindowSize;

	Character m_arrCharacters[128];
	std::map<char, Character> m_mapSloanCharacters;
	unsigned int m_uiFontPointSize;

	ShaderSet m_Shaders;

	Renderer::SceneViewInfo *m_sviMonoInfo;
	Renderer::SceneViewInfo m_sviLeftEyeInfo;
	Renderer::SceneViewInfo m_sviRightEyeInfo;
	Renderer::FramebufferDesc *m_pMonoFramebuffer;
	Renderer::FramebufferDesc *m_pLeftEyeFramebuffer;
	Renderer::FramebufferDesc *m_pRightEyeFramebuffer;
	
	Renderer::Camera m_WindowCamera;
	Renderer::SceneViewInfo m_sviWindowUIInfo;
	Renderer::SceneViewInfo m_sviWindow3DInfo;
	Renderer::FramebufferDesc *m_pWindowFramebuffer;

	std::vector<RendererSubmission> m_vStaticRenderQueue_Opaque;
	std::vector<RendererSubmission> m_vStaticRenderQueue_Transparency;
	std::vector<RendererSubmission> m_vDynamicRenderQueue_Opaque;
	std::vector<RendererSubmission> m_vDynamicRenderQueue_Transparency;
	std::vector<RendererSubmission> m_vTransparentRenderQueue;
	std::vector<RendererSubmission> m_vUIRenderQueue;

	bool m_bShowWireframe;

	std::map<std::string, GLuint*> m_mapShaders;

	std::map<std::string, unsigned long long> m_mapPrimitiveIndexByteOffsets;
	std::map<std::string, GLint> m_mapPrimitiveIndexBaseVertices;
	std::map<std::string, GLsizei> m_mapPrimitiveIndexCounts;

	std::map<std::string, GLTexture*> m_mapTextures; // holds a flag for texture with transparency

	std::vector<std::tuple<std::string, float, std::chrono::high_resolution_clock::time_point>> m_vMessages;

	Skybox m_Skybox;

	GLuint m_glPrimitivesVAO, m_glPrimitivesVBO, m_glPrimitivesEBO;

	GLuint m_glFrameUBO;

	glm::vec4 m_vec4ClearColor;

public:
	// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
	Renderer(Renderer const&) = delete;
	void operator=(Renderer const&) = delete;
};

