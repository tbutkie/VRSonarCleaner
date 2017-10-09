#include "ScaleTutorial.h"

#include "BehaviorManager.h"
#include "InfoBoxManager.h"
#include "ScaleDataVolumeBehavior.h"
#include "GrabDataVolumeBehavior.h"
#include <shared/glm/gtc/matrix_transform.hpp>
#include <shared/glm/gtc/random.hpp>
#include "Renderer.h"
#include "TaskCompleteBehavior.h"

ScaleTutorial::ScaleTutorial(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pDemoVolume(NULL)
	, m_pGoalVolume(NULL)
	, m_bWaitForTriggerRelease(true)
{
}


ScaleTutorial::~ScaleTutorial()
{
	if (m_bInitialized)
	{
		if (m_pDemoVolume)
			delete m_pDemoVolume;
		
		if (m_pGoalVolume)
			delete m_pGoalVolume;

		InfoBoxManager::getInstance().removeInfoBox("Scale Tut");
		InfoBoxManager::getInstance().removeInfoBox("Scale Tut Goal");
		BehaviorManager::getInstance().removeBehavior("Grab");
		BehaviorManager::getInstance().removeBehavior("Scale");
		BehaviorManager::getInstance().removeBehavior("Done");
	}
}

void ScaleTutorial::init()
{
	glm::vec3 grabVolPosition = glm::vec3(0.f, 1.1f, 0.f);
	glm::quat grabVolOrientation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	glm::vec3 grabVolSize = glm::vec3(0.5f);

	m_pDemoVolume = new DataVolume(grabVolPosition, grabVolOrientation, grabVolSize);

	glm::vec3 goalVolPosition = glm::vec3(1.f, 1.1f, 0.f);
	glm::quat goalVolOrientation = glm::angleAxis(glm::radians(glm::linearRand(0.f, 180.f)), glm::sphericalRand(1.f));
	glm::vec3 goalVolSize = glm::vec3(0.25f);
	
	m_pGoalVolume = new DataVolume(goalVolPosition, goalVolOrientation, goalVolSize);

	InfoBoxManager::getInstance().addInfoBox(
		"Scale Tut",
		"scalinginstructions.png",
		0.25f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -0.1f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::PRIMARY_CONTROLLER,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Scale Tut Goal",
		"scaletutgoal.png",
		1.f,
		glm::translate(glm::mat4(), goalVolPosition + 1.f * glm::vec3(0.f, 1.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::WORLD,
		true);

	BehaviorManager::getInstance().addBehavior("Grab", new GrabDataVolumeBehavior(m_pTDM, m_pDemoVolume));
	BehaviorManager::getInstance().addBehavior("Scale", new ScaleDataVolumeBehavior(m_pTDM, m_pDemoVolume));

	m_bInitialized = true;
}

void ScaleTutorial::update()
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getSecondaryController())
		return;

	if (m_bWaitForTriggerRelease && !m_pTDM->getSecondaryController()->isTriggerEngaged() && !m_pTDM->getPrimaryController()->isGripButtonPressed() && !m_pTDM->getSecondaryController()->isGripButtonPressed())
		m_bWaitForTriggerRelease = false;

	m_pDemoVolume->update();
	m_pGoalVolume->update();

	BehaviorBase* done = BehaviorManager::getInstance().getBehavior("Done");
	if (done)
	{
		done->update();
		if (!done->isActive())
			m_bActive = false;
	}
	else if (!m_bWaitForTriggerRelease && (m_pTDM->getSecondaryController()->justUnclickedTrigger() || m_pTDM->getSecondaryController()->justUnpressedGrip() || m_pTDM->getPrimaryController()->justUnpressedGrip()))
	{
		if (checkVolBounds())
		{
			BehaviorManager::getInstance().removeBehavior("Grab");
			BehaviorManager::getInstance().removeBehavior("Scale");
			InfoBoxManager::getInstance().removeInfoBox("Scale Tut");
			InfoBoxManager::getInstance().removeInfoBox("Scale Tut Goal");

			TaskCompleteBehavior* tcb = new TaskCompleteBehavior(m_pTDM);
			tcb->init();
			BehaviorManager::getInstance().addBehavior("Done", tcb);
		}
	}
}

void ScaleTutorial::draw()
{
	glm::vec4 goalVolBackingColor;

	if (BehaviorManager::getInstance().getBehavior("Done"))
		goalVolBackingColor = glm::vec4(0.4f, 0.4f, 0.4f, 1.f);
	else if (!m_bWaitForTriggerRelease && (m_pTDM->getSecondaryController()->isTriggerClicked() || (m_pTDM->getPrimaryController()->isGripButtonPressed() && m_pTDM->getSecondaryController()->isGripButtonPressed())))
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

	m_pDemoVolume->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), glm::vec4(0.15f, 0.21f, 0.31f, 1.f), 2.f);
	m_pDemoVolume->drawBBox(glm::vec4(0.f, 0.f, 0.f, 1.f), 0.f);
	
}

bool ScaleTutorial::checkVolBounds()
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
