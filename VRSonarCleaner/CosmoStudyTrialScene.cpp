#include "CosmoStudyTrialScene.h"
#include "BehaviorManager.h"
#include "GrabObjectBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include <gtc/quaternion.hpp>
#include "utilities.h"


CosmoStudyTrialScene::CosmoStudyTrialScene(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_glVBO(0)
	, m_glEBO(0)
	, m_glVAO(0)
	, m_glHaloVBO(0)
	, m_glHaloVAO(0)
	, m_bShowHalos(true)
{
}


CosmoStudyTrialScene::~CosmoStudyTrialScene()
{
}

void CosmoStudyTrialScene::init()
{
	m_pCosmoVolume = new CosmoVolume("resources/data/bin");

	m_pCosmoVolume->setBackingColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
	m_pCosmoVolume->setFrameColor(glm::vec4(1.f));

	generateStreamLines();

	BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pCosmoVolume));
	BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pCosmoVolume));
}

void CosmoStudyTrialScene::processSDLEvent(SDL_Event & ev)
{
	if (ev.type == SDL_KEYDOWN && ev.key.repeat == 0 && ev.key.keysym.sym == SDLK_r)
		init();

	if (ev.type == SDL_KEYDOWN && ev.key.repeat == 0 && ev.key.keysym.sym == SDLK_h)
		m_bShowHalos = !m_bShowHalos;

	if (ev.type == SDL_KEYDOWN && ev.key.repeat == 0 && ev.key.keysym.sym == SDLK_s)
		Renderer::getInstance().toggleSkybox();
}

void CosmoStudyTrialScene::update()
{
	m_pCosmoVolume->update();

	//{
	//	m_vvvec3TransformedStreamlines.clear();
	//
	//	for (auto &sl : m_vvvec3RawStreamlines)
	//	{
	//		std::vector<glm::vec3> tempSL;
	//
	//		for (auto &pt : sl)
	//			tempSL.push_back(m_pVFG->convertToWorldCoords(pt));
	//
	//		m_vvvec3TransformedStreamlines.push_back(tempSL);
	//	}
	//}

	if (m_pTDM->getPrimaryController())
	{
		if (m_pTDM->getPrimaryController()->justPressedMenu())
			init();
	}

	if (m_pTDM->getSecondaryController())
	{
		if (m_pTDM->getSecondaryController()->justPressedMenu())
		{
			
		}
	}

	if (m_pTDM->getPrimaryController() && m_pTDM->getSecondaryController())
	{
		if (!BehaviorManager::getInstance().getBehavior("grab"))
			BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pCosmoVolume));
		if (!BehaviorManager::getInstance().getBehavior("scale"))
			BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pCosmoVolume));
	}
}

void CosmoStudyTrialScene::draw()
{
	m_pCosmoVolume->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), 1.f);
	m_pCosmoVolume->drawBBox(0.f);

	glm::vec3 dimratio = m_pCosmoVolume->getDimensions() / m_pCosmoVolume->getOriginalDimensions();
	
	if (m_pTDM->getPrimaryController())
	{
		glm::mat4 menuButtonPose = m_pTDM->getDeviceComponentPose(m_pTDM->getPrimaryController()->getIndex(), m_pTDM->getPrimaryController()->getComponentID(vr::k_EButton_ApplicationMenu));
		glm::mat4 menuButtonTextAnchorTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.025f, 0.01f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

		Renderer::getInstance().drawText(
			"Load Random\nFlowGrid",
			glm::vec4(1.f),
			menuButtonTextAnchorTrans[3],
			glm::quat(menuButtonTextAnchorTrans),
			0.015f,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_LEFT
		);
		Renderer::getInstance().drawDirectedPrimitive("cylinder",
			menuButtonTextAnchorTrans[3],
			menuButtonPose[3],
			0.001f,
			glm::vec4(1.f, 1.f, 1.f, 0.75f)
		);
	}

	if (m_pTDM->getSecondaryController())
	{
		// CP Count Label
		glm::mat4 statusTextAnchorTrans = m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, 0.01f, 0.f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

		Renderer::getInstance().drawText(
			std::to_string(m_vvec3Zeros.size()),
			glm::vec4(0.8f, 0.2f, 0.8f, 1.f),
			statusTextAnchorTrans[3],
			glm::quat(statusTextAnchorTrans),
			0.02f,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_BOTTOM
		);

		Renderer::getInstance().drawText(
			std::string("Critical Points"),
			glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
			statusTextAnchorTrans[3],
			glm::quat(statusTextAnchorTrans),
			0.0075f,
			Renderer::TextSizeDim::HEIGHT,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_TOP
		);
	}

	
	m_rs.modelToWorldTransform = m_rsHalo.modelToWorldTransform = m_pCosmoVolume->getTransformRawDomainToVolume();

	Renderer::getInstance().addToDynamicRenderQueue(m_rs);
	if (m_bShowHalos)
		Renderer::getInstance().addToDynamicRenderQueue(m_rsHalo);
}

void CosmoStudyTrialScene::generateStreamLines()
{
	m_vvvec3RawStreamlines.clear();

	int gridRes = 10;
	float radius = 0.001f;
	int numSegments = 16;


	// 4x4x4 regularly-spaced seeding grid within volume
	for (int i = 0; i < gridRes; ++i)
		for (int j = 0; j < gridRes; ++j)
			for (int k = 0; k < gridRes; ++k)
			{
				glm::vec3 seedPos(glm::vec3(i, j, k) * (1.f/gridRes));
				//glm::vec3 seedPos(glm::vec3(-1.f + (2.f / (gridRes-1)) * glm::vec3(i, j, k)));
				m_vvvec3RawStreamlines.push_back(m_pCosmoVolume->getStreamline(seedPos, 1.f, 100, 0.f));
			}


	// For each streamline segment
	// construct local coordinate frame matrix using orthogonal axes u, v, w; where the w axis is the segment (Point_[n+1] - Point_n) itself
	// the u & v axes are unit vectors scaled to the radius of the tube
	// except for the first and last, each circular 'rib' will be on the uv-plane of the averaged coordinate frames between the two connected segments

	// make unit circle for 'rib' that will be moved along streamline
	int numCircleVerts = numSegments + 1;
	std::vector<glm::vec3> circleVerts;
	for (int i = 0; i < numCircleVerts; ++i)
	{
		float angle = ((float)i / (float)(numCircleVerts - 1)) * glm::two_pi<float>();
		circleVerts.push_back(glm::vec3(sin(angle), cos(angle), 0.f));
	}

	struct PrimVert {
		glm::vec3 p; // point
		glm::vec3 n; // normal
		glm::vec4 c; // color
		glm::vec2 t; // texture coord
	};

	std::vector<PrimVert> verts, haloverts;
	std::vector<GLuint> inds;
	for (auto &sl : m_vvvec3RawStreamlines)
	{
		if (sl.size() < 2)
			continue;

		std::vector<glm::quat> ribOrientations;

		ribOrientations.push_back(getSegmentOrientationMatrixNormalized(sl[1] - sl[0]));

		for (size_t i = 0; i < sl.size() - 2; ++i)
		{
			glm::quat q1(getSegmentOrientationMatrixNormalized(sl[i + 1] - sl[i]));
			glm::quat q2(getSegmentOrientationMatrixNormalized(sl[i + 2] - sl[i + 1]));

			ribOrientations.push_back(glm::slerp(q1, q2, 0.5f));
		}

		ribOrientations.push_back(getSegmentOrientationMatrixNormalized(sl[sl.size() - 1] - sl[sl.size() - 2]));

		assert(ribOrientations.size() == sl.size());
		
		// Make the shaft of the streamtube
		for (size_t i = 0; i < ribOrientations.size(); ++i)
		{
			glm::mat4 xform(glm::toMat3(ribOrientations[i]));
			xform[3] = glm::vec4(sl[i], 1.f);

			for (int j = 0; j < circleVerts.size(); ++j)
			{
				PrimVert pv;
				pv.p = glm::vec3(xform * glm::vec4(circleVerts[j] * radius, 1.f));
				pv.n = glm::normalize(pv.p - sl[i]);
				pv.c = glm::vec4(1.f);
				pv.t = glm::vec2(j / (circleVerts.size() - 1), i);

				GLuint thisInd(verts.size());

				verts.push_back(pv);

				pv.p = glm::vec3(xform * glm::vec4(circleVerts[j] * radius * 2.f, 1.f));
				haloverts.push_back(pv);

				if (i > 0 && j > 0)
				{
					inds.push_back(thisInd);
					inds.push_back(thisInd - numCircleVerts);
					inds.push_back(thisInd - numCircleVerts - 1);

					inds.push_back(thisInd);
					inds.push_back(thisInd - numCircleVerts - 1);
					inds.push_back(thisInd - 1);
				}
			}
		}

		// Make the endcaps
		glm::quat frontCap = ribOrientations.front();
		glm::quat endCap = glm::rotate(ribOrientations.back(), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));

		for (auto &q : { frontCap, endCap })
		{
			PrimVert pv;
			pv.n = glm::vec3(glm::rotate(q, glm::vec3(0.f, 0.f, -1.f)));
			pv.c = glm::vec4(1.f);
			pv.t = glm::vec2(0.5f, q == frontCap ? 0.f : 1.f * sl.size());

			// base vertex
			int baseVert = verts.size();

			// center vertex
			pv.p = q == frontCap ? sl.front() : sl.back();
			verts.push_back(pv);
			haloverts.push_back(pv);

			glm::mat4 xform(glm::toMat3(q));
			xform[3] = glm::vec4(pv.p, 1.f);

			// circle verts (no need for last and first vert to be same)
			for (int i = 0; i < circleVerts.size(); ++i)
			{
				GLuint thisVert = verts.size();

				pv.p = glm::vec3(xform * glm::vec4(circleVerts[i] * radius, 1.f));
				verts.push_back(pv);

				pv.p = glm::vec3(xform * glm::vec4(circleVerts[i] * radius * 2.f, 1.f));
				haloverts.push_back(pv);

				if (i > 0)
				{
					inds.push_back(baseVert);
					inds.push_back(thisVert - 1);
					inds.push_back(thisVert);
				}
			}
		}
	}

	if (!m_glVBO)
	{
		glCreateBuffers(1, &m_glVBO);
		glNamedBufferStorage(m_glVBO, gridRes * gridRes * gridRes * (103 * numCircleVerts + 2) * sizeof(PrimVert), NULL, GL_DYNAMIC_STORAGE_BIT);
	}

	if (!m_glHaloVBO)
	{
		glCreateBuffers(1, &m_glHaloVBO);
		glNamedBufferStorage(m_glHaloVBO, gridRes * gridRes * gridRes * (103 * numCircleVerts + 2) * sizeof(PrimVert), NULL, GL_DYNAMIC_STORAGE_BIT);
	}

	if (!m_glEBO)
	{
		glCreateBuffers(1, &m_glEBO);
		glNamedBufferStorage(m_glEBO, gridRes * gridRes * gridRes * (100 * 6 + 2*3) * numSegments * sizeof(GLuint), NULL, GL_DYNAMIC_STORAGE_BIT);
	}

	if (!m_glVAO)
	{
		glGenVertexArrays(1, &m_glVAO);
		glBindVertexArray(this->m_glVAO);
			// Load data into vertex buffers
			glBindBuffer(GL_ARRAY_BUFFER, this->m_glVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glEBO);

			// Set the vertex attribute pointers
			glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
			glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, p));
			glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
			glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, n));
			glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
			glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, c));
			glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
			glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, t));
		glBindVertexArray(0);

		m_rs.VAO = m_glVAO;
		m_rs.glPrimitiveType = GL_TRIANGLES;
		m_rs.shaderName = "streamline";
		m_rs.indexType = GL_UNSIGNED_INT;
		m_rs.diffuseColor = glm::vec4(1.f);
		m_rs.specularColor = glm::vec4(0.f);
		m_rs.specularExponent = 1.f;
		m_rs.hasTransparency = true;
	}

	if (!m_glHaloVAO)
	{
		glGenVertexArrays(1, &m_glHaloVAO);
		glBindVertexArray(this->m_glHaloVAO);
			// Load data into vertex buffers
			glBindBuffer(GL_ARRAY_BUFFER, this->m_glHaloVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_glEBO);

			// Set the vertex attribute pointers
			glEnableVertexAttribArray(POSITION_ATTRIB_LOCATION);
			glVertexAttribPointer(POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, p));
			glEnableVertexAttribArray(NORMAL_ATTRIB_LOCATION);
			glVertexAttribPointer(NORMAL_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, n));
			glEnableVertexAttribArray(COLOR_ATTRIB_LOCATION);
			glVertexAttribPointer(COLOR_ATTRIB_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, c));
			glEnableVertexAttribArray(TEXCOORD_ATTRIB_LOCATION);
			glVertexAttribPointer(TEXCOORD_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(PrimVert), (GLvoid*)offsetof(PrimVert, t));
		glBindVertexArray(0);

		m_rsHalo.VAO = m_glHaloVAO;
		m_rsHalo.glPrimitiveType = GL_TRIANGLES;
		m_rsHalo.shaderName = "flat";
		m_rsHalo.indexType = GL_UNSIGNED_INT;
		m_rsHalo.diffuseColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
		m_rsHalo.specularColor = glm::vec4(0.f);
		m_rsHalo.specularExponent = 0.f;
		m_rsHalo.hasTransparency = false;
		m_rsHalo.vertWindingOrder = GL_CW;
	}

	glNamedBufferSubData(m_glVBO, 0, verts.size() * sizeof(PrimVert), verts.data());
	glNamedBufferSubData(m_glHaloVBO, 0, haloverts.size() * sizeof(PrimVert), haloverts.data());
	glNamedBufferSubData(m_glEBO, 0, inds.size() * sizeof(GLuint), inds.data());

	m_rs.vertCount = inds.size();
	m_rsHalo.vertCount = inds.size();

}

glm::quat CosmoStudyTrialScene::getSegmentOrientationMatrixNormalized(glm::vec3 segmentDirection, glm::vec3 up)
{
	glm::vec3 w(glm::normalize(segmentDirection));
	glm::vec3 u(glm::normalize(glm::cross(up, w)));
	glm::vec3 v(glm::normalize(glm::cross(w, u)));
	return glm::toQuat(glm::mat3(u, v, w));
}
