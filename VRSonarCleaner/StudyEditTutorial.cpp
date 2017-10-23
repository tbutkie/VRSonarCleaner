#include "StudyEditTutorial.h"

#include "BehaviorManager.h"
#include "InfoBoxManager.h"
#include "ScaleDataVolumeBehavior.h"
#include "GrabDataVolumeBehavior.h"
#include "PointCleanProbe.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/random.hpp>
#include "Renderer.h"
#include "TaskCompleteBehavior.h"

StudyEditTutorial::StudyEditTutorial(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pDemoVolume(NULL)
{
}


StudyEditTutorial::~StudyEditTutorial()
{
	cleanup();
}

void StudyEditTutorial::init()
{
	glm::vec3 tablePosition = glm::vec3(0.f, 1.1f, 0.f);
	glm::quat tableOrientation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	glm::vec3 tableSize = glm::vec3(1.f, 1.f, 0.5f);

	m_pDemoVolume = new DataVolume(tablePosition, tableOrientation, tableSize);
	
	m_pColorScaler = new ColorScaler();
	m_pColorScaler->setColorMode(ColorScaler::Mode::ColorScale_BiValue);
	m_pColorScaler->setBiValueColorMap(ColorScaler::ColorMap_BiValued::Custom);

	m_pDemoCloud = new SonarPointCloud(m_pColorScaler, "tutorial_points.csv", SonarPointCloud::XYZF);

	m_pDemoVolume->add(m_pDemoCloud);
	
	refreshColorScale();

	BehaviorManager::getInstance().addBehavior("Scale", new ScaleDataVolumeBehavior(m_pTDM, m_pDemoVolume));
	BehaviorManager::getInstance().addBehavior("Grab", new GrabDataVolumeBehavior(m_pTDM, m_pDemoVolume));
	m_pProbe = new PointCleanProbe(m_pTDM, m_pDemoVolume, vr::VRSystem());
	BehaviorManager::getInstance().addBehavior("Editing", m_pProbe);

	m_bInitialized = true;
}

void StudyEditTutorial::update()
{
	if (!m_pTDM->getPrimaryController())
		return;
	
	m_pDemoVolume->update();
	m_pDemoCloud->update();

	BehaviorBase* done = BehaviorManager::getInstance().getBehavior("Done");
	if (done)
	{
		done->update();
		if (!done->isActive())
			m_bActive = false;
		if (static_cast<TaskCompleteBehavior*>(done)->restartRequested())
		{
			cleanup();
			init();
		}
	}
	else //if (m_pTDM->getPrimaryController()->isTriggerClicked() || m_pTDM->getPrimaryController()->justUnclickedTrigger())
	{
		bool allDone = true;
		m_vvec3BadPoints.clear();

		for (int i = 0; i < m_pDemoCloud->getPointCount(); ++i)
		{
			if (m_pDemoCloud->getPointDepthTPU(i) == 1.f && m_pDemoCloud->getPointMark(i) != 1)
			{
				allDone = false;
				if (m_vvec3BadPoints.size() == 5u)
					break;
				else
					m_vvec3BadPoints.push_back(m_pDemoVolume->getCurrentDataTransform(m_pDemoCloud) * glm::vec4(m_pDemoCloud->getAdjustedPointPosition(i), 1.f));
			}
		}
		if (allDone)
		{
			BehaviorManager::getInstance().removeBehavior("Scale");
			BehaviorManager::getInstance().removeBehavior("Grab");
			BehaviorManager::getInstance().removeBehavior("Editing");

			m_pDemoVolume->resetPositionAndOrientation();

			TaskCompleteBehavior* tcb = new TaskCompleteBehavior(m_pTDM);
			tcb->init();
			BehaviorManager::getInstance().addBehavior("Done", tcb);
		}
	}
}

void StudyEditTutorial::draw()
{
	m_pDemoVolume->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), glm::vec4(0.15f, 0.21f, 0.31f, 1.f), 2.f);
	m_pDemoVolume->drawBBox(glm::vec4(0.f, 0.f, 0.f, 1.f), 0.f);

	Renderer::RendererSubmission rs;
	rs.glPrimitiveType = GL_POINTS;
	rs.shaderName = "flat";
	rs.VAO = m_pDemoCloud->getVAO();
	rs.vertCount = m_pDemoCloud->getPointCount();
	rs.indexType = GL_UNSIGNED_INT;
	rs.modelToWorldTransform = m_pDemoVolume->getCurrentDataTransform(m_pDemoCloud);

	Renderer::getInstance().addToDynamicRenderQueue(rs);
	

	std::chrono::duration<float> elapsedTime(std::chrono::high_resolution_clock::now() - m_tpTimestamp);
	float cycleTime = 1.f;
	float amt = (sinf(glm::two_pi<float>() * fmodf(elapsedTime.count(), cycleTime) / cycleTime) + 1.f) * 0.5f;

	if (BehaviorManager::getInstance().getBehavior("Done") == nullptr)
	{
		float dvMaxSide = std::max(std::max(m_pDemoVolume->getDimensions().x, m_pDemoVolume->getDimensions().y), m_pDemoVolume->getDimensions().z);
		float tmp = std::sqrt(dvMaxSide * dvMaxSide * 2.f);
		float dvOffset = std::sqrt(tmp * tmp + dvMaxSide * dvMaxSide) * 0.5f;
		glm::mat4 dvPromptTrans = Renderer::getBillBoardTransform(m_pDemoVolume->getPosition() + dvOffset * glm::vec3(0.f, 1.f, 0.f), m_pTDM->getHMDToWorldTransform()[3], glm::vec3(0.f, 1.f, 0.f), true);

		Renderer::getInstance().drawText(
			"Bad Data Points",
			glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
			dvPromptTrans[3],
			glm::quat(dvPromptTrans),
			dvOffset * 2.f,
			Renderer::TextSizeDim::WIDTH,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_BOTTOM
		);

		for (auto &pt : m_vvec3BadPoints)
		{
			Renderer::getInstance().drawConnector(
				dvPromptTrans[3],
				pt,
				0.001f,
				glm::vec4(0.7f, 0.7f, 0.7f, 1.f)
			);
		}
	}
}

void StudyEditTutorial::cleanup()
{
	if (m_bInitialized)
	{
		delete m_pDemoCloud;

		delete m_pDemoVolume;

		delete m_pColorScaler;

		BehaviorManager::getInstance().removeBehavior("Scale");
		BehaviorManager::getInstance().removeBehavior("Grab");
		BehaviorManager::getInstance().removeBehavior("Editing");
		BehaviorManager::getInstance().removeBehavior("Done");

		m_bInitialized = false;
	}
}

void StudyEditTutorial::refreshColorScale()
{
	if (!m_pDemoCloud)
		return;

	m_pColorScaler->resetMinMaxForColorScale(m_pDemoVolume->getMinDataBound().z, m_pDemoVolume->getMaxDataBound().z);
	m_pColorScaler->resetBiValueScaleMinMax(
		m_pDemoCloud->getMinDepthTPU(),
		m_pDemoCloud->getMaxDepthTPU(),
		m_pDemoCloud->getMinPositionalTPU(),
		m_pDemoCloud->getMaxPositionalTPU()
	);

	// apply new color scale
	m_pDemoCloud->resetAllMarks();
}