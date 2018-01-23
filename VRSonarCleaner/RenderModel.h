#pragma once
#include <GL/glew.h>
#include <string>
#include <openvr.h>

class RenderModel
{
public:
	RenderModel(const std::string & sRenderModelName);
	~RenderModel();

	bool ready();
	void cleanup();
	const std::string & getName() const { return m_sModelName; }

	GLuint getVAO();
	int getVertexCount();
	float getMaterialShininess();

private:
	vr::RenderModel_t* m_pRenderModel;
	vr::RenderModel_TextureMap_t* m_pTexture;

	GLuint m_glVBO;
	GLuint m_glIBO;
	GLuint m_glVAO;
	vr::TextureID_t m_vrTexID;
	float m_fShininess;
	GLsizei m_unVertexCount;
	std::string m_sModelName;
	bool m_bModelLoaded;
	bool m_bTextureLoaded;

	bool isModelLoaded();
	bool isTextureLoaded();
	void loadGL();
};
