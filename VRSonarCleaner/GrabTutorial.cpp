#include "GrabTutorial.h"

#include "BehaviorManager.h"
#include "InfoBoxManager.h"
#include "GrabDataVolumeBehavior.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/random.hpp>
#include "Renderer.h"
#include "TaskCompleteBehavior.h"
#include <algorithm>

GrabTutorial::GrabTutorial(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pDemoVolume(NULL)
	, m_pGoalVolume(NULL)
	, m_bWaitForTriggerRelease(true)
{
	srand(time(NULL));
	m_tpTimestamp = std::chrono::high_resolution_clock::now();
}


GrabTutorial::~GrabTutorial()
{
	cleanup();
}

void GrabTutorial::init()
{
	glm::vec3 grabVolPosition = glm::vec3(0.f, 1.1f, 0.f);
	glm::quat grabVolOrientation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	glm::vec3 grabVolSize = glm::vec3(0.5f);

	m_pDemoVolume = new DataVolume(grabVolPosition, grabVolOrientation, grabVolSize);

	glm::vec3 goalVolPosition = glm::vec3(1.f, 1.1f, 0.f);
	glm::quat goalVolOrientation = glm::angleAxis(glm::radians(glm::linearRand(0.f, 180.f)), glm::sphericalRand(1.f));
	glm::vec3 goalVolSize = glm::vec3(0.65f);
	
	m_pGoalVolume = new DataVolume(goalVolPosition, goalVolOrientation, goalVolSize);
	
	BehaviorManager::getInstance().addBehavior("Grab", new GrabDataVolumeBehavior(m_pTDM, m_pDemoVolume));

	m_bInitialized = true;
}

void GrabTutorial::update()
{
	if (!m_pTDM->getSecondaryController())
		return;

	if (m_bWaitForTriggerRelease && !m_pTDM->getSecondaryController()->isTriggerEngaged())
		m_bWaitForTriggerRelease = false;

	m_pDemoVolume->update();
	m_pGoalVolume->update();

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
	else if (!m_bWaitForTriggerRelease && m_pTDM->getSecondaryController()->justUnclickedTrigger())
	{
		if (checkVolBounds())
		{
			BehaviorManager::getInstance().removeBehavior("Grab");

			TaskCompleteBehavior* tcb = new TaskCompleteBehavior(m_pTDM);
			tcb->init();
			BehaviorManager::getInstance().addBehavior("Done", tcb);
		}
	}
}

void GrabTutorial::draw()
{
	std::chrono::duration<float> elapsedTime(std::chrono::high_resolution_clock::now() - m_tpTimestamp);
	float cycleTime = 1.f;
	float amt = (sinf(glm::two_pi<float>() * fmodf(elapsedTime.count(), cycleTime) / cycleTime) + 1.f) * 0.5f;

	bool volumeHasBeenMoved = m_pDemoVolume->getOriginalPosition() != m_pDemoVolume->getPosition();

	glm::vec4 goalVolBackingColor;

	if (BehaviorManager::getInstance().getBehavior("Done"))
		goalVolBackingColor = glm::vec4(0.4f, 0.4f, 0.4f, 1.f);
	else if (!m_bWaitForTriggerRelease && m_pTDM->getSecondaryController()->isTriggerClicked())
	{
		if (checkVolBounds())
			goalVolBackingColor = glm::vec4(0.2f, 0.8f, 0.2f, 1.f);
		else
			goalVolBackingColor = glm::vec4(0.8f, 0.8f, 0.2f, 1.f);
	}
	else
		goalVolBackingColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);

	goalVolBackingColor.a = 0.5f;

	m_pGoalVolume->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), goalVolBackingColor, 2.f);
	m_pGoalVolume->drawBBox(glm::vec4(0.f, 0.f, 0.f, 1.f), 0.f);

	glm::vec4 dvColor = volumeHasBeenMoved ? glm::vec4(0.15f, 0.21f, 0.31f, 1.f) : glm::mix(glm::vec4(0.15f, 0.21f, 0.31f, 1.f), glm::vec4(0.85f, 0.81f, 0.31f, 1.f), amt);
	m_pDemoVolume->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), dvColor, 2.f);
	m_pDemoVolume->drawBBox(glm::vec4(0.f, 0.f, 0.f, 1.f), 0.f);

	if (BehaviorManager::getInstance().getBehavior("Done") == nullptr)
	{
		float dvMaxSide = std::max(std::max(m_pDemoVolume->getDimensions().x, m_pDemoVolume->getDimensions().y), m_pDemoVolume->getDimensions().z);
		float tmp = std::sqrt(dvMaxSide * dvMaxSide * 2.f);
		float dvOffset = std::sqrt(tmp * tmp + dvMaxSide * dvMaxSide) * 0.5f;
		glm::mat4 dvPromptTrans = Renderer::getBillBoardTransform(m_pDemoVolume->getPosition() + dvOffset * glm::vec3(0.f, 1.f, 0.f), m_pTDM->getHMDToWorldTransform()[3], glm::vec3(0.f, 1.f, 0.f), true);

		Renderer::getInstance().drawText(
			"Data Volume",
			volumeHasBeenMoved ? glm::vec4(0.7f, 0.7f, 0.7f, 1.f) : dvColor,
			dvPromptTrans[3],
			glm::quat(dvPromptTrans),
			dvOffset * 2.f,
			Renderer::TextSizeDim::WIDTH,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_BOTTOM
		);

		if (!volumeHasBeenMoved)
		{
			glm::mat4 grabTriggerTrans = m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(-0.025f, -0.031f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
			std::string grabInitialText("Press the trigger!");
			glm::vec2 dims = Renderer::getInstance().getTextDimensions(grabInitialText, 0.2f, Renderer::WIDTH);
			Renderer::getInstance().drawText(
				grabInitialText,
				dvColor,
				grabTriggerTrans[3],
				glm::quat(grabTriggerTrans),
				0.2f,
				Renderer::TextSizeDim::WIDTH,
				Renderer::TextAlignment::CENTER,
				Renderer::TextAnchor::CENTER_RIGHT
			);

			Renderer::getInstance().drawConnector(
				grabTriggerTrans[3] - dims.x * grabTriggerTrans[0],
				m_pDemoVolume->getPosition(),
				0.001f,
				dvColor
			);

			Renderer::getInstance().drawConnector(
				grabTriggerTrans[3],
				(m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, -0.031f, 0.05f)))[3],
				0.001f,
				dvColor
			);
		}
		else
		{
			float goalMaxSide = std::max(std::max(m_pGoalVolume->getDimensions().x, m_pGoalVolume->getDimensions().y), m_pGoalVolume->getDimensions().z);
			float tmp1 = std::sqrt(goalMaxSide * goalMaxSide * 2.f);
			float goalOffset = std::sqrt(tmp1 * tmp1 + goalMaxSide * goalMaxSide) * 0.5f;
			glm::mat4 goalTrans = Renderer::getBillBoardTransform(m_pGoalVolume->getPosition() - glm::vec3(0.f, goalOffset, 0.f), m_pTDM->getHMDToWorldTransform()[3], glm::vec3(0.f, 1.f, 0.f), true);

			Renderer::getInstance().drawText(
				"GOAL",
				glm::vec4(0.2f, 0.7f, 0.2f, 1.f),
				goalTrans[3],
				glm::quat(goalTrans),
				goalOffset * 2.f,
				Renderer::TextSizeDim::WIDTH,
				Renderer::TextAlignment::CENTER,
				Renderer::TextAnchor::CENTER_TOP
			);

			glm::mat4 goalPromptTrans = Renderer::getBillBoardTransform(m_pGoalVolume->getPosition() + glm::vec3(0.f, goalOffset, 0.f), m_pTDM->getHMDToWorldTransform()[3], glm::vec3(0.f, 1.f, 0.f), true);

			Renderer::getInstance().drawText(
				"Align and place\nthe data volume\ninside of the goal!",
				glm::vec4(0.7f, 0.7f, 0.7f, 1.f),
				goalPromptTrans[3],
				glm::quat(goalPromptTrans),
				2.f,
				Renderer::TextSizeDim::WIDTH,
				Renderer::TextAlignment::CENTER,
				Renderer::TextAnchor::CENTER_BOTTOM
			);
		}
	}
}

void GrabTutorial::cleanup()
{
	if (m_bInitialized)
	{
		if (m_pDemoVolume)
			delete m_pDemoVolume;

		if (m_pGoalVolume)
			delete m_pGoalVolume;

		BehaviorManager::getInstance().removeBehavior("Grab");
		BehaviorManager::getInstance().removeBehavior("Done");

		m_bInitialized = false;
	}
}

bool GrabTutorial::checkVolBounds()
{
	glm::vec4 bbMin(glm::vec3(-0.5f), 1.f);
	glm::vec4 bbMax(glm::vec3(0.5f), 1.f);

	std::vector<glm::vec4> ptsToCheck;

	ptsToCheck.push_back(glm::vec4(bbMin.x, bbMin.y, bbMin.z, 1.f));
	ptsToCheck.push_back(glm::vec4(bbMax.x, bbMin.y, bbMin.z, 1.f));
	ptsToCheck.push_back(glm::vec4(bbMin.x, bbMax.y, bbMin.z, 1.f));
	ptsToCheck.push_back(glm::vec4(bbMin.x, bbMin.y, bbMax.z, 1.f));
	ptsToCheck.push_back(glm::vec4(bbMax.x, bbMax.y, bbMin.z, 1.f));
	ptsToCheck.push_back(glm::vec4(bbMax.x, bbMin.y, bbMax.z, 1.f));
	ptsToCheck.push_back(glm::vec4(bbMin.x, bbMax.y, bbMax.z, 1.f));
	ptsToCheck.push_back(glm::vec4(bbMax.x, bbMax.y, bbMax.z, 1.f));

	glm::mat4 demoVolToWorld = m_pDemoVolume->getCurrentVolumeTransform();
	glm::mat4 worldToGoalVol = glm::inverse(m_pGoalVolume->getCurrentVolumeTransform());
	glm::mat4 demoVolToGoalVol = worldToGoalVol * demoVolToWorld;

	for (auto const &pt : ptsToCheck)
	{
		glm::vec4 ptXformed = demoVolToGoalVol * pt;

		if (ptXformed.x < bbMin.x || ptXformed.y < bbMin.y || ptXformed.z < bbMin.z ||
			ptXformed.x > bbMax.x || ptXformed.y > bbMax.y || ptXformed.z > bbMax.z)
			return false;
	}

	return true;
}
