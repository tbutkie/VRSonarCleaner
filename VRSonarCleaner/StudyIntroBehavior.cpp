#include "StudyIntroBehavior.h"

#include "InfoBoxManager.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/random.hpp>
#include <gtc/quaternion.hpp>
#include "Renderer.h"

StudyIntroBehavior::StudyIntroBehavior(TrackedDeviceManager* pTDM)
	: m_pTDM(pTDM)
	, m_bWaitForTriggerRelease(true)
{
}


StudyIntroBehavior::~StudyIntroBehavior()
{
}

void StudyIntroBehavior::init()
{
	m_bInitialized = true;
}

void StudyIntroBehavior::update()
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getSecondaryController())
		return;

	if (m_bWaitForTriggerRelease && !m_pTDM->getPrimaryController()->isTriggerEngaged() && !m_pTDM->getSecondaryController()->isTriggerEngaged())
		m_bWaitForTriggerRelease = false;

	if ((m_pTDM->getPrimaryController()->justClickedTrigger() && m_pTDM->getSecondaryController()->isTriggerClicked()) ||
		(m_pTDM->getSecondaryController()->justClickedTrigger() && m_pTDM->getPrimaryController()->isTriggerClicked()))
		m_bActive = false;
}

void StudyIntroBehavior::draw()
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getPrimaryController()->readyToRender() ||
		!m_pTDM->getSecondaryController() || !m_pTDM->getSecondaryController()->readyToRender())
		return;

	glm::vec3 primTrigPos = (m_pTDM->getPrimaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, -0.031f, 0.05f)))[3];
	glm::vec3 secTrigPos = (m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, -0.031f, 0.05f)))[3];
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
		rightHanded ? Renderer::TextAnchor::CENTER_RIGHT : Renderer::TextAnchor::CENTER_LEFT
	);

	Renderer::getInstance().drawConnector(
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
		rightHanded ? Renderer::TextAnchor::CENTER_LEFT : Renderer::TextAnchor::CENTER_RIGHT
	);

	Renderer::getInstance().drawConnector(
		secTriggerTextAnchorTrans[3],
		(m_pTDM->getSecondaryController()->getDeviceToWorldTransform() * glm::translate(glm::mat4(), glm::vec3(0.f, -0.031f, 0.05f)))[3],
		0.001f,
		glm::vec4(1.f, 1.f, 1.f, 0.75f)
	);

	std::string desc("In this study, you will be cleaning sonar data\nto remove noisey and unwanted data points.\n\nHold both triggers to start a training session.");
	float descSize = 2.f;
	glm::vec2 dims = Renderer::getInstance().getTextDimensions(desc, descSize, Renderer::WIDTH);

	glm::vec3 anchorPt(0.f, m_pTDM->getHMDToWorldTransform()[3].y, 0.f);

	glm::mat4 textTrans = Renderer::getBillBoardTransform(anchorPt, m_pTDM->getHMDToWorldTransform()[3], glm::vec3(0.f, 1.f, 0.f), true);

	Renderer::getInstance().drawText(
		"Sonar Data Cleaning",
		glm::vec4(0.2f, 0.2f, 0.77f, 1.f),
		anchorPt,
		glm::quat(textTrans),
		dims.x,
		Renderer::TextSizeDim::WIDTH,
		Renderer::TextAnchor::CENTER_BOTTOM
	);

	Renderer::getInstance().drawText(
		desc,
		glm::vec4(1.f),
		anchorPt,
		glm::quat(textTrans),
		descSize,
		Renderer::TextSizeDim::WIDTH,
		Renderer::TextAnchor::CENTER_TOP
	);
}
