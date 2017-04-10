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

private:
	GLuint m_glVertBuffer;
	GLuint m_glIndexBuffer;
	GLuint m_glVertArray;
	GLuint m_glDiffuseTexture;
	GLuint m_glSpecularTexture;
	GLuint m_glEmissiveTexture;
	float m_fShininess;
	GLsizei m_unVertexCount;
	std::string m_sModelName;
};
