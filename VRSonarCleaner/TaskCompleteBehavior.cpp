#include "TaskCompleteBehavior.h"

#include "BehaviorManager.h"
#include "InfoBoxManager.h"
#include "PointCleanProbe.h"
#include <shared/glm/gtc/matrix_transform.hpp>
#include "Renderer.h"
#include "GrabDataVolumeBehavior.h"

TaskCompleteBehavior::TaskCompleteBehavior(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
{
}


TaskCompleteBehavior::~TaskCompleteBehavior()
{
	if (m_bInitialized)
	{
		InfoBoxManager::getInstance().removeInfoBox("Task Complete");
		InfoBoxManager::getInstance().removeInfoBox("Activate Label (Primary)");
		InfoBoxManager::getInstance().removeInfoBox("Activate Label (Secondary)");
	}
}

void TaskCompleteBehavior::init()
{
	InfoBoxManager::getInstance().addInfoBox(
		"Task Complete",
		"taskcomplete.png",
		0.25f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -0.1f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::SECONDARY_CONTROLLER,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Activate Label (Primary)",
		"activaterightlabel.png",
		0.075f,
		glm::translate(glm::mat4(), glm::vec3(-0.05f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::PRIMARY_CONTROLLER,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Activate Label (Secondary)",
		"activateleftlabel.png",
		0.075f,
		glm::translate(glm::mat4(), glm::vec3(0.05f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::SECONDARY_CONTROLLER,
		false);

	m_bInitialized = true;
}

void TaskCompleteBehavior::update()
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getSecondaryController())
		return;

	if ((m_pTDM->getPrimaryController()->justClickedTrigger() && m_pTDM->getSecondaryController()->isTriggerClicked()) ||
		(m_pTDM->getSecondaryController()->justClickedTrigger() && m_pTDM->getPrimaryController()->isTriggerClicked()))
		m_bActive = false;
}

void TaskCompleteBehavior::draw()
{
}