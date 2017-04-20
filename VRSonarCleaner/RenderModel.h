#pragma once
#include <GL/glew.h>
#include <string>
#include <openvr.h>

class RenderModel
{
public:
	RenderModel(const std::string & sRenderModelName);
	~RenderModel();

	bool BInit(const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture);
	void Cleanup();
	void Draw();
	const std::string & GetName() const { return m_sModelName; }

	GLuint getVAO();
	int getVertexCount();
	float getMaterialShininess();
	GLuint getDiffuseTexture();
	GLuint getSpecularTexture();

private:
	GLuint m_glVBO;
	GLuint m_glIBO;
	GLuint m_glVAO;
	GLuint m_glDiffuseTexture;
	GLuint m_glSpecularTexture;
	float m_fShininess;
	GLsizei m_unVertexCount;
	std::string m_sModelName;
};
