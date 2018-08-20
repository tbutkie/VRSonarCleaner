#include "WelcomeBehavior.h"

#include "InfoBoxManager.h"
#include "Renderer.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/random.hpp>
#include <gtc/quaternion.hpp>

WelcomeBehavior::WelcomeBehavior(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
{
}


WelcomeBehavior::~WelcomeBehavior()
{
	if (m_bInitialized)
	{
		InfoBoxManager::getInstance().removeInfoBox("Welcome");
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
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getPrimaryController()->readyToRender() ||
		!m_pTDM->getSecondaryController() || !m_pTDM->getSecondaryController()->readyToRender())
		return;

	glm::vec3 primTrigPos = m_pTDM->getPrimaryController()->getTriggerPoint();
	glm::vec3 secTrigPos = m_pTDM->getSecondaryController()->getTriggerPoint();
	glm::vec3 trigToTrigVec = secTrigPos - primTrigPos;
	bool rightHanded = glm::dot(glm::cross(glm::vec3(m_pTDM->getHMDToWorldTransform()[3]) - primTrigPos, trigToTrigVec), glm::vec3(m_pTDM->getHMDToWorldTransform()[1])) < 0.f;

	glm::mat4 primTriggerTextAnchorTrans = m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(rightHanded ? -0.025f : 0.025f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 secTriggerTextAnchorTrans = m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(rightHanded ? 0.025f : -0.025f, -0.03f, 0.05f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	
	Renderer::getInstance().drawText(
		"Activate",
		m_pTDM->getPrimaryController()->getTriggerPullAmount() > 0.f ? glm::mix(glm::vec4(1.f), glm::vec4(0.f, 1.f, 0.f, 1.f), m_pTDM->getPrimaryController()->getTriggerPullAmount()) : glm::vec4(glm::linearRand(glm::vec3(0.f), glm::vec3(1.f)), 1.f),
		primTriggerTextAnchorTrans[3],
		glm::quat(primTriggerTextAnchorTrans),
		0.0075f,
		Renderer::TextSizeDim::HEIGHT,
		Renderer::TextAlignment::CENTER,
		rightHanded ? Renderer::TextAnchor::CENTER_RIGHT : Renderer::TextAnchor::CENTER_LEFT
	);

	Renderer::getInstance().drawDirectedPrimitive("cylinder",
		primTriggerTextAnchorTrans[3],
		(m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, -0.031f, 0.05f)))[3],
		0.001f,
		glm::vec4(1.f, 1.f, 1.f, 0.75f)
	);

	Renderer::getInstance().drawText(
		"Activate",
		m_pTDM->getSecondaryController()->getTriggerPullAmount() > 0.f ? glm::mix(glm::vec4(1.f), glm::vec4(0.f, 1.f, 0.f, 1.f), m_pTDM->getSecondaryController()->getTriggerPullAmount()) : glm::vec4(glm::linearRand(glm::vec3(0.f), glm::vec3(1.f)), 1.f),
		secTriggerTextAnchorTrans[3],
		glm::quat(secTriggerTextAnchorTrans),
		0.0075f,
		Renderer::TextSizeDim::HEIGHT,
		Renderer::TextAlignment::CENTER,
		rightHanded ? Renderer::TextAnchor::CENTER_LEFT : Renderer::TextAnchor::CENTER_RIGHT
	);

	Renderer::getInstance().drawDirectedPrimitive("cylinder",
		secTriggerTextAnchorTrans[3],
		(m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, -0.031f, 0.05f)))[3],
		0.001f,
		glm::vec4(1.f, 1.f, 1.f, 0.75f)
	);
}
