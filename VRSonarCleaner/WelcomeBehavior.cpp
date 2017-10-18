#include "WelcomeBehavior.h"

#include "InfoBoxManager.h"
#include <gtc/matrix_transform.hpp>

WelcomeBehavior::WelcomeBehavior(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
{
}


WelcomeBehavior::~WelcomeBehavior()
{
	if (m_bInitialized)
	{
		InfoBoxManager::getInstance().removeInfoBox("Welcome");
		InfoBoxManager::getInstance().removeInfoBox("Activate Label (Primary)");
		InfoBoxManager::getInstance().removeInfoBox("Activate Label (Secondary)");
	}
}

void WelcomeBehavior::init()
{
	InfoBoxManager::getInstance().addInfoBox(
		"Welcome",
		"welcome.png",
		1.f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -2.f)),
		InfoBoxManager::RELATIVE_TO::HMD,
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

void WelcomeBehavior::update()
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getSecondaryController())
		return;

	if ((m_pTDM->getPrimaryController()->justClickedTrigger() && m_pTDM->getSecondaryController()->isTriggerClicked()) ||
		(m_pTDM->getSecondaryController()->justClickedTrigger() && m_pTDM->getPrimaryController()->isTriggerClicked()))
		m_bActive = false;
}

void WelcomeBehavior::draw()
{
}
