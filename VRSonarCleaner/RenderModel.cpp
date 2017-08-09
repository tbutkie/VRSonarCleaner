#include "RenderModel.h"

#include <algorithm>
#include <windows.h>

#include "GLSLpreamble.h"
#include "Renderer.h"

//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
RenderModel::RenderModel(const std::string & sRenderModelName)
	: m_sModelName(sRenderModelName)
	, m_glVBO(0)
	, m_glIBO(0)
	, m_glVAO(0)
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
bool RenderModel::BInit()
{
	vr::RenderModel_t *pModel;
	vr::EVRRenderModelError error;
	while (1)
	{
		error = vr::VRRenderModels()->LoadRenderModel_Async(m_sModelName.c_str(), &pModel);
		if (error != vr::VRRenderModelError_Loading)
			break;

		Sleep(1);
	}

	if (error != vr::VRRenderModelError_None)
	{
		printf("Unable to load render model %s - %s\n", m_sModelName.c_str(), vr::VRRenderModels()->GetRenderModelErrorNameFromEnum(error));
		return NULL; // move on to the next tracked device
	}

	if (Renderer::getInstance().getTexture(m_sModelName) == NULL)
	{
		vr::RenderModel_TextureMap_t *pTexture;
		while (1)
		{
			error = vr::VRRenderModels()->LoadTexture_Async(pModel->diffuseTextureId, &pTexture);
			if (error != vr::VRRenderModelError_Loading)
				break;

			Sleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			printf("Unable to load render texture id:%d for render model %s\n", pModel->diffuseTextureId, m_sModelName.c_str());
			vr::VRRenderModels()->FreeRenderModel(pModel);
			return NULL; // move on to the next tracked device
		}

		Renderer::getInstance().addTexture(new GLTexture(m_sModelName, pTexture->unWidth, pTexture->unHeight, pTexture->rubTextureMapData, false));

		vr::VRRenderModels()->FreeTexture(pTexture);
	}

	// create and bind a VAO to hold state for this model
	glGenVertexArrays(1, &m_glVAO);
	glBindVertexArray(m_glVAO);

	// Populate a vertex buffer
	glGenBuffers(1, &m_glVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_glVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vr::RenderModel_Vertex_t) * pModel->unVertexCount, pModel->rVertexData, GL_STATIC_DRAW);

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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * pModel->unTriangleCount * 3, pModel->rIndexData, GL_STATIC_DRAW);

	glBindVertexArray(0);
	
	m_unVertexCount = pModel->unTriangleCount * 3;

	vr::VRRenderModels()->FreeRenderModel(pModel);

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
