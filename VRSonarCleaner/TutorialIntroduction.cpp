#include "TutorialIntroduction.h"

#include "InfoBoxManager.h"
#include <shared/glm/gtc/matrix_transform.hpp>

TutorialIntroduction::TutorialIntroduction(TrackedDeviceManager* pTDM, DataVolume* tableVolume)
	: m_pTDM(pTDM)
	, m_pTableVolume(tableVolume)
{
}


TutorialIntroduction::~TutorialIntroduction()
{
	if (m_bInitialized)
	{
		InfoBoxManager::getInstance().removeInfoBox("Intro");
	}
}

void TutorialIntroduction::init()
{
	InfoBoxManager::getInstance().addInfoBox(
		"Intro",
		"test.png",
		2.f,
		glm::translate(glm::mat4(), glm::vec3(0.f, 1.5f, 0.f)),
		InfoBoxManager::RELATIVE_TO::WORLD,
		true);

	m_bInitialized = true;
}

void TutorialIntroduction::update()
{
}

void TutorialIntroduction::draw()
{
}
