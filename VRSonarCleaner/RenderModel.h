#pragma once
#include <GL/glew.h>
#include <string>
#include <openvr.h>

class RenderModel
{
public:
	RenderModel(const std::string & sRenderModelName);
	~RenderModel();

	bool BInit();
	void Cleanup();
	const std::string & GetName() const { return m_sModelName; }

	GLuint getVAO();
	int getVertexCount();
	float getMaterialShininess();

private:
	GLuint m_glVBO;
	GLuint m_glIBO;
	GLuint m_glVAO;
	float m_fShininess;
	GLsizei m_unVertexCount;
	std::string m_sModelName;
};
