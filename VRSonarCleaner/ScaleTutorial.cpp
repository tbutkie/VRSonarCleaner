#include "ScaleTutorial.h"

#include "BehaviorManager.h"
#include "InfoBoxManager.h"
#include "ScaleDataVolumeBehavior.h"
#include "GrabDataVolumeBehavior.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/random.hpp>
#include "Renderer.h"
#include "TaskCompleteBehavior.h"
#include "utilities.h"

ScaleTutorial::ScaleTutorial(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_pDemoVolume(NULL)
	, m_pGoalVolume(NULL)
	, m_bWaitForTriggerRelease(true)
{
	//srand(time(NULL));
	m_tpTimestamp = std::chrono::high_resolution_clock::now();
}


ScaleTutorial::~ScaleTutorial()
{
	cleanup();
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
		if (static_cast<TaskCompleteBehavior*>(done)->restartRequested())
		{
			cleanup();
			init();
		}
	}
	else if (!m_bWaitForTriggerRelease && (m_pTDM->getSecondaryController()->justUnclickedTrigger() || m_pTDM->getSecondaryController()->justUnpressedGrip() || m_pTDM->getPrimaryController()->justUnpressedGrip()))
	{
		if (checkVolBounds())
		{
			BehaviorManager::getInstance().removeBehavior("Grab");
			BehaviorManager::getInstance().removeBehavior("Scale");

			TaskCompleteBehavior* tcb = new TaskCompleteBehavior(m_pTDM);
			tcb->init();
			BehaviorManager::getInstance().addBehavior("Done", tcb);
		}
	}
}

void ScaleTutorial::draw()
{
	std::chrono::duration<float> elapsedTime(std::chrono::high_resolution_clock::now() - m_tpTimestamp);
	float cycleTime = 1.f;
	float amt = (sinf(glm::two_pi<float>() * fmodf(elapsedTime.count(), cycleTime) / cycleTime) + 1.f) * 0.5f;

	bool volumeHasBeenScaled = m_pDemoVolume->getOriginalDimensions() != m_pDemoVolume->getDimensions();

	glm::vec4 goalVolBackingColor;

	if (BehaviorManager::getInstance().getBehavior("Done"))
		goalVolBackingColor = glm::vec4(0.4f, 0.4f, 0.4f, 1.f);
	else if (!m_bWaitForTriggerRelease && volumeHasBeenScaled && (m_pTDM->getSecondaryController()->isTriggerClicked() || (m_pTDM->getPrimaryController()->isGripButtonPressed() && m_pTDM->getSecondaryController()->isGripButtonPressed())))
	{
		if (checkVolBounds())
			goalVolBackingColor = glm::vec4(0.2f, 0.8f, 0.2f, 1.f);
		else
			goalVolBackingColor = glm::mix(glm::vec4(0.15f, 0.21f, 0.31f, 1.f), glm::vec4(0.85f, 0.81f, 0.31f, 1.f), amt);
	}
	else
		goalVolBackingColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.f);

	goalVolBackingColor.a = 0.5f;

	m_pGoalVolume->setBackingColor(goalVolBackingColor);
	m_pGoalVolume->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), 2.f);
	m_pGoalVolume->drawBBox(0.f);

	glm::vec4 dvColor = volumeHasBeenScaled ? glm::vec4(0.15f, 0.21f, 0.31f, 1.f) : glm::mix(glm::vec4(0.15f, 0.21f, 0.31f, 1.f), glm::vec4(0.85f, 0.81f, 0.31f, 1.f), amt);
	m_pDemoVolume->setBackingColor(dvColor);
	m_pDemoVolume->drawVolumeBacking(m_pTDM->getHMDToWorldTransform(), 2.f);
	m_pDemoVolume->drawBBox(0.f);

	if (BehaviorManager::getInstance().getBehavior("Done") == nullptr)
	{
		float dvMaxSide = std::max(std::max(m_pDemoVolume->getDimensions().x, m_pDemoVolume->getDimensions().y), m_pDemoVolume->getDimensions().z);
		float tmp = std::sqrt(dvMaxSide * dvMaxSide * 2.f);
		float dvOffset = std::sqrt(tmp * tmp + dvMaxSide * dvMaxSide) * 0.5f;
		glm::mat4 dvPromptTrans = ccomutils::getBillBoardTransform(m_pDemoVolume->getPosition() + dvOffset * glm::vec3(0.f, 1.f, 0.f), m_pTDM->getHMDToWorldTransform()[3], glm::vec3(0.f, 1.f, 0.f), true);

		Renderer::getInstance().drawText(
			"Data Volume",
			volumeHasBeenScaled ? glm::vec4(0.7f, 0.7f, 0.7f, 1.f) : dvColor,
			dvPromptTrans[3],
			glm::quat(dvPromptTrans),
			dvOffset * 2.f,
			Renderer::TextSizeDim::WIDTH,
			Renderer::TextAlignment::CENTER,
			Renderer::TextAnchor::CENTER_BOTTOM
		);

		if (!volumeHasBeenScaled)
		{
			glm::vec3 midPt = m_pTDM->getPrimaryController()->getDeviceToWorldTransform()[3] + (m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3] - m_pTDM->getPrimaryController()->getDeviceToWorldTransform()[3]) * 0.5f;

			glm::mat4 scaleVolTrans = ccomutils::getBillBoardTransform(midPt, m_pTDM->getHMDToWorldTransform()[3], m_pTDM->getHMDToWorldTransform()[1], false);
			std::string scaleInitialText("Squeeze the grips!");
			glm::vec2 dims = Renderer::getInstance().getTextDimensions(scaleInitialText, 0.2f, Renderer::WIDTH);
			Renderer::getInstance().drawText(
				scaleInitialText,
				dvColor,
				scaleVolTrans[3],
				glm::quat(scaleVolTrans),
				0.2f,
				Renderer::TextSizeDim::WIDTH,
				Renderer::TextAlignment::CENTER,
				Renderer::TextAnchor::CENTER_MIDDLE
			);

			Renderer::getInstance().drawConnector(
				scaleVolTrans[3] + dims.y * 0.5f * scaleVolTrans[1],
				m_pDemoVolume->getPosition(),
				0.001f,
				dvColor
			);

			glm::vec3 primGripCtrPos = (m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, -0.015f, 0.085f)))[3];
			glm::vec3 secGripCtrPos = (m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, -0.015f, 0.085f)))[3];
			glm::vec3 gripToGripVec = secGripCtrPos - primGripCtrPos;
			bool rightHanded = glm::dot(glm::cross(glm::vec3(m_pTDM->getHMDToWorldTransform()[3]) - primGripCtrPos, gripToGripVec), glm::vec3(m_pTDM->getHMDToWorldTransform()[1])) < 0.f;

			glm::vec3 pCtrlrGripOffsetAnchorPos = (m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, -0.05f, 0.085f)))[3];
			glm::vec3 pCtrlrRightGripAnchorPos = m_pTDM->getPrimaryController()->getRightGripPoint();
			glm::vec3 pCtrlrLeftGripAnchorPos = m_pTDM->getPrimaryController()->getLeftGripPoint();
			glm::vec3 sCtrlrGripOffsetAnchorPos = (m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, -0.05f, 0.085f)))[3];
			glm::vec3 sCtrlrRightGripAnchorPos = m_pTDM->getSecondaryController()->getRightGripPoint();
			glm::vec3 sCtrlrLeftGripAnchorPos = m_pTDM->getSecondaryController()->getLeftGripPoint();

			Renderer::getInstance().drawConnector(
				scaleVolTrans[3] - dims.x * 0.5f * scaleVolTrans[0],
				rightHanded ? sCtrlrGripOffsetAnchorPos : pCtrlrGripOffsetAnchorPos,
				0.001f,
				dvColor
			);
			Renderer::getInstance().drawConnector(
				rightHanded ? sCtrlrGripOffsetAnchorPos : pCtrlrGripOffsetAnchorPos,
				rightHanded ? sCtrlrRightGripAnchorPos : pCtrlrRightGripAnchorPos,
				0.001f,
				dvColor
			);
			Renderer::getInstance().drawConnector(
				rightHanded ? sCtrlrGripOffsetAnchorPos : pCtrlrGripOffsetAnchorPos,
				rightHanded ? sCtrlrLeftGripAnchorPos : pCtrlrLeftGripAnchorPos,
				0.001f,
				dvColor
			);

			Renderer::getInstance().drawConnector(
				scaleVolTrans[3] + dims.x * 0.5f * scaleVolTrans[0],
				rightHanded ? pCtrlrGripOffsetAnchorPos : sCtrlrGripOffsetAnchorPos,
				0.001f,
				dvColor
			);
			Renderer::getInstance().drawConnector(
				rightHanded ? pCtrlrGripOffsetAnchorPos : sCtrlrGripOffsetAnchorPos,
				rightHanded ? pCtrlrRightGripAnchorPos : sCtrlrRightGripAnchorPos,
				0.001f,
				dvColor
			);
			Renderer::getInstance().drawConnector(
				rightHanded ? pCtrlrGripOffsetAnchorPos : sCtrlrGripOffsetAnchorPos,
				rightHanded ? pCtrlrLeftGripAnchorPos : sCtrlrLeftGripAnchorPos,
				0.001f,
				dvColor
			);
		}
		else
		{
			float goalMaxSide = std::max(std::max(m_pGoalVolume->getDimensions().x, m_pGoalVolume->getDimensions().y), m_pGoalVolume->getDimensions().z);
			float tmp1 = std::sqrt(goalMaxSide * goalMaxSide * 2.f);
			float goalOffset = std::sqrt(tmp1 * tmp1 + goalMaxSide * goalMaxSide) * 0.5f;
			glm::mat4 goalTrans = ccomutils::getBillBoardTransform(m_pGoalVolume->getPosition() - glm::vec3(0.f, goalOffset, 0.f), m_pTDM->getHMDToWorldTransform()[3], glm::vec3(0.f, 1.f, 0.f), true);

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

			glm::mat4 goalPromptTrans = ccomutils::getBillBoardTransform(m_pGoalVolume->getPosition() + glm::vec3(0.f, goalOffset, 0.f), m_pTDM->getHMDToWorldTransform()[3], glm::vec3(0.f, 1.f, 0.f), true);

			Renderer::getInstance().drawText(
				"Scale the data volume\n and place it inside of the goal!",
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

void ScaleTutorial::cleanup()
{
	if (m_bInitialized)
	{
		if (m_pDemoVolume)
			delete m_pDemoVolume;

		if (m_pGoalVolume)
			delete m_pGoalVolume;

		BehaviorManager::getInstance().removeBehavior("Grab");
		BehaviorManager::getInstance().removeBehavior("Scale");
		BehaviorManager::getInstance().removeBehavior("Done");

		m_bInitialized = false;
	}
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

	glm::mat4 demoVolToWorld = m_pDemoVolume->getTransformVolume();
	glm::mat4 worldToGoalVol = glm::inverse(m_pGoalVolume->getTransformVolume());
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
