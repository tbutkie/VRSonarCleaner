#include "StudyTrialScene.h"
#include "Renderer.h"
#include "BehaviorManager.h"
#include "GrabObjectBehavior.h"
#include "ScaleDataVolumeBehavior.h"


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
}

void StudyTrialScene::generateStreamLines()
{
	m_vvvec3RawStreamlines.clear();

	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			for (int k = 0; k < 4; ++k)
			{
				m_vvvec3RawStreamlines.push_back(m_pVFG->getStreamline(glm::vec3(-1.f + (2.f/5.f)) + (2.f/5.f) * glm::vec3(i, j, k), 1.f / 32.f, 100, 0.1f));
			}
}
