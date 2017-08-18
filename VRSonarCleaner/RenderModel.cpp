#include "RenderModel.h"

#include <algorithm>

#include "GLSLpreamble.h"
#include "Renderer.h"

//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
RenderModel::RenderModel(const std::string & sRenderModelName)
	: m_pRenderModel(NULL)
	, m_pTexture(NULL)
	, m_sModelName(sRenderModelName)
	, m_glVBO(0u)
	, m_glIBO(0u)
	, m_glVAO(0u)
	, m_vrTexID(vr::INVALID_TEXTURE_ID)
	, m_fShininess(20.f)
	, m_bModelLoaded(false)
	, m_bTextureLoaded(false)
{
}


RenderModel::~RenderModel()
{
	cleanup();
}

bool RenderModel::ready()
{
	return isModelLoaded() && isTextureLoaded();
}


//-----------------------------------------------------------------------------
// Purpose: Frees the GL resources for a render model
//-----------------------------------------------------------------------------
void RenderModel::cleanup()
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

GLuint RenderModel::getVAO()
{
	return m_glVAO;
}

int RenderModel::getVertexCount()
{
	return m_unVertexCount;
}

float RenderModel::getMaterialShininess()
{
	return m_fShininess;
}

bool RenderModel::isModelLoaded()
{
	if (m_bModelLoaded)
		return true;

	vr::EVRRenderModelError error = vr::VRRenderModels()->LoadRenderModel_Async(m_sModelName.c_str(), &m_pRenderModel);

	if (error == vr::VRRenderModelError_Loading)
		return false;

	if (error != vr::VRRenderModelError_None)
	{
		printf("Unable to load render model %s - %s\n", m_sModelName.c_str(), vr::VRRenderModels()->GetRenderModelErrorNameFromEnum(error));
		return false;
	}

	loadGL();
	
	m_bModelLoaded = true;

	m_vrTexID = m_pRenderModel->diffuseTextureId;

	vr::VRRenderModels()->FreeRenderModel(m_pRenderModel);

	return true;
}

bool RenderModel::isTextureLoaded()
{
	if (m_bTextureLoaded)
		return true;

	if (Renderer::getInstance().getTexture(m_sModelName) == NULL)
	{
		if (m_bModelLoaded)
		{
			vr::EVRRenderModelError error = vr::VRRenderModels()->LoadTexture_Async(m_vrTexID, &m_pTexture);

			if (error == vr::VRRenderModelError_Loading)
				return false;

			if (error != vr::VRRenderModelError_None)
			{
				printf("Unable to load render texture id:%d for render model %s\n", m_vrTexID, m_sModelName.c_str());
				return false;
			}

			GLTexture* tex = new GLTexture(m_sModelName, m_pTexture->unWidth, m_pTexture->unHeight, m_pTexture->rubTextureMapData, false);
			if (!Renderer::getInstance().addTexture(tex))
			{
				delete tex;
				tex = NULL;
			}

			m_bTextureLoaded = true;

			vr::VRRenderModels()->FreeTexture(m_pTexture);

			return true;
		}

		return false;
	}

	return true;	
}

void RenderModel::loadGL()
{	
	// create and bind a VAO to hold state for this model
	glGenVertexArrays(1, &m_glVAO);
	glBindVertexArray(m_glVAO);

	// Populate a vertex buffer
	glGenBuffers(1, &m_glVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_glVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vr::RenderModel_Vertex_t) * m_pRenderModel->unVertexCount, m_pRenderModel->rVertexData, GL_STATIC_DRAW);

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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * m_pRenderModel->unTriangleCount * 3, m_pRenderModel->rIndexData, GL_STATIC_DRAW);

	glBindVertexArray(0);

	m_unVertexCount = m_pRenderModel->unTriangleCount * 3;
}
