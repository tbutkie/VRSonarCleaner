#include "StudyTrialScene.h"
#include "Renderer.h"
#include "BehaviorManager.h"
#include "GrabObjectBehavior.h"
#include "ScaleDataVolumeBehavior.h"
#include <gtc/quaternion.hpp>
#include "utilities.h"


StudyTrialScene::StudyTrialScene(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pVFG(NULL)
{
}


StudyTrialScene::~StudyTrialScene()
{
}

void StudyTrialScene::init()
{
	if (m_pVFG)
		delete m_pVFG;

	m_pVFG = new VectorFieldGenerator(glm::vec3(0.f, 1.f, 0.f), glm::quat(), glm::vec3(1.f));
	
	m_pVFG->setBackingColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
	m_pVFG->setFrameColor(glm::vec4(1.f));

	m_pVFG->setGridResolution(32u);
	m_pVFG->setGaussianShape(1.2f);
	m_pVFG->createRandomControlPoints(6u);
	m_pVFG->generate();

	generateStreamLines();

	BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pVFG));
	BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pVFG));
}

void StudyTrialScene::processSDLEvent(SDL_Event & ev)
{
}

void StudyTrialScene::update()
{
	m_pVFG->update();

	{
		m_vvvec3TransformedStreamlines.clear();

		for (auto &sl : m_vvvec3RawStreamlines)
		{
			std::vector<glm::vec3> tempSL;

			for (auto &pt : sl)
				tempSL.push_back(m_pVFG->convertToWorldCoords(pt));

			m_vvvec3TransformedStreamlines.push_back(tempSL);
		}
	}

	if (!BehaviorManager::getInstance().getBehavior("grab"))
		BehaviorManager::getInstance().addBehavior("grab", new GrabObjectBehavior(m_pTDM, m_pVFG));
	if (!BehaviorManager::getInstance().getBehavior("scale"))
		BehaviorManager::getInstance().addBehavior("scale", new ScaleDataVolumeBehavior(m_pTDM, m_pVFG));

	if (m_pTDM->getPrimaryController())
	{
		if (m_pTDM->getPrimaryController()->justPressedMenu())
			init();
	}
}

void StudyTrialScene::draw()
{
	m_pVFG->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), 1.f);
	m_pVFG->drawBBox(0.f);

	for (auto &sl : m_vvvec3TransformedStreamlines)
	{
		auto nPts = sl.size();

		for (size_t i = 0; i < nPts - 1; ++i)
		{
			Renderer::getInstance().drawDirectedPrimitiveLit("cylinder", sl[i], sl[i + 1], 0.005f, (i % 2) ? glm::vec4(0.2f, 0.2f, 0.2f, 1.f) : glm::vec4(0.8f, 0.8f, 0.8f, 1.f), glm::vec4(0.f));
		}
	}

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

	for (int i = 0; i < m_vvec3Pts.size() - 1; ++i)
	{
		glm::vec3 transPt(m_pVFG->getTransformRawDomainToVolume() * glm::vec4(m_vvec3Pts[i], 1.f));
		Renderer::getInstance().drawDirectedPrimitive("cylinder", transPt, m_pVFG->getTransformRawDomainToVolume() * glm::vec4(m_vvec3Pts[i + 1], 1.f), 0.001f, glm::vec4(1.f, 0.f, 0.f, 1.f));
		//Renderer::getInstance().drawText(
		//	std::to_string(i),
		//	glm::vec4(1.f),
		//	transPt,
		//	utils::getBillBoardTransform(transPt, m_pTDM->getHMDToWorldTransform()[3], glm::vec3(0.f, 1.f, 0.f), true),
		//	0.01f,
		//	Renderer::TextSizeDim::HEIGHT
		//);
	}
}

void StudyTrialScene::generateStreamLines()
{
	m_vvvec3RawStreamlines.clear();

	int gridRes = 2;
	// 4x4x4 regularly-spaced seeding grid within volume
	for (int i = 0; i < gridRes; ++i)
		for (int j = 0; j < gridRes; ++j)
			for (int k = 0; k < gridRes; ++k)
				m_vvvec3RawStreamlines.push_back(m_pVFG->getStreamline(glm::vec3(-1.f + (2.f/(gridRes + 1.f))) + (2.f / (gridRes + 1.f)) * glm::vec3(i, j, k), 1.f / 32.f, 100, 0.1f));


	// For each streamline segment
	// construct local coordinate frame matrix using orthogonal axes u, v, w; where the w axis is the segment (Point_[n+1] - Point_n) itself
	// the u & v axes are unit vectors scaled to the radius of the tube
	// except for the first and last, each circular 'rib' will be on the uv-plane of the averaged coordinate frames between the two connected segments

	// make unit circle for 'rib' that will be moved along streamline
	int numSegments = 8;
	std::vector<glm::vec3> circleVerts;
	for (int i = 0; i <= numSegments; ++i)
	{
		float angle = ((float)i / (float)(numSegments)) * glm::two_pi<float>();
		circleVerts.push_back(glm::vec3(sin(angle), cos(angle), 0.f));
	}

	std::vector<Renderer::PrimVert> verts;
	std::vector<GLushort> inds;
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

		for (size_t i = 0; i < ribOrientations.size(); ++i)
		{
			glm::mat4 xform(glm::toMat3(ribOrientations[i]));
			xform[3] = glm::vec4(sl[i], 1.f);

			for (int j = 0; j < circleVerts.size(); ++j)
			{
				Renderer::PrimVert pv;
				pv.p = glm::vec3(xform * glm::vec4(circleVerts[j] * 0.005f, 1.f));
				pv.n = glm::normalize(pv.p - sl[i]);
				pv.c = glm::vec4(1.f);
				pv.t = glm::vec2(j / (circleVerts.size() - 1), i);
			}

			int base = inds.size();
			if (i < ribOrientations.size() - 1)
			{

			}
		}
	}
	//
	//std::vector<GLushort> inds;
	//
	//
	//size_t baseInd = verts.size();
	//
	//// Base endcap
	//verts.push_back(Renderer::PrimVert({ glm::vec3(0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), glm::vec2(0.5f, 0.5f) }));
	//for (int i = 0; i < numSegments; ++i)
	//{
	//	float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
	//	verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec4(1.f), (glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f }));
	//
	//	if (i > 0)
	//	{
	//		inds.push_back(baseInd);
	//		inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 2);
	//		inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);
	//	}
	//}
	//inds.push_back(baseInd);
	//inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);
	//inds.push_back(baseInd + 1);
	//
	//// Distal endcap
	//verts.push_back(PrimVert({ glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), glm::vec2(0.5f, 0.5f) }));
	//for (int i = 0; i < numSegments; ++i)
	//{
	//	float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
	//	verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec4(1.f), (glm::vec2(sin(angle), cos(angle)) + 1.f) / 2.f }));
	//
	//	if (i > 0)
	//	{
	//		inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - (i + 2)); // ctr pt of endcap
	//		inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);
	//		inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 2);
	//	}
	//}
	//inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - (numSegments + 1));
	//inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - (numSegments));
	//inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);
	//
	//// Shaft
	//for (int i = 0; i < numSegments; ++i)
	//{
	//	float angle = ((float)i / (float)(numSegments - 1)) * glm::two_pi<float>();
	//	verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 0.f), glm::vec3(sin(angle), cos(angle), 0.f), glm::vec4(1.f), glm::vec2((float)i / (float)(numSegments - 1), 0.f) }));
	//
	//	verts.push_back(PrimVert({ glm::vec3(sin(angle), cos(angle), 1.f), glm::vec3(sin(angle), cos(angle), 0.f), glm::vec4(1.f), glm::vec2((float)i / (float)(numSegments - 1), 1.f) }));
	//
	//	if (i > 0)
	//	{
	//		inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 4);
	//		inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 3);
	//		inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 2);
	//
	//		inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 2);
	//		inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 3);
	//		inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);
	//	}
	//}
	//inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 2);
	//inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - numSegments * 2);
	//inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);
	//
	//inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - numSegments * 2);
	//inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - numSegments * 2 + 1);
	//inds.push_back(static_cast<GLushort>(verts.size() - baseInd) - 1);
}

glm::quat StudyTrialScene::getSegmentOrientationMatrixNormalized(glm::vec3 segmentDirection)
{
	glm::vec3 up(0.f, 1.f, 0.f);
	glm::vec3 w(glm::normalize(segmentDirection));
	glm::vec3 u(glm::normalize(glm::cross(up, w)));
	glm::vec3 v(glm::normalize(glm::cross(w, u)));
	return glm::toQuat(glm::mat3(u, v, w));
}
