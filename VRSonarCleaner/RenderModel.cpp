#include "RenderModel.h"

#include "GLSLpreamble.h"

#include <algorithm>

//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
RenderModel::RenderModel(const std::string & sRenderModelName)
	: m_sModelName(sRenderModelName)
	, m_glVBO(0)
	, m_glIBO(0)
	, m_glVAO(0)
	, m_glDiffuseTexture(0)
	, m_glSpecularTexture(0)
	, m_fShininess(20.f)
{
}


RenderModel::~RenderModel()
{
	Cleanup();
}


//-----------------------------------------------------------------------------
// Purpose: Allocates and populates the GL resources for a render model
//-----------------------------------------------------------------------------
bool RenderModel::BInit(const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture)
{
	// create and bind a VAO to hold state for this model
	glGenVertexArrays(1, &m_glVAO);
	glBindVertexArray(m_glVAO);

	// Populate a vertex buffer
	glGenBuffers(1, &m_glVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_glVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vr::RenderModel_Vertex_t) * vrModel.unVertexCount, vrModel.rVertexData, GL_STATIC_DRAW);

	// Identify the components in the vertex buffer
	glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
	glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, vPosition));
	glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
	glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, vNormal));
	glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
	glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, rfTextureCoord));

	// Create and populate the index buffer
	glGenBuffers(1, &m_glIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * vrModel.unTriangleCount * 3, vrModel.rIndexData, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// Calculate number of mipmap levels for diffuse texture
	// this is taken straight from the spec for glTexStorage2D
	int diffuseMipMapLevels = floor(log2(std::max(vrDiffuseTexture.unWidth, vrDiffuseTexture.unHeight))) + 1;

	// create and populate the texture
	glCreateTextures(GL_TEXTURE_2D, 1, &m_glDiffuseTexture);
	glTextureStorage2D(m_glDiffuseTexture, diffuseMipMapLevels, GL_RGBA8, vrDiffuseTexture.unWidth, vrDiffuseTexture.unHeight);
	glTextureSubImage2D(m_glDiffuseTexture, 0, 0, 0, vrDiffuseTexture.unWidth, vrDiffuseTexture.unHeight, GL_RGBA, GL_UNSIGNED_BYTE, vrDiffuseTexture.rubTextureMapData);

	glGenerateTextureMipmap(m_glDiffuseTexture);

	glTextureParameteri(m_glDiffuseTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glDiffuseTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glDiffuseTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_glDiffuseTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTextureParameterf(m_glDiffuseTexture, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

	GLsizei width = 1, height = 1;
	GLubyte gray[4] = { 0x80, 0x80, 0x80, 0xFF };

	// Specular map
	glCreateTextures(GL_TEXTURE_2D, 1, &m_glSpecularTexture);

	glTextureStorage2D(m_glSpecularTexture, 1, GL_RGBA8, width, height);
	glTextureSubImage2D(m_glSpecularTexture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &gray);

	glTextureParameteri(m_glSpecularTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glSpecularTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glSpecularTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_glSpecularTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	m_unVertexCount = vrModel.unTriangleCount * 3;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Frees the GL resources for a render model
//-----------------------------------------------------------------------------
void RenderModel::Cleanup()
{
	if (m_glVBO)
	{
		glDeleteBuffers(1, &m_glIBO);
		glDeleteBuffers(1, &m_glVBO);
		glDeleteVertexArrays(1, &m_glVAO);
		m_glIBO = 0;
		m_glVBO = 0;
		m_glVAO = 0;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Draws the render model
//-----------------------------------------------------------------------------
void RenderModel::Draw()
{

	glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_BINDING);
	glBindTexture(GL_TEXTURE_2D, m_glDiffuseTexture);

	glActiveTexture(GL_TEXTURE0 + SPECULAR_TEXTURE_BINDING);
	glBindTexture(GL_TEXTURE_2D, m_glSpecularTexture);

	glUniform1fv(MATERIAL_SHININESS_UNIFORM_LOCATION, 1, &m_fShininess);

	glBindVertexArray(m_glVAO);
	glDrawElements(GL_TRIANGLES, m_unVertexCount, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
}

GLuint RenderModel::getVAO()
{
	return m_glVAO;
}

float RenderModel::getMaterialShininess()
{
	return m_fShininess;
}

GLuint RenderModel::getDiffuseTexture()
{
	return m_glDiffuseTexture;
}

GLuint RenderModel::getSpecularTexture()
{
	return m_glSpecularTexture;
}
