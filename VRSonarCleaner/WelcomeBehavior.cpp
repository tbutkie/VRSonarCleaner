#include "WelcomeBehavior.h"

#include "InfoBoxManager.h"
#include <shared/glm/gtc/matrix_transform.hpp>

WelcomeBehavior::WelcomeBehavior(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
{
}


WelcomeBehavior::~WelcomeBehavior()
{
	InfoBoxManager::getInstance().removeInfoBox("Welcome");
	InfoBoxManager::getInstance().removeInfoBox("Activate Label (Primary)");
	InfoBoxManager::getInstance().removeInfoBox("Activate Label (Secondary)");
}

void WelcomeBehavior::init()
{
	InfoBoxManager::getInstance().addInfoBox(
		"Welcome",
		"cube_texture.png",
		1.f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, -2.f)),
		InfoBoxManager::RELATIVE_TO::HMD,
		false);

	InfoBoxManager::getInstance().addInfoBox(
		"Activate Label (Primary)",
		"activaterightlabel.png",
		0.075f,
		glm::translate(glm::mat4(), glm::vec3(-0.04f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::PRIMARY_CONTROLLER,
		false);
	InfoBoxManager::getInstance().addInfoBox(
		"Activate Label (Secondary)",
		"activateleftlabel.png",
		0.075f,
		glm::translate(glm::mat4(), glm::vec3(0.04f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::SECONDARY_CONTROLLER,
		false);
}

void WelcomeBehavior::update()
{
	if ((m_pTDM->getPrimaryController()->justClickedTrigger() && m_pTDM->getSecondaryController()->isTriggerClicked()) ||
		(m_pTDM->getSecondaryController()->justClickedTrigger() && m_pTDM->getPrimaryController()->isTriggerClicked()))
		m_bActive = false;
}

void WelcomeBehavior::draw()
{
}
