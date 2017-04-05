#include "CGLRenderModel.h"

#include "GLSLpreamble.h"

//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
CGLRenderModel::CGLRenderModel(const std::string & sRenderModelName)
	: m_sModelName(sRenderModelName)
	, m_glIndexBuffer(0)
	, m_glVertArray(0)
	, m_glVertBuffer(0)
	, m_glDiffuseTexture(0)
	, m_glSpecularTexture(0)
	, m_glEmissiveTexture(0)
	, m_fShininess(20.f)
{
}


CGLRenderModel::~CGLRenderModel()
{
	Cleanup();
}


//-----------------------------------------------------------------------------
// Purpose: Allocates and populates the GL resources for a render model
//-----------------------------------------------------------------------------
bool CGLRenderModel::BInit(const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture)
{
	// create and bind a VAO to hold state for this model
	glGenVertexArrays(1, &m_glVertArray);
	glBindVertexArray(m_glVertArray);

	// Populate a vertex buffer
	glGenBuffers(1, &m_glVertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_glVertBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vr::RenderModel_Vertex_t) * vrModel.unVertexCount, vrModel.rVertexData, GL_STATIC_DRAW);

	// Identify the components in the vertex buffer
	glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
	glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, vPosition));
	glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
	glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, vNormal));
	glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
	glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(vr::RenderModel_Vertex_t), (void *)offsetof(vr::RenderModel_Vertex_t, rfTextureCoord));

	// Create and populate the index buffer
	glGenBuffers(1, &m_glIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * vrModel.unTriangleCount * 3, vrModel.rIndexData, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// create and populate the texture
	glGenTextures(1, &m_glDiffuseTexture);
	glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_BINDING);
	glBindTexture(GL_TEXTURE_2D, m_glDiffuseTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vrDiffuseTexture.unWidth, vrDiffuseTexture.unHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, vrDiffuseTexture.rubTextureMapData);

	glBindTexture(GL_TEXTURE_2D, 0);

	// If this renders black ask McJohn what's wrong.
	glGenerateTextureMipmap(m_glDiffuseTexture);

	glTextureParameteri(m_glDiffuseTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glDiffuseTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glDiffuseTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_glDiffuseTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTextureParameterf(m_glDiffuseTexture, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);



	GLsizei width = 1, height = 1;
	GLubyte white[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	GLubyte gray[4] = { 0x80, 0x80, 0x80, 0xFF };
	GLubyte dkgray[4] = { 0x0F, 0x0F, 0x0F, 0xFF };
	GLubyte black[4] = { 0x00, 0x00, 0x00, 0xFF };

	// Specular map
	glGenTextures(1, &m_glSpecularTexture);
	glActiveTexture(GL_TEXTURE0 + SPECULAR_TEXTURE_BINDING);
	glBindTexture(GL_TEXTURE_2D, m_glSpecularTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &gray);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenerateTextureMipmap(m_glSpecularTexture);

	glTextureParameteri(m_glSpecularTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glSpecularTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glSpecularTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_glSpecularTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	// Emissive map
	glGenTextures(1, &m_glEmissiveTexture);
	glActiveTexture(GL_TEXTURE0 + EMISSIVE_TEXTURE_BINDING);
	glBindTexture(GL_TEXTURE_2D, m_glEmissiveTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &black);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenerateTextureMipmap(m_glEmissiveTexture);

	glTextureParameteri(m_glEmissiveTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glEmissiveTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_glEmissiveTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(m_glEmissiveTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	glActiveTexture(GL_TEXTURE0);

	m_unVertexCount = vrModel.unTriangleCount * 3;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Frees the GL resources for a render model
//-----------------------------------------------------------------------------
void CGLRenderModel::Cleanup()
{
	if (m_glVertBuffer)
	{
		glDeleteBuffers(1, &m_glIndexBuffer);
		glDeleteVertexArrays(1, &m_glVertArray);
		glDeleteBuffers(1, &m_glVertBuffer);
		m_glIndexBuffer = 0;
		m_glVertArray = 0;
		m_glVertBuffer = 0;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Draws the render model
//-----------------------------------------------------------------------------
void CGLRenderModel::Draw()
{
	glBindVertexArray(m_glVertArray);

	glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_BINDING);
	glBindTexture(GL_TEXTURE_2D, m_glDiffuseTexture);

	glActiveTexture(GL_TEXTURE0 + SPECULAR_TEXTURE_BINDING);
	glBindTexture(GL_TEXTURE_2D, m_glSpecularTexture);

	glActiveTexture(GL_TEXTURE0 + EMISSIVE_TEXTURE_BINDING);
	glBindTexture(GL_TEXTURE_2D, m_glEmissiveTexture);

	glUniform1fv(MATERIAL_SHININESS_UNIFORM_LOCATION, 1, &m_fShininess);

	glDrawElements(GL_TRIANGLES, m_unVertexCount, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
}