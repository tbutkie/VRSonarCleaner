#include "ScaleDataVolumeBehavior.h"
#include "InfoBoxManager.h"
#include "Renderer.h"

ScaleDataVolumeBehavior::ScaleDataVolumeBehavior(TrackedDeviceManager* pTDM, DataVolume* dataVolume)
	: m_pTDM(pTDM)
	, m_pDataVolume(dataVolume)
	, m_bScaling(false)
{
	InfoBoxManager::getInstance().addInfoBox(
		"Scale Label",
		"scalelabel.png",
		0.1f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.2f)) * glm::rotate(glm::mat4(), glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f)),
		InfoBoxManager::RELATIVE_TO::SECONDARY_CONTROLLER,
		false);
}


ScaleDataVolumeBehavior::~ScaleDataVolumeBehavior()
{
	InfoBoxManager::getInstance().removeInfoBox("Scale Label");
}



void ScaleDataVolumeBehavior::update()
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getSecondaryController())
		return;

	updateState();

	if (m_bScaling)
	{
		float currentDist = controllerDistance();
		float delta = currentDist - m_fInitialDistance;
		m_pDataVolume->setDimensions(glm::vec3(exp(delta * 10.f) * m_vec3InitialDimensions));
	}
}

void ScaleDataVolumeBehavior::draw()
{
}

void ScaleDataVolumeBehavior::updateState()
{
	if (!m_pTDM->getPrimaryController() || !m_pTDM->getSecondaryController())
		return;
	
	if (m_pTDM->getSecondaryController()->justUnclickedTrigger())
	{
		m_bScaling = false;
	}
	
	if ((m_pTDM->getSecondaryController()->justPressedGrip() && m_pTDM->getPrimaryController()->isGripButtonPressed()) ||
		(m_pTDM->getPrimaryController()->justPressedGrip() && m_pTDM->getSecondaryController()->isGripButtonPressed()))
	{
		m_bScaling = true;

		m_fInitialDistance = controllerDistance();
		m_vec3InitialDimensions = m_pDataVolume->getDimensions();
	}

	if (!m_pTDM->getSecondaryController()->isGripButtonPressed() || !m_pTDM->getPrimaryController()->isGripButtonPressed())
		m_bScaling = false;
}

float ScaleDataVolumeBehavior::controllerDistance()
{
	return glm::length(m_pTDM->getSecondaryController()->getDeviceToWorldTransform()[3] - m_pTDM->getPrimaryController()->getDeviceToWorldTransform()[3]);
}
